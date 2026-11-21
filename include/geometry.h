/*
 * geometry.h - Géométries sphère et cube : distances, entrées, cordes
 * GeometryType enum : GEOMETRY_SPHERE, GEOMETRY_CUBE.
 * Fonctions : distance_to_sphere/cube, mean_chord_length, incident_direction,
 * entry_position (uniform projection), disk_radius_from_uniform.
 * _Static_assert taille GeometryType = 4 bytes (int).
 *
 * Partie de scatter-cloud-mc
 */

#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "rng.h"
#include "vec3.h"
#include <assert.h>

typedef enum {
    GEOMETRY_SPHERE,
    GEOMETRY_CUBE,
} GeometryType;

/*
 * _Static_assert vérifie la taille de l'enum à la compilation pour:
 * - Garantir que l'enum utilise 4 octets (int standard) sur toutes les plateformes
 * - Détecter si le compilateur utilise une taille différente (ex: short enum)
 * - Assurer la compatibilité binaire (fichiers de config, MPI, sérialisation)
 * Échoue à la compilation si la taille change
 */
_Static_assert(sizeof(GeometryType) == 4, "GeometryType size mismatch: expected 4 bytes (int)");

double geometry_distance_to_boundary(GeometryType geometry, Vec3 pos, Vec3 dir, double size);
double geometry_mean_chord_length(GeometryType geometry, double size, Vec3 direction);
Vec3 geometry_incident_direction(double theta_rad);
Vec3 geometry_entry_position(GeometryType geometry, double size, double theta_rad, Vec3 direction,
                             Rng *rng);
double distance_to_sphere(Vec3 pos, Vec3 dir, double radius);
double distance_to_cube(Vec3 pos, Vec3 dir, double half_side);
double disk_radius_from_uniform(double radius, double uniform_sample);
Vec3 sphere_entry_position(double radius, double impact_x, double impact_y, double theta_rad);
Vec3 cube_entry_position(double half_side, double face_sample, double first_coordinate_sample,
                         double second_coordinate_sample, Vec3 direction);

#endif
