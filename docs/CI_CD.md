# Intégration Continue - scatter-cloud-mc

## Workflows GitHub Actions

### 1. Quality (`.github/workflows/quality.yml`)

Matrice multi-OS / multi-compilateur / multi-C standard :

| OS | Compilateur | C Standard |
|----|-------------|------------|
| ubuntu-latest | gcc | C11 |
| ubuntu-latest | clang | C11 |
| ubuntu-latest | gcc | C17 |
| macos-latest | clang | C11 |
| windows-latest | clang | C11 |

```yaml
jobs:
  quality:
    strategy:
      matrix:
        include:
          - os: ubuntu-latest; compiler: gcc; cstd: 11
          - os: ubuntu-latest; compiler: clang; cstd: 11
          - os: ubuntu-latest; compiler: gcc; cstd: 17
          - os: macos-latest; compiler: clang; cstd: 11
          - os: windows-latest; compiler: clang; cstd: 11
```

Chaque job :
1. Installe outils (apt/brew/choco)
2. `make quality` (format + cppcheck + clang-tidy)
3. `make build test` (111 tests)

### 2. Benchmark (`.github/workflows/benchmark.yml`)

Déclencheurs : `push` sur main, `pull_request`, manuel (`workflow_dispatch`)

Matrice :
| OS | Compilateur |
|----|-------------|
| ubuntu-latest | gcc |
| ubuntu-latest | clang |
| macos-latest | clang |

Étapes :
1. Build `CFLAGS="-O3 -std=c11"`
2. `make bench-ci` → génère `bench.txt`
3. Upload artefact `bench-{os}-{compiler}`
4. Commentaire auto sur PR avec résultats

### 3. Release (`.github/workflows/release.yml`)

Déclencheur : `release` publié (GitHub Releases)

Matrice multi-plateforme :
| OS | Arch | Artifact |
|----|------|----------|
| ubuntu-latest | x86_64 | scatter-cloud-mc-linux-x86_64 |
| ubuntu-latest | aarch64 | scatter-cloud-mc-linux-arm64 (cross-compile) |
| macos-latest | x86_64 | scatter-cloud-mc-macos-x86_64 |
| macos-latest | arm64 | scatter-cloud-mc-macos-arm64 |
| windows-latest | x86_64 | scatter-cloud-mc-windows-x86_64.exe |

Étapes :
1. Build statique (`-static` pour Linux/Windows)
2. Package binaire + README + SHA256SUMS
3. Upload assets sur GitHub Release

## Déclencheurs

| Workflow | Événement | Action |
|----------|-----------|--------|
| Quality | `push`, `pull_request` | Matrice complète quality + build + test |
| Benchmark | `push` main, PR, manual | Benchmarks + commentaire PR |
| Release | `release` published | Build multi-plateforme + upload assets |

## Jobs Quality

### `quality` (5 jobs parallèles)
- **Runners** : ubuntu-latest ×3, macos-latest, windows-latest
- **Durée** : ~2-3 min/job
- **Étapes** : Install tools → `make quality` → `make build test`

## Vérifications Effectuées

### `make quality` (local & CI)
```bash
# 1. Format
clang-format --dry-run --Werror src/*.c include/*.h tests/*.c

# 2. Static analysis - cppcheck
cppcheck --enable=all --std=c11 --error-exitcode=1 \
  --suppress=missingIncludeSystem src/ include/

# 3. Static analysis - clang-tidy
clang-tidy -checks='*,-bugprone-signed-bitwise' \
  src/*.c -- -std=c11 -Iinclude
```

### `make test`
- Compile `build/test_runner` + 15 suites
- Exécute 111 tests
- Échoue si ANY test FAIL (exit code 1)

## Matrice Configurée

| Dimension | État |
|-----------|------|
| **OS** | ✅ Ubuntu, macOS, Windows |
| **Compilateur** | ✅ GCC, Clang |
| **C Standard** | ✅ C11, C17 |
| **Architecture** | ✅ x86_64, ARM64 (cross-compile Linux) |

## Exclusions Tests Windows

Certains tests utilisent des APIs POSIX indisponibles sur Windows MinGW et sont exclus automatiquement via `ifeq ($(OS),Windows_NT)` dans le `Makefile` :

| Fichier de test | APIs POSIX utilisées | Raison |
|-----------------|----------------------|--------|
| `test_plot_main.c` | `mkdtemp`, `mkdir(mode)`, `chmod`, `setenv`/`unsetenv`, `/tmp/`, gnuplot | CLI graphiques |
| `test_simulate.c` | `mkdtemp`, `mkdir(mode)`, `chmod`, `/tmp/` | Entrypoint simulation |
| `test_config.c` | `mkstemp`, `/tmp/` | Parsing config |
| `test_plot_csv.c` | `mkstemp`, `mkdtemp`, `/tmp/` | Lecture/validation CSV |
| `test_simulation_csv.c` | `mkstemp`, `/tmp/` | Écriture CSV résultats |
| `test_file_utils.c` (support) | `mkstemp`, `/tmp/` | Utilitaires fichiers temporaires |

Tests conservés sur Windows (10 fichiers, ~71 tests) : APIs C11 purement portables (`test_basis`, `test_file_path`, `test_geometry`, `test_hg`, `test_plot_data`, `test_plot_gnuplot`, `test_rng`, `test_simulation`, `test_simulation_report`, `test_vec3`).

## Artefacts

- **Quality** : Aucun (logs seulement)
- **Benchmark** : `bench.txt` par job (upload 30 jours)
- **Release** : Binaires multi-plateforme + SHA256SUMS + README

## Badges README

```markdown
![Quality](https://github.com/<owner>/scatter-cloud-mc/actions/workflows/quality.yml/badge.svg)
![Benchmark](https://github.com/<owner>/scatter-cloud-mc/actions/workflows/benchmark.yml/badge.svg)
![Release](https://github.com/<owner>/scatter-cloud-mc/actions/workflows/release.yml/badge.svg)
```

## Maintenance

| Tâche | Fréquence |
|-------|-----------|
| Mettre à jour `actions/checkout@v4` | Trimestriel |
| Mettre à jour versions outils (clang, cppcheck) | Semestriel |
| Vérifier compatibilité Ubuntu LTS | À chaque LTS |
| Revue temps d'exécution CI | Mensuel (cible < 5 min/job) |
| Tester release workflow | À chaque tag |

### 2. Performance Benchmark
```yaml
- name: Benchmark
  run: |
    ./scripts/bench.sh > bench.txt
    cat bench.txt
- uses: benchmark-action/github-action-benchmark@v1
  with:
    tool: 'custom'
    output-file-path: bench.txt
    github-token: ${{ secrets.GITHUB_TOKEN }}
```

### 3. Release Automation
```yaml
on:
  release:
    types: [published]

jobs:
  release:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Build release
        run: make clean && make
      - name: Package
        run: tar -czf scatter-cloud-mc-linux.tar.gz build/simulate
      - uses: softprops/action-gh-release@v1
        with:
          files: scatter-cloud-mc-linux.tar.gz
```

### 4. Dependency Scanning
```yaml
- name: Check dependencies
  run: |
    # Aucune dep externe runtime (sauf Gnuplot/Python optionnels)
    # Vérifier versions minimales
    gcc --version | head -1
    gnuplot --version | head -1
    python3 --version
```

## Monitoring

- **Status** : Badge workflow sur README
- **Notifications** : Email GitHub par défaut (configurable dans Settings → Notifications)
- **Historique** : Onglet Actions GitHub (rétention 90 jours par défaut)

## Secrets Requis

Aucun secret requis pour le workflow actuel.

## Maintenance

| Tâche | Fréquence |
|-------|-----------|
| Mettre à jour `actions/checkout@v4` | Trimestriel |
| Mettre à jour versions outils (clang, cppcheck) | Semestriel |
| Vérifier compatibilité Ubuntu LTS | À chaque LTS |
| Revue temps d'exécution CI | Mensuel (cible < 5 min) |