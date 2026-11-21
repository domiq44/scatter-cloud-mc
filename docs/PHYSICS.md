# Modèle Physique

## Épaisseur Optique Moyenne

$\bar\tau = \sigma_e \bar\ell$ où $\bar\ell$ = longueur moyenne des cordes traversées.

| Géométrie | $\bar\ell$ | $\bar\tau$ |
| ----------- | ------------ | ------------ |
| Sphère | $4R/3$ | $4\sigma_e R/3$ |
| Cube (côté 2R) | $2R/(|\sin\theta_0|+|\cos\theta_0|)$ | $2\sigma_e R/(|\sin\theta_0|+|\cos\theta_0|)$ |

Dans la config, `radius` = $R$ (rayon sphère ou demi-côté cube).
Le titre affiche $\bar\tau$ selon la géométrie et l'angle.
`sigma_e` et `radius` utilisés directement sans conversion d'unités.

## Fonction de Phase

**HG simple** : $P_{HG}(\mu; g) = \frac{1-g^2}{(1+g^2-2g\mu)^{3/2}}$

**Double HG** : $P(\mu) = p\,P_{HG}(\mu;g) + (1-p)\,P_{HG}(\mu;g_2)$

où $p$ = `hg_fraction`, $g$ = premier lobe, $g_2$ = second lobe.

Asymétrie moyenne : $\langle\mu\rangle = p g + (1-p)g_2$.

Référence : Hu & Stamnes (1993), *J. Climate* 6(4), 728–742.
Le triplet par défaut ($g=0.9, g_2=-0.2, p=0.9545$) donne $\langle\mu\rangle\approx0.85$ (nuage liquide visible).

## Faisceau Incident

Faisceau parallèle, angle $\theta_0$ (`theta_zero`) par rapport à $+Oz$.

Direction : $\mathbf{d} = (\sin\theta_0, 0, -\cos\theta_0)$

**Sphère** : impact uniforme sur disque projeté rayon $R$ perpendiculaire à $\mathbf{d}$.
$u\sim[0,1], \phi\sim[0,2\pi], \rho = R\sqrt{u}$
$(x', y', z') = (\rho\cos\phi, \rho\sin\phi, 0)$ dans plan ⊥ au faisceau.

Point entrée : $\mathbf{p} = \mathbf{b} - \mathbf{d}\sqrt{R^2-\rho^2}$ (surface d'entrée complète)

**Cube** : faces d'entrée pondérées par aire projetée.

## Angle de Diffusion $\alpha_d$

Mesuré par rapport au **faisceau incident** (pas l'axe vertical).

$\alpha_d = \arccos(\mathbf{d}\cdot\mathbf{v}_{sortie})$, $\mu = \cos\alpha_d = \mathbf{d}\cdot\mathbf{v}_{sortie}$

- Transmission balistique : $\alpha_d = 0^\circ, \mu = 1$
- Sortie opposée : $\alpha_d = 180^\circ, \mu = -1$

`alpha_d_deg` = degrés, champ CSV `mu` = cosinus.
`theta_zero` = angle incidence par rapport à $+Oz$ (n'est PAS l'axe de mesure de $\alpha_d$).

## Albedo & Absorption

`albedo = 1` → hypothèse conservative (absorption négligeable).
Pour comparaison quantitative : extinction et albédo doivent correspondre à la longueur d'onde et microphysique étudiées.

## Roulette Russe

Poids < `ROULETTE_WEIGHT_THRESHOLD` (1e-4) : survie prob. `ROULETTE_SURVIVAL_PROBABILITY` (0.1).
Si survit : poids /= 0.1. Sinon : photon tué, énergie comptabilisée.
Sans biais (expected weight conservé).

