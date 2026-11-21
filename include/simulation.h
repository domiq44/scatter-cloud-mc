/*
 * simulation.h - Structures et API simulation Monte Carlo
 * DetectedPhoton : alpha_d_deg, mu, weight, bin (32 bytes, _Static_assert).
 * SimulationResult : photons détectés, histogrammes, énergies, stats Kahan (opaque).
 * Accesseurs inline pour encapsulation (photon_count, detected_energy, etc.).
 * simulate_photons(), simulate_photons_stream(), simulation_result_free().
 *
 * Partie de scatter-cloud-mc
 */

#ifndef SIMULATION_H
#define SIMULATION_H

#include "config.h"
#include <assert.h>
#include <stdio.h>

typedef struct {
    double alpha_d_deg;
    double mu;
    double weight;
    int bin;
} DetectedPhoton;

/*
 * _Static_assert vérifie la taille à la compilation pour:
 * - Détecter les changements d'ABI accidentels
 * - Valider l'alignement/padding: 3 doubles (24) + int (4) + padding (4) = 32 octets
 * - Garantir la compatibilité binaire (écriture binaire des photons détectés, MPI)
 * Échoue à la compilation si la taille change
 */
_Static_assert(sizeof(DetectedPhoton) == 32,
               "DetectedPhoton size mismatch: expected 3 doubles + int + padding (32 bytes)");

typedef struct {
    DetectedPhoton *photons;
    FILE *photon_stream;
    int photon_count;
    int roulette_killed_count;
    long long scatter_count;
    double detected_energy;
    int ballistic_photon_count;
    double ballistic_detected_energy;
    double diffuse_upward_energy;
    double diffuse_downward_energy;
    double direct_downward_energy;
    double roulette_energy;
    double absorbed_energy;
    int bin_count;
    double *exit_pdf;
    double *exit_probability_standard_error;
    double *diffuse_exit_pdf;
    double *diffuse_exit_probability_standard_error;
} SimulationResult;

/*
 * SimulationResult: taille dépendante de la plateforme (pointeurs 64/32 bits)
 * _Static_assert non utilisé car la taille varie selon l'architecture:
 * - 32-bit: pointeurs = 4 octets
 * - 64-bit: pointeurs = 8 octets
 * Pour vérifier la taille sur une plateforme donnée, utiliser:
 *   _Static_assert(sizeof(SimulationResult) == TAILLE_ATTENDUE, "...");
 * dans un fichier de test spécifique à la plateforme.
 */

/* Accessor functions for encapsulation (optional, for new code) */
static inline int simulation_result_photon_count(const SimulationResult *result) {
    return result ? result->photon_count : 0;
}
static inline int simulation_result_roulette_killed_count(const SimulationResult *result) {
    return result ? result->roulette_killed_count : 0;
}
static inline long long simulation_result_scatter_count(const SimulationResult *result) {
    return result ? result->scatter_count : 0;
}
static inline double simulation_result_detected_energy(const SimulationResult *result) {
    return result ? result->detected_energy : 0.0;
}
static inline int simulation_result_ballistic_photon_count(const SimulationResult *result) {
    return result ? result->ballistic_photon_count : 0;
}
static inline double simulation_result_ballistic_detected_energy(const SimulationResult *result) {
    return result ? result->ballistic_detected_energy : 0.0;
}
static inline double simulation_result_diffuse_upward_energy(const SimulationResult *result) {
    return result ? result->diffuse_upward_energy : 0.0;
}
static inline double simulation_result_diffuse_downward_energy(const SimulationResult *result) {
    return result ? result->diffuse_downward_energy : 0.0;
}
static inline double simulation_result_direct_downward_energy(const SimulationResult *result) {
    return result ? result->direct_downward_energy : 0.0;
}
static inline double simulation_result_roulette_energy(const SimulationResult *result) {
    return result ? result->roulette_energy : 0.0;
}
static inline double simulation_result_absorbed_energy(const SimulationResult *result) {
    return result ? result->absorbed_energy : 0.0;
}
static inline int simulation_result_bin_count(const SimulationResult *result) {
    return result ? result->bin_count : 0;
}
static inline const double *simulation_result_exit_pdf(const SimulationResult *result) {
    return result ? result->exit_pdf : NULL;
}
static inline const double *
simulation_result_exit_probability_standard_error(const SimulationResult *result) {
    return result ? result->exit_probability_standard_error : NULL;
}
static inline const double *simulation_result_diffuse_exit_pdf(const SimulationResult *result) {
    return result ? result->diffuse_exit_pdf : NULL;
}
static inline const double *
simulation_result_diffuse_exit_probability_standard_error(const SimulationResult *result) {
    return result ? result->diffuse_exit_probability_standard_error : NULL;
}

int simulate_photons(const Config *config, int seed, SimulationResult *result);
int simulate_photons_stream(const Config *config, int seed, SimulationResult *result);
void simulation_result_free(SimulationResult *result);

#endif