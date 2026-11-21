/*
 * test_assertions.c - Fonction d'assertion unique
 * assert_condition(condition, description) : retourne TEST_PASSED/TEST_FAILED.
 * Affiche description sur stderr si échec.
 *
 * Partie de scatter-cloud-mc
 */

#include "test_assertions.h"

#include <stdio.h>

TestResult assert_condition(int condition, const char *description) {
    if (!condition) {
        fprintf(stderr, "Assertion failed: %s\n", description);
        return TEST_FAILED;
    }
    return TEST_PASSED;
}