#include "plot_csv.h"
#include "test_assertions.h"
#include "test_runner.h"
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int write_temp_csv(char *path, size_t path_size, const char *content) {
    char template[] = "/tmp/scatter-sphere-csv.XXXXXX";
    int descriptor = mkstemp(template);
    if (descriptor < 0)
        return 0;

    FILE *file = fdopen(descriptor, "w");
    if (!file) {
        close(descriptor);
        remove(template);
        return 0;
    }

    int write_succeeded = fputs(content, file) >= 0;
    int close_succeeded = fclose(file) == 0;
    if (!write_succeeded || !close_succeeded || strlen(template) + 1 > path_size) {
        remove(template);
        return 0;
    }

    memcpy(path, template, strlen(template) + 1);
    return 1;
}

/* Vérifie la lecture d'un CSV valide. */
static TestResult test_read_csv_valid(void) {
    /* Given */
    char filename[64];
    const char *content = "alpha_d,mu,pdf\n"
                          "0,0.1,1.0\n"
                          "0,0.2,2.0\n"
                          "0,0.3,3.0\n";
    if (!write_temp_csv(filename, sizeof(filename), content))
        return assert_condition(0, "le fichier CSV temporaire est créé");

    Data d;
    /* When */
    int loaded = read_csv(filename, &d);

    /* Then */
    int data_matches =
        loaded && d.len == 3 && fabs(d.mu[1] - 0.2) < 1e-12 && fabs(d.pdf[1] - 2.0) < 1e-12;
    if (loaded)
        data_free(&d);
    remove(filename);
    return assert_condition(data_matches, "les colonnes mu et pdf sont lues correctement");
}

/* Vérifie qu'un fichier CSV absent est signalé comme erreur. */
static TestResult test_read_csv_missing(void) {
    /* Given */
    char filename[64];
    if (!write_temp_csv(filename, sizeof(filename), ""))
        return assert_condition(0, "le chemin temporaire est créé");
    if (remove(filename) != 0)
        return assert_condition(0, "le fichier temporaire est supprimé avant lecture");
    Data d;

    /* When */
    int loaded = read_csv(filename, &d);

    /* Then */
    return assert_condition(!loaded, "un fichier CSV absent est refusé");
}

/* Vérifie que le lecteur accepte un CSV de tracé valide. */
static TestResult test_csv_fixture_is_readable(void) {
    /* Given */
    char filename[64];
    const char *content = "alpha_d,mu,pdf\n0,30.0,1000000\n";
    if (!write_temp_csv(filename, sizeof(filename), content))
        return assert_condition(0, "le fichier CSV temporaire est créé");

    /* When */
    Data data;
    int loaded = read_csv(filename, &data);

    /* Then */
    int data_matches = loaded && data.len == 1 && fabs(data.mu[0] - 30.0) < 1e-12;
    if (loaded)
        data_free(&data);
    remove(filename);
    return assert_condition(data_matches, "la valeur mu du CSV est lue correctement");
}

/* Vérifie que les lignes incomplètes sont refusées. */
static TestResult test_read_csv_rejects_incomplete_rows(void) {
    /* Given */
    const char *invalid_contents[] = {
        "alpha_d,mu,pdf\n0\n",
        "alpha_d,mu,pdf\n0,0.5\n",
    };
    int every_incomplete_row_was_rejected = 1;

    /* When */
    for (size_t i = 0; i < sizeof(invalid_contents) / sizeof(invalid_contents[0]); i++) {
        char filename[64];
        if (!write_temp_csv(filename, sizeof(filename), invalid_contents[i])) {
            every_incomplete_row_was_rejected = 0;
            break;
        }

        Data data;
        every_incomplete_row_was_rejected &= !read_csv(filename, &data);
        remove(filename);
    }

    /* Then */
    return assert_condition(every_incomplete_row_was_rejected,
                            "les lignes sans les trois colonnes attendues sont refusées");
}

/* Vérifie que les lignes vides et les espaces autour des nombres sont acceptés. */
static TestResult test_read_csv_skips_blank_lines_and_trims_numbers(void) {
    /* Given */
    char filename[64];
    const char *content = "alpha_d,mu,pdf\n\n 0 , 0.5 , 1.0 \t\n";
    if (!write_temp_csv(filename, sizeof(filename), content))
        return assert_condition(0, "le fichier CSV temporaire est créé");
    Data data;

    /* When */
    int loaded = read_csv(filename, &data);

    /* Then */
    int values_match = loaded && data.len == 1 && fabs(data.mu[0] - 0.5) < 1e-12 &&
                       fabs(data.pdf[0] - 1.0) < 1e-12;
    if (loaded)
        data_free(&data);
    remove(filename);
    return assert_condition(values_match,
                            "les espaces périphériques sont ignorés sans garder les lignes vides");
}

/* Vérifie que les erreurs de lecture libèrent le conteneur de données. */
static TestResult test_read_csv_rejects_read_errors(void) {
    /* Given */
    char directory[] = "/tmp/scatter-sphere-csv-dir.XXXXXX";
    if (!mkdtemp(directory))
        return assert_condition(0, "le répertoire temporaire est créé");
    Data data;

    /* When */
    int loaded = read_csv(directory, &data);

    /* Then */
    int data_was_released = !loaded && data.mu == NULL && data.pdf == NULL && data.len == 0;
    rmdir(directory);
    return assert_condition(data_was_released, "une erreur de lecture est signalée et nettoyée");
}

/* Vérifie que les nombres mal formés ou non finis sont rejetés. */
static TestResult test_read_csv_rejects_invalid_numeric_fields(void) {
    /* Given */
    const char *invalid_contents[] = {
        "alpha_d,mu,pdf\n0,not-a-number,1\n",
        "alpha_d,mu,pdf\n0,0.5,1trailing\n",
        "alpha_d,mu,pdf\n0,nan,1\n",
        "alpha_d,mu,pdf\n0,0.5,inf\n",
        "alpha_d,mu,pdf\n0,,1\n",
    };
    int every_invalid_file_was_rejected = 1;

    /* When */
    for (size_t i = 0; i < sizeof(invalid_contents) / sizeof(invalid_contents[0]); i++) {
        char filename[64];
        if (!write_temp_csv(filename, sizeof(filename), invalid_contents[i])) {
            every_invalid_file_was_rejected = 0;
            break;
        }

        Data data;
        int loaded = read_csv(filename, &data);
        every_invalid_file_was_rejected &= !loaded;
        remove(filename);
    }

    /* Then */
    return assert_condition(every_invalid_file_was_rejected,
                            "les champs numériques invalides sont refusés sans être tronqués");
}

static const TestCase test_cases[] = {
    {"test_read_csv_valid", test_read_csv_valid},
    {"test_read_csv_missing", test_read_csv_missing},
    {"test_csv_fixture_is_readable", test_csv_fixture_is_readable},
    {"test_read_csv_rejects_incomplete_rows", test_read_csv_rejects_incomplete_rows},
    {"test_read_csv_skips_blank_lines_and_trims_numbers",
     test_read_csv_skips_blank_lines_and_trims_numbers},
    {"test_read_csv_rejects_read_errors", test_read_csv_rejects_read_errors},
    {"test_read_csv_rejects_invalid_numeric_fields", test_read_csv_rejects_invalid_numeric_fields},
};

int main(void) {
    return run_test_suite("plot_csv", test_cases, sizeof(test_cases) / sizeof(test_cases[0]));
}
