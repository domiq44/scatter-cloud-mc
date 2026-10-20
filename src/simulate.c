#include "config.h"
#include "file_path.h"
#include "simulation.h"
#include "simulation_csv.h"
#include "simulation_distribution_csv.h"
#include "simulation_report.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

static int build_output_filename(const Config *config, const char *output_dir, char *filename,
                                 size_t filename_size) {
    if (!config || !output_dir || !filename || filename_size == 0)
        return 0;

    const char *geometry = config->geometry == GEOMETRY_CUBE ? "cube" : "sphere";
    int length;
    if (config->phase_function == PHASE_FUNCTION_DOUBLE_HG) {
        length =
            snprintf(filename, filename_size,
                     "%s/%s_theta%.0f_radius%.8g_albedo%.2f_doubleHG_g1%.6g_g2%.6g_p%.6g_N%d.csv",
                     output_dir, geometry, config->theta_zero, config->radius, config->albedo,
                     config->g, config->g2, config->hg_fraction, config->n_photons);
    } else {
        length =
            snprintf(filename, filename_size, "%s/%s_theta%.0f_radius%.8g_albedo%.2f_g%.2f_N%d.csv",
                     output_dir, geometry, config->theta_zero, config->radius, config->albedo,
                     config->g, config->n_photons);
    }
    return length >= 0 && (size_t)length < filename_size;
}

int main(int argc, char **argv) {
    const char *config_path = argc > 1 ? argv[1] : "config/config.dat";
    const char *output_dir = argc > 3 ? argv[3] : "output";
    Config config;
    if (!load_config(config_path, &config)) {
        fprintf(stderr, "Erreur lors du chargement de %s\n", config_path);
        return 1;
    }

    int seed = config.seed;
    if (argc > 2) {
        char *end = NULL;
        errno = 0;
        long parsed_seed = strtol(argv[2], &end, 10);
        if (end == argv[2] || *end != '\0' || errno == ERANGE || parsed_seed < INT_MIN ||
            parsed_seed > INT_MAX) {
            fprintf(stderr, "Erreur : graine invalide\n");
            return 1;
        }
        seed = (int)parsed_seed;
    }

    SimulationResult result;
    if (!simulate_photons_stream(&config, seed, &result)) {
        fprintf(stderr, "Erreur : impossible d'exécuter la simulation\n");
        return 1;
    }

    mkdir(output_dir, 0755);
    char filename[256];
    if (!build_output_filename(&config, output_dir, filename, sizeof(filename))) {
        fprintf(stderr, "Erreur : chemin de sortie trop long\n");
        simulation_result_free(&result);
        return 1;
    }
    char distribution_filename[512] = {0};
    replace_extension(filename, ".csv", ".distribution.csv", distribution_filename,
                      sizeof(distribution_filename));
    char flux_filename[512] = {0};
    replace_extension(filename, ".csv", ".flux.csv", flux_filename, sizeof(flux_filename));

    if (!distribution_filename[0] || !flux_filename[0] ||
        !write_simulation_csv(filename, &result) ||
        !write_simulation_flux_csv(flux_filename, &result) ||
        !write_simulation_distribution_csv(distribution_filename, &result)) {
        simulation_result_free(&result);
        return 1;
    }

    printf("Fichier généré : %s\n", filename);
    printf("Distribution générée : %s\n", distribution_filename);
    printf("Bilan des flux généré : %s\n", flux_filename);
    print_simulation_report(&result, &config);
    simulation_result_free(&result);
    return 0;
}
