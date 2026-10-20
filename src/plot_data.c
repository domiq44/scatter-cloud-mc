#include "plot_data.h"
#include <stdlib.h>
#include <string.h>

void data_init(Data *data) {
    data->mu = NULL;
    data->pdf = NULL;
    data->len = 0;
    data->cap = 0;
}

void data_free(Data *data) {
    free(data->mu);
    free(data->pdf);
    data->mu = NULL;
    data->pdf = NULL;
    data->len = 0;
    data->cap = 0;
}

int data_push(Data *data, double mu, double pdf) {
    if (data->len == data->cap) {
        size_t new_cap = data->cap == 0 ? 128 : data->cap * 2;
        double *new_mu = malloc(new_cap * sizeof(*new_mu));
        double *new_pdf = malloc(new_cap * sizeof(*new_pdf));
        if (!new_mu || !new_pdf) {
            free(new_mu);
            free(new_pdf);
            return 0;
        }

        if (data->len > 0) {
            memcpy(new_mu, data->mu, data->len * sizeof(*new_mu));
            memcpy(new_pdf, data->pdf, data->len * sizeof(*new_pdf));
        }
        free(data->mu);
        free(data->pdf);
        data->mu = new_mu;
        data->pdf = new_pdf;
        data->cap = new_cap;
    }

    data->mu[data->len] = mu;
    data->pdf[data->len] = pdf;
    data->len++;
    return 1;
}
