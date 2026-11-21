/*
 * file_path.h - Utilitaires chemins et extensions
 * replace_extension() : change extension fichier, gère troncature buffer.
 *
 * Partie de scatter-cloud-mc
 */

#ifndef FILE_PATH_H
#define FILE_PATH_H

#include <stddef.h>

void replace_extension(const char *filename, const char *old_ext, const char *new_ext, char *out,
                       size_t out_size);

#endif
