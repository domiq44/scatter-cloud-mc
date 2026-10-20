#include <stdio.h>

#include "plot_gnuplot.h"
#include "plot_gnuplot_script.h"

#ifdef UNIT_TESTING
#define fprintf fake_fprintf
#endif

int plot_with_gnuplot(const char *csv_filename, const char *png_filename, const Config *cfg,
                      GnuplotPlotType plot_type) {
    char script[4096];
    if (!build_gnuplot_script(script, sizeof(script), cfg, csv_filename, png_filename, plot_type)) {
        fprintf(stderr, "Erreur: impossible de construire le script Gnuplot\n");
        return 0;
    }

    FILE *gp = popen("gnuplot -persistent", "w");
    if (!gp) {
        fprintf(stderr, "Erreur: impossible de lancer gnuplot\n");
        return 0;
    }

    int success = fputs(script, gp) >= 0 && fflush(gp) == 0;
    int status = pclose(gp);
    return success && status == 0;
}
