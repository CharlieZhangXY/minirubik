#include <stdint.h>

enum {
    P_COUNT = 5040,
    O_COUNT = 729
};

extern const uint16_t p_move[3][P_COUNT];
extern const uint16_t o_move[3][O_COUNT];
extern const uint8_t p_distance[P_COUNT];
extern const uint8_t o_distance[O_COUNT];

#ifdef COUNT_OPS
typedef struct {
    uint64_t nodes;
    uint64_t transitions;
    uint64_t heuristic_loads;
    uint64_t face_tests;
    uint64_t divisions;
    uint64_t remainders;
    uint64_t maximum_tests;
} search_counts;

#define COUNT(s, field, amount) ((s).field += (amount))
#define RESET(s) ((s) = (search_counts) {0})
#else
#define COUNT(s, field, amount) ((void) 0)
#define RESET(s) ((void) 0)
#endif

/* 每一層記錄原狀態、連續轉動的結果，以及下一個要試的動作 */
typedef struct {
    uint16_t p, o;
    uint16_t next_p, next_o;
    uint8_t face, turn, move, last_face;
} frame;

static frame stack[12];

#ifdef COUNT_OPS
search_counts optimized_counts;
#endif

unsigned search_bytes(void)
{
    return (unsigned) sizeof stack;
}

int search(uint16_t p, uint16_t o, uint8_t path[11])
{
    RESET(optimized_counts);

    unsigned a = p_distance[p];
    unsigned b = o_distance[o];

    COUNT(optimized_counts, heuristic_loads, 2);
    COUNT(optimized_counts, maximum_tests, 1);

    /* 從 heuristic 下界開始，逐步增加允許的解答長度 */
    for (unsigned bound = a > b ? a : b; bound <= 11; ++bound) {
        unsigned depth = 0;
        stack[0] = (frame) {p, o, p, o, 0, 0, 0, 3};

        COUNT(optimized_counts, nodes, 1);

        if (p == 0 && o == 0)
            return 0;

        while (1) {
            frame *s = &stack[depth];

            if (s->face == 3) {
                if (depth == 0)
                    break;

                --depth;
                continue;
            }

            if (s->turn == 0) {
                COUNT(optimized_counts, face_tests, 1);

                if (s->face == s->last_face) {
                    ++s->face;
                    s->move += 3;
                    continue;
                }

                s->next_p = s->p;
                s->next_o = s->o;
            }

            unsigned face = s->face;
            unsigned move = s->move++;

            /* 同一面的 90、180、270 度候選，沿用上一次轉動的結果 */
            unsigned np = s->next_p = p_move[face][s->next_p];
            unsigned no = s->next_o = o_move[face][s->next_o];

            COUNT(optimized_counts, transitions, 2);

            if (++s->turn == 3) {
                s->turn = 0;
                ++s->face;
            }

            COUNT(optimized_counts, nodes, 1);

            unsigned remaining = bound - depth - 1;

            /* 排列距離已超出剩餘步數時，不必再讀方向距離 */
            COUNT(optimized_counts, heuristic_loads, 1);
            if (p_distance[np] > remaining)
                continue;

            COUNT(optimized_counts, heuristic_loads, 1);
            if (o_distance[no] > remaining)
                continue;

            path[depth] = (uint8_t) move;

            if (np == 0 && no == 0)
                return (int) depth + 1;

            ++depth;
            stack[depth] = (frame) {(uint16_t) np, (uint16_t) no, (uint16_t) np, (uint16_t) no, 0, 0, 0, (uint8_t) face};
        }
    }

    return -1;
}
