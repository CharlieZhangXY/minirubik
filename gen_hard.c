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

static uint8_t distance[STATES];
static uint32_t queue[STATES];

int main(void)
{
    unsigned head = 0, tail = 1, diameter = 0, count = 0;
    memset(distance, 255, sizeof distance);
    distance[0] = 0;
    queue[0] = 0;
    while (head < tail) {
        unsigned here = queue[head++];
        for (unsigned face = 0; face < 3; ++face) {
            unsigned p = here / ORIENTATIONS, o = here % ORIENTATIONS;
            for (unsigned turn = 0; turn < 3; ++turn) {
                p = p_move[face][p];
                o = o_move[face][o];
                unsigned next = p * ORIENTATIONS + o;
                if (distance[next] != 255) continue;
                distance[next] = (uint8_t) (distance[here] + 1);
                if (distance[next] > diameter) diameter = distance[next];
                queue[tail++] = next;
            }
        }
    }
    for (unsigned rank = 0; rank < STATES; ++rank)
        if (distance[rank] == 11) ++count;
    if (tail != STATES || diameter != 11 || count != 2644) {
        fprintf(stderr, "Unexpected BFS result: states=%u, diameter=%u, hard=%u\n", tail, diameter, count);
        return 1;
    }
    for (unsigned rank = 0; rank < STATES; ++rank) {
        if (distance[rank] != 11) continue;
        state_t state;
        unrank_state(rank, &state);
        if (!valid(&state) || rank_state(&state) != rank) return 1;
        for (unsigned i = 0; i < 7; ++i) printf("%u", state.p[i] + 1U);
        for (unsigned i = 0; i < 7; ++i) printf("%u", state.o[i] + 1U);
        putchar('\n');
    }
    return output_failed();
}
