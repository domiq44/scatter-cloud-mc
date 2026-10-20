#include <locale.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "geometry.h"
#include "plot_gnuplot_script.h"

#define GNUPLOT_DISTRIBUTION_TEMPLATE_PATH "gnuplot/angular_distribution.gp.in"
#define GNUPLOT_PHASE_TEMPLATE_PATH "gnuplot/phase_function.gp.in"

static int append_bytes(char *out, size_t max, size_t *length, const char *text,
                        size_t text_length) {
    if (*length >= max || text_length > max - *length - 1) {
        return 0;
    }
    memcpy(out + *length, text, text_length);
    *length += text_length;
    out[*length] = '\0';
    return 1;
}

static int append_format(char *out, size_t max, size_t *length, const char *format, ...) {
    if (*length >= max) {
        return 0;
    }

    va_list args;
    va_start(args, format);
    int written = vsnprintf(out + *length, max - *length, format, args);
    va_end(args);
    if (written < 0 || (size_t)written >= max - *length) {
        return 0;
    }

    *length += (size_t)written;
    return 1;
}

static int append_gnuplot_string(char *out, size_t max, size_t *length, const char *value) {
    if (!append_bytes(out, max, length, "\"", 1)) {
        return 0;
    }

    for (const unsigned char *character = (const unsigned char *)value; *character; character++) {
        char escaped[2] = {'\\', (char)*character};
        if (*character == '"' || *character == '\\') {
            if (!append_bytes(out, max, length, escaped, sizeof(escaped))) {
                return 0;
            }
        } else if (*character == '\n' || *character == '\r' || *character == '\t') {
            escaped[1] = (char)(*character == '\n' ? 'n' : (*character == '\r' ? 'r' : 't'));
            if (!append_bytes(out, max, length, escaped, sizeof(escaped))) {
                return 0;
            }
        } else if (*character < 0x20 ||
                   !append_bytes(out, max, length, (const char *)character, 1)) {
            return 0;
        }
    }

    return append_bytes(out, max, length, "\"\n", 2);
}

static int append_string_variable(char *out, size_t max, size_t *length, const char *name,
                                  const char *value) {
    return append_format(out, max, length, "%s = ", name) &&
           append_gnuplot_string(out, max, length, value);
}

static int append_template(char *out, size_t max, size_t *length, const char *template_path) {
    FILE *template = fopen(template_path, "r");
    if (!template) {
        return 0;
    }

    int character;
    int success = 1;
    while ((character = fgetc(template)) != EOF) {
        char byte = (char)character;
        if (!append_bytes(out, max, length, &byte, 1)) {
            success = 0;
            break;
        }
    }
    if (ferror(template)) {
        success = 0;
    }
    if (fclose(template) != 0) {
        success = 0;
    }
    return success;
}

int build_gnuplot_script(char *out, size_t max, const Config *cfg, const char *csv_filename,
                         const char *png_filename, GnuplotPlotType plot_type) {
    if (!out || max == 0 || !cfg || !csv_filename || !png_filename ||
        (plot_type != GNUPLOT_PLOT_DISTRIBUTION && plot_type != GNUPLOT_PLOT_PHASE &&
         plot_type != GNUPLOT_PLOT_DISTRIBUTION_LINEAR &&
         plot_type != GNUPLOT_PLOT_DISTRIBUTION_MU &&
         plot_type != GNUPLOT_PLOT_DISTRIBUTION_MU_LINEAR)) {
        return 0;
    }

    const char *template_path = plot_type == GNUPLOT_PLOT_PHASE
                                    ? GNUPLOT_PHASE_TEMPLATE_PATH
                                    : GNUPLOT_DISTRIBUTION_TEMPLATE_PATH;

    out[0] = '\0';
    size_t length = 0;
    char *old_locale = setlocale(LC_NUMERIC, NULL);
    char *saved_locale = old_locale ? strdup(old_locale) : NULL;
    if (old_locale && !saved_locale) {
        return 0;
    }
    setlocale(LC_NUMERIC, "C");

    double theta_rad = cfg->theta_zero * acos(-1.0) / 180.0;
    Vec3 incident_direction = geometry_incident_direction(theta_rad);
    double mean_chord_length =
        geometry_mean_chord_length(cfg->geometry, cfg->radius, incident_direction);
    double tau = cfg->sigma_e * mean_chord_length;
    int success = append_string_variable(out, max, &length, "data_file", csv_filename) &&
                  append_string_variable(out, max, &length, "output_file", png_filename) &&
                  append_format(out, max, &length, "theta_zero = %.0f\n", cfg->theta_zero) &&
                  append_format(out, max, &length, "tau = %.2f\n", tau) &&
                  append_format(out, max, &length, "g = %.17g\n", cfg->g) &&
                  append_format(out, max, &length, "g2 = %.17g\n", cfg->g2) &&
                  append_format(out, max, &length, "hg_fraction = %.17g\n", cfg->hg_fraction) &&
                  append_format(out, max, &length, "double_hg = %d\n",
                                cfg->phase_function == PHASE_FUNCTION_DOUBLE_HG) &&
                  append_format(out, max, &length, "log_scale = %d\n",
                                plot_type != GNUPLOT_PLOT_DISTRIBUTION_LINEAR &&
                                    plot_type != GNUPLOT_PLOT_DISTRIBUTION_MU_LINEAR) &&
                  append_format(out, max, &length, "cos_alpha_d = %d\n",
                                plot_type == GNUPLOT_PLOT_DISTRIBUTION_MU ||
                                    plot_type == GNUPLOT_PLOT_DISTRIBUTION_MU_LINEAR) &&
                  append_format(out, max, &length, "albedo = %.2f\n", cfg->albedo) &&
                  append_format(out, max, &length, "n_photons = %d\n", cfg->n_photons) &&
                  append_template(out, max, &length, template_path);

    if (saved_locale) {
        setlocale(LC_NUMERIC, saved_locale);
        free(saved_locale);
    }
    return success;
}