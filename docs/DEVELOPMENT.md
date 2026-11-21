# Qualité & Nettoyage - scatter-cloud-mc

## Qualité Code

```bash
make format         # applique clang-format (PATH ou CLANG_FORMAT=clang-format-XX)
make quality        # vérifie format + cppcheck + clang-tidy (zéro warning requis)
make coverage       # tests avec gcov + rapport *.gcov
make coverage-html  # rapport HTML interactif (requiert lcov, genhtml)
make coverage-check # échoue si couverture moyenne < 85%
make bench          # benchmarks locaux (sphère + cube, 10k→1M photons)
make bench-ci       # sortie formatée pour CI (écrire bench.txt)
```

CI utilise clang-format générique distribution. Pour forcer version :
`make format CLANG_FORMAT=clang-format-23`

## Nettoyage

| Commande | Action |
| ---------- | -------- |
| `make clean` | supprime `build/` seulement |
| `make clean-sweep-configs` | supprime configs générées sweep (garde snapshots résultats) |
| `make clean-output` | supprime `output/` + profil rapide |
| `make clean-coverage` | supprime données couverture |
| `make clean-all` | tous les artefacts ci-dessus |

## CI/CD

Workflows GitHub Actions (`.github/workflows/`) :
- `quality.yml` : Matrice 5 jobs (Ubuntu×3, macOS, Windows ; GCC/Clang ; C11/C17)
- `benchmark.yml` : Performance (push main, PR, manual) + auto-comment PR
- `release.yml` : Binaires multi-plateforme (Linux x86_64/ARM64, macOS x86_64/ARM64, Windows x86_64)

Déclencheur release : `git tag vX.Y.Z && git push origin vX.Y.Z` → GitHub Release auto.