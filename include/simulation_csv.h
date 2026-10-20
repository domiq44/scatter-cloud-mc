#ifndef SIMULATION_CSV_H
#define SIMULATION_CSV_H

#include "simulation.h"

int write_simulation_csv(const char *filename, const SimulationResult *result);
int write_simulation_flux_csv(const char *filename, const SimulationResult *result);

#endif