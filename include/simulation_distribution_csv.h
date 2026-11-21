/*
 * simulation_distribution_csv.h - Distribution agrégée CSV
 * write_simulation_distribution_csv() : tous bins avec densité, erreurs, largeurs.
 * Champs : alpha_d_deg, mu, density_per_mu, density_per_degree, bin_width,
 * standard_error, detected_energy_fraction, diffuse_density, ballistic_fraction.
 * n_photons passé explicitement (pas photon_count + roulette_killed).
 *
 * Partie de scatter-cloud-mc
 */

#ifndef SIMULATION_DISTRIBUTION_CSV_H
#define SIMULATION_DISTRIBUTION_CSV_H

#include "simulation.h"

int write_simulation_distribution_csv(const char *filename, const SimulationResult *result,
                                      int n_photons);

#endif