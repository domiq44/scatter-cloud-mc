/*
 * vec3.h - Vecteurs 3D pour directions, positions, bases
 * Vec3 struct : x, y, z (3 doubles = 24 bytes, _Static_assert).
 * Opérations : new, add, scale, dot, cross, norm, normalize.
 * Utilisé pour directions, positions, bases orthonormales.
 *
 * Partie de scatter-cloud-mc
 */

#ifndef VEC3_H
#define VEC3_H

#include <assert.h>

typedef struct {
    double x, y, z;
} Vec3;

/*
 * _Static_assert vérifie la taille à la compilation pour:
 * - Détecter les changements d'ABI accidentels (ajout/suppression de champs)
 * - Valider l'alignement/padding attendu (3 doubles = 24 octets sans padding)
 * - Empêcher les bugs silencieux lors de sérialisation, MPI, ou partage mémoire
 * Échoue à la compilation si la taille change (ex: compilateur différent, flags différents)
 */
_Static_assert(sizeof(Vec3) == 24, "Vec3 size mismatch: expected 3 doubles (24 bytes)");

Vec3 vec3_new(double x, double y, double z);
Vec3 vec3_add(Vec3 a, Vec3 b);
Vec3 vec3_scale(Vec3 v, double s);
double vec3_dot(Vec3 a, Vec3 b);
Vec3 vec3_cross(Vec3 a, Vec3 b);
double vec3_norm(Vec3 v);
Vec3 vec3_normalize(Vec3 v);

#endif
