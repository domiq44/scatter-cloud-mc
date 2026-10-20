#ifndef HG_H
#define HG_H

#include "rng.h"
#include "vec3.h"

Vec3 sample_henyey_greenstein(Rng *rng, double g, Vec3 dir);
Vec3 sample_double_henyey_greenstein(Rng *rng, double g1, double g2, double hg_fraction, Vec3 dir);

#endif
