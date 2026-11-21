/*
 * config.h - Configuration simulation : parsing, validation, structures
 * Config struct : n_photons, sigma_e, albedo, g, radius, n_mu_bins, seed,
 * theta_zero, phase_function, g2, hg_fraction, geometry.
 * Validation : bornes, clés obligatoires double_hg, theta_zero ∈ [0,90].
 * _Static_assert taille Config (88/96 bytes selon plateforme).
 *
 * Partie de scatter-cloud-mc
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "geometry.h"
#include <assert.h>

typedef enum {
    PHASE_FUNCTION_HG,
    PHASE_FUNCTION_DOUBLE_HG,
} PhaseFunction;

typedef struct {
    int n_photons;
    double sigma_e;
    double albedo;
    double g;
    double radius;
    int n_mu_bins;
    int seed;
    double theta_zero;
    PhaseFunction phase_function;
    double g2;
    double hg_fraction;
    GeometryType geometry;
} Config;

/*
 * _Static_assert vérifie la taille de Config à la compilation pour:
 * - Détecter les changements d'ABI accidentels (ajout/suppression/réordonnancement de champs)
 * - Valider l'alignement/padding attendu
 * - Taille dépendante de la plateforme: 88 octets (32-bit pointeurs) ou 96 octets (64-bit)
 *   car Config contient un pointeur implicite via GeometryType (enum) et l'alignement des doubles
 * - Garantir la compatibilité binaire (fichiers de config binaires, MPI, checkpoints)
 * Échoue à la compilation si la taille change inattendument
 */
_Static_assert(sizeof(Config) == 88 || sizeof(Config) == 96,
               "Config size mismatch: expected 88 (32-bit) or 96 (64-bit) bytes");

int config_is_valid(const Config *cfg);
int load_config(const char *filename, Config *cfg);

#endif
