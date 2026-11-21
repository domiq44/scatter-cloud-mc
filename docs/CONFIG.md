# Configuration

## Clés Principales

| Clé | Type | Requis | Description |
| ----- | ---- | ------ | ----------- |
| `n_photons` | int | oui | Nombre de photons à simuler (>0) |
| `sigma_e` | double | oui | Coefficient d'extinction (>0) |
| `albedo` | double | oui | Albédo simple diffusion [0,1] |
| `g` | double | oui | Paramètre HG premier lobe [-1,1] |
| `radius` | double | oui | Rayon sphère ou demi-côté cube (>0) |
| `geometry` | enum | non | `sphere` (défaut) ou `cube` |
| `n_mu_bins` | int | non | Nombre de bins angulaires (défaut: 200) |
| `seed` | int | non | Graine RNG (défaut: 123) |
| `theta_zero` | double | non | Angle incidence degrés [0,90] (défaut: 0) |

## Phase Function

| Clé | Type | Requis | Description |
| ----- | ---- | ------ | ----------- |
| `phase_function` | enum | non | `hg` ou `double_hg` (défaut: `double_hg`) |
| `g2` | double | si double_hg | Paramètre HG second lobe [-1,1] |
| `hg_fraction` | double | si double_hg | Probabilité premier lobe [0,1] |

**Compatibilité** : absence de `phase_function` → HG simple.

## Géométries

- **Sphère** : `radius` = rayon. Longueur moyenne corde = 4R/3.
- **Cube** : `radius` = demi-côté (côté = 2×radius). Longueur moyenne corde = 2R/(|sin θ₀|+|cos θ₀|).

Incidence oblique : échantillonnage uniforme sur la projection. Pour le cube, les deux faces amont sont pondérées par leur aire projetée.

## Validation

- `n_photons`, `sigma_e`, `radius`, `n_mu_bins` > 0
- `albedo` ∈ [0,1], `g` ∈ [-1,1], `g2` ∈ [-1,1]
- `hg_fraction` ∈ [0,1]
- `theta_zero` ∈ [0, 90] (rejet si hors bornes)
- `g2`, `hg_fraction` obligatoires si `phase_function = double_hg`

## Fichiers

- `config/config.dat` : configuration par défaut
- `config/config.template.dat` : modèle pour balayages (sweep)
