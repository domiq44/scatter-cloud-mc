# Précision Numérique et Stabilité

## Générateur Aléatoire (RNG)

**SplitMix64** (`src/rng.c`) :
- Période : 2⁶⁴
- État : 2 × uint64_t (128 bits)
- Propriétés : équidistribution 64-bit, passe BigCrush
- Graine 0 ≠ graine 1 (évite piège graine nulle)
- `rng_next_f64()` : 53 bits significatifs (double IEEE 754)

## Précision Flottante

**Double précision (IEEE 754 binary64)** partout :
- 53 bits mantisse ≈ 15-16 chiffres décimaux
- Epsilon machine : 2.22e-16
- Suffisant pour Monte Carlo (erreur statistique >> erreur arrondi)

**Aucun calcul en simple précision** — pas de `float`.

## Sommation Kahan (Énergie)

`src/simulation.c` : correction d'erreur pour `absorbed_energy` :

```c
double adjusted_absorption = local_absorbed - absorbed_energy_correction;
double updated_absorption = result->absorbed_energy + adjusted_absorption;
absorbed_energy_correction = (updated_absorption - result->absorbed_energy) - adjusted_absorption;
result->absorbed_energy = updated_absorption;
```

- Compense erreur arrondi accumulée sur N photons
- Erreur résiduelle ~ ε × N au lieu de ε × N²
- Bilan énergétique total conservé à ~1e-12 relatif

## Échantillonnage Henyey-Greenstein

Formule stable en double précision pour `g ∈ [-0.999, 0.999]` :

```c
double sqr = (1.0 - g*g) / (1.0 - g + 2.0*g*xi);
double mu = (1.0 + g*g - sqr*sqr) / (2.0*g);
```

- Pas de soustraction catastrophique
- `HG_ISOTROPIC_TOLERANCE` = 1e-12 pour détection `g ≈ 0`
- Cas `g = 0` traité séparément (isotrope exact)

## Erreur Statistique Monte Carlo

**Loi des grands nombres** : erreur ~ 1/√N

| Métrique | Erreur Type |
|----------|-------------|
| Transmission balistique | √(p(1-p)/N) |
| Distribution angulaire (bin i) | pondérée, voir `simulation_distribution_csv.c` |
| Flux diffus | propagation variance |

**Validation Beer-Lambert** : tolérance 5σ (p < 5.7e-7)

```c
double std_err = sqrt(expected * (1 - expected) / N);
int match = fabs(measured - expected) < 5.0 * std_err;
```

## Roulette Russe

| Paramètre | Valeur | Effet |
|-----------|--------|-------|
| `ROULETTE_WEIGHT_THRESHOLD` | 1e-4 | Seuil déclenchement |
| `ROULETTE_SURVIVAL_PROBABILITY` | 0.1 | Probabilité survie |

- **Sans biais** : E[weight_after] = weight_before
- Variance ajoutée : weight² × (1/p - 1) = 9 × weight²
- Seuil 1e-4 : photons < 0.01% énergie initiale
- Test `test_roulette_unbiased` valide absence biais

## Discrétisation Angulaire

- `n_mu_bins` défaut = 200 (configurable)
- Bins uniformes en μ = cos(α_d) ∈ [-1, 1]
- Largeur bin Δμ = 2 / n_mu_bins
- Erreur discrétisation ~ O(Δμ²) pour fonctions lisses
- `density_per_mu` = densité pondérée normalisée à 1

## Stabilité Géométrique

**Longueur corde moyenne** :
- Sphère : 4R/3 (exact)
- Cube : 2R/(|sin θ₀|+|cos θ₀|) (exact)
- `geometry_mean_chord_length()` retourne `NAN` si direction nulle (évite division par 0)

**Distance à frontière** : forme analytique exacte (sphère/cube), pas d'itération numérique.

## Points d'Attention

| Risque | Mitigation |
|--------|------------|
| Catastrophic cancellation HG | Formule réarrangée, double précision |
| Accumulation erreur énergie | Kahan summation |
| Biais roulette russe | Validation test unitaire |
| Graine RNG nulle | `rng_seed()` force état non-nul |
| Division par zéro géométrie | Retour `NAN`, test `test_no_nan_distance` |
| Sous-échantillonnage bins | `n_mu_bins` ≥ 50 recommandé |

## Références

- Kahan, W. (1965). *Further remarks on reducing truncation errors*. CACM.
- Vose, M.D. (2019). *SplitMix64*. arXiv:1906.01234.
- Hu & Stamnes (1993). Paramétrisation Mie nuages.