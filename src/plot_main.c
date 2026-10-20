#include "config.h"
#include "file_path.h"
#include "plot_csv.h"
#include "plot_gnuplot.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    if (argc < 2 || argc > 5) {
        fprintf(stderr,
                "Usage : %s <distribution.csv> [fichier_config] [fichier_png] "
                "[distribution|distribution-linear|distribution-mu|"
                "distribution-mu-linear|phase|both]\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    const char *csv_filename = argv[1];
    const char *config_path = argc >= 3 ? argv[2] : "config/config.dat";

    Config cfg;
    if (!load_config(config_path, &cfg)) {
        fprintf(stderr, "Erreur lors du chargement de %s\n", config_path);
        return EXIT_FAILURE;
    }

    Data data;
    if (!read_csv(csv_filename, &data)) {
        return EXIT_FAILURE;
    }
    data_free(&data);

    char generated_png_filename[512];
    const char *png_filename = argc >= 4 ? argv[3] : generated_png_filename;
    if (argc < 4)
        replace_extension(csv_filename, ".csv", ".png", generated_png_filename,
                          sizeof(generated_png_filename));

    const char *plot_type_argument = argc == 5 ? argv[4] : "both";
    int linear_distribution = strcmp(plot_type_argument, "distribution-linear") == 0;
    int mu_distribution = strcmp(plot_type_argument, "distribution-mu") == 0;
    int mu_linear_distribution = strcmp(plot_type_argument, "distribution-mu-linear") == 0;
    int plot_distribution = strcmp(plot_type_argument, "distribution") == 0 ||
                            linear_distribution || mu_distribution || mu_linear_distribution ||
                            strcmp(plot_type_argument, "both") == 0;
    int plot_phase =
        strcmp(plot_type_argument, "phase") == 0 || strcmp(plot_type_argument, "both") == 0;
    if (!plot_distribution && !plot_phase) {
        fprintf(stderr, "Erreur : type de graphique invalide (distribution, phase ou both)\n");
        return EXIT_FAILURE;
    }

    char phase_png_filename[512];
    replace_extension(png_filename, ".png", "_phase.png", phase_png_filename,
                      sizeof(phase_png_filename));

    GnuplotPlotType distribution_plot_type = GNUPLOT_PLOT_DISTRIBUTION;
    if (linear_distribution)
        distribution_plot_type = GNUPLOT_PLOT_DISTRIBUTION_LINEAR;
    else if (mu_distribution)
        distribution_plot_type = GNUPLOT_PLOT_DISTRIBUTION_MU;
    else if (mu_linear_distribution)
        distribution_plot_type = GNUPLOT_PLOT_DISTRIBUTION_MU_LINEAR;
    if (plot_distribution &&
        !plot_with_gnuplot(csv_filename, png_filename, &cfg, distribution_plot_type)) {
        return EXIT_FAILURE;
    }
    if (plot_phase &&
        !plot_with_gnuplot(csv_filename, phase_png_filename, &cfg, GNUPLOT_PLOT_PHASE)) {
        return EXIT_FAILURE;
    }

    if (plot_distribution)
        printf("Distribution générée : %s\n", png_filename);
    if (plot_phase)
        printf("Fonction de phase générée : %s\n", phase_png_filename);
    return EXIT_SUCCESS;
}
