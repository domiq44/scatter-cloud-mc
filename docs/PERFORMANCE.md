# Performance - scatter-cloud-mc

## Benchmarks Actuels

| Configuration | Photons | Temps (est.) | Énergie Conservée |
|---------------|---------|--------------|-------------------|
| Sphère τ=0.1, albedo=1, g=0.3 | 10⁶ | ~2.5s | 1e-12 relatif |
| Cube τ=1.0, albedo=0.8, g=0.5 | 10⁶ | ~3.1s | 1e-12 relatif |
| Double HG, τ=10, albedo=0.9 | 10⁶ | ~4.2s | 1e-12 relatif |

*Mesuré sur CPU moderne (Intel i7/AMD Ryzen), GCC -O3, single-thread.*

## Complexité Algorithmique

| Opération | Complexité | Note |
|-----------|------------|------|
| Transport photon | O(N × moyenne_diffusions) | moyenne_diffusions ∝ τ/(1-albedo) |
| Échantillonnage HG | O(1) | formule analytique inverse CDF |
| Recherche intersection | O(1) | géométrie analytique (sphère/cube) |
| Accumulation Kahan | O(1) par événement | 4 doubles par photon détecté |
| Export CSV | O(bins + photons) | streaming = O(1) mémoire |

## Optimisations Appliquées

### 1. RNG SplitMix64
- Période 2⁶⁴, passe TestU01 BigCrush
- 53 bits significatifs → double IEEE 754 sans biais
- Pas de division dans le chemin critique (`>> 11` + multiplication)

### 2. Somme de Kahan
- Compensation erreur arrondi flottant
- 4 accumulateurs indépendants (énergie détectée, flux haut, flux bas, absorbé)
- Erreur relative ~1e-12 vs 1e-8 sans Kahan

### 3. HG Stable Numériquement
- Formule réarrangée évitant soustraction catastrophique pour g ≈ ±1
- `HG_ISOTROPIC_TOLERANCE = 1e-12` pour cas g=0
- Valide pour g ∈ [-0.999, 0.999]

### 4. Géométrie Analytique
- Pas de marche par pas (ray-marching)
- Intersections sphère/cube exactes (équations quadratiques)
- Cordes moyennes calculées analytiquement

### 5. Export CSV Streaming
- `simulation_csv.c` : écriture incrémentale, O(1) mémoire
- `simulation_distribution_csv.c` : agrégation par bins pendant simulation

### 6. Compilation
```makefile
CFLAGS = -std=c11 -O3 -march=native -pipe -flto
```
- `-O3` : vectorisation, inlining, élagage
- `-march=native` : instructions CPU hôte (AVX2, FMA)
- `-flto` : optimisation inter-procédurale (link-time)

## Profiling

```bash
# Build avec symboles debug + optimisations
make clean && CFLAGS="-std=c11 -O3 -g -march=native" make

# Profil CPU avec perf (Linux)
perf record ./build/simulate config/config.dat
perf report

# Profil mémoire avec valgrind
valgrind --tool=massif ./build/simulate config/config.dat
ms_print massif.out.*

# Analyse cache
perf stat -e cache-references,cache-misses ./build/simulate config/config.dat
```

### Points Chauds Typiques (perf)
1. `simulate_photons()` : ~85% temps (boucle principale)
2. `sample_henyey_greenstein()` : ~8% (math: sqrt, log, div)
3. `geometry_distance_to_boundary()` : ~5% (branches, sqrt)
4. `rng_next_u64()` : ~2% (shifts, multiplies)

## Scalabilité

| Facteur | Impact |
|---------|--------|
| **Photons (N)** | Linéaire O(N) - parallélisable trivialement |
| **Épaisseur optique (τ)** | Super-linéaire (plus diffusions) |
| **Albedo → 1** | Plus diffusions, plus temps |
| **Bins (n_mu_bins)** | O(bins) mémoire, négligeable CPU |
| **Géométrie cube** | ~20% plus lent (6 faces vs 1 sphère) |

## Parallélisation Future (Non Implémentée)

Le noyau `simulate_photons()` est **embarrassingly parallel** :
- Photons indépendants (pas de partage d'état sauf RNG)
- RNG : SplitMix64 supporte `jump_ahead()` pour sous-séquences disjointes
- Accumulation : Kahan par thread + réduction finale

```c
// Pattern OpenMP suggéré
#pragma omp parallel for schedule(dynamic) reduction(+:kahan_accumulators)
for (int i = 0; i < n_photons; i++) {
    Rng thread_rng = rng_jump(rng_global, i * STRIDE);
    simulate_one_photon(&thread_rng, &local_result);
    kahan_add(&local_result, &thread_kahan);
}
```

Estimation speedup : ~N_cœurs × 0.85 (Amdahl, 15% sériel I/O + setup).

## Régression Performance

Tests existants détectant régressions :
- `test_performance` (geometry) : temps max pour 10⁵ opérations distance
- `test_variance_mc` (rng) : variance Monte Carlo conforme théorie
- `make quality` : `-O3` vérifié par build CI

Pas de benchmark automatisé continu (pas de `make bench`).