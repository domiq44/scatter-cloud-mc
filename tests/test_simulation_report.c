#include "simulation_report.h"
#include "test_assertions.h"
#include "test_runner.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int capture_simulation_report(const SimulationResult *result, const Config *config,
                                     char *output, size_t output_size) {
    FILE *capture = tmpfile();
    if (!capture)
        return 0;

    fflush(stdout);
    int saved_stdout = dup(STDOUT_FILENO);
    if (saved_stdout < 0) {
        fclose(capture);
        return 0;
    }

    int redirected = dup2(fileno(capture), STDOUT_FILENO) >= 0;
    if (redirected) {
        print_simulation_report(result, config);
        fflush(stdout);
    }
    int restored = dup2(saved_stdout, STDOUT_FILENO) >= 0;
    close(saved_stdout);

    rewind(capture);
    size_t output_length = fread(output, 1, output_size - 1, capture);
    output[output_length] = '\0';
    fclose(capture);
    return redirected && restored;
}

/* Vérifie le résidu Monte Carlo et le marqueur consommé par les scripts. */
static TestResult test_report_contains_energy_summary(void) {
    /* Given */
    const Config config = {.n_photons = 2, .sigma_e = 1.0, .radius = 1.0};
    const SimulationResult result = {
        .photon_count = 1,
        .roulette_killed_count = 1,
        .scatter_count = 3,
        .detected_energy = 1.25,
        .diffuse_upward_energy = 0.2,
        .diffuse_downward_energy = 0.3,
        .direct_downward_energy = 0.75,
        .roulette_energy = 0.25,
        .absorbed_energy = 0.4,
    };
    char output[2048];

    /* When */
    int captured = capture_simulation_report(&result, &config, output, sizeof(output));

    /* Then */
    return assert_condition(captured && strstr(output, "E comptabilisée") != NULL &&
                                strstr(output, "Flux diffusé montant") != NULL &&
                                strstr(output, "Flux diffusé descendant") != NULL &&
                                strstr(output, "Flux absorbé") != NULL &&
                                strstr(output, "Flux direct descendant") != NULL &&
                                strstr(output, "Résidu Monte Carlo") != NULL &&
                                strstr(output, "moyenne attendue") != NULL &&
                                strstr(output, "E_total (déduit)") == NULL &&
                                strstr(output, "MC_DIFF -0.100000000000") != NULL,
                            "le rapport qualifie le résidu et conserve son marqueur machine");
}

static const TestCase test_cases[] = {
    {"test_report_contains_energy_summary", test_report_contains_energy_summary},
};

int main(void) {
    return run_test_suite("simulation_report", test_cases,
                          sizeof(test_cases) / sizeof(test_cases[0]));
}
