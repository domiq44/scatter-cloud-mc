#include "geometry.h"
#include <math.h>

Vec3 geometry_incident_direction(double theta_rad) {
    return vec3_new(sin(theta_rad), 0.0, -cos(theta_rad));
}

double geometry_distance_to_boundary(GeometryType geometry, Vec3 pos, Vec3 dir, double size) {
    if (geometry == GEOMETRY_CUBE)
        return distance_to_cube(pos, dir, size);
    return distance_to_sphere(pos, dir, size);
}

double geometry_mean_chord_length(GeometryType geometry, double size, Vec3 direction) {
    if (geometry == GEOMETRY_SPHERE)
        return 4.0 * size / 3.0;
    if (geometry != GEOMETRY_CUBE)
        return -1.0;

    double direction_norm = vec3_norm(direction);
    if (direction_norm == 0.0)
        return -1.0;

    double projected_area_factor =
        (fabs(direction.x) + fabs(direction.y) + fabs(direction.z)) / direction_norm;
    return 2.0 * size / projected_area_factor;
}

Vec3 geometry_entry_position(GeometryType geometry, double size, double theta_rad, Vec3 direction,
                             Rng *rng) {
    if (geometry == GEOMETRY_CUBE) {
        return cube_entry_position(size, rng_next_f64(rng), rng_next_f64(rng), rng_next_f64(rng),
                                   direction);
    }

    double rho = disk_radius_from_uniform(size, rng_next_f64(rng));
    double phi = 2.0 * acos(-1.0) * rng_next_f64(rng);
    return sphere_entry_position(size, rho * cos(phi), rho * sin(phi), theta_rad);
}

double disk_radius_from_uniform(double radius, double uniform_sample) {
    return radius * sqrt(uniform_sample);
}

Vec3 sphere_entry_position(double radius, double impact_x, double impact_y, double theta_rad) {
    double half_chord =
        sqrt(fmax(0.0, radius * radius - impact_x * impact_x - impact_y * impact_y));
    Vec3 impact = vec3_new(impact_x * cos(theta_rad), impact_y, impact_x * sin(theta_rad));
    Vec3 direction = geometry_incident_direction(theta_rad);
    return vec3_add(impact, vec3_scale(direction, -half_chord));
}

double distance_to_sphere(Vec3 pos, Vec3 dir, double radius) {
    double b = vec3_dot(pos, dir);
    double c = vec3_dot(pos, pos) - radius * radius;

    double disc = b * b - c;
    if (disc < 0.0) {
        return -1.0;
    }

    return -b + sqrt(disc);
}

double distance_to_cube(Vec3 pos, Vec3 dir, double half_side) {
    double near_distance = -INFINITY;
    double far_distance = INFINITY;
    const double positions[] = {pos.x, pos.y, pos.z};
    const double directions[] = {dir.x, dir.y, dir.z};

    for (int axis = 0; axis < 3; axis++) {
        if (directions[axis] == 0.0) {
            if (positions[axis] < -half_side || positions[axis] > half_side)
                return -1.0;
            continue;
        }

        double first = (-half_side - positions[axis]) / directions[axis];
        double second = (half_side - positions[axis]) / directions[axis];
        double axis_near = fmin(first, second);
        double axis_far = fmax(first, second);
        near_distance = fmax(near_distance, axis_near);
        far_distance = fmin(far_distance, axis_far);
        if (near_distance > far_distance)
            return -1.0;
    }

    return far_distance >= 0.0 ? far_distance : -1.0;
}

Vec3 cube_entry_position(double half_side, double face_sample, double first_coordinate_sample,
                         double second_coordinate_sample, Vec3 direction) {
    double absolute_x = fabs(direction.x);
    double absolute_y = fabs(direction.y);
    double total_projected_weight = absolute_x + absolute_y + fabs(direction.z);
    double selected_face = face_sample * total_projected_weight;
    double first_coordinate = (2.0 * first_coordinate_sample - 1.0) * half_side;
    double second_coordinate = (2.0 * second_coordinate_sample - 1.0) * half_side;

    if (selected_face < absolute_x) {
        return vec3_new(direction.x > 0.0 ? -half_side : half_side, first_coordinate,
                        second_coordinate);
    }
    if (selected_face < absolute_x + absolute_y) {
        return vec3_new(first_coordinate, direction.y > 0.0 ? -half_side : half_side,
                        second_coordinate);
    }
    return vec3_new(first_coordinate, second_coordinate,
                    direction.z > 0.0 ? -half_side : half_side);
}
