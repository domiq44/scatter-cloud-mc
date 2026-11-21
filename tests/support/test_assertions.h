/*
 * test_assertions.h - Interface assertion unique
 * assert_condition(condition, description) : TEST_PASSED/TEST_FAILED.
 *
 * Partie de scatter-cloud-mc
 */

#ifndef TEST_ASSERTIONS_H
#define TEST_ASSERTIONS_H

#include "test_reporter.h"

TestResult assert_condition(int condition, const char *description);

#endif