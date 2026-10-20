#include "simulation_distribution_csv.h"
#include <math.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int write_simulation_distribution_csv(const char *filename, const SimulationResult *result) {
    if (!filename || !result || result->bin_count <= 0 || !result->exit_pdf ||
        !result->exit_probability_standard_error || !result->diffuse_exit_pdf ||
        !result->diffuse_exit_probability_standard_error)
        return 0;

    int injected_count = result->photon_count + result->roulette_killed_count;
    if (injected_count <= 0)
        return 0;

    FILE *file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Erreur : impossible d'ouvrir %s\n", filename);
        return 0;
    }

    int write_succeeded = fprintf(file, "alpha_d_deg,mu,density_per_mu,density_per_degree,"
                                        "alpha_d_bin_width_deg,standard_error_per_mu,"
                                        "standard_error_per_degree,"
                                        "detected_energy_fraction_of_injected,"
                                        "diffuse_density_per_mu,diffuse_standard_error_per_mu,"
                                        "ballistic_energy_fraction_of_detected\n") >= 0;
    double ballistic_fraction = result->detected_energy > 0.0
                                    ? result->ballistic_detected_energy / result->detected_energy
                                    : 0.0;
    for (int bin = result->bin_count - 1; write_succeeded && bin >= 0; bin--) {
        double mu_lower = -1.0 + 2.0 * (double)bin / result->bin_count;
        double mu_upper = -1.0 + 2.0 * ((double)bin + 1.0) / result->bin_count;
        double mu = -1.0 + 2.0 * ((double)bin + 0.5) / result->bin_count;
        double alpha_lower_deg = acos(mu_upper) * 180.0 / M_PI;
        double alpha_upper_deg = acos(mu_lower) * 180.0 / M_PI;
        double alpha_d_deg = (alpha_lower_deg + alpha_upper_deg) / 2.0;
        double alpha_width_deg = alpha_upper_deg - alpha_lower_deg;
        double mu_width = 2.0 / result->bin_count;
        double density_per_mu = result->exit_pdf[bin] / mu_width;
        double density_per_degree = result->exit_pdf[bin] / alpha_width_deg;
        double standard_error_per_mu = result->exit_probability_standard_error[bin] / mu_width;
        double standard_error_per_degree =
            result->exit_probability_standard_error[bin] / alpha_width_deg;
        double detected_energy_fraction =
            result->exit_pdf[bin] * result->detected_energy / injected_count;
        double diffuse_density_per_mu = result->diffuse_exit_pdf[bin] / mu_width;
        double diffuse_standard_error_per_mu =
            result->diffuse_exit_probability_standard_error[bin] / mu_width;
        write_succeeded =
            fprintf(file, "%.6f,%.6f,%.12g,%.12g,%.12g,%.12g,%.12g,%.12g,%.12g,%.12g,%.12g\n",
                    alpha_d_deg, mu, density_per_mu, density_per_degree, alpha_width_deg,
                    standard_error_per_mu, standard_error_per_degree, detected_energy_fraction,
                    diffuse_density_per_mu, diffuse_standard_error_per_mu, ballistic_fraction) >= 0;
    }

    if (fclose(file) != 0)
        write_succeeded = 0;
    if (!write_succeeded)
        fprintf(stderr, "Erreur : écriture incomplète dans %s\n", filename);
    return write_succeeded;
}