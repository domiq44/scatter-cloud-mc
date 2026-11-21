# Utilisation Avancée

## Targets Make Principaux

```bash
# Simulation unique
make run-one CONFIG=config/config.dat NAME=essai

# Retracer graphiques sans relancer simulation
make plot DISTRIBUTION=output/essai/nom.distribution.csv \
  CONFIG=config/config.dat PNG=output/essai/nom.png

# Type de graphique
make plot ... PLOT_TYPE=distribution|distribution-linear|distribution-mu|distribution-mu-linear|phase|both
# both = distribution + phase (défaut)
# phase produit *_phase.png
```

## Multi-Runs & Balayages (Sweep)

```bash
# Plusieurs graines, même config
make run-many N_RUNS=20
python3 analyse_diff.py  # analyse résidus dans output/diffs.txt

# Balayage paramétrique
make sweep-plan N_PHOTONS=5000 ANGLES="0 30 60" TAUS="0.1 1 10"
make sweep N_PHOTONS=5000 ANGLES="0 30 60" TAUS="0.1 1 10"
make sweep RESUME=1 N_PHOTONS=5000 ANGLES="0 30 60" TAUS="0.1 1 10"
make report  # galerie HTML interactive

# Profil rapide (9 cas, 2000 photons)
make quick
make quick GEOMETRY=both QUICK_N_PHOTONS=3000
```

### Paramètres Sweep

| Variable | Description |
| ---------- | ------------- |
| `N_PHOTONS` | photons par cas (défaut: template) |
| `ANGLES` | liste angles degrés (ex: "0 30 60") |
| `TAUS` | liste épaisseurs optiques (ex: "0.1 1 10") |
| `GEOMETRY` | `sphere` (défaut), `cube`, `both` |
| `RESUME=1` | saute cas terminés (config snapshot identique) |
| `ESTIMATE_SECONDS_PER_PHOTON` | estimation durée manuelle |

**Angles par défaut** : 0°, 30°, 60°, 70°, 80°, 85°, 90°
**Tau par défaut** : 0.01, 0.02, 0.05, 0.1, 0.2, 0.5, 1, 2, 5, 10, 20, 50, 100

### Calcul Radius par Tau

- Sphère : `radius = 3τ / (4σₑ)`
- Cube : `radius = τ(|sin θ₀| + |cos θ₀|) / (2σₑ)`

### Artefacts Sweep

- `output/<cas>/sweep-config.dat` : snapshot config exacte (précision aller-retour)
- Répertoires cube préfixés `cube_`
- `RESUME=1` compare snapshot + existence distribution

## Rapport Interactif

```bash
make report
```

Génère `output/report_interactif.html` :
- Distributions angulaires
- Réflectance directionnelle $R_{dir} = F_{diffus\ montant}/F_{incident}$ vs $\tau$
- Sélecteur "Forme" (sphère/cube) + filtre "Photons injectés"
- Échelle log/lin + abscisse $\alpha_d$ ou $\cos(\alpha_d)$ indépendants
- `web/report_interactif.html` lie vers rapport généré

## Exécution Complète

```bash
make full  # build → test → sweep → report (séquentiel, même avec make -j)
```

## Variables d'Ajustement Rapide

- `QUICK_ANGLES`, `QUICK_TAUS`, `QUICK_N_PHOTONS` pour `make quick`
- `CLANG_FORMAT=clang-format-23` pour `make format`

