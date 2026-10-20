#include "plot_csv.h"
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parse_csv_double(const char *text, double *value) {
    char *end;
    errno = 0;
    double parsed = strtod(text, &end);
    if (end == text || errno == ERANGE || !isfinite(parsed))
        return 0;

    while (isspace((unsigned char)*end))
        end++;
    if (*end != '\0')
        return 0;

    *value = parsed;
    return 1;
}

int read_csv(const char *filename, Data *data) {
    if (!filename || !data)
        return 0;

    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Erreur: impossible d'ouvrir %s\n", filename);
        return 0;
    }

    data_init(data);

    char line[512];
    size_t line_no = 0;

    while (fgets(line, sizeof(line), f)) {
        line_no++;

        if (line_no == 1)
            continue;

        line[strcspn(line, "\r\n")] = 0;
        if (line[0] == '\0')
            continue;

        char *alpha_str = line;
        char *mu_str = strchr(alpha_str, ',');
        if (!mu_str) {
            fprintf(stderr, "Erreur: ligne CSV invalide (%s:%zu)\n", filename, line_no);
            fclose(f);
            data_free(data);
            return 0;
        }
        *mu_str++ = '\0';

        char *pdf_str = strchr(mu_str, ',');
        if (!pdf_str) {
            fprintf(stderr, "Erreur: ligne CSV invalide (%s:%zu)\n", filename, line_no);
            fclose(f);
            data_free(data);
            return 0;
        }
        *pdf_str++ = '\0';
        char *extra_column = strchr(pdf_str, ',');
        if (extra_column)
            *extra_column = '\0';

        double alpha;
        double mu;
        double pdf;
        if (!parse_csv_double(alpha_str, &alpha) || !parse_csv_double(mu_str, &mu) ||
            !parse_csv_double(pdf_str, &pdf)) {
            fprintf(stderr, "Erreur: valeur CSV invalide (%s:%zu)\n", filename, line_no);
            fclose(f);
            data_free(data);
            return 0;
        }

        if (!data_push(data, mu, pdf)) {
            fprintf(stderr, "Erreur: allocation memoire\n");
            fclose(f);
            data_free(data);
            return 0;
        }
    }

    if (ferror(f)) {
        fclose(f);
        data_free(data);
        return 0;
    }

    fclose(f);
    return 1;
}
