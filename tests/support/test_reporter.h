/*
 * test_reporter.h - Enum résultats et affichage tests
 * TestResult enum : SKIPPED=-1, FAILED=0, PASSED=1.
 * report_test_result() : affiche [PASS/FAIL/SKIP] nom_test.
 *
 * Partie de scatter-cloud-mc
 */

#ifndef TEST_REPORTER_H
#define TEST_REPORTER_H

typedef enum {
    TEST_SKIPPED = -1,
    TEST_FAILED = 0,
    TEST_PASSED = 1,
} TestResult;

void report_test_result(const char *test_name, TestResult result);

#endif