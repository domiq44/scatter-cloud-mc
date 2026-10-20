#include "config.h"
#include "test_assertions.h"
#include "test_runner.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Crée un fichier temporaire avec le contenu de configuration fourni. */
static char *write_temp_config(const char *content) {
    char template[] = "/tmp/scatter-config.XXXXXX";
    int fd = mkstemp(template);
    if (fd < 0) {
        fprintf(stderr, "Erreur: Impossible de créer le fichier temporaire.\n");
        return NULL;
    }

    FILE *f = fdopen(fd, "w");
    if (!f) {
        fprintf(stderr, "Erreur: Impossible d'ouvrir le fichier temporaire.\n");
        return NULL;
    }
    fputs(content, f);
    fclose(f);
    return strdup(template);
}

/* Vérifie le chargement et les valeurs d'une configuration complète. */
static TestResult test_config_valid(void) {
    /* Given */
    const char *cfg_text = "n_photons = 100000\n"
                           "sigma_e = 5.0\n"
                           "albedo = 0.9\n"
                           "g = 0.85\n"
                           "radius = 1.0\n"
                           "n_mu_bins = 64\n"
                           "seed = 12345\n"
                           "theta_zero = 30.0\n";

    char *fname = write_temp_config(cfg_text);
    if (!fname)
        return assert_condition(0, "le fichier temporaire de configuration est créé");

    Config cfg = {0};
    /* When */
    int ok = load_config(fname, &cfg);

    /* Then */
    int values_match = ok && cfg.n_photons == 100000 && fabs(cfg.sigma_e - 5.0) < 1e-12 &&
                       fabs(cfg.albedo - 0.9) < 1e-12 && fabs(cfg.g - 0.85) < 1e-12 &&
                       fabs(cfg.radius - 1.0) < 1e-12 && cfg.n_mu_bins == 64 && cfg.seed == 12345 &&
                       fabs(cfg.theta_zero - 30.0) < 1e-12 &&
                       cfg.phase_function == PHASE_FUNCTION_HG && cfg.geometry == GEOMETRY_SPHERE;
    remove(fname);
    free(fname);
    return assert_condition(values_match, "tous les paramètres sont chargés avec leur valeur");
}

/* Vérifie que la géométrie cubique est sélectionnable dans la configuration. */
static TestResult test_config_cube_geometry(void) {
    /* Given */
    const char *cfg_text = "n_photons = 1000\n"
                           "sigma_e = 5.0\n"
                           "albedo = 0.9\n"
                           "g = 0.85\n"
                           "radius = 1.0\n"
                           "n_mu_bins = 64\n"
                           "seed = 12345\n"
                           "theta_zero = 30.0\n"
                           "geometry = cube\n";
    char *fname = write_temp_config(cfg_text);
    if (!fname)
        return assert_condition(0, "le fichier temporaire de configuration est créé");

    Config cfg = {0};
    /* When */
    int ok = load_config(fname, &cfg);

    /* Then */
    remove(fname);
    free(fname);
    return assert_condition(ok && cfg.geometry == GEOMETRY_CUBE, "la géométrie cube est chargée");
}

/* Vérifie le chargement des paramètres de la fonction double HG. */
static TestResult test_config_double_hg_valid(void) {
    /* Given */
    const char *cfg_text = "n_photons = 1000\n"
                           "sigma_e = 5.0\n"
                           "albedo = 0.9\n"
                           "g = 0.85\n"
                           "radius = 1.0\n"
                           "n_mu_bins = 64\n"
                           "seed = 12345\n"
                           "theta_zero = 30.0\n"
                           "phase_function = double_hg\n"
                           "g2 = -0.2\n"
                           "hg_fraction = 0.9\n";
    char *fname = write_temp_config(cfg_text);
    if (!fname)
        return assert_condition(0, "le fichier temporaire de configuration est créé");

    Config cfg = {0};
    /* When */
    int ok = load_config(fname, &cfg);

    /* Then */
    int values_match = ok && cfg.phase_function == PHASE_FUNCTION_DOUBLE_HG &&
                       fabs(cfg.g2 + 0.2) < 1e-12 && fabs(cfg.hg_fraction - 0.9) < 1e-12;
    remove(fname);
    free(fname);
    return assert_condition(values_match, "le modèle double HG et ses paramètres sont chargés");
}

/* Vérifie que le modèle double HG exige ses deux paramètres propres. */
static TestResult test_config_double_hg_requires_parameters(void) {
    /* Given */
    const char *cfg_text = "n_photons = 1000\n"
                           "sigma_e = 5.0\n"
                           "albedo = 0.9\n"
                           "g = 0.85\n"
                           "radius = 1.0\n"
                           "n_mu_bins = 64\n"
                           "seed = 12345\n"
                           "theta_zero = 30.0\n"
                           "phase_function = double_hg\n"
                           "g2 = -0.2\n";
    char *fname = write_temp_config(cfg_text);
    if (!fname)
        return assert_condition(0, "le fichier temporaire de configuration est créé");

    Config cfg = {0};
    /* When */
    int ok = load_config(fname, &cfg);

    /* Then */
    remove(fname);
    free(fname);
    return assert_condition(!ok, "un paramètre du modèle double HG manquant est rejeté");
}

/* Vérifie qu'une valeur entière invalide fait échouer le chargement. */
static TestResult test_config_invalid(void) {
    /* Given */
    const char *cfg_text = "n_photons = abc\n" /* valeur invalide */
                           "sigma_e = 5.0\n";

    char *fname = write_temp_config(cfg_text);
    if (!fname)
        return assert_condition(0, "le fichier temporaire de configuration est créé");

    Config cfg = {0};
    /* When */
    int ok = load_config(fname, &cfg);

    /* Then */
    remove(fname);
    free(fname);
    return assert_condition(ok == 0, "une valeur entière invalide est rejetée");
}

/* Vérifie qu'une clé répétée ne remplace pas un champ manquant. */
static TestResult test_config_duplicate_key_is_rejected(void) {
    /* Given */
    const char *cfg_text = "n_photons = 1000\n"
                           "n_photons = 2000\n"
                           "sigma_e = 5.0\n"
                           "albedo = 0.9\n"
                           "g = 0.85\n"
                           "radius = 1.0\n"
                           "n_mu_bins = 64\n"
                           "seed = 12345\n";
    char *fname = write_temp_config(cfg_text);
    if (!fname)
        return assert_condition(0, "le fichier temporaire de configuration est créé");

    Config cfg = {0};
    /* When */
    int ok = load_config(fname, &cfg);

    /* Then */
    remove(fname);
    free(fname);
    return assert_condition(!ok, "une clé dupliquée est rejetée même si huit valeurs sont lues");
}

/* Vérifie que les nombres non finis sont rejetés au chargement. */
static TestResult test_config_non_finite_value_is_rejected(void) {
    /* Given */
    const char *cfg_text = "n_photons = 1000\n"
                           "sigma_e = 5.0\n"
                           "albedo = 0.9\n"
                           "g = nan\n"
                           "radius = 1.0\n"
                           "n_mu_bins = 64\n"
                           "seed = 12345\n"
                           "theta_zero = 30.0\n";
    char *fname = write_temp_config(cfg_text);
    if (!fname)
        return assert_condition(0, "le fichier temporaire de configuration est créé");

    Config cfg = {0};
    /* When */
    int ok = load_config(fname, &cfg);

    /* Then */
    remove(fname);
    free(fname);
    return assert_condition(!ok, "une valeur NaN est rejetée");
}

/* Vérifie que les paramètres physiques restent dans leur domaine valide. */
static TestResult test_config_out_of_range_albedo_is_rejected(void) {
    /* Given */
    const char *cfg_text = "n_photons = 1000\n"
                           "sigma_e = 5.0\n"
                           "albedo = 1.1\n"
                           "g = 0.85\n"
                           "radius = 1.0\n"
                           "n_mu_bins = 64\n"
                           "seed = 12345\n"
                           "theta_zero = 30.0\n";
    char *fname = write_temp_config(cfg_text);
    if (!fname)
        return assert_condition(0, "le fichier temporaire de configuration est créé");

    Config cfg = {0};
    /* When */
    int ok = load_config(fname, &cfg);

    /* Then */
    remove(fname);
    free(fname);
    return assert_condition(!ok, "un albédo supérieur à un est rejeté");
}

static const TestCase test_cases[] = {
    {"test_config_valid", test_config_valid},
    {"test_config_cube_geometry", test_config_cube_geometry},
    {"test_config_double_hg_valid", test_config_double_hg_valid},
    {"test_config_double_hg_requires_parameters", test_config_double_hg_requires_parameters},
    {"test_config_invalid", test_config_invalid},
    {"test_config_duplicate_key_is_rejected", test_config_duplicate_key_is_rejected},
    {"test_config_non_finite_value_is_rejected", test_config_non_finite_value_is_rejected},
    {"test_config_out_of_range_albedo_is_rejected", test_config_out_of_range_albedo_is_rejected},
};

int main(void) {
    return run_test_suite("config", test_cases, sizeof(test_cases) / sizeof(test_cases[0]));
}
