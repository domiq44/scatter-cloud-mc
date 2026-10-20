#include "vec3.h"
#include <math.h>

Vec3 vec3_new(double x, double y, double z) {
    Vec3 v = {x, y, z};
    return v;
}

Vec3 vec3_add(Vec3 a, Vec3 b) { return vec3_new(a.x + b.x, a.y + b.y, a.z + b.z); }

Vec3 vec3_scale(Vec3 v, double s) { return vec3_new(v.x * s, v.y * s, v.z * s); }

double vec3_dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

Vec3 vec3_cross(Vec3 a, Vec3 b) {
    return vec3_new(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

double vec3_norm(Vec3 v) { return sqrt(vec3_dot(v, v)); }

Vec3 vec3_normalize(Vec3 v) {
    double n = vec3_norm(v);
    if (n == 0.0)
        return v;
    return vec3_scale(v, 1.0 / n);
}
