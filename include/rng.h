#ifndef RNG_H
#define RNG_H

#include <stdint.h>

typedef struct {
    uint64_t state;
} Rng;

void rng_seed(Rng *rng, uint64_t seed);
uint64_t rng_next_u64(Rng *rng);
double rng_next_f64(Rng *rng);

#endif
