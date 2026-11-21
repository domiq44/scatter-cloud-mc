/*
 * plot_data.h - Conteneur dynamique données graphiques
 * Data struct : mu[], pdf[] avec allocation croissante (×2).
 * data_init(), data_free(), data_push() pour construction incrémentale.
 *
 * Partie de scatter-cloud-mc
 */

#ifndef PLOT_DATA_H
#define PLOT_DATA_H

#include <stddef.h>

typedef struct {
    double *mu;
    double *pdf;
    size_t len;
    size_t cap;
} Data;

void data_init(Data *data);
void data_free(Data *data);
int data_push(Data *data, double mu, double pdf);

#endif
