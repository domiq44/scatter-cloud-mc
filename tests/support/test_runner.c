#include "test_runner.h"

#include <stdio.h>

int run_test_suite(const char *suite_name, const TestCase *test_cases, size_t test_count) {
    size_t passed_count = 0U;
    size_t failed_count = 0U;
    size_t skipped_count = 0U;
    size_t test_index;

    for (test_index = 0U; test_index < test_count; test_index++) {
        TestResult result = test_cases[test_index].run();

        report_test_result(test_cases[test_index].name, result);
        switch (result) {
        case TEST_PASSED:
            passed_count++;
            break;
        case TEST_SKIPPED:
            skipped_count++;
            break;
        default:
            failed_count++;
            break;
        }
    }

    printf("Résultat %s: %zu réussis, %zu échecs, %zu ignorés (%zu tests).\n", suite_name,
           passed_count, failed_count, skipped_count, test_count);
    return failed_count == 0U ? 0 : 1;
}