#include "file_path.h"
#include "test_assertions.h"
#include "test_runner.h"
#include <string.h>

/* Vérifie le remplacement standard de l'extension CSV. */
static TestResult test_replace_extension_basic(void) {
    /* Given */
    char out[256];
    /* When */
    replace_extension("data.csv", ".csv", ".png", out, sizeof(out));

    /* Then */
    return assert_condition(strcmp(out, "data.png") == 0, "l'extension CSV devient PNG");
}

/* Vérifie l'ajout de l'extension lorsque le chemin n'en a pas. */
static TestResult test_replace_extension_no_ext(void) {
    /* Given */
    char out[256];
    /* When */
    replace_extension("result", ".csv", ".png", out, sizeof(out));

    /* Then */
    return assert_condition(strcmp(out, "result.png") == 0, "l'extension PNG est ajoutée");
}

/* Vérifie qu'une extension différente est conservée avant .png. */
static TestResult test_replace_extension_wrong_ext(void) {
    /* Given */
    char out[256];
    /* When */
    replace_extension("image.txt", ".csv", ".png", out, sizeof(out));

    /* Then */
    return assert_condition(strcmp(out, "image.txt.png") == 0,
                            "une extension non reconnue est conservée");
}

/* Vérifie que la sortie reste terminée par zéro avec un petit tampon. */
static TestResult test_replace_extension_small_buffer(void) {
    /* Given */
    char out[8];
    /* When */
    replace_extension("verylongfilename.csv", ".csv", ".png", out, sizeof(out));

    /* Then */
    return assert_condition(out[sizeof(out) - 1] == '\0',
                            "le tampon tronqué reste terminé par zéro");
}

/* Vérifie qu'une capacité nulle ne modifie pas le tampon. */
static TestResult test_replace_extension_zero_capacity(void) {
    /* Given */
    char out = 'x';

    /* When */
    replace_extension("data.csv", ".csv", ".png", &out, 0);

    /* Then */
    return assert_condition(out == 'x', "un tampon de capacité nulle reste intact");
}

static const TestCase test_cases[] = {
    {"test_replace_extension_basic", test_replace_extension_basic},
    {"test_replace_extension_no_ext", test_replace_extension_no_ext},
    {"test_replace_extension_wrong_ext", test_replace_extension_wrong_ext},
    {"test_replace_extension_small_buffer", test_replace_extension_small_buffer},
    {"test_replace_extension_zero_capacity", test_replace_extension_zero_capacity},
};

int main(void) {
    return run_test_suite("file_path", test_cases, sizeof(test_cases) / sizeof(test_cases[0]));
}
