/*
 * constants.h - Constantes centralisées (magic numbers)
 * Roulette russe : ROULETTE_WEIGHT_THRESHOLD=1e-3, ROULETTE_SURVIVAL_PROBABILITY=0.1
 * HG isotrope : HG_ISOTROPIC_TOLERANCE=1e-3
 * Bins par défaut : DEFAULT_N_MU_BINS=200
 * Valeurs défaut tests : QUICK_TEST_N_PHOTONS, ENERGY_TEST_N_PHOTONS, etc.
 * Évite les magic numbers dispersés dans le code.
 *
 * Partie de scatter-cloud-mc
 */

#ifndef CONSTANTS_H
#define CONSTANTS_H

/* Seuil de poids pour déclencher la roulette russe (sans unité). */
#define ROULETTE_WEIGHT_THRESHOLD 1e-3

/* Probabilité de survie à la roulette russe (sans unité). */
#define ROULETTE_SURVIVAL_PROBABILITY 0.1

/* Tolérance pour considérer g comme isotrope (sans unité). */
#define HG_ISOTROPIC_TOLERANCE 1e-3

/* Nombre de bins par défaut pour la distribution angulaire mu = cos(alpha_d). */
#define DEFAULT_N_MU_BINS 200

/* Nombre de photons par défaut pour les tests rapides. */
#define QUICK_TEST_N_PHOTONS 16

/* Nombre de photons par défaut pour les tests de conservation d'énergie. */
#define ENERGY_TEST_N_PHOTONS 1000

/* Valeur par défaut de sigma_e pour les tests (sans unité, inverse de la longueur). */
#define TEST_SIGMA_E 0.5

/* Valeur par défaut de g pour les tests (sans unité). */
#define TEST_G 0.3

/* Valeur par défaut de albedo pour les tests (sans unité). */
#define TEST_ALBEDO 1.0

/* Valeur par défaut de radius pour les tests (sans unité). */
#define TEST_RADIUS 1.0

/* Valeur par défaut de theta_zero pour les tests (degrés). */
#define TEST_THETA_ZERO 0.0

/* Valeur par défaut de seed pour les tests. */
#define TEST_SEED 42

/* Valeur par défaut de n_mu_bins pour les tests. */
#define TEST_N_MU_BINS 200

#endif