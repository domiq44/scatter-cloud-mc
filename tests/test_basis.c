#include "basis.h"
#include "test_assertions.h"
#include "test_runner.h"
#include "vec3.h"
#include <math.h>
#include <stdio.h>

// ===================================================================
// Fonctions utilitaires de test
// ===================================================================

/* Vérifie que la base construite pour une direction générale est orthonormale. */
static TestResult test_basis_orthonormal(void) {
    /* Given */
    Vec3 dir = vec3_normalize(vec3_new(0.3, 0.4, -0.866));

    /* When */
    Vec3 u, v, w;
    orthonormal_basis(dir, &u, &v, &w);

    /* Then */
    return assert_condition(fabs(vec3_dot(u, v)) < 1e-6 && fabs(vec3_dot(u, w)) < 1e-6 &&
                                fabs(vec3_dot(v, w)) < 1e-6 && fabs(vec3_norm(u) - 1.0) < 1e-6 &&
                                fabs(vec3_norm(v) - 1.0) < 1e-6 && fabs(vec3_norm(w) - 1.0) < 1e-6,
                            "la base générale est orthonormale");
}

/* Vérifie que la construction reste normalisée sur l'axe Z. */
static TestResult test_basis_special_case(void) {
    /* Given */
    Vec3 dir = vec3_new(0, 0, 1);

    /* When */
    Vec3 u, v, w;
    orthonormal_basis(dir, &u, &v, &w);

    /* Then */
    return assert_condition(fabs(vec3_norm(u) - 1.0) < 1e-6 && fabs(vec3_norm(v) - 1.0) < 1e-6 &&
                                fabs(vec3_norm(w) - 1.0) < 1e-6,
                            "la base sur l'axe Z est normalisée");
}

static const TestCase test_cases[] = {
    {"test_basis_orthonormal", test_basis_orthonormal},
    {"test_basis_special_case", test_basis_special_case},
};

int main(void) {
    return run_test_suite("basis", test_cases, sizeof(test_cases) / sizeof(test_cases[0]));
}
