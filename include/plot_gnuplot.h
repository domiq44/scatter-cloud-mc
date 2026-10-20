#ifndef PLOT_GNUPLOT_H
#define PLOT_GNUPLOT_H

#include "plot_gnuplot_script.h"

int plot_with_gnuplot(const char *csv_filename, const char *png_filename, const Config *cfg,
                      GnuplotPlotType plot_type);

#endif
