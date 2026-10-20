#include "test_reporter.h"

#include <stdio.h>

void report_test_result(const char *test_name, TestResult result) {
    const char *status;

    switch (result) {
    case TEST_PASSED:
        status = "PASS";
        break;
    case TEST_SKIPPED:
        status = "SKIP";
        break;
    default:
        status = "FAIL";
        break;
    }

    printf("[%s] %s\n", status, test_name);
}