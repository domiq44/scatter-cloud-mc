#include "simulation_csv.h"
#include <stdio.h>

int write_simulation_csv(const char *filename, const SimulationResult *result) {
    if (!filename || !result || result->bin_count <= 0 || !result->exit_pdf ||
        !result->exit_probability_standard_error ||
        (result->photon_count > 0 && !result->photons && !result->photon_stream))
        return 0;

    FILE *file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Erreur : impossible d'ouvrir %s\n", filename);
        return 0;
    }

    int write_succeeded = fprintf(file, "alpha_d_deg,mu,pdf,weight\n") >= 0;
    if (result->photon_stream) {
        clearerr(result->photon_stream);
        write_succeeded = write_succeeded && fseek(result->photon_stream, 0, SEEK_SET) == 0;
    }
    for (int i = 0; write_succeeded && i < result->photon_count; i++) {
        DetectedPhoton streamed_photon;
        const DetectedPhoton *photon = result->photons ? &result->photons[i] : &streamed_photon;
        if (result->photon_stream &&
            fread(&streamed_photon, sizeof(streamed_photon), 1, result->photon_stream) != 1) {
            write_succeeded = 0;
            break;
        }
        if (photon->bin < 0 || photon->bin >= result->bin_count) {
            write_succeeded = 0;
        } else {
            write_succeeded = fprintf(file, "%f,%f,%f,%f\n", photon->alpha_d_deg, photon->mu,
                                      result->exit_pdf[photon->bin], photon->weight) >= 0;
        }
    }
    if (result->photon_stream) {
        clearerr(result->photon_stream);
        if (fseek(result->photon_stream, 0, SEEK_SET) != 0)
            write_succeeded = 0;
    }

    if (fclose(file) != 0)
        write_succeeded = 0;
    if (!write_succeeded)
        fprintf(stderr, "Erreur : écriture incomplète dans %s\n", filename);
    return write_succeeded;
}

int write_simulation_flux_csv(const char *filename, const SimulationResult *result) {
    if (!filename || !result || result->photon_count + result->roulette_killed_count <= 0)
        return 0;

    FILE *file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Erreur : impossible d'ouvrir %s\n", filename);
        return 0;
    }

    const double injected_energy = result->photon_count + result->roulette_killed_count;
    int write_succeeded = fprintf(file, "component,energy,energy_fraction_of_injected\n") >= 0;
    write_succeeded = write_succeeded &&
                      fprintf(file, "diffuse_upward,%.12g,%.12g\n", result->diffuse_upward_energy,
                              result->diffuse_upward_energy / injected_energy) >= 0;
    write_succeeded =
        write_succeeded &&
        fprintf(file, "diffuse_downward,%.12g,%.12g\n", result->diffuse_downward_energy,
                result->diffuse_downward_energy / injected_energy) >= 0;
    write_succeeded =
        write_succeeded && fprintf(file, "absorbed,%.12g,%.12g\n", result->absorbed_energy,
                                   result->absorbed_energy / injected_energy) >= 0;
    write_succeeded = write_succeeded &&
                      fprintf(file, "direct_downward,%.12g,%.12g\n", result->direct_downward_energy,
                              result->direct_downward_energy / injected_energy) >= 0;

    if (fclose(file) != 0)
        write_succeeded = 0;
    if (!write_succeeded)
        fprintf(stderr, "Erreur : écriture incomplète dans %s\n", filename);
    return write_succeeded;
}