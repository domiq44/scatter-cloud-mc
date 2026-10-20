#include "test_assertions.h"
#include "test_runner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int simulate_entrypoint(int argc, char **argv);

static int write_text_file(const char *path, const char *contents) {
    FILE *file = fopen(path, "w");
    if (!file)
        return 0;
    int written = fputs(contents, file) >= 0;
    return fclose(file) == 0 && written;
}

static int write_test_config(const char *path, const char *geometry) {
    char contents[256];
    int length = snprintf(contents, sizeof(contents),
                          "n_photons = 16\nsigma_e = 0.5\nalbedo = 1.0\ng = 0.3\n"
                          "radius = 0.00015\nn_mu_bins = 200\nseed = 42\ntheta_zero = 0.0\n"
                          "geometry = %s\n",
                          geometry);
    return length >= 0 && (size_t)length < sizeof(contents) && write_text_file(path, contents);
}

/* Vérifie que le point d'entrée produit le CSV configuré dans son répertoire de travail. */
static TestResult test_simulate_entrypoint_writes_csv(void) {
    /* Given */
    char temp_dir[] = "/tmp/scatter-sphere-simulate.XXXXXX";
    if (!mkdtemp(temp_dir))
        return assert_condition(0, "un répertoire temporaire est créé");

    char original_dir[1024];
    if (!getcwd(original_dir, sizeof(original_dir))) {
        rmdir(temp_dir);
        return assert_condition(0, "le répertoire courant est récupéré");
    }

    char config_path[128];
    char config_dir[96];
    snprintf(config_dir, sizeof(config_dir), "%s/config", temp_dir);
    snprintf(config_path, sizeof(config_path), "%s/config.dat", config_dir);
    int config_dir_created = mkdir(config_dir, 0700) == 0;
    int config_written = config_dir_created && write_test_config(config_path, "sphere");
    int directory_changed = config_written && chdir(temp_dir) == 0;

    /* When */
    char *arguments[] = {"simulate", "config/config.dat", "123", "output", NULL};
    int app_status = directory_changed ? simulate_entrypoint(4, arguments) : 1;

    /* Then */
    FILE *csv = directory_changed
                    ? fopen("output/sphere_theta0_radius0.00015_albedo1.00_g0.30_N16.csv", "r")
                    : NULL;
    FILE *distribution =
        directory_changed
            ? fopen("output/sphere_theta0_radius0.00015_albedo1.00_g0.30_N16.distribution.csv", "r")
            : NULL;
    FILE *flux =
        directory_changed
            ? fopen("output/sphere_theta0_radius0.00015_albedo1.00_g0.30_N16.flux.csv", "r")
            : NULL;
    char header[128] = {0};
    char row[256];
    int csv_written = csv && fgets(header, sizeof(header), csv) &&
                      strcmp(header, "alpha_d_deg,mu,pdf,weight\n") == 0;
    int csv_row_count = 0;
    while (csv && fgets(row, sizeof(row), csv))
        csv_row_count++;
    char distribution_header[256] = {0};
    int distribution_written =
        distribution && fgets(distribution_header, sizeof(distribution_header), distribution) &&
        strcmp(distribution_header,
               "alpha_d_deg,mu,density_per_mu,density_per_degree,alpha_d_bin_width_deg,"
               "standard_error_per_mu,standard_error_per_degree,"
               "detected_energy_fraction_of_injected,diffuse_density_per_mu,"
               "diffuse_standard_error_per_mu,ballistic_energy_fraction_of_detected\n") == 0;
    int distribution_row_count = 0;
    while (distribution && fgets(row, sizeof(row), distribution))
        distribution_row_count++;
    char flux_header[128] = {0};
    int flux_written = flux && fgets(flux_header, sizeof(flux_header), flux) &&
                       strcmp(flux_header, "component,energy,energy_fraction_of_injected\n") == 0;
    if (csv)
        fclose(csv);
    if (distribution)
        fclose(distribution);
    if (flux)
        fclose(flux);

    int cube_config_written = directory_changed && write_test_config(config_path, "cube");
    char *cube_arguments[] = {"simulate", "config/config.dat", "123", "output", NULL};
    int cube_app_status = cube_config_written ? simulate_entrypoint(4, cube_arguments) : 1;
    FILE *cube_csv = cube_config_written
                         ? fopen("output/cube_theta0_radius0.00015_albedo1.00_g0.30_N16.csv", "r")
                         : NULL;
    FILE *cube_distribution =
        cube_config_written
            ? fopen("output/cube_theta0_radius0.00015_albedo1.00_g0.30_N16.distribution.csv", "r")
            : NULL;
    FILE *cube_flux =
        cube_config_written
            ? fopen("output/cube_theta0_radius0.00015_albedo1.00_g0.30_N16.flux.csv", "r")
            : NULL;
    int cube_outputs_written = cube_csv && cube_distribution && cube_flux;
    if (cube_csv)
        fclose(cube_csv);
    if (cube_distribution)
        fclose(cube_distribution);
    if (cube_flux)
        fclose(cube_flux);

    int directory_restored = !directory_changed || chdir(original_dir) == 0;
    char csv_path[160];
    snprintf(csv_path, sizeof(csv_path),
             "%s/output/sphere_theta0_radius0.00015_albedo1.00_g0.30_N16.csv", temp_dir);
    remove(csv_path);
    char distribution_path[192];
    snprintf(distribution_path, sizeof(distribution_path),
             "%s/output/sphere_theta0_radius0.00015_albedo1.00_g0.30_N16.distribution.csv",
             temp_dir);
    remove(distribution_path);
    char flux_path[192];
    snprintf(flux_path, sizeof(flux_path),
             "%s/output/sphere_theta0_radius0.00015_albedo1.00_g0.30_N16.flux.csv", temp_dir);
    remove(flux_path);
    snprintf(csv_path, sizeof(csv_path),
             "%s/output/cube_theta0_radius0.00015_albedo1.00_g0.30_N16.csv", temp_dir);
    remove(csv_path);
    snprintf(distribution_path, sizeof(distribution_path),
             "%s/output/cube_theta0_radius0.00015_albedo1.00_g0.30_N16.distribution.csv", temp_dir);
    remove(distribution_path);
    snprintf(flux_path, sizeof(flux_path),
             "%s/output/cube_theta0_radius0.00015_albedo1.00_g0.30_N16.flux.csv", temp_dir);
    remove(flux_path);
    char output_path[128];
    snprintf(output_path, sizeof(output_path), "%s/output", temp_dir);
    rmdir(output_path);
    remove(config_path);
    rmdir(config_dir);
    rmdir(temp_dir);

    return assert_condition(config_written && directory_changed && app_status == 0 && csv_written &&
                                csv_row_count == 16 && distribution_written && flux_written &&
                                distribution_row_count == 200 && cube_config_written &&
                                cube_app_status == 0 && cube_outputs_written && directory_restored,
                            "les noms incluent la géométrie et conservent la précision du rayon");
}

/* Vérifie que le point d'entrée refuse une graine qui n'est pas un entier. */
static TestResult test_simulate_entrypoint_rejects_invalid_seed(void) {
    /* Given */
    char temp_dir[] = "/tmp/scatter-sphere-simulate-seed.XXXXXX";
    if (!mkdtemp(temp_dir))
        return assert_condition(0, "un répertoire temporaire est créé");
    char config_path[128];
    char output_path[128];
    snprintf(config_path, sizeof(config_path), "%s/config.dat", temp_dir);
    snprintf(output_path, sizeof(output_path), "%s/output", temp_dir);
    int config_written = write_test_config(config_path, "sphere");
    char *arguments[] = {"simulate", config_path, "not-an-integer", output_path, NULL};

    /* When */
    int app_status = config_written ? simulate_entrypoint(4, arguments) : 0;

    /* Then */
    int output_not_created = access(output_path, F_OK) != 0;
    remove(config_path);
    rmdir(temp_dir);
    return assert_condition(config_written && app_status == 1 && output_not_created,
                            "une graine invalide échoue avant de créer le répertoire de sortie");
}

/* Vérifie que l'échec de création des CSV est remonté par le point d'entrée. */
static TestResult test_simulate_entrypoint_rejects_file_as_output_directory(void) {
    /* Given */
    char temp_dir[] = "/tmp/scatter-sphere-simulate-output.XXXXXX";
    if (!mkdtemp(temp_dir))
        return assert_condition(0, "un répertoire temporaire est créé");
    char config_path[128];
    char output_path[128];
    snprintf(config_path, sizeof(config_path), "%s/config.dat", temp_dir);
    snprintf(output_path, sizeof(output_path), "%s/output", temp_dir);
    int config_written = write_test_config(config_path, "sphere");
    int output_blocked = config_written && write_text_file(output_path, "not a directory");
    char *arguments[] = {"simulate", config_path, "123", output_path, NULL};

    /* When */
    int app_status = output_blocked ? simulate_entrypoint(4, arguments) : 0;

    /* Then */
    remove(config_path);
    remove(output_path);
    rmdir(temp_dir);
    return assert_condition(config_written && output_blocked && app_status == 1,
                            "une erreur d'écriture des CSV produit un statut d'échec");
}

static const TestCase test_cases[] = {
    {"test_simulate_entrypoint_writes_csv", test_simulate_entrypoint_writes_csv},
    {"test_simulate_entrypoint_rejects_invalid_seed",
     test_simulate_entrypoint_rejects_invalid_seed},
    {"test_simulate_entrypoint_rejects_file_as_output_directory",
     test_simulate_entrypoint_rejects_file_as_output_directory},
};

int main(void) {
    return run_test_suite("simulate", test_cases, sizeof(test_cases) / sizeof(test_cases[0]));
}
