#include "rng.h"
#include <stdint.h>

void rng_seed(Rng *rng, uint64_t seed) {
    if (seed == 0)
        seed = UINT64_C(0x9e3779b97f4a7c15);
    rng->state = seed;
}

uint64_t rng_next_u64(Rng *rng) {
    // Xorshift64*
    uint64_t x = rng->state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    rng->state = x;
    return x * UINT64_C(2685821657736338717);
}

double rng_next_f64(Rng *rng) {
    // 53 bits de précision
    return (double)(rng_next_u64(rng) >> 11) * (1.0 / 9007199254740992.0);
}
