/*
 * plot_csv.h - Lecture CSV pour graphiques
 * read_csv() : parse *.distribution.csv → Data (mu, pdf).
 * Validation : 3 colonnes, nombres finis, pas colonnes extra.
 *
 * Partie de scatter-cloud-mc
 */

#ifndef PLOT_CSV_H
#define PLOT_CSV_H

#include "plot_data.h"

int read_csv(const char *filename, Data *data);

#endif
