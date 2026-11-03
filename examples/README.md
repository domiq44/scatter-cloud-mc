# Exemples guidés

Ces dix scripts sont des parcours autonomes, pas une seconde interface au simulateur. Chacun lance le calcul requis et produit ses données et graphiques. Ils appellent les cibles Make existantes, fonctionnent depuis n’importe quel répertoire et n’altèrent pas `config/config.dat`.

Lancez un parcours avec `bash examples/<nom>.sh`. Les valeurs par défaut gardent les sweeps d’exploration modestes.

Chaque script termine par un résumé « Terminé » avec le dossier des résultats,
le nombre de CSV et de PNG, et le chemin du rapport HTML s’il a été généré. Les
parcours qui tracent des images individuelles indiquent explicitement qu’ils
n’ont pas créé de galerie HTML. Une erreur Make ou Python arrête le script avec
un code de sortie non nul.

| Script | Couverture |
| --- | --- |
| `demarrer_rapidement.sh` | Première simulation et galerie courte |
| `comparer_geometries.sh` | Sphère et cube dans les mêmes conditions |
| `explorer_grille.sh` | Angles, tau et géométrie réglables |
| `comparer_tailles_echantillon.sh` | Deux nombres de photons dans une galerie filtrable |
| `reprendre_un_sweep.sh` | Snapshot, `RESUME=1`, puis rapport après `make clean` |
| `etudier_cube_oblique.sh` | Incidence oblique sur les faces du cube |
| `etudier_albedo_et_phase.sh` | Albédo 0, intermédiaire, 1, HG et double-HG |
| `stabilite_par_graines.sh` | Une simulation graphique, puis les résidus de plusieurs graines |
| `balayer_grille_complete.sh` | Préflight, sweep complet, puis rapport sphère/cube |
| `tracer_tous_les_modes.sh` | Génère une distribution, puis les cinq représentations |

`balayer_grille_complete.sh` affiche le plan puis lance les 182 cas par défaut : deux géométries, sept angles et treize valeurs de tau, à 1 000 photons par cas. Cette grille peut être longue, même avec un N réduit, car elle produit les graphiques de chaque cas.

Les paramètres sont personnalisables sans modifier les scripts :

```bash
N_PHOTONS=8000 GEOMETRY=cube ANGLES="30 60" TAUS="1 5" bash examples/explorer_grille.sh
PHOTON_COUNTS="1000 10000" ANGLES="0 90" bash examples/comparer_tailles_echantillon.sh
ALBEDOS="0 0.8 1" PHASE_FUNCTIONS="hg" N_PHOTONS=4000 bash examples/etudier_albedo_et_phase.sh
N_PHOTONS=500 GEOMETRY=both bash examples/balayer_grille_complete.sh
```

`N_PHOTONS`, `ANGLES`, `TAUS` et `GEOMETRY` règlent les sweeps; `PHOTON_COUNTS` règle la comparaison de N; `ALBEDOS` et `PHASE_FUNCTIONS` règlent le scénario physique. `BASE_CONFIG` permet de partir d’une autre configuration pour ce dernier. `EXAMPLE_OUTPUT_ROOT` déplace l’ensemble des sorties et configs; `OUTPUT_DIR` remplace le dossier d’un parcours.

Les configs d’essai sont isolées sous `output/examples/configs/` par défaut. Les variantes HG simples retirent les paramètres propres au double-HG; `config/config.dat` reste inchangé. Le préflight de la grille complète estime la durée; `RESUME=1` reprend les cas déjà terminés. Si `EXAMPLE_OUTPUT_ROOT` change de profondeur dans l’arborescence, adaptez aussi `EXAMPLE_WEB_ASSET_PREFIX`.
