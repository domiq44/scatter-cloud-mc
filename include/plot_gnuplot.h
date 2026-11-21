/*
 * plot_gnuplot.h - Interface exécution Gnuplot
 * plot_with_gnuplot() : lance gnuplot -persistent avec script construit.
 * Types : GNUPLOT_PLOT_DISTRIBUTION, DISTRIBUTION_LINEAR, DISTRIBUTION_MU,
 * DISTRIBUTION_MU_LINEAR, PHASE.
 *
 * Partie de scatter-cloud-mc
 */

#ifndef PLOT_GNUPLOT_H
#define PLOT_GNUPLOT_H

#include "plot_gnuplot_script.h"

int plot_with_gnuplot(const char *csv_filename, const char *png_filename, const Config *cfg,
                      GnuplotPlotType plot_type);

#endif
