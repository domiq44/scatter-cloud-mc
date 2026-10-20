#ifndef VEC3_H
#define VEC3_H

typedef struct {
    double x, y, z;
} Vec3;

Vec3 vec3_new(double x, double y, double z);
Vec3 vec3_add(Vec3 a, Vec3 b);
Vec3 vec3_scale(Vec3 v, double s);
double vec3_dot(Vec3 a, Vec3 b);
Vec3 vec3_cross(Vec3 a, Vec3 b);
double vec3_norm(Vec3 v);
Vec3 vec3_normalize(Vec3 v);

#endif
