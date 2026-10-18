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
#include <stdio.h>

static void moves(const char *name, unsigned count, const uint16_t values[3][count])
{
    printf(".align 1\n%s:\n", name);
    for (unsigned f = 0; f < 3; ++f) {
        for (unsigned i = 0; i < count; ++i) {
            if (i % 16 == 0) printf("    .half ");
            printf("%u", values[f][i]);
            if (i % 16 == 15 || i + 1 == count) putchar('\n'); else putchar(',');
        }
    }
}

static void distances(const char *name, unsigned count, const uint8_t *values)
{
    printf("%s:\n", name);
    for (unsigned i = 0; i < count; ++i) {
        if (i % 16 == 0) printf("    .byte ");
        printf("%u", values[i]);
        if (i % 16 == 15 || i + 1 == count) putchar('\n'); else putchar(',');
    }
}

int main(void)
{
    puts(".data");
    moves("p_move", P_COUNT, p_move);
    moves("o_move", O_COUNT, o_move);
    distances("p_distance", P_COUNT, p_distance);
    distances("o_distance", O_COUNT, o_distance);
    return fflush(stdout) || ferror(stdout);
}
