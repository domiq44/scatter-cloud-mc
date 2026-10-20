#include "rng.h"
#include "test_assertions.h"
#include "test_runner.h"
#include <math.h>

/* Échantillonne un libre parcours exponentiel de taux unitaire. */
static double sample_free_path(Rng *rng) {
    double sample = rng_next_f64(rng);
    return -log(1.0 - sample);
}

/* Estime la variance de la moyenne à partir de sous-échantillons indépendants. */
static double block_variance(int sample_count, int block_count, uint64_t seed) {
    Rng rng;
    rng_seed(&rng, seed);
    int block_size = sample_count / block_count;
    double block_means[block_count];

    for (int block = 0; block < block_count; block++) {
        double sum = 0.0;
        for (int i = 0; i < block_size; i++)
            sum += sample_free_path(&rng);
        block_means[block] = sum / block_size;
    }

    double mean = 0.0;
    for (int block = 0; block < block_count; block++)
        mean += block_means[block];
    mean /= block_count;

    double variance = 0.0;
    for (int block = 0; block < block_count; block++) {
        double difference = block_means[block] - mean;
        variance += difference * difference;
    }
    return variance / block_count;
}

/* Vérifie que les échantillons du générateur appartiennent à [0, 1). */
static TestResult test_rng_values(void) {
    /* Given */
    Rng rng;
    rng_seed(&rng, 123);

    /* When */
    double a = rng_next_f64(&rng);
    double b = rng_next_f64(&rng);
    double c = rng_next_f64(&rng);

    /* Then */
    return assert_condition(a >= 0.0 && a < 1.0 && b >= 0.0 && b < 1.0 && c >= 0.0 && c < 1.0,
                            "les échantillons aléatoires sont dans [0, 1)");
}

/* Vérifie qu'une graine identique reproduit la même séquence. */
static TestResult test_rng_reproducibility(void) {
    /* Given */
    Rng r1, r2;
    rng_seed(&r1, 42);
    rng_seed(&r2, 42);

    /* When */
    int sequences_match = 1;
    for (int i = 0; i < 10; i++)
        sequences_match &= rng_next_f64(&r1) == rng_next_f64(&r2);

    /* Then */
    return assert_condition(sequences_match, "une même graine reproduit la séquence aléatoire");
}

/* Vérifie que la graine zéro ne réutilise pas la séquence de la graine un. */
static TestResult test_rng_zero_seed_differs_from_one(void) {
    /* Given */
    Rng zero_seed_rng, one_seed_rng;
    rng_seed(&zero_seed_rng, 0);
    rng_seed(&one_seed_rng, 1);

    /* When */
    uint64_t zero_seed_value = rng_next_u64(&zero_seed_rng);
    uint64_t one_seed_value = rng_next_u64(&one_seed_rng);

    /* Then */
    return assert_condition(zero_seed_value != one_seed_value,
                            "les graines zéro et un produisent des séquences distinctes");
}

/* Vérifie que la roulette russe conserve l'espérance du poids. */
static TestResult test_roulette_unbiased(void) {
    /* Given */
    Rng rng;
    rng_seed(&rng, 12345);
    const double initial_weight = 0.001;
    const double survival_probability = 0.1;
    const int sample_count = 50000;
    double sum = 0.0;

    /* When */
    for (int i = 0; i < sample_count; i++) {
        double weight = initial_weight;
        if (weight < 1e-3) {
            if (rng_next_f64(&rng) < survival_probability)
                weight /= survival_probability;
            else
                weight = 0.0;
        }
        sum += weight;
    }
    double mean = sum / sample_count;

    /* Then */
    return assert_condition(mean > 0.9 * initial_weight && mean < 1.1 * initial_weight,
                            "l'espérance du poids reste dans la tolérance de 10 %");
}

/* Vérifie que la variance diminue lorsque la taille d'échantillon quadruple. */
static TestResult test_variance_mc(void) {
    /* Given */
    const int first_sample_count = 200000;
    const int second_sample_count = 800000;
    const int block_count = 20;

    /* When */
    double first_variance = block_variance(first_sample_count, block_count, 123);
    double second_variance = block_variance(second_sample_count, block_count, 456);
    double ratio = first_variance / second_variance;

    /* Then */
    return assert_condition(ratio > 2.5 && ratio < 5.5,
                            "le rapport des variances est proche de quatre");
}

static const TestCase test_cases[] = {
    {"test_rng_values", test_rng_values},
    {"test_rng_reproducibility", test_rng_reproducibility},
    {"test_rng_zero_seed_differs_from_one", test_rng_zero_seed_differs_from_one},
    {"test_roulette_unbiased", test_roulette_unbiased},
    {"test_variance_mc", test_variance_mc},
};

int main(void) {
    return run_test_suite("rng", test_cases, sizeof(test_cases) / sizeof(test_cases[0]));
}
