#include "geometry.h"
#include "hg.h"
#include "rng.h"
#include "test_assertions.h"
#include "test_runner.h"
#include "vec3.h"
#include <math.h>
#include <time.h>

/* Simule un photon et indique s'il sort par le côté z positif. */
static int simulate_exit_top(Rng *rng) {
    const double sigma_e = 1.0;
    const double g = 0.0;
    const double radius = 1.0;
    Vec3 pos = vec3_new(0, 0, 0);
    Vec3 dir = vec3_new(0, 0, -1);

    while (1) {
        double xi = rng_next_f64(rng);
        double free_path = -log(1.0 - xi) / sigma_e;
        double d_border = distance_to_sphere(pos, dir, radius);
        if (free_path >= d_border) {
            Vec3 exit_pos = vec3_add(pos, vec3_scale(dir, d_border));
            return exit_pos.z > 0 ? 1 : 0;
        }
        pos = vec3_add(pos, vec3_scale(dir, free_path));
        dir = sample_henyey_greenstein(rng, g, dir);
    }
}

/* Simule un photon et renvoie la profondeur de son premier événement. */
static double simulate_first_depth(Rng *rng) {
    const double sigma_e = 50.0;
    const double radius = 1.0;
    Vec3 pos = vec3_new(0, 0, 0);
    Vec3 dir = vec3_new(0, 0, -1);
    double xi = rng_next_f64(rng);
    double free_path = -log(1.0 - xi) / sigma_e;
    double d_border = distance_to_sphere(pos, dir, radius);
    return free_path >= d_border ? d_border : free_path;
}

/* Simule un photon et renvoie le cosinus de son angle de sortie. */
static double simulate_exit_mu(Rng *rng) {
    const double sigma_e = 1.0;
    const double g = 0.0;
    const double radius = 1.0;
    Vec3 pos = vec3_new(0, 0, 0);
    Vec3 dir = vec3_new(0, 0, -1);

    while (1) {
        double xi = rng_next_f64(rng);
        double free_path = -log(1.0 - xi) / sigma_e;
        double d_border = distance_to_sphere(pos, dir, radius);
        if (free_path >= d_border)
            return dir.z;
        pos = vec3_add(pos, vec3_scale(dir, free_path));
        dir = sample_henyey_greenstein(rng, g, dir);
    }
}

/* Simule un photon et renvoie le rayon de sa position de sortie. */
static double simulate_exit_radius(Rng *rng) {
    const double sigma_e = 1.0;
    const double g = 0.0;
    const double radius = 1.0;
    Vec3 pos = vec3_new(0, 0, 0);
    Vec3 dir = vec3_new(0, 0, -1);

    while (1) {
        double xi = rng_next_f64(rng);
        double free_path = -log(1.0 - xi) / sigma_e;
        double d_border = distance_to_sphere(pos, dir, radius);
        if (free_path >= d_border) {
            Vec3 exit_pos = vec3_add(pos, vec3_scale(dir, d_border));
            return sqrt(exit_pos.x * exit_pos.x + exit_pos.y * exit_pos.y);
        }
        pos = vec3_add(pos, vec3_scale(dir, free_path));
        dir = sample_henyey_greenstein(rng, g, dir);
    }
}

/* Calcule la somme des écarts absolus entre deux histogrammes. */
static double histogram_distance(const double *first, const double *second, int bins) {
    double sum = 0.0;
    for (int i = 0; i < bins; i++)
        sum += fabs(first[i] - second[i]);
    return sum;
}

/* Simule un photon et renvoie son nombre d'interactions. */
static int simulate_interaction_count(Rng *rng) {
    const double sigma_e = 1.0;
    const double g = 0.0;
    const double radius = 1.0;
    Vec3 pos = vec3_new(0, 0, 0);
    Vec3 dir = vec3_new(0, 0, -1);
    int count = 0;

    while (1) {
        double xi = rng_next_f64(rng);
        double free_path = -log(1.0 - xi) / sigma_e;
        double d_border = distance_to_sphere(pos, dir, radius);
        if (free_path >= d_border)
            return count;
        count++;
        pos = vec3_add(pos, vec3_scale(dir, free_path));
        dir = sample_henyey_greenstein(rng, g, dir);
    }
}

/* Simule un photon et renvoie sa longueur totale de trajet. */
static double simulate_path_length(Rng *rng) {
    const double sigma_e = 1.0;
    const double g = 0.0;
    const double radius = 1.0;
    Vec3 pos = vec3_new(0, 0, 0);
    Vec3 dir = vec3_new(0, 0, -1);
    double total = 0.0;

    while (1) {
        double xi = rng_next_f64(rng);
        double free_path = -log(1.0 - xi) / sigma_e;
        double d_border = distance_to_sphere(pos, dir, radius);
        if (free_path >= d_border)
            return total + d_border;
        total += free_path;
        pos = vec3_add(pos, vec3_scale(dir, free_path));
        dir = sample_henyey_greenstein(rng, g, dir);
    }
}

/* Simule un photon jusqu'à sa sortie de la sphère. */
static void simulate_one(Rng *rng) {
    const double sigma_e = 1.0;
    const double g = 0.0;
    const double radius = 1.0;
    Vec3 pos = vec3_new(0, 0, 0);
    Vec3 dir = vec3_new(0, 0, -1);

    while (1) {
        double xi = rng_next_f64(rng);
        double free_path = -log(1.0 - xi) / sigma_e;
        double d_border = distance_to_sphere(pos, dir, radius);
        if (free_path >= d_border)
            return;
        pos = vec3_add(pos, vec3_scale(dir, free_path));
        dir = sample_henyey_greenstein(rng, g, dir);
    }
}

/* Vérifie la distance de sortie depuis le centre de la sphère. */
static TestResult test_dts_center(void) {
    /* Given */
    Vec3 pos = vec3_new(0, 0, 0);
    Vec3 dir = vec3_new(1, 0, 0);

    /* When */
    double distance = distance_to_sphere(pos, dir, 1.0);

    /* Then */
    return assert_condition(fabs(distance - 1.0) < 1e-12,
                            "la distance depuis le centre vaut le rayon");
}

/* Vérifie la distance depuis un point intérieur décentré. */
static TestResult test_dts_inside(void) {
    /* Given */
    Vec3 pos = vec3_new(0.5, 0, 0);
    Vec3 dir = vec3_new(1, 0, 0);

    /* When */
    double distance = distance_to_sphere(pos, dir, 1.0);

    /* Then */
    return assert_condition(fabs(distance - 0.5) < 1e-12,
                            "la distance intérieure correspond au bord");
}

/* Vérifie la distance le long d'une direction oblique. */
static TestResult test_dts_oblique(void) {
    /* Given */
    Vec3 pos = vec3_new(0, 0, 0);
    Vec3 dir = vec3_normalize(vec3_new(1, 1, 0));

    /* When */
    double distance = distance_to_sphere(pos, dir, 1.0);

    /* Then */
    return assert_condition(fabs(distance - 1.0) < 1e-12, "la distance oblique vaut le rayon");
}

/* Vérifie la distance lorsque le point est sur la surface. */
static TestResult test_dts_surface(void) {
    /* Given */
    Vec3 pos = vec3_new(1, 0, 0);
    Vec3 dir = vec3_new(1, 0, 0);

    /* When */
    double distance = distance_to_sphere(pos, dir, 1.0);

    /* Then */
    return assert_condition(fabs(distance) < 1e-12, "un point de surface sort immédiatement");
}

/* Vérifie l'intersection depuis l'extérieur de la sphère. */
static TestResult test_dts_outside_hit(void) {
    /* Given */
    Vec3 pos = vec3_new(2, 0, 0);
    Vec3 dir = vec3_new(-1, 0, 0);

    /* When */
    double distance = distance_to_sphere(pos, dir, 1.0);

    /* Then */
    return assert_condition(fabs(distance - 3.0) < 1e-12, "la première intersection est trouvée");
}

/* Vérifie le cas où la trajectoire ne rencontre pas la sphère. */
static TestResult test_dts_no_hit(void) {
    /* Given */
    Vec3 pos = vec3_new(2, 0, 0);
    Vec3 dir = vec3_new(1, 0, 0);

    /* When */
    double distance = distance_to_sphere(pos, dir, 1.0);

    /* Then */
    return assert_condition(fabs(distance + 1.0) < 1e-12,
                            "une trajectoire sans intersection vaut -1");
}

/* Vérifie que les distances calculées ne deviennent jamais NaN. */
static TestResult test_no_nan_distance(void) {
    /* Given */
    const double radius = 1.0;

    /* When */
    int distances_are_finite = 1;
    for (int i = 0; i < 10000; i++) {
        double angle = (double)i * 0.1;
        Vec3 pos = vec3_new(0.999 * cos(angle), 0.999 * sin(angle), 0.001 * (i % 10));
        Vec3 dir = vec3_normalize(vec3_new(cos(angle * 0.1), sin(angle * 0.1), 0.1));
        double distance = distance_to_sphere(pos, dir, radius);
        distances_are_finite &= !isnan(distance);
    }

    /* Then */
    return assert_condition(distances_are_finite, "aucune distance calculée n'est NaN");
}

/* Vérifie que la dispersion diminue quand la taille d'échantillon augmente. */
static TestResult test_convergence(void) {
    /* Given */
    const int replication_count = 20;
    const int small_photon_count = 1000;
    const int large_photon_count = 5000;
    double small_estimates[replication_count];
    double large_estimates[replication_count];

    /* When */
    for (int replication = 0; replication < replication_count; replication++) {
        Rng small_rng;
        Rng large_rng;
        rng_seed(&small_rng, 1234 + replication);
        rng_seed(&large_rng, 4321 + replication);

        int small_inside_count = 0;
        int large_inside_count = 0;
        for (int i = 0; i < small_photon_count; i++)
            small_inside_count += simulate_exit_top(&small_rng);
        for (int i = 0; i < large_photon_count; i++)
            large_inside_count += simulate_exit_top(&large_rng);

        small_estimates[replication] = (double)small_inside_count / small_photon_count;
        large_estimates[replication] = (double)large_inside_count / large_photon_count;
    }

    double small_mean = 0.0;
    double large_mean = 0.0;
    for (int replication = 0; replication < replication_count; replication++) {
        small_mean += small_estimates[replication];
        large_mean += large_estimates[replication];
    }
    small_mean /= replication_count;
    large_mean /= replication_count;

    double small_variance = 0.0;
    double large_variance = 0.0;
    for (int replication = 0; replication < replication_count; replication++) {
        double small_difference = small_estimates[replication] - small_mean;
        double large_difference = large_estimates[replication] - large_mean;
        small_variance += small_difference * small_difference;
        large_variance += large_difference * large_difference;
    }
    double variance_ratio = small_variance / large_variance;

    /* Then */
    return assert_condition(variance_ratio > 2.0 && variance_ratio < 10.0,
                            "la variance baisse avec un échantillon cinq fois plus grand");
}

/* Vérifie que la profondeur moyenne est proche du libre parcours moyen. */
static TestResult test_depth_mean(void) {
    /* Given */
    Rng rng;
    rng_seed(&rng, 123);
    const int photon_count = 20000;
    double sum_depth = 0.0;

    /* When */
    for (int i = 0; i < photon_count; i++)
        sum_depth += simulate_first_depth(&rng);
    double mean_depth = sum_depth / photon_count;

    /* Then */
    return assert_condition(mean_depth > 0.015 && mean_depth < 0.03,
                            "la profondeur moyenne est proche de 1/sigma_e");
}

/* Vérifie la fonction de répartition du premier événement. */
static TestResult test_depth_cdf(void) {
    /* Given */
    Rng rng;
    rng_seed(&rng, 456);
    const int photon_count = 20000;
    const double sigma_e = 50.0;
    const double test_depth = 0.5;
    int count_before_depth = 0;

    /* When */
    for (int i = 0; i < photon_count; i++)
        count_before_depth += simulate_first_depth(&rng) < test_depth;
    double empirical = (double)count_before_depth / photon_count;
    double theoretical = 1.0 - exp(-sigma_e * test_depth);

    /* Then */
    return assert_condition(fabs(empirical - theoretical) < 0.05,
                            "la fonction de répartition empirique respecte la tolérance");
}

/* Vérifie la tendance croissante du flux angulaire de Lambert. */
static TestResult test_flux_angular_lambert(void) {
    /* Given */
    enum { bins = 10 };
    double histogram[bins] = {0.0};
    Rng rng;
    rng_seed(&rng, 123);
    const int photon_count = 30000;

    /* When */
    for (int i = 0; i < photon_count; i++) {
        double mu = simulate_exit_mu(&rng);
        if (mu < 0.0)
            continue;
        int bin = (int)(mu * bins);
        if (bin >= bins)
            bin = bins - 1;
        histogram[bin] += 1.0 / photon_count;
    }
    int histogram_is_monotonic = 1;
    for (int i = 0; i < bins - 1; i++)
        histogram_is_monotonic &= histogram[i] < histogram[i + 1] + 0.01;

    /* Then */
    return assert_condition(histogram_is_monotonic,
                            "l'histogramme angulaire suit la tendance attendue");
}

/* Vérifie la proximité des histogrammes radiaux à deux tailles d'échantillon. */
static TestResult test_flux_radial_convergence(void) {
    /* Given */
    enum { bins = 10 };
    double small_histogram[bins] = {0.0};
    double large_histogram[bins] = {0.0};

    /* When */
    Rng small_rng;
    rng_seed(&small_rng, 111);
    const int small_count = 5000;
    for (int i = 0; i < small_count; i++) {
        int bin = (int)(simulate_exit_radius(&small_rng) * bins);
        if (bin >= bins)
            bin = bins - 1;
        small_histogram[bin] += 1.0 / small_count;
    }

    Rng large_rng;
    rng_seed(&large_rng, 222);
    const int large_count = 20000;
    for (int i = 0; i < large_count; i++) {
        int bin = (int)(simulate_exit_radius(&large_rng) * bins);
        if (bin >= bins)
            bin = bins - 1;
        large_histogram[bin] += 1.0 / large_count;
    }
    double distance = histogram_distance(small_histogram, large_histogram, bins);

    /* Then */
    return assert_condition(distance < 0.15, "les deux histogrammes radiaux restent proches");
}

/* Vérifie la probabilité d'absence d'interaction sur une longueur unité. */
static TestResult test_interaction_zero_probability(void) {
    /* Given */
    Rng rng;
    rng_seed(&rng, 123);
    const int photon_count = 20000;
    int zero_interactions = 0;

    /* When */
    for (int i = 0; i < photon_count; i++)
        zero_interactions += simulate_interaction_count(&rng) == 0;
    double empirical = (double)zero_interactions / photon_count;
    const double theoretical = exp(-1.0);

    /* Then */
    return assert_condition(fabs(empirical - theoretical) < 0.03,
                            "la probabilité sans interaction approche exp(-1)");
}

/* Vérifie que le nombre moyen d'interactions reste fini et plausible. */
static TestResult test_interaction_mean(void) {
    /* Given */
    Rng rng;
    rng_seed(&rng, 456);
    const int photon_count = 20000;
    double sum = 0.0;

    /* When */
    for (int i = 0; i < photon_count; i++)
        sum += simulate_interaction_count(&rng);
    double mean = sum / photon_count;

    /* Then */
    return assert_condition(mean > 0.4 && mean < 1.5,
                            "le nombre moyen d'interactions reste dans les bornes attendues");
}

/* Vérifie que la longueur moyenne suit la plage attendue. */
static TestResult test_path_length_mean(void) {
    /* Given */
    Rng rng;
    rng_seed(&rng, 123);
    const int photon_count = 20000;
    double sum = 0.0;

    /* When */
    for (int i = 0; i < photon_count; i++)
        sum += simulate_path_length(&rng);
    double mean = sum / photon_count;

    /* Then */
    return assert_condition(mean > 1.2 && mean < 1.8,
                            "la longueur moyenne reste dans la plage attendue");
}

/* Vérifie qu'aucune trajectoire ne dépasse la limite de longueur fixée. */
static TestResult test_path_length_no_outliers(void) {
    /* Given */
    Rng rng;
    rng_seed(&rng, 456);
    const int photon_count = 20000;
    int no_outliers = 1;

    /* When */
    for (int i = 0; i < photon_count; i++)
        no_outliers &= simulate_path_length(&rng) < 10.0;

    /* Then */
    return assert_condition(no_outliers, "aucun trajet ne dépasse dix unités");
}

/* Vérifie que vingt mille trajectoires s'exécutent sous le seuil prévu. */
static TestResult test_performance(void) {
    /* Given */
    const int photon_count = 20000;
    Rng rng;
    rng_seed(&rng, 123);

    /* When */
    clock_t start = clock();
    for (int i = 0; i < photon_count; i++)
        simulate_one(&rng);
    double elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;

    /* Then */
    return assert_condition(elapsed < 0.5, "la durée mesurée reste inférieure à 0,5 seconde");
}

/* Vérifie que le rayon échantillonné suit la loi uniforme en aire. */
static TestResult test_uniform_disk_radius(void) {
    /* Given */
    Rng rng;
    rng_seed(&rng, 321);
    const double disk_radius = 2.0;
    const int sample_count = 50000;
    double squared_radius_sum = 0.0;

    /* When */
    for (int i = 0; i < sample_count; i++) {
        double uniform_sample = rng_next_f64(&rng);
        double sampled_radius = disk_radius_from_uniform(disk_radius, uniform_sample);
        squared_radius_sum += sampled_radius * sampled_radius;
    }
    double mean_squared_radius = squared_radius_sum / sample_count;

    /* Then */
    return assert_condition(fabs(mean_squared_radius - disk_radius * disk_radius / 2.0) < 0.04,
                            "le rayon moyen quadratique correspond à un disque uniforme en aire");
}

/* Vérifie que l'entrée est sur la sphère et couvre la corde complète pour plusieurs incidences. */
static TestResult test_entry_position_spans_full_chord(void) {
    /* Given */
    const double radius = 1.0;
    const double impact_x = 0.3;
    const double impact_y = -0.2;
    const double angles[] = {0.0, acos(-1.0) / 4.0, acos(-1.0) / 2.0};
    int geometry_is_correct = 1;

    /* When */
    for (size_t i = 0; i < sizeof(angles) / sizeof(angles[0]); i++) {
        double theta = angles[i];
        Vec3 direction = vec3_new(sin(theta), 0.0, -cos(theta));
        Vec3 impact = vec3_new(impact_x * cos(theta), impact_y, impact_x * sin(theta));
        Vec3 entry = sphere_entry_position(radius, impact_x, impact_y, theta);
        double half_chord = sqrt(radius * radius - impact_x * impact_x - impact_y * impact_y);

        geometry_is_correct &= fabs(vec3_norm(entry) - radius) < 1e-12;
        geometry_is_correct &= fabs(vec3_dot(impact, direction)) < 1e-12;
        geometry_is_correct &=
            fabs(distance_to_sphere(entry, direction, radius) - 2.0 * half_chord) < 1e-12;
    }

    /* Then */
    return assert_condition(
        geometry_is_correct,
        "l'entrée est sur la surface et le faisceau traverse la corde complète");
}

/* Vérifie la direction incidente pour un angle mesuré depuis la verticale. */
static TestResult test_geometry_incident_direction(void) {
    /* Given */
    const double theta = acos(-1.0) / 6.0;
    const double expected_x = 0.5;
    const double expected_z = -sqrt(3.0) / 2.0;

    /* When */
    Vec3 direction = geometry_incident_direction(theta);

    /* Then */
    return assert_condition(fabs(direction.x - expected_x) < 1e-12 && fabs(direction.y) < 1e-12 &&
                                fabs(direction.z - expected_z) < 1e-12,
                            "la direction incidente suit la convention de l'angle theta");
}

/* Vérifie la distance de sortie du cube depuis son centre et en diagonale. */
static TestResult test_cube_distance_inside(void) {
    /* Given */
    const Vec3 center = vec3_new(0.0, 0.0, 0.0);
    const Vec3 axis_direction = vec3_new(1.0, 0.0, 0.0);
    const Vec3 diagonal_direction = vec3_normalize(vec3_new(1.0, 1.0, 0.0));

    /* When */
    double axis_distance = distance_to_cube(center, axis_direction, 1.0);
    double diagonal_distance = distance_to_cube(center, diagonal_direction, 1.0);

    /* Then */
    return assert_condition(fabs(axis_distance - 1.0) < 1e-12 &&
                                fabs(diagonal_distance - sqrt(2.0)) < 1e-12,
                            "la distance intérieure atteint la bonne face du cube");
}

/* Vérifie qu'un rayon extérieur qui manque le cube est rejeté. */
static TestResult test_cube_distance_no_hit(void) {
    /* Given */
    const Vec3 position = vec3_new(2.0, 2.0, 0.0);
    const Vec3 direction = vec3_new(-1.0, 0.0, 0.0);

    /* When */
    double distance = distance_to_cube(position, direction, 1.0);

    /* Then */
    return assert_condition(fabs(distance + 1.0) < 1e-12,
                            "un rayon parallèle à une face et hors du cube ne le rencontre pas");
}

/* Vérifie que l'entrée oblique peut se faire par chacune des deux faces amont. */
static TestResult test_cube_entry_position_spans_cube(void) {
    /* Given */
    const double half_side = 1.0;
    const double theta = acos(-1.0) / 6.0;
    const Vec3 direction = vec3_new(sin(theta), 0.0, -cos(theta));

    /* When */
    Vec3 x_face_entry = cube_entry_position(half_side, 0.2, 0.3, 0.7, direction);
    Vec3 z_face_entry = cube_entry_position(half_side, 0.6, 0.3, 0.7, direction);
    double x_face_exit_distance = distance_to_cube(x_face_entry, direction, half_side);
    double z_face_exit_distance = distance_to_cube(z_face_entry, direction, half_side);

    /* Then */
    return assert_condition(fabs(x_face_entry.x + half_side) < 1e-12 &&
                                fabs(z_face_entry.z - half_side) < 1e-12 &&
                                x_face_exit_distance > 0.0 && z_face_exit_distance > 0.0 &&
                                x_face_entry.y >= -half_side && x_face_entry.y <= half_side &&
                                z_face_entry.x >= -half_side && z_face_entry.x <= half_side,
                            "les deux faces amont exposées sont échantillonnées et traversées");
}

static const TestCase test_cases[] = {
    {"test_dts_center", test_dts_center},
    {"test_dts_inside", test_dts_inside},
    {"test_dts_oblique", test_dts_oblique},
    {"test_dts_surface", test_dts_surface},
    {"test_dts_outside_hit", test_dts_outside_hit},
    {"test_dts_no_hit", test_dts_no_hit},
    {"test_no_nan_distance", test_no_nan_distance},
    {"test_convergence", test_convergence},
    {"test_depth_mean", test_depth_mean},
    {"test_depth_cdf", test_depth_cdf},
    {"test_flux_angular_lambert", test_flux_angular_lambert},
    {"test_flux_radial_convergence", test_flux_radial_convergence},
    {"test_interaction_zero_probability", test_interaction_zero_probability},
    {"test_interaction_mean", test_interaction_mean},
    {"test_path_length_mean", test_path_length_mean},
    {"test_path_length_no_outliers", test_path_length_no_outliers},
    {"test_performance", test_performance},
    {"test_uniform_disk_radius", test_uniform_disk_radius},
    {"test_entry_position_spans_full_chord", test_entry_position_spans_full_chord},
    {"test_geometry_incident_direction", test_geometry_incident_direction},
    {"test_cube_distance_inside", test_cube_distance_inside},
    {"test_cube_distance_no_hit", test_cube_distance_no_hit},
    {"test_cube_entry_position_spans_cube", test_cube_entry_position_spans_cube},
};

int main(void) {
    return run_test_suite("geometry", test_cases, sizeof(test_cases) / sizeof(test_cases[0]));
}
