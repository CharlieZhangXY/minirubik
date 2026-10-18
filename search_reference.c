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

/* 以固定陣列保存各層搜尋進度，不使用遞迴 */
typedef struct {
    uint16_t p, o;
    uint8_t move, last_face;
} frame;

static frame stack[12];

#ifdef COUNT_OPS
search_counts reference_counts;
#endif

unsigned reference_bytes(void)
{
    return (unsigned) sizeof stack;
}

static unsigned h(unsigned p, unsigned o)
{
    unsigned a = p_distance[p];
    unsigned b = o_distance[o];

    COUNT(reference_counts, heuristic_loads, 2);
    COUNT(reference_counts, maximum_tests, 1);

    return a > b ? a : b;
}

int search_reference(uint16_t p, uint16_t o, uint8_t path[11])
{
    RESET(reference_counts);

    for (unsigned bound = h(p, o); bound <= 11; ++bound) {
        unsigned depth = 0;
        stack[0] = (frame) {p, o, 0, 3};

        COUNT(reference_counts, nodes, 1);

        if (p == 0 && o == 0)
            return 0;

        while (1) {
            frame *s = &stack[depth];

            if (s->move == 9 || depth == bound) {
                if (depth == 0)
                    break;

                --depth;
                continue;
            }

            /* 保留除法與餘數，從動作編號拆出面與轉動次數 */
            unsigned move = s->move++;
            unsigned face = move / 3;

            COUNT(reference_counts, divisions, 1);
            COUNT(reference_counts, face_tests, 1);

            if (face == s->last_face)
                continue;

            unsigned turns = move % 3 + 1;
            unsigned np = s->p;
            unsigned no = s->o;

            COUNT(reference_counts, remainders, 1);

            /* 每個候選都從前一狀態開始，重新做所需的四分之一圈轉動 */
            for (unsigned t = 0; t < turns; ++t) {
                np = p_move[face][np];
                no = o_move[face][no];
                COUNT(reference_counts, transitions, 2);
            }

            COUNT(reference_counts, nodes, 1);

            if (h(np, no) > bound - depth - 1)
                continue;

            path[depth] = (uint8_t) move;

            if (np == 0 && no == 0)
                return (int) depth + 1;

            if (depth + 1 == bound)
                continue;

            ++depth;
            stack[depth] = (frame) {
                (uint16_t) np, (uint16_t) no, 0, (uint8_t) face
            };
        }
    }

    return -1;
}
