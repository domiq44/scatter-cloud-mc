#include "hg.h"
#include "rng.h"
#include "test_assertions.h"
#include "test_runner.h"
#include "vec3.h"
#include <math.h>

/* Applique plusieurs diffusions isotropes et renvoie la direction finale. */
static Vec3 random_walk_direction(Rng *rng, int scatter_count) {
    const double g = 0.0;
    Vec3 direction = vec3_new(0, 0, -1);
    for (int i = 0; i < scatter_count; i++)
        direction = sample_henyey_greenstein(rng, g, direction);
    return direction;
}

/* Vérifie que les directions HG échantillonnées sont normalisées. */
static TestResult test_hg_normalization(void) {
    /* Given */
    Rng rng;
    rng_seed(&rng, 123);

    Vec3 dir = vec3_new(0, 0, 1);

    /* When */
    int normalized = 1;
    for (int i = 0; i < 100; i++) {
        Vec3 d = sample_henyey_greenstein(&rng, 0.5, dir);
        normalized &= fabs(vec3_norm(d) - 1.0) < 1e-6;
    }

    /* Then */
    return assert_condition(normalized, "les directions HG ont une norme unitaire");
}

/* Vérifie que la moyenne de cos(theta) approche le paramètre g. */
static TestResult test_hg_mean_mu(void) {
    /* Given */
    Rng rng;
    rng_seed(&rng, 42);

    Vec3 dir = vec3_new(0, 0, 1);
    double g = 0.85;

    double sum = 0.0;
    int N = 20000;

    /* When */
    for (int i = 0; i < N; i++) {
        Vec3 d = sample_henyey_greenstein(&rng, g, dir);
        sum += vec3_dot(dir, d);
    }

    double mu_mean = sum / N;

    /* Then */
    return assert_condition(mu_mean > g - 0.05 && mu_mean < g + 0.05,
                            "la moyenne angulaire est proche de g");
}

/* Vérifie que les diffusions isotropes annulent le moment angulaire moyen. */
static TestResult test_angular_moment_zero(void) {
    /* Given */
    Rng rng;
    rng_seed(&rng, 123);
    const int sample_count = 50000;
    const int scatter_count = 5;
    double sum_x = 0.0;
    double sum_y = 0.0;
    double sum_z = 0.0;

    /* When */
    for (int i = 0; i < sample_count; i++) {
        Vec3 direction = random_walk_direction(&rng, scatter_count);
        sum_x += direction.x;
        sum_y += direction.y;
        sum_z += direction.z;
    }

    /* Then */
    return assert_condition(fabs(sum_x / sample_count) < 0.02 &&
                                fabs(sum_y / sample_count) < 0.02 &&
                                fabs(sum_z / sample_count) < 0.02,
                            "les composantes du moment angulaire moyen sont proches de zéro");
}

/* Vérifie que la moyenne de cos(theta) est proche du paramètre d'asymétrie. */
static TestResult test_hg_mean_matches_g(void) {
    /* Given */
    Rng rng;
    rng_seed(&rng, 12345);
    const double g = 0.7;
    const int sample_count = 50000;
    const Vec3 initial_direction = {0, 0, 1};
    double sum = 0.0;

    /* When */
    for (int i = 0; i < sample_count; i++) {
        Vec3 direction = sample_henyey_greenstein(&rng, g, initial_direction);
        sum += vec3_dot(direction, initial_direction);
    }

    /* Then */
    double mean = sum / sample_count;
    return assert_condition(mean > g - 0.03 && mean < g + 0.03,
                            "la moyenne HG respecte la tolérance autour de g");
}

/* Vérifie le moment angulaire moyen du mélange de deux lois HG. */
static TestResult test_double_hg_mean_matches_weighted_asymmetry(void) {
    /* Given */
    Rng rng;
    rng_seed(&rng, 5678);
    const double g1 = 0.8;
    const double g2 = -0.3;
    const double hg_fraction = 0.7;
    const int sample_count = 50000;
    const Vec3 initial_direction = {0, 0, 1};
    double sum = 0.0;

    /* When */
    for (int i = 0; i < sample_count; i++) {
        Vec3 direction =
            sample_double_henyey_greenstein(&rng, g1, g2, hg_fraction, initial_direction);
        sum += vec3_dot(direction, initial_direction);
    }

    /* Then */
    double mean = sum / sample_count;
    double expected_mean = hg_fraction * g1 + (1.0 - hg_fraction) * g2;
    return assert_condition(fabs(mean - expected_mean) < 0.03,
                            "la moyenne du mélange est la moyenne pondérée des deux paramètres");
}

/* Vérifie que la diffusion isotrope a une moyenne angulaire nulle. */
static TestResult test_hg_isotropic(void) {
    /* Given */
    Rng rng;
    rng_seed(&rng, 98765);
    const double g = 0.0;
    const int sample_count = 50000;
    const Vec3 initial_direction = {0, 0, 1};
    double sum = 0.0;

    /* When */
    for (int i = 0; i < sample_count; i++) {
        Vec3 direction = sample_henyey_greenstein(&rng, g, initial_direction);
        sum += vec3_dot(direction, initial_direction);
    }

    /* Then */
    double mean = sum / sample_count;
    return assert_condition(mean > -0.03 && mean < 0.03,
                            "la diffusion isotrope a une moyenne proche de zéro");
}

/* Vérifie les directions incidentes et les cosinus obtenus après diffusion. */
static TestResult test_sampled_direction_mu_is_in_range(void) {
    /* Given */
    Rng rng;
    rng_seed(&rng, 12345);
    const double g = 0.85;
    const double theta = 30.0 * acos(-1.0) / 180.0;

    /* When */
    Vec3 initial_direction = vec3_normalize(vec3_new(sin(theta), 0.0, -cos(theta)));
    int directions_are_valid = fabs(vec3_norm(initial_direction) - 1.0) < 1e-12;
    for (int i = 0; i < 100; i++) {
        Vec3 direction = sample_henyey_greenstein(&rng, g, initial_direction);
        double mu = vec3_dot(direction, initial_direction);
        directions_are_valid &= mu >= -1.0 && mu <= 1.0;
    }

    /* Then */
    return assert_condition(directions_are_valid,
                            "les directions restent normalisées et mu reste dans [-1, 1]");
}

/* Vérifie les directions déterministes aux bornes du paramètre d'asymétrie. */
static TestResult test_hg_handles_asymmetry_endpoints(void) {
    /* Given */
    Rng rng;
    rng_seed(&rng, 12345);
    const Vec3 initial_direction = vec3_normalize(vec3_new(1.0, 2.0, -3.0));

    /* When */
    const Vec3 forward_direction = sample_henyey_greenstein(&rng, 1.0, initial_direction);
    const Vec3 backward_direction = sample_henyey_greenstein(&rng, -1.0, initial_direction);

    /* Then */
    return assert_condition(fabs(vec3_dot(forward_direction, initial_direction) - 1.0) < 1e-12 &&
                                fabs(vec3_dot(backward_direction, initial_direction) + 1.0) < 1e-12,
                            "g = 1 et g = -1 donnent les directions limites exactes");
}

static const TestCase test_cases[] = {
    {"test_hg_normalization", test_hg_normalization},
    {"test_hg_mean_mu", test_hg_mean_mu},
    {"test_angular_moment_zero", test_angular_moment_zero},
    {"test_hg_mean_matches_g", test_hg_mean_matches_g},
    {"test_double_hg_mean_matches_weighted_asymmetry",
     test_double_hg_mean_matches_weighted_asymmetry},
    {"test_hg_isotropic", test_hg_isotropic},
    {"test_sampled_direction_mu_is_in_range", test_sampled_direction_mu_is_in_range},
    {"test_hg_handles_asymmetry_endpoints", test_hg_handles_asymmetry_endpoints},
};

int main(void) {
    return run_test_suite("hg", test_cases, sizeof(test_cases) / sizeof(test_cases[0]));
}
