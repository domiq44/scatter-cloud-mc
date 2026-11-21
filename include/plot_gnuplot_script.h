/*
 * plot_gnuplot_script.h - Génération scripts Gnuplot
 * GnuplotPlotType enum : DISTRIBUTION, PHASE, DISTRIBUTION_LINEAR, MU, MU_LINEAR.
 * build_gnuplot_script() : remplit template .gp.in avec paramètres Config.
 * Injection : theta_zero, tau, geometry, g, g2, hg_fraction, double_hg.
 *
 * Partie de scatter-cloud-mc
 */

#ifndef PLOT_GNUPLOT_SCRIPT_H
#define PLOT_GNUPLOT_SCRIPT_H

#include "config.h"
#include <stddef.h>

typedef enum {
    GNUPLOT_PLOT_DISTRIBUTION,
    GNUPLOT_PLOT_PHASE,
    GNUPLOT_PLOT_DISTRIBUTION_LINEAR,
    GNUPLOT_PLOT_DISTRIBUTION_MU,
    GNUPLOT_PLOT_DISTRIBUTION_MU_LINEAR
} GnuplotPlotType;

int build_gnuplot_script(char *out, size_t max, const Config *cfg, const char *csv_filename,
                         const char *png_filename, GnuplotPlotType plot_type);

#endif