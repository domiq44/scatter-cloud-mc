#include "simulation_report.h"
#include <stdio.h>

void print_simulation_report(const SimulationResult *result, const Config *config) {
    int injected_count = config->n_photons;
    double detected_percentage = 100.0 * result->photon_count / injected_count;
    double roulette_percentage = 100.0 * result->roulette_killed_count / injected_count;
    double scatter_mean_percentage = 100.0 * (double)result->scatter_count / injected_count;
    double total_direct_energy =
        result->absorbed_energy + result->detected_energy + result->roulette_energy;
    double difference = total_direct_energy - (double)injected_count;
    double output_percentage = 100.0 * result->detected_energy / injected_count;
    double absorbed_percentage = 100.0 * result->absorbed_energy / injected_count;
    double roulette_energy_percentage = 100.0 * result->roulette_energy / injected_count;

    printf("\n=== Bilan énergétique ===\n");
    printf("Photons injectés : %d\n", injected_count);
    printf("Photons sortis : %d (%.3f %%)\n", result->photon_count, detected_percentage);
    printf("Photons tués RR : %d (%.3f %%)\n", result->roulette_killed_count, roulette_percentage);
    printf("Photons diffusés : %lld (%.3f %% par photon en moyenne)\n", result->scatter_count,
           scatter_mean_percentage);

    printf("\n--- Énergies ---\n");
    printf("E_detected           = %.6f (%.3f %%)\n", result->detected_energy, output_percentage);
    printf("Flux diffusé montant = %.12f\n", result->diffuse_upward_energy);
    printf("Flux diffusé descendant = %.12f\n", result->diffuse_downward_energy);
    printf("Flux absorbé         = %.12f\n", result->absorbed_energy);
    printf("Flux direct descendant = %.12f\n", result->direct_downward_energy);
    printf("Poids arrêté par RR  = %.6f (%.3f %%)\n", result->roulette_energy,
           roulette_energy_percentage);
    printf("E_absorbed (direct)  = %.12f (%.3f %%)\n", result->absorbed_energy,
           absorbed_percentage);
    printf("\nE comptabilisée      = %.12f (référence : %d)\n", total_direct_energy,
           injected_count);
    printf("Résidu Monte Carlo   = %.12f (fluctue avec la roulette russe; moyenne attendue ≈ 0)\n",
           difference);

    printf("\n=== Résumé ===\n");
    printf("Sortie %.3f %% | Absorption %.3f %% | RR %.3f %%\n", output_percentage,
           absorbed_percentage, roulette_energy_percentage);
    printf("\nMC_DIFF %.12f\n", difference);

    if (result->photon_count == 0) {
        printf("\n⚠️  Aucun photon n'a atteint la surface.\n");
        printf("   La sphère est probablement trop opaque (tau = 4 * sigma_e * R / 3).\n");
    }
}