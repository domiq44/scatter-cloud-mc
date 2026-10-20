#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "rng.h"
#include "vec3.h"

typedef enum {
    GEOMETRY_SPHERE,
    GEOMETRY_CUBE,
} GeometryType;

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
