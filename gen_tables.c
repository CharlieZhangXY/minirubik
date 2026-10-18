#define main baseline_main
#include "../../solver.c"
#undef main
#include <stdint.h>

enum {
    P_COUNT = 5040,
    O_COUNT = 729,
    STATE_COUNT = P_COUNT * O_COUNT
};

static uint16_t pm[3][P_COUNT], om[3][O_COUNT], queue[P_COUNT];
static uint8_t pd[P_COUNT], od[O_COUNT];

static int distances(uint8_t *d, unsigned count, uint16_t moves[3][count])
{
    unsigned head = 0, tail = 1;
    memset(d, 255, count);
    d[0] = 0;
    queue[0] = 0;
    while (head < tail) {
        unsigned here = queue[head++];
        for (unsigned face = 0; face < 3; ++face) {
            unsigned next = here;
            for (unsigned turn = 0; turn < 3; ++turn) {
                next = moves[face][next];
                if (d[next] == 255) {
                    d[next] = (uint8_t) (d[here] + 1);
                    queue[tail++] = (uint16_t) next;
                }
            }
        }
    }
    return tail == count;
}

static void emit_moves(FILE *f, const char *name, unsigned count, uint16_t table[3][count])
{
    fprintf(f, "const uint16_t %s[3][%u] = {\n", name, count);
    for (unsigned face = 0; face < 3; ++face) {
        fputs("{\n", f);
        for (unsigned i = 0; i < count; ++i)
            fprintf(f, "%u,%s", table[face][i], i % 16 == 15 ? "\n" : " ");
        fputs("\n},\n", f);
    }
    fputs("};\n", f);
}

static void emit_distances(FILE *f, const char *name, const uint8_t *table, unsigned count)
{
    fprintf(f, "const uint8_t %s[%u] = {\n", name, count);
    for (unsigned i = 0; i < count; ++i)
        fprintf(f, "%u,%s", table[i], i % 32 == 31 ? "\n" : " ");
    fputs("\n};\n", f);
}

int main(int argc, char **argv)
{
    if (argc != 2) { fprintf(stderr, "Usage: %s OUTPUT.c\n", argv[0]); return 2; }
    state_t state;
    for (unsigned p = 0; p < P_COUNT; ++p) {
        unrank_state(p * O_COUNT, &state);
        for (unsigned face = 0; face < 3; ++face) {
            state_t next = quarter_turn(state, (uint8_t) face);
            pm[face][p] = (uint16_t) (rank_state(&next) / O_COUNT);
        }
    }
    for (unsigned o = 0; o < O_COUNT; ++o) {
        unrank_state(o, &state);
        for (unsigned face = 0; face < 3; ++face) {
            state_t next = quarter_turn(state, (uint8_t) face);
            om[face][o] = (uint16_t) (rank_state(&next) % O_COUNT);
        }
    }
    if (!distances(pd, P_COUNT, pm) || !distances(od, O_COUNT, om)) return 1;
    FILE *f = fopen(argv[1], "w");
    if (!f) { perror(argv[1]); return 1; }
    fputs("#include <stdint.h>\n", f);
    emit_moves(f, "p_move", P_COUNT, pm);
    emit_moves(f, "o_move", O_COUNT, om);
    emit_distances(f, "p_distance", pd, P_COUNT);
    emit_distances(f, "o_distance", od, O_COUNT);
    int failed = ferror(f);
    if (fclose(f) != 0) failed = 1;
    if (failed) return 1;
    puts("Generated tables: 40383 data bytes.");
    return 0;
}
