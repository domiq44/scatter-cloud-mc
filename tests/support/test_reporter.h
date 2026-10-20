#ifndef TEST_REPORTER_H
#define TEST_REPORTER_H

typedef enum {
    TEST_SKIPPED = -1,
    TEST_FAILED = 0,
    TEST_PASSED = 1,
} TestResult;

void report_test_result(const char *test_name, TestResult result);

#endif