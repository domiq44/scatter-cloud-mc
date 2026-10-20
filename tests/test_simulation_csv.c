#include "simulation_csv.h"
#include "simulation_distribution_csv.h"
#include "test_assertions.h"
#include "test_runner.h"
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int create_temp_path(char *path, size_t path_size) {
    const char template[] = "/tmp/scatter-simulation-csv.XXXXXX";
    if (sizeof(template) > path_size)
        return 0;
    memcpy(path, template, sizeof(template));
    int descriptor = mkstemp(path);
    if (descriptor < 0)
        return 0;
    return close(descriptor) == 0;
}

static int files_match(const char *left_path, const char *right_path) {
    FILE *left = fopen(left_path, "r");
    FILE *right = fopen(right_path, "r");
    if (!left || !right) {
        if (left)
            fclose(left);
        if (right)
            fclose(right);
        return 0;
    }

    int match = 1;
    int left_char;
    int right_char;
    do {
        left_char = fgetc(left);
        right_char = fgetc(right);
        if (left_char != right_char)
            match = 0;
    } while (match && left_char != EOF);
    fclose(left);
    fclose(right);
    return match;
}

/* Vérifie que le mode à mémoire bornée produit le même CSV que le mode en mémoire. */
static TestResult test_streamed_csv_matches_in_memory_csv(void) {
    /* Given */
    const Config config = {
        .n_photons = 256,
        .sigma_e = 1.2,
        .albedo = 0.8,
        .g = 0.35,
        .radius = 1.0,
        .n_mu_bins = 24,
        .seed = 456,
        .theta_zero = 25.0,
    };
    char memory_csv[64];
    char streamed_csv[64];
    if (!create_temp_path(memory_csv, sizeof(memory_csv)) ||
        !create_temp_path(streamed_csv, sizeof(streamed_csv)))
        return assert_condition(0, "les chemins CSV temporaires sont créés");
    SimulationResult memory_result = {0};
    SimulationResult streamed_result = {0};

    /* When */
    int both_simulated = simulate_photons(&config, config.seed, &memory_result) &&
                         simulate_photons_stream(&config, config.seed, &streamed_result);
    int both_written = both_simulated && write_simulation_csv(memory_csv, &memory_result) &&
                       write_simulation_csv(streamed_csv, &streamed_result);
    int output_matches = both_written && files_match(memory_csv, streamed_csv);
    int memory_is_bounded =
        streamed_result.photons == NULL && streamed_result.photon_stream != NULL;

    /* Then */
    simulation_result_free(&memory_result);
    simulation_result_free(&streamed_result);
    remove(memory_csv);
    remove(streamed_csv);
    return assert_condition(output_matches && memory_is_bounded,
                            "le CSV en flux est identique sans tableau de photons en mémoire");
}

/* Vérifie l'en-tête et les champs écrits à partir d'un résultat. */
static TestResult test_simulation_csv_contents(void) {
    /* Given */
    char filename[64];
    if (!create_temp_path(filename, sizeof(filename)))
        return assert_condition(0, "un chemin CSV temporaire est créé");
    DetectedPhoton photon = {
        .alpha_d_deg = 60.0,
        .mu = 0.5,
        .weight = 1.0,
        .bin = 7,
    };
    SimulationResult result = {0};
    double exit_pdf[8] = {0.0};
    double exit_probability_standard_error[8] = {0.0};
    result.photons = &photon;
    result.photon_count = 1;
    result.bin_count = 8;
    result.exit_pdf = exit_pdf;
    result.exit_probability_standard_error = exit_probability_standard_error;
    result.exit_pdf[7] = 0.25;
    result.exit_probability_standard_error[7] = 0.02;
    const char *expected_header = "alpha_d_deg,mu,pdf,weight\n";
    const char *expected_row = "60.000000,0.500000,0.250000,1.000000\n";

    /* When */
    int written = write_simulation_csv(filename, &result);
    FILE *file = written ? fopen(filename, "r") : NULL;
    char header[128] = {0};
    char row[128] = {0};
    int content_matches =
        file && fgets(header, sizeof(header), file) && fgets(row, sizeof(row), file);
    if (file) {
        content_matches = content_matches && fgetc(file) == EOF;
        fclose(file);
    }
    remove(filename);

    /* Then */
    return assert_condition(content_matches && strcmp(header, expected_header) == 0 &&
                                strcmp(row, expected_row) == 0,
                            "le CSV contient l'en-tête et la ligne attendus");
}

/* Vérifie qu'un résultat incomplet ne peut pas être exporté. */
static TestResult test_simulation_csv_rejects_invalid_result(void) {
    /* Given */
    const SimulationResult invalid_result = {0};

    /* When */
    int null_filename_rejected = !write_simulation_csv(NULL, &invalid_result);
    int invalid_result_rejected = !write_simulation_csv("unused.csv", &invalid_result);

    /* Then */
    return assert_condition(null_filename_rejected && invalid_result_rejected,
                            "un chemin nul ou un résultat incomplet est refusé");
}

/* Vérifie que l'échec d'ouverture du CSV est signalé. */
static TestResult test_simulation_csv_rejects_unwritable_path(void) {
    /* Given */
    char parent_path[64];
    if (!create_temp_path(parent_path, sizeof(parent_path)))
        return assert_condition(0, "un fichier temporaire est créé comme parent invalide");
    char filename[96];
    snprintf(filename, sizeof(filename), "%s/child.csv", parent_path);
    const SimulationResult result = {
        .bin_count = 1,
        .exit_pdf = (double[]){0.0},
        .exit_probability_standard_error = (double[]){0.0},
    };

    /* When */
    int written = write_simulation_csv(filename, &result);

    /* Then */
    remove(parent_path);
    return assert_condition(!written, "un chemin dont le parent n'est pas un répertoire échoue");
}

/* Vérifie qu'un bin hors limites interrompt l'export sans annoncer le succès. */
static TestResult test_simulation_csv_rejects_out_of_range_bin(void) {
    /* Given */
    char filename[64];
    if (!create_temp_path(filename, sizeof(filename)))
        return assert_condition(0, "un chemin CSV temporaire est créé");
    DetectedPhoton photon = {.alpha_d_deg = 30.0, .mu = 0.5, .weight = 1.0, .bin = 1};
    const SimulationResult result = {
        .photons = &photon,
        .photon_count = 1,
        .bin_count = 1,
        .exit_pdf = (double[]){1.0},
        .exit_probability_standard_error = (double[]){0.0},
    };

    /* When */
    int written = write_simulation_csv(filename, &result);

    /* Then */
    remove(filename);
    return assert_condition(!written, "un bin hors limites est signalé comme écriture incomplète");
}

/* Vérifie l'export des quatre composantes du bilan de flux. */
static TestResult test_simulation_flux_csv_contents(void) {
    /* Given */
    char filename[64];
    if (!create_temp_path(filename, sizeof(filename)))
        return assert_condition(0, "un chemin CSV temporaire est créé");
    const SimulationResult result = {
        .photon_count = 3,
        .roulette_killed_count = 1,
        .diffuse_upward_energy = 0.5,
        .diffuse_downward_energy = 1.0,
        .absorbed_energy = 0.25,
        .direct_downward_energy = 1.25,
    };
    const char *expected = "component,energy,energy_fraction_of_injected\n"
                           "diffuse_upward,0.5,0.125\n"
                           "diffuse_downward,1,0.25\n"
                           "absorbed,0.25,0.0625\n"
                           "direct_downward,1.25,0.3125\n";

    /* When */
    int written = write_simulation_flux_csv(filename, &result);
    FILE *file = written ? fopen(filename, "r") : NULL;
    char content[512] = {0};
    size_t content_length = file ? fread(content, 1, sizeof(content) - 1, file) : 0;
    if (file)
        fclose(file);
    remove(filename);

    /* Then */
    return assert_condition(written && content_length == strlen(expected) &&
                                strcmp(content, expected) == 0,
                            "le CSV contient les flux et fractions attendus");
}

/* Vérifie que l'export agrégé conserve les bins vides et leur ordre angulaire. */
static TestResult test_simulation_distribution_csv_contains_all_bins(void) {
    /* Given */
    char filename[64];
    if (!create_temp_path(filename, sizeof(filename)))
        return assert_condition(0, "un chemin CSV temporaire est créé");
    SimulationResult result = {0};
    double exit_pdf[8] = {0.0};
    double exit_probability_standard_error[8] = {0.0};
    double diffuse_exit_pdf[8] = {0.0};
    double diffuse_exit_probability_standard_error[8] = {0.0};
    result.photon_count = 1;
    result.roulette_killed_count = 1;
    result.detected_energy = 1.0;
    result.ballistic_detected_energy = 0.5;
    result.bin_count = 8;
    result.exit_pdf = exit_pdf;
    result.exit_probability_standard_error = exit_probability_standard_error;
    result.diffuse_exit_pdf = diffuse_exit_pdf;
    result.diffuse_exit_probability_standard_error = diffuse_exit_probability_standard_error;
    result.exit_pdf[7] = 0.25;
    result.exit_pdf[0] = 0.75;
    result.exit_probability_standard_error[7] = 0.02;
    result.diffuse_exit_pdf[7] = 0.15;
    result.diffuse_exit_pdf[0] = 0.35;
    result.diffuse_exit_probability_standard_error[7] = 0.02;
    result.diffuse_exit_probability_standard_error[0] = 0.04;
    const char *expected_header = "alpha_d_deg,mu,density_per_mu,density_per_degree,"
                                  "alpha_d_bin_width_deg,standard_error_per_mu,"
                                  "standard_error_per_degree,"
                                  "detected_energy_fraction_of_injected,diffuse_density_per_mu,"
                                  "diffuse_standard_error_per_mu,"
                                  "ballistic_energy_fraction_of_detected\n";

    /* When */
    int written = write_simulation_distribution_csv(filename, &result);
    FILE *file = written ? fopen(filename, "r") : NULL;
    char header[256] = {0};
    int header_matches =
        file && fgets(header, sizeof(header), file) && strcmp(header, expected_header) == 0;
    int row_count = 0;
    int all_bins_match = 1;
    int angles_ascending = 1;
    double previous_alpha = -1.0;
    double total_width = 0.0;
    double total_detected_energy_fraction = 0.0;
    double total_diffuse_probability = 0.0;
    char row[256];
    while (header_matches && fgets(row, sizeof(row), file)) {
        double alpha_d_deg;
        double mu;
        double density_per_mu;
        double density_per_degree;
        double width;
        double standard_error_per_mu;
        double standard_error_per_degree;
        double detected_energy_fraction;
        double diffuse_density_per_mu;
        double diffuse_standard_error_per_mu;
        double ballistic_fraction;
        if (sscanf(row, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf", &alpha_d_deg, &mu,
                   &density_per_mu, &density_per_degree, &width, &standard_error_per_mu,
                   &standard_error_per_degree, &detected_energy_fraction, &diffuse_density_per_mu,
                   &diffuse_standard_error_per_mu, &ballistic_fraction) != 11) {
            header_matches = 0;
            break;
        }
        if (row_count > 0 && alpha_d_deg <= previous_alpha)
            angles_ascending = 0;
        previous_alpha = alpha_d_deg;
        int bin = result.bin_count - 1 - row_count;
        double mu_lower = -1.0 + 2.0 * (double)bin / result.bin_count;
        double mu_upper = -1.0 + 2.0 * ((double)bin + 1.0) / result.bin_count;
        double expected_mu = -1.0 + 2.0 * ((double)bin + 0.5) / result.bin_count;
        double alpha_lower = acos(mu_upper) * 180.0 / acos(-1.0);
        double alpha_upper = acos(mu_lower) * 180.0 / acos(-1.0);
        double expected_width = alpha_upper - alpha_lower;
        double mu_width = 2.0 / result.bin_count;
        double expected_density_per_mu = result.exit_pdf[bin] / mu_width;
        double expected_density_per_degree = result.exit_pdf[bin] / expected_width;
        double expected_error_per_mu = result.exit_probability_standard_error[bin] / mu_width;
        double expected_error_per_degree =
            result.exit_probability_standard_error[bin] / expected_width;
        double expected_diffuse_density_per_mu = result.diffuse_exit_pdf[bin] / mu_width;
        double expected_diffuse_error_per_mu =
            result.diffuse_exit_probability_standard_error[bin] / mu_width;
        double expected_energy_fraction = result.exit_pdf[bin] * result.detected_energy /
                                          (result.photon_count + result.roulette_killed_count);
        all_bins_match =
            all_bins_match && fabs(alpha_d_deg - (alpha_lower + alpha_upper) / 2.0) < 1e-6 &&
            fabs(mu - expected_mu) < 1e-6 && fabs(width - expected_width) < 1e-9 &&
            fabs(density_per_mu - expected_density_per_mu) < 1e-10 &&
            fabs(density_per_degree - expected_density_per_degree) < 1e-10 &&
            fabs(standard_error_per_mu - expected_error_per_mu) < 1e-10 &&
            fabs(standard_error_per_degree - expected_error_per_degree) < 1e-10 &&
            fabs(detected_energy_fraction - expected_energy_fraction) < 1e-10 &&
            fabs(diffuse_density_per_mu - expected_diffuse_density_per_mu) < 1e-10 &&
            fabs(diffuse_standard_error_per_mu - expected_diffuse_error_per_mu) < 1e-10 &&
            fabs(ballistic_fraction - 0.5) < 1e-10;
        total_width += width;
        total_detected_energy_fraction += detected_energy_fraction;
        total_diffuse_probability += diffuse_density_per_mu * mu_width;
        row_count++;
    }
    if (file)
        fclose(file);
    remove(filename);

    /* Then */
    return assert_condition(header_matches && row_count == result.bin_count && all_bins_match &&
                                angles_ascending && fabs(total_width - 180.0) < 1e-7 &&
                                fabs(total_detected_energy_fraction - 0.5) < 1e-10 &&
                                fabs(total_diffuse_probability - 0.5) < 1e-10,
                            "les densités en μ et en degrés respectent les largeurs des bins");
}

/* Vérifie qu'une distribution sans énergie détectée reste exportable sans division par zéro. */
static TestResult test_simulation_distribution_csv_handles_no_detected_energy(void) {
    /* Given */
    char filename[64];
    if (!create_temp_path(filename, sizeof(filename)))
        return assert_condition(0, "un chemin CSV temporaire est créé");
    const SimulationResult result = {
        .roulette_killed_count = 1,
        .bin_count = 1,
        .exit_pdf = (double[]){0.0},
        .exit_probability_standard_error = (double[]){0.0},
        .diffuse_exit_pdf = (double[]){0.0},
        .diffuse_exit_probability_standard_error = (double[]){0.0},
    };

    /* When */
    int written = write_simulation_distribution_csv(filename, &result);
    FILE *file = written ? fopen(filename, "r") : NULL;
    char header[256];
    char row[256];
    double ballistic_fraction = -1.0;
    int row_is_valid = file && fgets(header, sizeof(header), file) && fgets(row, sizeof(row), file);
    if (row_is_valid) {
        char *last_field = strrchr(row, ',');
        char *end = NULL;
        if (last_field)
            ballistic_fraction = strtod(last_field + 1, &end);
        row_is_valid = last_field && end != last_field + 1 && (*end == '\n' || *end == '\0');
    }
    if (file)
        fclose(file);
    remove(filename);

    /* Then */
    return assert_condition(written && row_is_valid && ballistic_fraction == 0.0,
                            "l'absence d'énergie détectée produit une fraction balistique nulle");
}

static const TestCase test_cases[] = {
    {"test_streamed_csv_matches_in_memory_csv", test_streamed_csv_matches_in_memory_csv},
    {"test_simulation_csv_contents", test_simulation_csv_contents},
    {"test_simulation_csv_rejects_invalid_result", test_simulation_csv_rejects_invalid_result},
    {"test_simulation_csv_rejects_unwritable_path", test_simulation_csv_rejects_unwritable_path},
    {"test_simulation_csv_rejects_out_of_range_bin", test_simulation_csv_rejects_out_of_range_bin},
    {"test_simulation_flux_csv_contents", test_simulation_flux_csv_contents},
    {"test_simulation_distribution_csv_contains_all_bins",
     test_simulation_distribution_csv_contains_all_bins},
    {"test_simulation_distribution_csv_handles_no_detected_energy",
     test_simulation_distribution_csv_handles_no_detected_energy},
};

int main(void) {
    return run_test_suite("simulation_csv", test_cases, sizeof(test_cases) / sizeof(test_cases[0]));
}
