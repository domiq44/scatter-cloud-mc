# Architecture - scatter-cloud-mc

## Diagramme Modules (Mermaid)

```mermaid
graph TD
    %% Entry point
    simulate[simulate.c<br/>CLI Entrypoint]
    
    %% Core modules
    config[config.c/h<br/>Parsing & Validation]
    rng[rng.c/h<br/>SplitMix64 RNG]
    vec3[vec3.c/h<br/>Vecteurs 3D]
    basis[basis.c/h<br/>Repère Orthonormal]
    geometry[geometry.c/h<br/>Sphère / Cube]
    hg[hg.c/h<br/>Phase HG / Double HG]
    constants[constants.h<br/>Magic Numbers]
    
    %% Simulation core
    sim[simulation.c/h<br/>Boucle Photons + Kahan]
    
    %% Output modules
    sim_csv[simulation_csv.c/h<br/>CSV Mémoire]
    sim_dist[simulation_distribution_csv.c/h<br/>Distribution Angulaire]
    sim_report[simulation_report.c/h<br/>Bilan Console]
    plot[plot_*.c/h<br/>Gnuplot & PNG]
    
    %% Data structures
    cfg[Config]
    sr[SimulationResult]
    dp[DetectedPhoton]
    vn[Vec3]
    rg[Rng]
    
    %% Flow
    simulate --> config
    simulate --> sim
    config --> cfg
    sim --> sr
    sr --> dp
    sim --> geometry
    sim --> hg
    sim --> rng
    sim --> vec3
    sim --> basis
    geometry --> constants
    hg --> constants
    rng --> constants
    sim --> sim_csv
    sim --> sim_dist
    sim --> sim_report
    sim_csv --> plot
    sim_dist --> plot
    sim_report --> plot
    vec3 --> vn
    rng --> rg
    
    %% Styles
    classDef core fill:#e1f5fe,stroke:#01579b
    classDef output fill:#f3e5f5,stroke:#4a148c
    classDef data fill:#fff3e0,stroke:#e65100
    class simulate,config,rng,vec3,basis,geometry,hg,constants,sim core
    class sim_csv,sim_dist,sim_report,plot output
    class cfg,sr,dp,vn,rg data
```

## Structure Répertoires

- **src/** : code source C11 (19 fichiers)
- **include/** : headers publics (16 fichiers)
- **tests/** : tests unitaires (15 suites, 108 tests, runner partagé)
- **output/** : fichiers générés (CSV, PNG, logs)
- **docs/** : documentation (11 fichiers) + screenshots/
- **examples/** : 10 scripts guidés (bash, python, gnuplot)

## Modules Principaux

| Fichier | Responsabilité |
| ------- | -------------- |
| `simulation.c/h` | boucle photons, transport, statistiques Kahan |
| `geometry.c/h` | distances, entrées, cordes (sphère/cube) |
| `hg.c/h` | échantillonnage HG / Double HG |
| `rng.c/h` | SplitMix64 |
| `vec3.c/h` | vecteurs 3D |
| `config.c/h` | parsing, validation config |
| `simulate.c` | entrypoint CLI |
| `simulation_csv.c/h` | export CSV (mémoire + streaming) |
| `simulation_distribution_csv.c/h` | distribution agrégée |
| `simulation_report.c/h` | bilan énergétique console |
| `plot_*.c/h` | génération graphiques Gnuplot |
| `file_path.c/h` | utilitaires chemins |
| `basis.c/h` | repère orthonormal local |

## Modules Utilitaires

### `file_path.c/h`
Utilitaires manipulation chemins fichiers :
- `replace_extension(const char *path, const char *new_ext, char *out, size_t cap)` : remplace extension, gère absence extension, buffer trop petit
- `ensure_dir(const char *path)` : crée répertoires parents récursivement

Tests : `test_file_path.c` (5 tests - extensions, buffers, edge cases)

### `basis.c/h`
Construction repère orthonormal local à partir d'un vecteur direction :
- `make_basis(const Vec3 *dir, Vec3 *u, Vec3 *v, Vec3 *w)` : base (u,v,w) avec w = dir normalisé
- `make_basis_from_z(Vec3 *u, Vec3 *v, Vec3 *w)` : cas spécial dir = +Z (évite singularité)

Tests : `test_basis.c` (2 tests - orthonormalité, cas Z)