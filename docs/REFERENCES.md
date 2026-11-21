# Références - scatter-cloud-mc

## Bibliographie

- **Hu & Stamnes (1993)** - *A parameterization of the radiative properties of water clouds*
  - Paramétrisation Mie pour nuages d'eau
  - Utilisé pour : phase functions, albédo, asymétrie g
  - DOI: 10.1175/1520-0442(1993)006<0728:AAPOTR>2.0.CO;2

## Standards & Outils

- **C11** (ISO/IEC 9899:2011) - Standard langage
- **GNU Make** - Build system
- **clang-format / clang-tidy / cppcheck** - Qualité code (CI)
- **Gnuplot** - Visualisation (optionnel)
- **Python 3.8+** - Scripts auxiliaires (optionnel)

## Algorithmes Implémentés

- **SplitMix64** (Vigna, 2015) - RNG, période 2⁶⁴, BigCrush
- **Henyey-Greenstein** (1941) - Phase function analytique
- **Kahan Summation** (1965) - Compensation erreur flottante
- **Russian Roulette** (Kahn, 1950) - Terminaison photons poids faible sans biais