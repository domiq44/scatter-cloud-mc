#ifndef CONFIG_H
#define CONFIG_H

#include "geometry.h"

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

int config_is_valid(const Config *cfg);
int load_config(const char *filename, Config *cfg);

#endif
