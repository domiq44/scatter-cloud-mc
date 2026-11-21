/*
 * rng.h - Générateur aléatoire SplitMix64
 * Rng struct : state, inc (2×uint64_t = 16 bytes, _Static_assert).
 * rng_seed() : initialise état non-nul (graine 0 ≠ graine 1).
 * rng_next_u64() : SplitMix64, période 2^64.
 * rng_next_f64() : 53 bits significatifs (double IEEE 754).
 *
 * Partie de scatter-cloud-mc
 */

#ifndef RNG_H
#define RNG_H

#include <assert.h>
#include <stdint.h>

typedef struct {
    uint64_t state;
    uint64_t inc;
} Rng;

/*
 * _Static_assert vérifie la taille à la compilation pour:
 * - Détecter les changements d'ABI accidentels
 * - Valider l'alignement/padding attendu (2 uint64_t = 16 octets)
 * - Garantir la compatibilité binaire (sérialisation, MPI, fichiers binaires)
 * Échoue à la compilation si la taille change
 */
_Static_assert(sizeof(Rng) == 16, "Rng size mismatch: expected 2 uint64_t (16 bytes)");

void rng_seed(Rng *rng, uint64_t seed);
uint64_t rng_next_u64(Rng *rng);
double rng_next_f64(Rng *rng);

#endif
