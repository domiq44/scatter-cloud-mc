#include "test_assertions.h"
#include "test_runner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int plot_main_entrypoint(int argc, char **argv);

typedef struct {
    char directory[64];
    char original_directory[1024];
    char bin_path[128];
    char config_path[128];
    char config_dir[96];
    char csv_path[128];
    char capture_path[160];
    char gnuplot_path[160];
    char template_path[160];
    char phase_template_path[160];
    char template_dir[128];
    char *saved_path;
    char *saved_capture;
    int had_path;
    int had_capture;
    int path_changed;
    int capture_changed;
    int directory_changed;
} PlotFixture;

static int write_text_file(const char *path, const char *contents) {
    FILE *file = fopen(path, "w");
    if (!file)
        return 0;
    int written = fputs(contents, file) >= 0;
    return fclose(file) == 0 && written;
}

static int setup_plot_fixture(PlotFixture *fixture) {
    memset(fixture, 0, sizeof(*fixture));
    memcpy(fixture->directory, "/tmp/scatter-sphere-plot.XXXXXX",
           sizeof("/tmp/scatter-sphere-plot.XXXXXX"));
    if (!mkdtemp(fixture->directory) ||
        !getcwd(fixture->original_directory, sizeof(fixture->original_directory)))
        return 0;

    snprintf(fixture->bin_path, sizeof(fixture->bin_path), "%s/bin", fixture->directory);
    snprintf(fixture->config_dir, sizeof(fixture->config_dir), "%s/config", fixture->directory);
    snprintf(fixture->config_path, sizeof(fixture->config_path), "%s/config.dat",
             fixture->config_dir);
    snprintf(fixture->csv_path, sizeof(fixture->csv_path), "%s/data.csv", fixture->directory);
    snprintf(fixture->capture_path, sizeof(fixture->capture_path), "%s/gnuplot-input.txt",
             fixture->directory);
    snprintf(fixture->gnuplot_path, sizeof(fixture->gnuplot_path), "%s/bin/gnuplot",
             fixture->directory);
    snprintf(fixture->template_dir, sizeof(fixture->template_dir), "%s/gnuplot",
             fixture->directory);
    snprintf(fixture->template_path, sizeof(fixture->template_path),
             "%s/angular_distribution.gp.in", fixture->template_dir);
    snprintf(fixture->phase_template_path, sizeof(fixture->phase_template_path),
             "%s/phase_function.gp.in", fixture->template_dir);

    const char *config_contents = "n_photons = 10\n"
                                  "sigma_e = 1.0\n"
                                  "albedo = 1.0\n"
                                  "g = 0.3\n"
                                  "radius = 1.0\n"
                                  "n_mu_bins = 200\n"
                                  "seed = 42\n"
                                  "theta_zero = 30.0\n";
    const char *csv_contents = "alpha_d_deg,mu,pdf,weight\n30,0.5,0.25,1.0\n";
    const char *gnuplot_contents =
        "#!/bin/sh\nprintf '\\n# invocation\\n' >> \"$GNUPLOT_CAPTURE\"\n"
        "cat >> \"$GNUPLOT_CAPTURE\"\n";
    const char *template_contents = "set output output_file\nplot data_file using 1:3\n";
    const char *phase_template_contents = "set output output_file\nplot x title 'phase template'\n";
    int fixture_ready = mkdir(fixture->bin_path, 0700) == 0 &&
                        mkdir(fixture->config_dir, 0700) == 0 &&
                        mkdir(fixture->template_dir, 0700) == 0 &&
                        write_text_file(fixture->config_path, config_contents) &&
                        write_text_file(fixture->csv_path, csv_contents) &&
                        write_text_file(fixture->gnuplot_path, gnuplot_contents) &&
                        write_text_file(fixture->template_path, template_contents) &&
                        write_text_file(fixture->phase_template_path, phase_template_contents) &&
                        chmod(fixture->gnuplot_path, 0700) == 0;
    if (!fixture_ready)
        return 0;

    const char *previous_path = getenv("PATH");
    const char *previous_capture = getenv("GNUPLOT_CAPTURE");
    fixture->had_path = previous_path != NULL;
    fixture->had_capture = previous_capture != NULL;
    fixture->saved_path = previous_path ? strdup(previous_path) : NULL;
    fixture->saved_capture = previous_capture ? strdup(previous_capture) : NULL;
    if ((previous_path && !fixture->saved_path) || (previous_capture && !fixture->saved_capture))
        return 0;

    size_t test_path_size =
        strlen(fixture->bin_path) + (fixture->saved_path ? strlen(fixture->saved_path) : 0) + 2;
    char *test_path = malloc(test_path_size);
    if (!test_path)
        return 0;
    snprintf(test_path, test_path_size, "%s:%s", fixture->bin_path,
             fixture->saved_path ? fixture->saved_path : "");
    fixture->path_changed = setenv("PATH", test_path, 1) == 0;
    fixture->capture_changed =
        fixture->path_changed && setenv("GNUPLOT_CAPTURE", fixture->capture_path, 1) == 0;
    free(test_path);
    fixture->directory_changed = fixture->capture_changed && chdir(fixture->directory) == 0;
    return fixture->directory_changed;
}

static void cleanup_plot_fixture(PlotFixture *fixture) {
    if (fixture->directory_changed)
        chdir(fixture->original_directory);
    if (fixture->path_changed) {
        if (fixture->had_path)
            setenv("PATH", fixture->saved_path, 1);
        else
            unsetenv("PATH");
    }
    if (fixture->capture_changed) {
        if (fixture->had_capture)
            setenv("GNUPLOT_CAPTURE", fixture->saved_capture, 1);
        else
            unsetenv("GNUPLOT_CAPTURE");
    }

    remove(fixture->config_path);
    remove(fixture->csv_path);
    remove(fixture->capture_path);
    remove(fixture->gnuplot_path);
    remove(fixture->template_path);
    remove(fixture->phase_template_path);
    rmdir(fixture->bin_path);
    rmdir(fixture->config_dir);
    rmdir(fixture->template_dir);
    rmdir(fixture->directory);
    free(fixture->saved_path);
    free(fixture->saved_capture);
}

static int read_gnuplot_capture(const PlotFixture *fixture, char *script, size_t script_size) {
    FILE *capture = fopen(fixture->capture_path, "r");
    if (!capture)
        return 0;
    size_t script_length = fread(script, 1, script_size - 1, capture);
    script[script_length] = '\0';
    int read_succeeded = !ferror(capture);
    fclose(capture);
    return read_succeeded;
}

/* Vérifie que le point d'entrée refuse une commande sans fichier CSV. */
static TestResult test_plot_main_rejects_missing_arguments(void) {
    /* Given */
    char *arguments[] = {"plot_main", NULL};

    /* When */
    int app_status = plot_main_entrypoint(1, arguments);

    /* Then */
    return assert_condition(app_status == EXIT_FAILURE,
                            "le point d'entrée refuse les arguments manquants");
}

/* Vérifie que le mode both envoie distribution et phase à Gnuplot. */
static TestResult test_plot_main_both_writes_distribution_and_phase(void) {
    /* Given */
    PlotFixture fixture;
    if (!setup_plot_fixture(&fixture)) {
        cleanup_plot_fixture(&fixture);
        return assert_condition(0, "le bac à sable Gnuplot est préparé");
    }

    /* When */
    char *arguments[] = {"plot_main", "data.csv", "config/config.dat", "custom.png", "both", NULL};
    int app_status = plot_main_entrypoint(5, arguments);

    /* Then */
    char script[8192] = {0};
    int script_captured = read_gnuplot_capture(&fixture, script, sizeof(script));
    int script_matches = strstr(script, "output_file = \"custom.png\"") != NULL &&
                         strstr(script, "output_file = \"custom_phase.png\"") != NULL &&
                         strstr(script, "data_file = \"data.csv\"") != NULL &&
                         strstr(script, "plot data_file using 1:3") != NULL &&
                         strstr(script, "plot x title 'phase template'") != NULL;
    cleanup_plot_fixture(&fixture);
    return assert_condition(app_status == EXIT_SUCCESS && script_captured && script_matches,
                            "le mode both transmet les scripts de distribution et de phase");
}

/* Vérifie que le mode mu sélectionne l'axe cos(alpha_d) du script. */
static TestResult test_plot_main_mu_mode_selects_cos_alpha_d(void) {
    /* Given */
    PlotFixture fixture;
    if (!setup_plot_fixture(&fixture)) {
        cleanup_plot_fixture(&fixture);
        return assert_condition(0, "le bac à sable Gnuplot est préparé");
    }

    /* When */
    char *arguments[] = {"plot_main",     "data.csv",        "config/config.dat",
                         "custom_mu.png", "distribution-mu", NULL};
    int app_status = plot_main_entrypoint(5, arguments);

    /* Then */
    char script[4096] = {0};
    int script_captured = read_gnuplot_capture(&fixture, script, sizeof(script));
    int script_matches = strstr(script, "output_file = \"custom_mu.png\"") != NULL &&
                         strstr(script, "cos_alpha_d = 1") != NULL &&
                         strstr(script, "custom_mu_phase.png") == NULL;
    cleanup_plot_fixture(&fixture);
    return assert_condition(app_status == EXIT_SUCCESS && script_captured && script_matches,
                            "le CLI active cos(alpha_d) sans tracer la phase");
}

/* Vérifie que le mode linéaire transmet l'option d'échelle au script. */
static TestResult test_plot_main_linear_mode_disables_log_scale(void) {
    /* Given */
    PlotFixture fixture;
    if (!setup_plot_fixture(&fixture)) {
        cleanup_plot_fixture(&fixture);
        return assert_condition(0, "le bac à sable Gnuplot est préparé");
    }

    /* When */
    char *arguments[] = {"plot_main",           "data.csv",
                         "config/config.dat",   "custom_linear.png",
                         "distribution-linear", NULL};
    int app_status = plot_main_entrypoint(5, arguments);

    /* Then */
    char script[4096] = {0};
    int script_captured = read_gnuplot_capture(&fixture, script, sizeof(script));
    int script_matches = strstr(script, "output_file = \"custom_linear.png\"") != NULL &&
                         strstr(script, "log_scale = 0") != NULL &&
                         strstr(script, "custom_linear_phase.png") == NULL;
    cleanup_plot_fixture(&fixture);
    return assert_condition(app_status == EXIT_SUCCESS && script_captured && script_matches,
                            "le CLI désactive l'échelle logarithmique sans tracer la phase");
}

/* Vérifie que le mode mu linéaire transmet ses deux options au script. */
static TestResult test_plot_main_mu_linear_mode_selects_linear_cos_alpha_d(void) {
    /* Given */
    PlotFixture fixture;
    if (!setup_plot_fixture(&fixture)) {
        cleanup_plot_fixture(&fixture);
        return assert_condition(0, "le bac à sable Gnuplot est préparé");
    }

    /* When */
    char *arguments[] = {"plot_main",
                         "data.csv",
                         "config/config.dat",
                         "custom_mu_linear.png",
                         "distribution-mu-linear",
                         NULL};
    int app_status = plot_main_entrypoint(5, arguments);

    /* Then */
    char script[4096] = {0};
    int script_captured = read_gnuplot_capture(&fixture, script, sizeof(script));
    int script_matches = strstr(script, "output_file = \"custom_mu_linear.png\"") != NULL &&
                         strstr(script, "cos_alpha_d = 1") != NULL &&
                         strstr(script, "log_scale = 0") != NULL &&
                         strstr(script, "custom_mu_linear_phase.png") == NULL;
    cleanup_plot_fixture(&fixture);
    return assert_condition(app_status == EXIT_SUCCESS && script_captured && script_matches,
                            "le CLI trace cos(alpha_d) en échelle linéaire uniquement");
}

static const TestCase test_cases[] = {
    {"test_plot_main_rejects_missing_arguments", test_plot_main_rejects_missing_arguments},
    {"test_plot_main_both_writes_distribution_and_phase",
     test_plot_main_both_writes_distribution_and_phase},
    {"test_plot_main_mu_mode_selects_cos_alpha_d", test_plot_main_mu_mode_selects_cos_alpha_d},
    {"test_plot_main_linear_mode_disables_log_scale",
     test_plot_main_linear_mode_disables_log_scale},
    {"test_plot_main_mu_linear_mode_selects_linear_cos_alpha_d",
     test_plot_main_mu_linear_mode_selects_linear_cos_alpha_d},
};

int main(void) {
    return run_test_suite("plot_main", test_cases, sizeof(test_cases) / sizeof(test_cases[0]));
}
