#define main baseline_main
#include "../../solver.c"
#undef main
#include <stdint.h>

enum {
    P_COUNT = 5040,
    O_COUNT = 729,
    STATE_COUNT = P_COUNT * O_COUNT
};

extern const uint16_t p_move[3][P_COUNT];
extern const uint16_t o_move[3][O_COUNT];
extern const uint8_t p_distance[P_COUNT];
extern const uint8_t o_distance[O_COUNT];

typedef struct {
    uint64_t nodes;
    uint64_t transitions;
    uint64_t heuristic_loads;
    uint64_t face_tests;
    uint64_t divisions;
    uint64_t remainders;
    uint64_t maximum_tests;
} search_counts;

extern search_counts optimized_counts;
extern search_counts reference_counts;

int search(uint16_t p, uint16_t o, uint8_t path[11]);
int search_reference(uint16_t p, uint16_t o, uint8_t path[11]);
unsigned search_bytes(void);
unsigned reference_bytes(void);
#include <inttypes.h>

static uint8_t exact[STATE_COUNT];
static uint32_t queue[STATE_COUNT];

static int check_distances(const uint8_t *d, unsigned count, const uint16_t moves[3][count])
{
    for (unsigned i = 0; i < count; ++i) {
        if (d[i] > 11 || (d[i] == 0) != (i == 0)) return 0;
        unsigned best = 255;
        for (unsigned face = 0; face < 3; ++face) {
            unsigned next = i;
            for (unsigned turn = 0; turn < 3; ++turn) {
                next = moves[face][next];
                unsigned candidate = (unsigned) d[next] + 1;
                if (candidate < best) best = candidate;
            }
        }
        if (i != 0 && d[i] != best) return 0;
    }
    return 1;
}

static int check_tables(void)
{
    state_t state;
    unsigned max_pd = 0, max_od = 0, max_pm = 0, max_om = 0;
    for (unsigned p = 0; p < P_COUNT; ++p) {
        unrank_state(p * O_COUNT, &state);
        if (p_distance[p] > max_pd) max_pd = p_distance[p];
        for (unsigned face = 0; face < 3; ++face) {
            state_t next = quarter_turn(state, (uint8_t) face);
            if (p_move[face][p] != rank_state(&next) / O_COUNT) return 0;
            if (p_move[face][p] > max_pm) max_pm = p_move[face][p];
        }
    }
    for (unsigned o = 0; o < O_COUNT; ++o) {
        unrank_state(o, &state);
        if (o_distance[o] > max_od) max_od = o_distance[o];
        for (unsigned face = 0; face < 3; ++face) {
            state_t next = quarter_turn(state, (uint8_t) face);
            if (o_move[face][o] != rank_state(&next) % O_COUNT) return 0;
            if (o_move[face][o] > max_om) max_om = o_move[face][o];
        }
    }
    if (!check_distances(p_distance, P_COUNT, p_move) ||
        !check_distances(o_distance, O_COUNT, o_move)) return 0;
    printf("H2 passed: all transition entries match the cube model; distance tables complete and exact.\n");
    printf("Table maxima: p_move=%u, o_move=%u, p_distance=%u, o_distance=%u; solved distances=0,0\n",
           max_pm, max_om, max_pd, max_od);
    printf("Solved quarter-turn entries (R,B,D): p=%u,%u,%u; o=%u,%u,%u\n",
           p_move[0][0], p_move[1][0], p_move[2][0], o_move[0][0], o_move[1][0], o_move[2][0]);
    return 1;
}

static int make_oracle(void)
{
    unsigned head = 0, tail = 1, diameter = 0;
    memset(exact, 255, sizeof exact);
    exact[0] = 0;
    queue[0] = 0;
    while (head < tail) {
        unsigned here = queue[head++], p = here / O_COUNT, o = here % O_COUNT;
        for (unsigned face = 0; face < 3; ++face) {
            unsigned np = p, no = o;
            for (unsigned turn = 0; turn < 3; ++turn) {
                np = p_move[face][np]; no = o_move[face][no];
                unsigned next = np * O_COUNT + no;
                if (exact[next] == 255) {
                    exact[next] = (uint8_t) (exact[here] + 1);
                    if (exact[next] > diameter) diameter = exact[next];
                    queue[tail++] = next;
                }
            }
        }
    }
    if (tail != STATE_COUNT || diameter != 11) return 0;
    for (unsigned r = 0; r < STATE_COUNT; ++r)
        if (p_distance[r / O_COUNT] > exact[r] || o_distance[r % O_COUNT] > exact[r]) return 0;
    puts("H1 passed on 3674160 states; oracle diameter=11.");
    return 1;
}

static int check_one(unsigned rank, int compare, uint8_t path[11])
{
    uint8_t reference_path[11];
    int length = search((uint16_t) (rank / O_COUNT), (uint16_t) (rank % O_COUNT), path);
    if (length != exact[rank]) return -1;
    if (compare) {
        int other = search_reference((uint16_t) (rank / O_COUNT), (uint16_t) (rank % O_COUNT), reference_path);
        if (other != length || memcmp(path, reference_path, (size_t) length) != 0 ||
            optimized_counts.nodes != reference_counts.nodes) return -1;
    }
    state_t state;
    unrank_state(rank, &state);
    for (int i = 0; i < length; ++i) {
        if (path[i] >= MOVES) return -1;
        state = apply_move(state, path[i]);
    }
    return rank_state(&state) == 0 ? length : -1;
}

static void add_counts(search_counts *a, search_counts b)
{
    a->nodes += b.nodes; a->transitions += b.transitions;
    a->heuristic_loads += b.heuristic_loads; a->face_tests += b.face_tests;
    a->divisions += b.divisions; a->remainders += b.remainders;
    a->maximum_tests += b.maximum_tests;
}

static void print_counts(const char *name, search_counts c)
{
    printf("%s: nodes=%" PRIu64 ", transition_loads=%" PRIu64 ", heuristic_loads=%" PRIu64 "\n",
           name, c.nodes, c.transitions, c.heuristic_loads);
    printf("  same_face_tests=%" PRIu64 ", divisions=%" PRIu64 ", remainders=%" PRIu64 ", max_comparisons=%" PRIu64 "\n",
           c.face_tests, c.divisions, c.remainders, c.maximum_tests);
}

int main(int argc, char **argv)
{
    int all = argc == 2 && strcmp(argv[1], "--all") == 0;
    if (argc > 2 || (argc == 2 && !all)) { fprintf(stderr, "Usage: %s [--all]\n", argv[0]); return 2; }
    if (!check_tables() || !make_oracle()) { fputs("Table/oracle check failed.\n", stderr); return 1; }
    printf("Search frame bytes: reference=%u, optimized=%u; tables=40383; path=11\n", reference_bytes(), search_bytes());
    const char *cases[] = {"12345671111111", "25314672313211", "21345671111111"};
    uint8_t path[11];
    for (unsigned i = 0; i < 3; ++i) {
        state_t state;
        if (!parse_state(cases[i], &state)) return 1;
        int n = check_one(rank_state(&state), 1, path);
        if (n < 0) return 1;
        printf("%s: length=%d, nodes=%" PRIu64 ", solution:", cases[i], n, optimized_counts.nodes);
        for (int j = 0; j < n; ++j) printf(" %s", move_names[path[j]]);
        putchar('\n');
    }
    search_counts opt = {0}, ref = {0};
    uint64_t maximum = 0;
    unsigned checked = 0, worst = 0;
    for (unsigned r = 0; r < STATE_COUNT; ++r) {
        if (!all && exact[r] != 11) continue;
        if (check_one(r, !all, path) < 0) { fprintf(stderr, "Search failed at rank %u\n", r); return 1; }
        ++checked;
        add_counts(&opt, optimized_counts);
        if (!all) add_counts(&ref, reference_counts);
        if (optimized_counts.nodes > maximum) { maximum = optimized_counts.nodes; worst = r; }
        if (all && checked % 500000 == 0) { printf("Checked %u/%u states\n", checked, STATE_COUNT); fflush(stdout); }
    }
    printf("%s passed: %u optimal paths replayed to solved.\n", all ? "H3" : "Distance-11 comparison", checked);
    if (!all) { puts("Reference and optimized paths and evaluated-node counts match."); print_counts("Reference", ref); }
    print_counts("Optimized", opt);
    state_t state;
    unrank_state(worst, &state);
    printf("Worst evaluated-node count=%" PRIu64 ", input=", maximum);
    for (unsigned i = 0; i < 7; ++i) printf("%u", state.p[i] + 1U);
    for (unsigned i = 0; i < 7; ++i) printf("%u", state.o[i] + 1U);
    putchar('\n');
    puts(all ? "H4: not applicable (unpacked tables)." : "H3 for this optimized version: pending; run with --all.");
    return output_failed();
}
