# Formats de Sortie

## CSV Bruts (`*.csv`)

Colonnes : `alpha_d_deg`, `mu`, `pdf`, `weight`
- Une ligne = un photon détecté
- `alpha_d_deg` : angle diffusion degrés (par rapport au faisceau incident)
- `mu` : $\cos(\alpha_d)$
- `pdf` : probabilité pondérée du bin (pas densité par unité d'angle)
- `weight` : poids du photon

Regroupe tous azimuts autour de la direction incidente initiale $\mathbf{d}$, quel que soit `theta_zero`.

## Distribution Agrégée (`*.distribution.csv`)

Contient :
- Distribution agrégée par bin
- Erreurs-types pondérées
- Largeurs des bins
- `detected_energy_fraction_of_injected`

**Métriques clés** :
- `density_per_mu` : densité pondérée conditionnelle à la détection, normalisée à 1 si détections
- `diffuse_density_per_mu` : photons ayant subi ≥1 diffusion (intégrale = fraction énergie détectée diffuse)
- `ballistic_energy_fraction_of_detected` : fraction photons sortis sans diffusion
- Bins diffusifs nuls omis du graphique log
- Si aucun bin diffusif positif → message explicite au lieu d'échec

Somme `detected_energy_fraction_of_injected` = $E_{detected} / n_{photons}$.

## Bilan Énergétique (`*.flux.csv`)

Flux avec fractions de l'énergie injectée :
- Flux diffusé montant
- Flux diffusé descendant
- Énergie absorbée
- Flux direct descendant
- Poids arrêté par roulette russe

Haut/bas déterminé par composante verticale ($z$) de la direction de propagation à la sortie.

## Statistiques (`stats.txt`)

- Nombre photons détectés
- Résumé simulation

## Bilan Énergétique Console

À la fin de chaque run :

```
=== Bilan énergétique ===
Photons injectés : N
Photons sortis : M (X%)
Photons tués RR : K (Y%)
Photons diffusés : D (Z% par photon en moyenne)

--- Énergies ---
E_detected           = ...
Flux diffusé montant = ...
Flux diffusé descendant = ...
Flux absorbé         = ...
Flux direct descendant = ...
Poids arrêté par RR  = ...
E_absorbed (direct)  = ...

E comptabilisée      = ... (référence : N)
Résidu Monte Carlo   = ... (fluctue avec roulette russe; moyenne attendue ≈ 0)

=== Résumé ===
Sortie X% | Absorption Y% | RR Z%
MC_DIFF <valeur>
```

**MC_DIFF** : résidu bilan pondéré. Roulette russe stochastique sans biais → résidu varie, attendu ≈0 en moyenne multi-graines.

## Validation Tests

- Transmission balistique vs moyenne analytique Beer-Lambert sur cordes disque projeté
- Tolérance : 5 erreurs-types Monte-Carlo
- Conservation énergie vérifiée dans tests unitaires
