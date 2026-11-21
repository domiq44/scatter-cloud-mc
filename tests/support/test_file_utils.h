/*
 * test_file_utils.h - Interface fichiers temporaires tests
 * create_temporary_file() : crée /tmp fichier avec contenu.
 * remove_temporary_file() : nettoyage.
 *
 * Partie de scatter-cloud-mc
 */

#ifndef TEST_FILE_UTILS_H
#define TEST_FILE_UTILS_H

#include <stddef.h>

int create_temporary_file(char *path, size_t path_size, const char *contents);
void remove_temporary_file(const char *path);

#endif