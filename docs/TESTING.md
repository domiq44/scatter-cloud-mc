# Tests Unitaires et d'Intégration

## Architecture de Test

```
tests/
├── support/                 # Framework de test partagé
│   ├── test_runner.h/c      # Runner (suite, reporting, PASS/FAIL/SKIP)
│   ├── test_assertions.h/c  # assert_condition(condition, description)
│   └── test_reporter.h      # Enum TestResult + report_test_result()
├── test_*.c                 # 15 suites de test (108 tests total)
└── Makefile                 # Build & exécution
```

## Framework de Test

### Exécution

```bash
make test        # Lance toutes les suites
```

### Runner Partagé (`test_runner.h`)

```c
typedef TestResult (*TestFunction)(void);

typedef struct {
    const char *name;
    TestFunction run;
} TestCase;

int run_test_suite(const char *suite_name, const TestCase *test_cases, size_t test_count);
```

- Exécute tous les cas, continue après échec
- Reporting : `PASS`/`FAIL`/`SKIP` + bilan final
- Retour : 0 si tout passe, 1 sinon

### Assertions (`test_assertions.h`)

```c
TestResult assert_condition(int condition, const char *description);
```

- Une seule fonction d'assertion
- Affiche description en cas d'échec
- Retourne `TEST_PASSED` ou `TEST_FAILED`

### Résultats (`test_reporter.h`)

```c
typedef enum {
    TEST_SKIPPED = -1,
    TEST_FAILED = 0,
    TEST_PASSED = 1,
} TestResult;

void report_test_result(const char *test_name, TestResult result);
```

## Structure AAA (Given/When/Then)

Chaque test suit la structure :

```c
static TestResult test_nom_du_comportement(void) {
    /* Given */
    const Config config = { ... };    // entrées, résultats attendus (const)
    Type attendu = VALEUR;
    SimulationResult result;

    /* When */
    int simulated = simulate_photons(&config, config.seed, &result);

    /* Then */
    int ok = simulated && fabs(result.champ - attendu) < EPSILON;
    cleanup(&result);
    return assert_condition(ok, "description du comportement");
}
```

**Règles** :
- `Given` : déclarer entrées, résultats attendus `const`, contexte
- `When` : action principale unique
- `Then` : vérifications observables + diagnostic utile
- Nettoyage explicite (`simulation_result_free`, etc.)
- Un comportement = un test
- Cas nominaux, limites, erreurs séparés

## Suites de Test (15 fichiers, 108 tests)

| Suite | Tests | Domaine |
| ----- | ----- | ------- |
| `test_basis.c` | 2 | Base orthonormale, cas limites |
| `test_config.c` | 12 | Parsing, validation, géométries, θ₀ ∈ [0,90] |
| `test_file_path.c` | 5 | Manipulation chemins/extensions |
| `test_geometry.c` | 23 | Distances, entrées, cordes (sphère/cube) |
| `test_hg.c` | 8 | HG simple/double, normalisation, moyenne |
| `test_plot_csv.c` | 7 | Lecture/validation CSV |
| `test_plot_data.c` | 3 | Conteneur données dynamiques |
| `test_plot_gnuplot.c` | 9 | Génération scripts Gnuplot |
| `test_plot_main.c` | 5 | Entrée CLI plotting |
| `test_rng.c` | 5 | SplitMix64, reproductibilité, roulette |
| `test_simulate.c` | 5 | Entrypoint CLI, cube, énergie, graines |
| `test_simulation.c` | 8 | Transport, énergie, HG double, roulette, Beer-Lambert |
| `test_simulation_csv.c` | 8 | Export CSV mémoire/streaming, flux, distribution |
| `test_simulation_report.c` | 2 | Bilan énergétique console |
| `test_vec3.c` | 6 | Opérations vecteurs 3D |

## Exclusions Windows (6 suites exclues sur MinGW)

| Suite | Tests | APIs POSIX | Raison |
| ----- | ----- | ---------- | ------ |
| `test_config.c` | 12 | `mkstemp`, `/tmp/` | Parsing config |
| `test_plot_csv.c` | 7 | `mkstemp`, `mkdtemp`, `/tmp/` | Lecture/validation CSV |
| `test_plot_main.c` | 5 | `mkdtemp`, `mkdir(mode)`, `chmod`, `setenv`/`unsetenv`, `/tmp/`, gnuplot | CLI graphiques |
| `test_simulate.c` | 5 | `mkdtemp`, `mkdir(mode)`, `chmod`, `/tmp/` | Entrypoint simulation |
| `test_simulation_csv.c` | 8 | `mkstemp`, `/tmp/` | Export CSV résultats |
| `test_file_utils.c` (support) | — | `mkstemp`, `/tmp/` | Utilitaires fichiers temporaires |

Exclusion automatique via `ifeq ($(OS),Windows_NT)` dans le `Makefile` (filtre `TEST_SOURCES` et `SUPPORT_SOURCES`).

Tests conservés sur Windows (10 fichiers, ~71 tests) : APIs C11 purement portables.

## Exemples de Patterns

### Test Nominal

```c
static TestResult test_hg_normalization(void) {
    /* Given */
    Rng rng; rng_seed(&rng, 123);
    const int samples = 1000000;
    double sum = 0.0;

    /* When */
    for (int i = 0; i < samples; i++) {
        double mu = sample_henyey_greenstein(&rng, 0.5, vec3_new(0,0,1)).z;
        sum += mu;
    }
    double mean_mu = sum / samples;

    /* Then */
    return assert_condition(fabs(mean_mu - 0.5) < 0.01,
                            "la moyenne de mu converge vers g=0.5");
}
```

### Test Limite (Edge Case)

```c
static TestResult test_hg_isotropic(void) {
    /* Given */
    Rng rng; rng_seed(&rng, 456);
    const int samples = 100000;
    double sum = 0.0;

    /* When */
    for (int i = 0; i < samples; i++) {
        Vec3 dir = sample_henyey_greenstein(&rng, 0.0, vec3_new(0,0,1));
        sum += dir.z;
    }

    /* Then */
    return assert_condition(fabs(sum / samples) < 0.01,
                            "g=0 produit une distribution isotrope");
}
```

### Test Erreur / Validation

```c
static TestResult test_config_theta_zero_negative_rejected(void) {
    /* Given */
    char *config_text = "n_photons=100 sigma_e=1 albedo=1 g=0 radius=1 theta_zero=-10";
    Config cfg = {0};

    /* When */
    int ok = parse_config_string(config_text, &cfg);

    /* Then */
    return assert_condition(!ok, "theta_zero négatif est rejeté");
}
```

### Test Intégration (Beer-Lambert)

```c
static TestResult test_ballistic_transmission_matches_beer_lambert(void) {
    /* Given */
    const Config config = { .n_photons = 200000, .sigma_e = 0.65, .albedo = 0.0, ... };
    const double expected = ...;  // intégrale analytique sur disque projeté

    /* When */
    SimulationResult result = {0};
    simulate_photons(&config, config.seed, &result);

    /* Then */
    double measured = result.ballistic_detected_energy / config.n_photons;
    double std_err = sqrt(expected * (1 - expected) / config.n_photons);
    int match = fabs(measured - expected) < 5.0 * std_err;  // 5σ
    simulation_result_free(&result);
    return assert_condition(match, "transmission balistique = Beer-Lambert (5σ)");
}
```

## Helpers & Fixtures

- `setup_*`, `assert_*` dans le fichier de test si usage local
- Partagés dans `support/` si multi-suites
- État frais par test (pas de dépendance d'ordre)
- Fixtures immuables (`const Config`) partagées OK

## Couverture Notable

| Domaine | Tests Clés |
| ------- | ---------- |
| **Énergie** | Bilan conservé (Kahan), roulette sans biais, absorption |
| **Géométrie** | Cordes moyennes, entrées uniformes, sphère + cube |
| **HG** | Normalisation, moyenne = g, double HG, isotrope (g=0) |
| **RNG** | Reproductibilité, graine 0 ≠ graine 1, variance MC |
| **CSV** | Mémoire = streaming, bins complets, erreurs pondérées |
| **Validation** | Config invalide, bornes, clés obligatoires double HG |
| **Physique** | Transmission balistique vs Beer-Lambert (5σ MC) |

## Ajouter un Test

1. Créer `tests/test_nouveau.c` ou ajouter à suite existante
2. Inclure `test_assertions.h`, `test_runner.h`
3. Écrire fonction `static TestResult test_...(void)` avec AAA
4. Ajouter au tableau `test_cases[]`
5. `make test` compile et exécute automatiquement

## Intégration Continue

```bash
make test        # Doit passer (108 tests)
make quality     # clang-format + cppcheck + clang-tidy
```

Pas de tests d'intégration séparés — les tests de simulation complète (`test_simulate.c`, `test_simulation.c`) valident le pipeline de bout en bout.

