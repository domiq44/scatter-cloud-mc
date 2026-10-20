#include "basis.h"
#include "vec3.h"
#include <math.h>

void orthonormal_basis(Vec3 dir, Vec3 *u, Vec3 *v, Vec3 *w) {
    *w = vec3_normalize(dir);

    Vec3 tmp;
    if (fabs(w->z) < 0.9) {
        tmp = vec3_new(0.0, 0.0, 1.0);
    } else {
        tmp = vec3_new(0.0, 1.0, 0.0);
    }

    *u = vec3_normalize(vec3_cross(tmp, *w));
    *v = vec3_cross(*w, *u);
}
