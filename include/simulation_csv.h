/*
 * simulation_csv.h - Export CSV résultats simulation
 * write_simulation_csv() : photons bruts (mémoire ou streaming) + pdf par bin.
 * write_simulation_flux_csv() : bilan énergétique avec fractions injectées.
 * Formats : alpha_d_deg, mu, pdf, weight / component, energy, fraction.
 *
 * Partie de scatter-cloud-mc
 */

#ifndef SIMULATION_CSV_H
#define SIMULATION_CSV_H

#include "simulation.h"

int write_simulation_csv(const char *filename, const SimulationResult *result);
int write_simulation_flux_csv(const char *filename, const SimulationResult *result);

#endif