#include "test_assertions.h"
#include "test_runner.h"
#include "vec3.h"
#include <math.h>

/* Vérifie l'addition composante par composante de deux vecteurs. */
static TestResult test_vec3_add(void) {
    /* Given */
    Vec3 a = vec3_new(1, 2, 3);
    Vec3 b = vec3_new(4, 5, 6);

    /* When */
    Vec3 c = vec3_add(a, b);

    /* Then */
    return assert_condition(fabs(c.x - 5.0) < 1e-12 && fabs(c.y - 7.0) < 1e-12 &&
                                fabs(c.z - 9.0) < 1e-12,
                            "l'addition produit les trois composantes attendues");
}

/* Vérifie les produits scalaires orthogonal et non orthogonal. */
static TestResult test_vec3_dot(void) {
    /* Given */
    Vec3 a = vec3_new(1, 0, 0);
    Vec3 b = vec3_new(0, 1, 0);
    Vec3 c = vec3_new(2, 2, 2);
    const double expected = 2.0;

    /* When */
    double orthogonal_dot = vec3_dot(a, b);
    double general_dot = vec3_dot(a, c);

    /* Then */
    return assert_condition(fabs(orthogonal_dot) < 1e-12 && fabs(general_dot - expected) < 1e-12,
                            "les produits scalaires respectent les composantes des vecteurs");
}

/* Vérifie le produit vectoriel des axes X et Y. */
static TestResult test_vec3_cross(void) {
    /* Given */
    Vec3 a = vec3_new(1, 0, 0);
    Vec3 b = vec3_new(0, 1, 0);

    /* When */
    Vec3 c = vec3_cross(a, b);

    /* Then */
    return assert_condition(fabs(c.x) < 1e-12 && fabs(c.y) < 1e-12 && fabs(c.z - 1.0) < 1e-12,
                            "le produit vectoriel de X par Y vaut Z");
}

/* Vérifie la normalisation des vecteurs simples et généraux. */
static TestResult test_vec3_normalize(void) {
    /* Given */
    Vec3 v1 = vec3_new(3, 0, 0);
    Vec3 v2 = vec3_new(1, 1, 1);
    const double norm_check = sqrt(3.0);

    /* When */
    Vec3 n1 = vec3_normalize(v1);
    Vec3 n2 = vec3_normalize(v2);

    /* Then */
    return assert_condition(fabs(n1.x - 1.0) < 1e-12 && fabs(n1.y) < 1e-12 &&
                                fabs(n2.x - 1.0 / norm_check) < 1e-12 &&
                                fabs(vec3_norm(n2) - 1.0) < 1e-12,
                            "les vecteurs normalisés ont une norme unitaire");
}

/* Vérifie que l'incidence nulle pointe vers l'axe Z négatif. */
static TestResult test_initial_dir_zero(void) {
    /* Given */
    const double theta = 0.0;

    /* When */
    Vec3 direction = vec3_normalize(vec3_new(sin(theta), 0.0, -cos(theta)));

    /* Then */
    return assert_condition(fabs(direction.x) < 1e-12 && fabs(direction.y) < 1e-12 &&
                                fabs(direction.z + 1.0) < 1e-12,
                            "la direction initiale à zéro degré vaut -Z");
}

/* Vérifie la direction initiale construite à trente degrés. */
static TestResult test_initial_dir_30deg(void) {
    /* Given */
    const double theta = acos(-1.0) / 6.0;

    /* When */
    Vec3 direction = vec3_normalize(vec3_new(sin(theta), 0.0, -cos(theta)));

    /* Then */
    return assert_condition(fabs(direction.x - 0.5) < 1e-12 &&
                                fabs(direction.z + sqrt(3.0) / 2.0) < 1e-12,
                            "la direction à trente degrés a les composantes attendues");
}

static const TestCase test_cases[] = {
    {"test_vec3_add", test_vec3_add},
    {"test_vec3_dot", test_vec3_dot},
    {"test_vec3_cross", test_vec3_cross},
    {"test_vec3_normalize", test_vec3_normalize},
    {"test_initial_dir_zero", test_initial_dir_zero},
    {"test_initial_dir_30deg", test_initial_dir_30deg},
};

int main(void) {
    return run_test_suite("vec3", test_cases, sizeof(test_cases) / sizeof(test_cases[0]));
}
