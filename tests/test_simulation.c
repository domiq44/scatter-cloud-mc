#include "simulation.h"
#include "test_assertions.h"
#include "test_runner.h"
#include <math.h>
#include <stdlib.h>

/* Vérifie le bilan énergétique dans le cas sans roulette russe. */
static TestResult test_simulation_accounts_for_energy(void) {
    /* Given */
    const Config config = {
        .n_photons = 1000,
        .sigma_e = 2.0,
        .albedo = 1.0,
        .g = 0.3,
        .radius = 1.0,
        .n_mu_bins = 200,
        .seed = 123,
        .theta_zero = 20.0,
    };
    SimulationResult result;

    /* When */
    int simulated = simulate_photons(&config, config.seed, &result);

    /* Then */
    double total_energy = result.detected_energy + result.roulette_energy + result.absorbed_energy;
    int roulette_killed_count = result.roulette_killed_count;
    int photon_count_matches =
        result.photon_count + result.roulette_killed_count == config.n_photons;
    double diffuse_probability = 0.0;
    for (int bin = 0; bin < result.bin_count; bin++)
        diffuse_probability += result.diffuse_exit_pdf[bin];
    int exit_components_match =
        fabs(result.ballistic_detected_energy + diffuse_probability * result.detected_energy -
             result.detected_energy) < 1e-8;
    simulation_result_free(&result);
    return assert_condition(simulated && photon_count_matches && roulette_killed_count == 0 &&
                                exit_components_match &&
                                fabs(total_energy - config.n_photons) < 1e-8,
                            "sans roulette russe, le bilan énergétique est conservé");
}

/* Vérifie que l'angle de sortie est référencé au faisceau incident. */
static TestResult test_simulation_measures_alpha_from_incident_direction(void) {
    /* Given */
    const Config config = {
        .n_photons = 1,
        .sigma_e = 1e-12,
        .albedo = 1.0,
        .g = 0.0,
        .radius = 1.0,
        .n_mu_bins = 18,
        .seed = 123,
        .theta_zero = 20.0,
    };
    const double expected_alpha_d_deg = 0.0;
    const double expected_mu = 1.0;
    SimulationResult result;

    /* When */
    int simulated = simulate_photons(&config, config.seed, &result);

    /* Then */
    int direction_matches = simulated && result.photon_count == 1 &&
                            fabs(result.photons[0].alpha_d_deg - expected_alpha_d_deg) < 1e-10 &&
                            fabs(result.photons[0].mu - expected_mu) < 1e-12 &&
                            result.ballistic_photon_count == 1 &&
                            fabs(result.ballistic_detected_energy - 1.0) < 1e-12;
    double diffuse_probability = 0.0;
    if (simulated) {
        for (int bin = 0; bin < result.bin_count; bin++)
            diffuse_probability += result.diffuse_exit_pdf[bin];
    }
    direction_matches = direction_matches && diffuse_probability == 0.0;
    if (simulated)
        simulation_result_free(&result);
    return assert_condition(
        direction_matches,
        "alpha_d vaut zéro pour un photon balistique, même en incidence oblique");
}

/* Vérifie que la simulation accepte le modèle double HG et conserve l'énergie. */
static TestResult test_simulation_double_hg_accounts_for_energy(void) {
    /* Given */
    const Config config = {
        .n_photons = 512,
        .sigma_e = 2.0,
        .albedo = 0.8,
        .g = 0.75,
        .radius = 1.0,
        .n_mu_bins = 32,
        .seed = 321,
        .theta_zero = 20.0,
        .phase_function = PHASE_FUNCTION_DOUBLE_HG,
        .g2 = -0.25,
        .hg_fraction = 0.85,
    };
    SimulationResult result;

    /* When */
    int simulated = simulate_photons(&config, config.seed, &result);

    /* Then */
    int photon_count_matches =
        simulated && result.photon_count + result.roulette_killed_count == config.n_photons;
    double total_energy = result.detected_energy + result.roulette_energy + result.absorbed_energy;
    simulation_result_free(&result);
    return assert_condition(photon_count_matches && fabs(total_energy - config.n_photons) < 1e-8,
                            "le double HG conserve le bilan énergétique");
}

/* Vérifie que la roulette russe tue et laisse survivre des photons. */
static TestResult test_simulation_exercises_russian_roulette(void) {
    /* Given */
    const Config config = {
        .n_photons = 1,
        .sigma_e = 100.0,
        .albedo = 0.1,
        .g = 0.0,
        .radius = 1.0,
        .n_mu_bins = 8,
        .seed = 1,
        .theta_zero = 0.0,
    };
    int roulette_killed_photon_observed = 0;
    int roulette_survivor_observed = 0;
    int photon_counts_match = 1;

    /* When */
    for (int seed = 1;
         seed <= 512 && (!roulette_killed_photon_observed || !roulette_survivor_observed); seed++) {
        SimulationResult result;
        if (!simulate_photons(&config, seed, &result)) {
            photon_counts_match = 0;
            break;
        }

        roulette_killed_photon_observed |= result.roulette_killed_count > 0;
        roulette_survivor_observed |= result.scatter_count > 4;
        photon_counts_match &=
            result.photon_count + result.roulette_killed_count == config.n_photons;
        simulation_result_free(&result);
    }

    /* Then */
    return assert_condition(roulette_killed_photon_observed && roulette_survivor_observed &&
                                photon_counts_match,
                            "les deux issues de roulette respectent le décompte des photons");
}

/* Vérifie qu'une configuration sans photons est refusée proprement. */
static TestResult test_simulation_rejects_empty_run(void) {
    /* Given */
    Config config = {
        .n_photons = 0,
        .sigma_e = 1.0,
        .albedo = 1.0,
        .g = 0.0,
        .radius = 1.0,
        .n_mu_bins = 200,
        .seed = 42,
        .theta_zero = 0.0,
    };
    SimulationResult result;

    /* When */
    int simulated = simulate_photons(&config, config.seed, &result);

    /* Then */
    simulation_result_free(&result);
    return assert_condition(!simulated, "une simulation vide est refusée");
}

/* Vérifie que la simulation rejette les paramètres physiques invalides. */
static TestResult test_simulation_rejects_invalid_parameters(void) {
    /* Given */
    Config config = {
        .n_photons = 100,
        .sigma_e = 2.0,
        .albedo = 0.7,
        .g = NAN,
        .radius = 1.0,
        .n_mu_bins = 32,
        .seed = 123,
        .theta_zero = 20.0,
    };
    SimulationResult result;

    /* When */
    int simulated = simulate_photons(&config, config.seed, &result);

    /* Then */
    simulation_result_free(&result);
    return assert_condition(!simulated, "un paramètre non fini est rejeté avant le transport");
}

/* Vérifie l'erreur-type pondérée avec les poids des photons détectés. */
static TestResult test_weighted_distribution_standard_errors(void) {
    /* Given */
    const Config config = {
        .n_photons = 512,
        .sigma_e = 2.0,
        .albedo = 0.7,
        .g = 0.3,
        .radius = 1.0,
        .n_mu_bins = 37,
        .seed = 123,
        .theta_zero = 20.0,
    };
    SimulationResult result;

    /* When */
    int simulated = simulate_photons(&config, config.seed, &result);

    /* Then */
    double *bin_weight = calloc((size_t)config.n_mu_bins, sizeof(*bin_weight));
    double *bin_squared_weight = calloc((size_t)config.n_mu_bins, sizeof(*bin_squared_weight));
    int can_check =
        simulated && result.bin_count == config.n_mu_bins && bin_weight && bin_squared_weight;
    if (can_check) {
        for (int i = 0; i < result.photon_count; i++) {
            const DetectedPhoton *photon = &result.photons[i];
            bin_weight[photon->bin] += photon->weight;
            bin_squared_weight[photon->bin] += photon->weight * photon->weight;
        }
    }

    double total_weight = 0.0;
    double total_squared_weight = 0.0;
    if (can_check) {
        for (int bin = 0; bin < config.n_mu_bins; bin++) {
            total_weight += bin_weight[bin];
            total_squared_weight += bin_squared_weight[bin];
        }
    }

    int uncertainties_match = can_check && total_weight > 0.0;
    for (int bin = 0; uncertainties_match && bin < config.n_mu_bins; bin++) {
        double probability = bin_weight[bin] / total_weight;
        double other_squared_weight = total_squared_weight - bin_squared_weight[bin];
        double variance_numerator =
            (1.0 - probability) * (1.0 - probability) * bin_squared_weight[bin] +
            probability * probability * other_squared_weight;
        double expected_standard_error = sqrt(variance_numerator) / total_weight;
        uncertainties_match =
            isfinite(result.exit_probability_standard_error[bin]) &&
            fabs(result.exit_pdf[bin] - probability) < 1e-12 &&
            fabs(result.exit_probability_standard_error[bin] - expected_standard_error) < 1e-12;
    }
    free(bin_weight);
    free(bin_squared_weight);
    if (simulated)
        simulation_result_free(&result);

    return assert_condition(uncertainties_match,
                            "l'erreur-type respecte la variance de la proportion pondérée");
}

/* Compare la transmission balistique à l'intégrale analytique sur le disque projeté. */
static TestResult test_ballistic_transmission_matches_beer_lambert(void) {
    /* Given */
    const Config config = {
        .n_photons = 200000,
        .sigma_e = 0.65,
        .albedo = 0.0,
        .g = 0.0,
        .radius = 1.25,
        .n_mu_bins = 32,
        .seed = 9876,
        .theta_zero = 0.0,
    };
    const double optical_chord = 2.0 * config.sigma_e * config.radius;
    const double expected_transmission =
        (1.0 - (1.0 + optical_chord) * exp(-optical_chord)) /
        (2.0 * config.sigma_e * config.sigma_e * config.radius * config.radius);
    SimulationResult result = {0};

    /* When */
    int simulated = simulate_photons(&config, config.seed, &result);

    /* Then */
    double measured_transmission = result.ballistic_detected_energy / config.n_photons;
    double standard_error =
        sqrt(expected_transmission * (1.0 - expected_transmission) / config.n_photons);
    int matches_reference =
        simulated && fabs(measured_transmission - expected_transmission) < 5.0 * standard_error;
    simulation_result_free(&result);
    return assert_condition(matches_reference,
                            "la transmission balistique suit la référence Beer-Lambert");
}

static const TestCase test_cases[] = {
    {"test_simulation_accounts_for_energy", test_simulation_accounts_for_energy},
    {"test_simulation_measures_alpha_from_incident_direction",
     test_simulation_measures_alpha_from_incident_direction},
    {"test_simulation_double_hg_accounts_for_energy",
     test_simulation_double_hg_accounts_for_energy},
    {"test_simulation_exercises_russian_roulette", test_simulation_exercises_russian_roulette},
    {"test_simulation_rejects_empty_run", test_simulation_rejects_empty_run},
    {"test_simulation_rejects_invalid_parameters", test_simulation_rejects_invalid_parameters},
    {"test_weighted_distribution_standard_errors", test_weighted_distribution_standard_errors},
    {"test_ballistic_transmission_matches_beer_lambert",
     test_ballistic_transmission_matches_beer_lambert},
};

int main(void) {
    return run_test_suite("simulation", test_cases, sizeof(test_cases) / sizeof(test_cases[0]));
}
