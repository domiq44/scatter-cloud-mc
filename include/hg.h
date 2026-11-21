/*
 * hg.h - Interface échantillonnage Henyey-Greenstein
 * sample_henyey_greenstein() : simple HG, g ∈ [-1,1], stable double précision.
 * sample_double_henyey_greenstein() : mélange deux lobes g1, g2, fraction p.
 * Gère g=±1 (limite delta) et g=0 (isotrope exact).
 *
 * Partie de scatter-cloud-mc
 */

#ifndef HG_H
#define HG_H

#include "rng.h"
#include "vec3.h"

Vec3 sample_henyey_greenstein(Rng *rng, double g, Vec3 dir);
Vec3 sample_double_henyey_greenstein(Rng *rng, double g1, double g2, double hg_fraction, Vec3 dir);

#endif
