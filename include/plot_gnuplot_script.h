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