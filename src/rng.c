/*
 * rng.c - Générateur aléatoire SplitMix64
 * rng_seed() : initialise état 128-bit (state + inc).
 * rng_next_u64() : SplitMix64, période 2^64, passe BigCrush.
 * rng_next_f64() : 53 bits significatifs (double IEEE 754).
 *
 * Partie de scatter-cloud-mc
 */

#include "rng.h"
#include <stdint.h>

void rng_seed(Rng *rng, uint64_t seed) { rng->state = seed; }

uint64_t rng_next_u64(Rng *rng) {
    uint64_t z = (rng->state += UINT64_C(0x9e3779b97f4a7c15));
    z = (z ^ (z >> 30u)) * UINT64_C(0xbf58476d1ce4e5b9);
    z = (z ^ (z >> 27u)) * UINT64_C(0x94d049bb133111eb);
    return z ^ (z >> 31u);
}

double rng_next_f64(Rng *rng) {
    return (double)(rng_next_u64(rng) >> 11u) * (1.0 / 9007199254740992.0);
}