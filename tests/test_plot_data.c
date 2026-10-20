#include "plot_data.h"
#include "test_assertions.h"
#include "test_runner.h"
#include <math.h>

/* Vérifie l'initialisation d'un conteneur de données vide. */
static TestResult test_data_init(void) {
    /* Given */
    Data data;

    /* When */
    data_init(&data);

    /* Then */
    return assert_condition(data.mu == NULL && data.pdf == NULL && data.len == 0 && data.cap == 0,
                            "le conteneur est initialisé vide");
}

/* Vérifie l'ajout et la conservation des données après agrandissement. */
static TestResult test_data_push_grows_and_preserves_values(void) {
    /* Given */
    Data data;
    data_init(&data);
    const size_t item_count = 129;

    /* When */
    int all_items_added = 1;
    for (size_t i = 0; i < item_count; i++)
        all_items_added &= data_push(&data, (double)i, (double)i * 2.0);

    /* Then */
    int values_match = all_items_added && data.len == item_count && data.cap >= item_count &&
                       fabs(data.mu[0]) < 1e-12 && fabs(data.pdf[0]) < 1e-12 &&
                       fabs(data.mu[item_count - 1] - (double)(item_count - 1)) < 1e-12 &&
                       fabs(data.pdf[item_count - 1] - 2.0 * (double)(item_count - 1)) < 1e-12;
    data_free(&data);
    return assert_condition(values_match, "les valeurs restent intactes après agrandissement");
}

/* Vérifie que la libération remet le conteneur dans son état vide. */
static TestResult test_data_free_resets_container(void) {
    /* Given */
    Data data;
    data_init(&data);
    int item_added = data_push(&data, 0.25, 0.75);

    /* When */
    data_free(&data);

    /* Then */
    return assert_condition(item_added && data.mu == NULL && data.pdf == NULL && data.len == 0 &&
                                data.cap == 0,
                            "la libération remet les pointeurs et compteurs à zéro");
}

static const TestCase test_cases[] = {
    {"test_data_init", test_data_init},
    {"test_data_push_grows_and_preserves_values", test_data_push_grows_and_preserves_values},
    {"test_data_free_resets_container", test_data_free_resets_container},
};

int main(void) {
    return run_test_suite("plot_data", test_cases, sizeof(test_cases) / sizeof(test_cases[0]));
}
