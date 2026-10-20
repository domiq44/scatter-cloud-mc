#ifndef SIMULATION_H
#define SIMULATION_H

#include "config.h"
#include <stdio.h>

typedef struct {
    double alpha_d_deg;
    double mu;
    double weight;
    int bin;
} DetectedPhoton;

typedef struct {
    DetectedPhoton *photons;
    FILE *photon_stream;
    int photon_count;
    int roulette_killed_count;
    long long scatter_count;
    double detected_energy;
    int ballistic_photon_count;
    double ballistic_detected_energy;
    double roulette_energy;
    double absorbed_energy;
    int bin_count;
    double *exit_pdf;
    double *exit_probability_standard_error;
    double *diffuse_exit_pdf;
    double *diffuse_exit_probability_standard_error;
} SimulationResult;

int simulate_photons(const Config *config, int seed, SimulationResult *result);
int simulate_photons_stream(const Config *config, int seed, SimulationResult *result);
void simulation_result_free(SimulationResult *result);

#endif