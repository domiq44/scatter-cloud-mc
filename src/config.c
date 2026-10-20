#include "config.h"
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Utilitaires ---

static char *trim(char *s) {
    // Version robuste de trim
    while (*s && isspace((unsigned char)*s))
        s++;
    if (*s == 0)
        return s;
    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end))
        end--;
    end[1] = '\0';
    return s;
}

static int parse_int(const char *s, int *out) {
    char *end;
    errno = 0;
    long value = strtol(s, &end, 10);

    // Vérification stricte de l'échec de parsing :
    if (end == s || *end != '\0' || errno == ERANGE) {
        return 0; // Échec de parsing
    }

    if (value < INT_MIN || value > INT_MAX) {
        return 0; // Débordement
    }

    *out = (int)value;
    return 1; // Succès
}

static int parse_double(const char *s, double *out) {
    char *end;
    errno = 0;
    double value = strtod(s, &end);
    if (end == s || *end != '\0' || errno == ERANGE || !isfinite(value))
        return 0;
    *out = value;
    return 1;
}

// --- Fonction principale ---

int config_is_valid(const Config *cfg) {
    return cfg && cfg->n_photons > 0 && isfinite(cfg->sigma_e) && cfg->sigma_e > 0.0 &&
           isfinite(cfg->albedo) && cfg->albedo >= 0.0 && cfg->albedo <= 1.0 && isfinite(cfg->g) &&
           cfg->g >= -1.0 && cfg->g <= 1.0 && isfinite(cfg->radius) && cfg->radius > 0.0 &&
           cfg->n_mu_bins > 0 && isfinite(cfg->theta_zero) &&
           (cfg->geometry == GEOMETRY_SPHERE || cfg->geometry == GEOMETRY_CUBE) &&
           (cfg->phase_function == PHASE_FUNCTION_HG ||
            cfg->phase_function == PHASE_FUNCTION_DOUBLE_HG) &&
           (cfg->phase_function != PHASE_FUNCTION_DOUBLE_HG ||
            (isfinite(cfg->g2) && cfg->g2 >= -1.0 && cfg->g2 <= 1.0 && isfinite(cfg->hg_fraction) &&
             cfg->hg_fraction >= 0.0 && cfg->hg_fraction <= 1.0));
}

int load_config(const char *filename, Config *cfg) {
    if (!filename || !cfg)
        return 0;

    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Erreur : impossible d'ouvrir %s\n", filename);
        return 0;
    }

    char line[256];
    Config parsed = {
        .phase_function = PHASE_FUNCTION_HG,
        .hg_fraction = 1.0,
        .geometry = GEOMETRY_SPHERE,
    };
    unsigned int seen = 0;

    while (fgets(line, sizeof(line), f)) {
        char *p = trim(line);
        if (*p == '\0' || *p == '#')
            continue;

        char *eq = strchr(p, '=');
        if (!eq) {
            fclose(f);
            return 0;
        }

        *eq = '\0';
        char *key = trim(p);
        char *val_str = trim(eq + 1);

        int success = 0;
        unsigned int field = 0;

        if (strcmp(key, "n_photons") == 0) {
            field = 1u << 0;
            success = parse_int(val_str, &parsed.n_photons);
        } else if (strcmp(key, "sigma_e") == 0) {
            field = 1u << 1;
            success = parse_double(val_str, &parsed.sigma_e);
        } else if (strcmp(key, "albedo") == 0) {
            field = 1u << 2;
            success = parse_double(val_str, &parsed.albedo);
        } else if (strcmp(key, "g") == 0) {
            field = 1u << 3;
            success = parse_double(val_str, &parsed.g);
        } else if (strcmp(key, "radius") == 0) {
            field = 1u << 4;
            success = parse_double(val_str, &parsed.radius);
        } else if (strcmp(key, "n_mu_bins") == 0) {
            field = 1u << 5;
            success = parse_int(val_str, &parsed.n_mu_bins);
        } else if (strcmp(key, "seed") == 0) {
            field = 1u << 6;
            success = parse_int(val_str, &parsed.seed);
        } else if (strcmp(key, "theta_zero") == 0) {
            field = 1u << 7;
            success = parse_double(val_str, &parsed.theta_zero);
        } else if (strcmp(key, "phase_function") == 0) {
            field = 1u << 8;
            if (strcmp(val_str, "hg") == 0) {
                parsed.phase_function = PHASE_FUNCTION_HG;
                success = 1;
            } else if (strcmp(val_str, "double_hg") == 0) {
                parsed.phase_function = PHASE_FUNCTION_DOUBLE_HG;
                success = 1;
            }
        } else if (strcmp(key, "g2") == 0) {
            field = 1u << 9;
            success = parse_double(val_str, &parsed.g2);
        } else if (strcmp(key, "hg_fraction") == 0) {
            field = 1u << 10;
            success = parse_double(val_str, &parsed.hg_fraction);
        } else if (strcmp(key, "geometry") == 0) {
            field = 1u << 11;
            if (strcmp(val_str, "sphere") == 0) {
                parsed.geometry = GEOMETRY_SPHERE;
                success = 1;
            } else if (strcmp(val_str, "cube") == 0) {
                parsed.geometry = GEOMETRY_CUBE;
                success = 1;
            }
        }

        if (!success || (seen & field) != 0) {
            fclose(f);
            return 0;
        }
        seen |= field;
    }

    int read_succeeded = !ferror(f);
    fclose(f);

    unsigned int double_hg_fields = (1u << 9) | (1u << 10);
    int phase_fields_valid = parsed.phase_function == PHASE_FUNCTION_DOUBLE_HG
                                 ? (seen & double_hg_fields) == double_hg_fields
                                 : (seen & double_hg_fields) == 0;
    if (!read_succeeded || (seen & 0xffu) != 0xffu || !phase_fields_valid ||
        !config_is_valid(&parsed)) {
        fprintf(stderr, "Erreur : configuration invalide ou paramètres manquants dans %s\n",
                filename);
        return 0;
    }

    *cfg = parsed;
    return 1;
}
