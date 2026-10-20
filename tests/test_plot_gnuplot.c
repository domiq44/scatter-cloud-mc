#include "config.h"
#include "plot_gnuplot.h"
#include "plot_gnuplot_script.h"
#include "test_assertions.h"
#include "test_runner.h"
#include <string.h>

static Config setup_plot_config(void) {
    return (Config){
        .n_photons = 1000,
        .sigma_e = 2.0,
        .albedo = 0.9,
        .g = 0.5,
        .radius = 3.0,
        .n_mu_bins = 200,
        .seed = 42,
        .theta_zero = 30.0,
    };
}

/* Vérifie le script de distribution logarithmique en alpha_d. */
static TestResult test_distribution_script_uses_alpha_d_and_log_scale(void) {
    /* Given */
    const Config config = setup_plot_config();
    char script[4096];

    /* When */
    int built = build_gnuplot_script(script, sizeof(script), &config, "results.csv", "results.png",
                                     GNUPLOT_PLOT_DISTRIBUTION);

    /* Then */
    return assert_condition(
        built && strstr(script, "data_file = \"results.csv\"") != NULL &&
            strstr(script, "output_file = \"results.png\"") != NULL &&
            strstr(script, "log_scale = 1") != NULL &&
            strstr(script, "set datafile separator ','") != NULL &&
            strstr(script, "set logscale y 10") != NULL &&
            strstr(script, "set yrange [*:*]") != NULL &&
            strstr(script, "set xrange [0:180]") != NULL &&
            strstr(script, "tau = 8.00\n") != NULL && strstr(script, "cos_alpha_d = 0") != NULL &&
            strstr(script, "set xlabel 'Angle de diffusion α_d (degrés)'") != NULL &&
            strstr(script, "stats data_file using 11 nooutput") != NULL &&
            strstr(script, "using (column(x_column)):($9 > 0 ? $9 : 1/0)") != NULL &&
            strstr(script, "($9 > 2.0*$10 ? $9-$10 : $9)") != NULL &&
            strstr(script, "Henyey-Greenstein") == NULL && strstr(script, "using 1:4") == NULL,
        "le script utilise alpha_d, la densité diffuse et son erreur-type");
}

/* Vérifie que le titre utilise la corde moyenne du cube en incidence oblique. */
static TestResult test_distribution_script_cube_optical_thickness(void) {
    /* Given */
    Config config = setup_plot_config();
    config.geometry = GEOMETRY_CUBE;
    const char *expected_tau_line = "tau = 8.78\n";
    char script[4096];

    /* When */
    int built = build_gnuplot_script(script, sizeof(script), &config, "results.csv", "cube.png",
                                     GNUPLOT_PLOT_DISTRIBUTION);

    /* Then */
    return assert_condition(built && strstr(script, expected_tau_line) != NULL,
                            "le tau du cube suit sa corde moyenne à l'angle configuré");
}

/* Vérifie que l'échelle linéaire désactive le logarithme pour alpha_d. */
static TestResult test_distribution_linear_script_disables_log_scale(void) {
    /* Given */
    const Config config = setup_plot_config();
    char script[4096];

    /* When */
    int built = build_gnuplot_script(script, sizeof(script), &config, "results.csv",
                                     "results_linear.png", GNUPLOT_PLOT_DISTRIBUTION_LINEAR);

    /* Then */
    return assert_condition(built && strstr(script, "log_scale = 0") != NULL &&
                                strstr(script, "unset logscale y") != NULL &&
                                strstr(script, "set yrange [0:*") != NULL,
                            "le script linéaire désactive l'échelle logarithmique");
}

/* Vérifie que l'option mu utilise cos(alpha_d) comme abscisse. */
static TestResult test_distribution_mu_script_uses_cos_alpha_d(void) {
    /* Given */
    const Config config = setup_plot_config();
    char script[4096];

    /* When */
    int built = build_gnuplot_script(script, sizeof(script), &config, "results.csv", "mu.png",
                                     GNUPLOT_PLOT_DISTRIBUTION_MU);

    /* Then */
    return assert_condition(built && strstr(script, "cos_alpha_d = 1") != NULL &&
                                strstr(script, "log_scale = 1") != NULL &&
                                strstr(script, "x_column = 2") != NULL &&
                                strstr(script, "set xlabel 'cos(α_d)'") != NULL &&
                                strstr(script, "set xrange [-1:1]") != NULL,
                            "le script mu trace en fonction de cos(alpha_d)");
}

/* Vérifie que le mode mu linéaire désactive l'échelle logarithmique. */
static TestResult test_distribution_mu_linear_script_disables_log_scale(void) {
    /* Given */
    const Config config = setup_plot_config();
    char script[4096];

    /* When */
    int built = build_gnuplot_script(script, sizeof(script), &config, "results.csv",
                                     "mu_linear.png", GNUPLOT_PLOT_DISTRIBUTION_MU_LINEAR);

    /* Then */
    return assert_condition(built && strstr(script, "cos_alpha_d = 1") != NULL &&
                                strstr(script, "log_scale = 0") != NULL &&
                                strstr(script, "unset logscale y") != NULL,
                            "le script mu linéaire désactive l'échelle logarithmique");
}

/* Vérifie le script de phase HG simple. */
static TestResult test_phase_script_uses_hg_function(void) {
    /* Given */
    const Config config = setup_plot_config();
    char script[4096];

    /* When */
    int built = build_gnuplot_script(script, sizeof(script), &config, "results.csv",
                                     "results_phase.png", GNUPLOT_PLOT_PHASE);

    /* Then */
    return assert_condition(
        built && strstr(script, "Fonction de phase de Henyey-Greenstein") != NULL &&
            strstr(script, "P(α_d) (sr^{-1}, log)") != NULL && strstr(script, "1.0-g*g") != NULL &&
            strstr(script, "set arrow 1") != NULL &&
            strstr(script, "set output output_file") != NULL &&
            strstr(script, "Composante diffuse") == NULL,
        "le script de phase utilise la fonction HG simple");
}

/* Vérifie le script de phase du mélange double HG. */
static TestResult test_phase_script_uses_double_hg_function(void) {
    /* Given */
    Config config = setup_plot_config();
    config.phase_function = PHASE_FUNCTION_DOUBLE_HG;
    config.g2 = -0.2;
    config.hg_fraction = 0.9;
    char script[4096];

    /* When */
    int built = build_gnuplot_script(script, sizeof(script), &config, "results.csv",
                                     "results_phase.png", GNUPLOT_PLOT_PHASE);

    /* Then */
    return assert_condition(built && strstr(script, "Fonction de phase double HG") != NULL &&
                                strstr(script, "hg_component(g2,x)") != NULL &&
                                strstr(script, "g2 = -0.20000000000000001") != NULL &&
                                strstr(script, "hg_fraction = 0.90000000000000002") != NULL,
                            "le script de phase utilise les deux lobes HG configurés");
}

/* Vérifie que les chemins sont échappés dans les chaînes Gnuplot. */
static TestResult test_gnuplot_script_escapes_filenames(void) {
    /* Given */
    const Config config = {0};
    const char *csv_filename = "results \"final\"\\table.csv";
    const char *expected_data_file = "data_file = \"results \\\"final\\\"\\\\table.csv\"";
    char script[4096];

    /* When */
    int script_built = build_gnuplot_script(script, sizeof(script), &config, csv_filename,
                                            "plot.png", GNUPLOT_PLOT_DISTRIBUTION);

    /* Then */
    return assert_condition(script_built && strstr(script, expected_data_file) != NULL,
                            "les guillemets et antislashs du chemin sont échappés");
}

/* Vérifie qu'un type de représentation inconnu est refusé avant le lancement de Gnuplot. */
static TestResult test_plot_with_gnuplot_rejects_unknown_type(void) {
    /* Given */
    const Config config = {.g = 0.5};

    /* When */
    int plotted = plot_with_gnuplot("results.csv", "results.png", &config, (GnuplotPlotType)99);

    /* Then */
    return assert_condition(!plotted, "un type de représentation inconnu est refusé");
}

static const TestCase test_cases[] = {
    {"test_distribution_script_uses_alpha_d_and_log_scale",
     test_distribution_script_uses_alpha_d_and_log_scale},
    {"test_distribution_script_cube_optical_thickness",
     test_distribution_script_cube_optical_thickness},
    {"test_distribution_linear_script_disables_log_scale",
     test_distribution_linear_script_disables_log_scale},
    {"test_distribution_mu_script_uses_cos_alpha_d", test_distribution_mu_script_uses_cos_alpha_d},
    {"test_distribution_mu_linear_script_disables_log_scale",
     test_distribution_mu_linear_script_disables_log_scale},
    {"test_phase_script_uses_hg_function", test_phase_script_uses_hg_function},
    {"test_phase_script_uses_double_hg_function", test_phase_script_uses_double_hg_function},
    {"test_gnuplot_script_escapes_filenames", test_gnuplot_script_escapes_filenames},
    {"test_plot_with_gnuplot_rejects_unknown_type", test_plot_with_gnuplot_rejects_unknown_type},
};

int main(void) {
    return run_test_suite("plot_gnuplot", test_cases, sizeof(test_cases) / sizeof(test_cases[0]));
}
