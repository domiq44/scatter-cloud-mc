/*
 * simulation_report.h - Bilan énergétique console
 * print_simulation_report() : photons, énergies, pourcentages, résidu MC_DIFF.
 * Cas zero photon : géométrie + épaisseur optique moyenne.
 *
 * Partie de scatter-cloud-mc
 */

#ifndef SIMULATION_REPORT_H
#define SIMULATION_REPORT_H

#include "simulation.h"

void print_simulation_report(const SimulationResult *result, const Config *config);

#endif