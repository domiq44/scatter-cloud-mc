#include "simulation.h"
#include "geometry.h"
#include "hg.h"
#include "rng.h"
#include "vec3.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static Vec3 sample_configured_phase_function(Rng *rng, const Config *config, Vec3 direction) {
    if (config->phase_function == PHASE_FUNCTION_DOUBLE_HG) {
        return sample_double_henyey_greenstein(rng, config->g, config->g2, config->hg_fraction,
                                               direction);
    }
    return sample_henyey_greenstein(rng, config->g, direction);
}

typedef struct {
    double *weight_by_bin;
    double *squared_weight_by_bin;
    double *diffuse_weight_by_bin;
    double *diffuse_squared_weight_by_bin;
} SimulationHistograms;

static void free_simulation_histograms(SimulationHistograms *histograms) {
    free(histograms->weight_by_bin);
    free(histograms->squared_weight_by_bin);
    free(histograms->diffuse_weight_by_bin);
    free(histograms->diffuse_squared_weight_by_bin);
    memset(histograms, 0, sizeof(*histograms));
}

static int initialize_simulation_result(const Config *config, SimulationResult *result,
                                        SimulationHistograms *histograms, int stream_photons) {
    result->photons =
        stream_photons ? NULL : malloc((size_t)config->n_photons * sizeof(*result->photons));
    result->photon_stream = stream_photons ? tmpfile() : NULL;
    result->bin_count = config->n_mu_bins;
    result->exit_pdf = calloc((size_t)result->bin_count, sizeof(*result->exit_pdf));
    result->exit_probability_standard_error =
        calloc((size_t)result->bin_count, sizeof(*result->exit_probability_standard_error));
    result->diffuse_exit_pdf = calloc((size_t)result->bin_count, sizeof(*result->diffuse_exit_pdf));
    result->diffuse_exit_probability_standard_error =
        calloc((size_t)result->bin_count, sizeof(*result->diffuse_exit_probability_standard_error));
    histograms->weight_by_bin =
        calloc((size_t)result->bin_count, sizeof(*histograms->weight_by_bin));
    histograms->squared_weight_by_bin =
        calloc((size_t)result->bin_count, sizeof(*histograms->squared_weight_by_bin));
    histograms->diffuse_weight_by_bin =
        calloc((size_t)result->bin_count, sizeof(*histograms->diffuse_weight_by_bin));
    histograms->diffuse_squared_weight_by_bin =
        calloc((size_t)result->bin_count, sizeof(*histograms->diffuse_squared_weight_by_bin));
    if ((!stream_photons && !result->photons) || (stream_photons && !result->photon_stream) ||
        !result->exit_pdf || !result->exit_probability_standard_error ||
        !result->diffuse_exit_pdf || !result->diffuse_exit_probability_standard_error ||
        !histograms->weight_by_bin || !histograms->squared_weight_by_bin ||
        !histograms->diffuse_weight_by_bin || !histograms->diffuse_squared_weight_by_bin) {
        return 0;
    }

    return 1;
}

static int transport_photons(const Config *config, Rng *rng, Vec3 fixed_direction,
                             SimulationResult *result, SimulationHistograms *histograms,
                             int stream_photons) {
    double absorbed_energy_correction = 0.0;
    int photon_write_failed = 0;

    for (int i = 0; i < config->n_photons && !photon_write_failed; i++) {
        Vec3 direction = fixed_direction;
        Vec3 position = geometry_entry_position(config->geometry, config->radius,
                                                config->theta_zero * M_PI / 180.0, direction, rng);
        double weight = 1.0;
        double local_absorbed = 0.0;
        int has_scattered = 0;

        while (1) {
            double xi = rng_next_f64(rng);
            double free_path = -log(1.0 - xi) / config->sigma_e;
            double distance_to_border = geometry_distance_to_boundary(config->geometry, position,
                                                                      direction, config->radius);

            if (free_path >= distance_to_border) {
                double mu = vec3_dot(direction, fixed_direction);
                if (mu > 1.0)
                    mu = 1.0;
                if (mu < -1.0)
                    mu = -1.0;

                int bin = (int)((mu + 1.0) * 0.5 * result->bin_count);
                if (bin < 0)
                    bin = 0;
                if (bin >= result->bin_count)
                    bin = result->bin_count - 1;

                histograms->weight_by_bin[bin] += weight;
                histograms->squared_weight_by_bin[bin] += weight * weight;
                if (has_scattered) {
                    histograms->diffuse_weight_by_bin[bin] += weight;
                    histograms->diffuse_squared_weight_by_bin[bin] += weight * weight;
                    if (direction.z > 0.0)
                        result->diffuse_upward_energy += weight;
                    else if (direction.z < 0.0)
                        result->diffuse_downward_energy += weight;
                } else {
                    result->ballistic_photon_count++;
                    result->ballistic_detected_energy += weight;
                    if (direction.z < 0.0)
                        result->direct_downward_energy += weight;
                }
                DetectedPhoton detected_photon = {
                    .alpha_d_deg = acos(mu) * 180.0 / M_PI,
                    .mu = mu,
                    .weight = weight,
                    .bin = bin,
                };
                if (stream_photons) {
                    photon_write_failed = fwrite(&detected_photon, sizeof(detected_photon), 1,
                                                 result->photon_stream) != 1;
                } else {
                    result->photons[result->photon_count] = detected_photon;
                }
                if (!photon_write_failed)
                    result->photon_count++;
                result->detected_energy += weight;
                break;
            }

            position = vec3_add(position, vec3_scale(direction, free_path));

            if (weight < 1e-3) {
                const double survival_probability = 0.1;
                if (rng_next_f64(rng) < survival_probability) {
                    weight /= survival_probability;
                } else {
                    result->roulette_killed_count++;
                    result->roulette_energy += weight;
                    break;
                }
            }

            double old_weight = weight;
            weight *= config->albedo;
            local_absorbed += old_weight - weight;
            direction = sample_configured_phase_function(rng, config, direction);
            has_scattered = 1;
            result->scatter_count++;
        }

        if (local_absorbed != 0.0) {
            double adjusted_absorption = local_absorbed - absorbed_energy_correction;
            double updated_absorption = result->absorbed_energy + adjusted_absorption;
            absorbed_energy_correction =
                (updated_absorption - result->absorbed_energy) - adjusted_absorption;
            result->absorbed_energy = updated_absorption;
        }
    }

    return !photon_write_failed;
}

static void calculate_exit_statistics(SimulationResult *result,
                                      const SimulationHistograms *histograms) {
    double detected_weight = 0.0;
    double total_squared_weight = 0.0;
    for (int i = 0; i < result->bin_count; i++) {
        detected_weight += histograms->weight_by_bin[i];
        total_squared_weight += histograms->squared_weight_by_bin[i];
    }
    for (int i = 0; i < result->bin_count; i++) {
        double probability =
            detected_weight > 0.0 ? histograms->weight_by_bin[i] / detected_weight : 0.0;
        double diffuse_probability =
            detected_weight > 0.0 ? histograms->diffuse_weight_by_bin[i] / detected_weight : 0.0;
        result->exit_pdf[i] = probability;
        result->diffuse_exit_pdf[i] = diffuse_probability;
        if (detected_weight > 0.0) {
            double bin_squared_weight = histograms->squared_weight_by_bin[i];
            double other_squared_weight = total_squared_weight - bin_squared_weight;
            double variance_numerator =
                (1.0 - probability) * (1.0 - probability) * bin_squared_weight +
                probability * probability * other_squared_weight;
            result->exit_probability_standard_error[i] = sqrt(variance_numerator) / detected_weight;

            double diffuse_bin_squared_weight = histograms->diffuse_squared_weight_by_bin[i];
            double diffuse_other_squared_weight = total_squared_weight - diffuse_bin_squared_weight;
            double diffuse_variance_numerator =
                (1.0 - diffuse_probability) * (1.0 - diffuse_probability) *
                    diffuse_bin_squared_weight +
                diffuse_probability * diffuse_probability * diffuse_other_squared_weight;
            result->diffuse_exit_probability_standard_error[i] =
                sqrt(diffuse_variance_numerator) / detected_weight;
        }
    }
}

static int simulate_photons_internal(const Config *config, int seed, SimulationResult *result,
                                     int stream_photons) {
    if (!config || !result)
        return 0;

    memset(result, 0, sizeof(*result));
    if (!config_is_valid(config))
        return 0;

    SimulationHistograms histograms = {0};
    if (!initialize_simulation_result(config, result, &histograms, stream_photons)) {
        free_simulation_histograms(&histograms);
        simulation_result_free(result);
        return 0;
    }

    Rng rng;
    rng_seed(&rng, seed);

    double theta0_rad = config->theta_zero * M_PI / 180.0;
    Vec3 fixed_direction = geometry_incident_direction(theta0_rad);
    if (!transport_photons(config, &rng, fixed_direction, result, &histograms, stream_photons) ||
        (stream_photons &&
         (fflush(result->photon_stream) != 0 || fseek(result->photon_stream, 0, SEEK_SET) != 0))) {
        free_simulation_histograms(&histograms);
        simulation_result_free(result);
        return 0;
    }

    calculate_exit_statistics(result, &histograms);
    free_simulation_histograms(&histograms);
    return 1;
}

int simulate_photons(const Config *config, int seed, SimulationResult *result) {
    return simulate_photons_internal(config, seed, result, 0);
}

int simulate_photons_stream(const Config *config, int seed, SimulationResult *result) {
    return simulate_photons_internal(config, seed, result, 1);
}

void simulation_result_free(SimulationResult *result) {
    if (!result)
        return;
    free(result->photons);
    if (result->photon_stream)
        fclose(result->photon_stream);
    free(result->exit_pdf);
    free(result->exit_probability_standard_error);
    free(result->diffuse_exit_pdf);
    free(result->diffuse_exit_probability_standard_error);
    memset(result, 0, sizeof(*result));
}