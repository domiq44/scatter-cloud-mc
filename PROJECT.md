# PROJECT.md - Spécifications Projet scatter-cloud-mc

## Description

Simulateur Monte Carlo de transport de photons dans les nuages (géométries sphère/cube).

## Dépendances

- GNU Make, GCC/Clang (C11)
- Gnuplot (visualisation)
- Python 3.8+ (scripts auxiliaires)

## Structure des Données

- **Config** : paramètres simulation (n_photons, sigma_e, albedo, g, géométrie, etc.)
- **SimulationResult** : résultats (énergies, histogrammes, photons détectés)
- **Vec3** : vecteurs 3D
- **Rng** : générateur SplitMix64

## Fonctionnalités Clés

- Phase functions : Henyey-Greenstein, Double HG
- Roulette russe pour photons poids faible
- Kahan summation pour précision énergie
- Géométries : sphère, cube
- Export CSV (distribution angulaire, flux, bilan énergétique)

## Validation

- Conservation énergie vérifiée dans tests
- Régressions : `test_uniform_disk_radius`, `test_variance_mc`
- 108 tests passent, quality clean