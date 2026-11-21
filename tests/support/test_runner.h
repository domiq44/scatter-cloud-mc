/*
 * test_runner.h - Runner suite de tests
 * TestFunction : fonction test sans paramètre retournant TestResult.
 * TestCase : nom + fonction.
 * run_test_suite() : exécute suite, rapporte PASS/FAIL/SKIP, continue après échec.
 *
 * Partie de scatter-cloud-mc
 */

#ifndef TEST_RUNNER_H
#define TEST_RUNNER_H

#include "test_reporter.h"

#include <stddef.h>

typedef TestResult (*TestFunction)(void);

typedef struct {
    const char *name;
    TestFunction run;
} TestCase;

int run_test_suite(const char *suite_name, const TestCase *test_cases, size_t test_count);

#endif