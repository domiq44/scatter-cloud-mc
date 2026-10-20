#include "test_assertions.h"

#include <stdio.h>

TestResult assert_condition(int condition, const char *description) {
    if (!condition) {
        fprintf(stderr, "Assertion failed: %s\n", description);
        return TEST_FAILED;
    }
    return TEST_PASSED;
}