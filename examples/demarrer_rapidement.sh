#!/usr/bin/env bash
# shellcheck source=examples/_commun.sh
# Objectif : lancer une premiere galerie interactive avec la sphere.
# Produit : neuf cas, distributions, phases et rapport sous output/examples/.
# Charge par defaut : 2 000 photons par cas; QUICK_* ajuste le profil.
set -euo pipefail
source "$(dirname -- "${BASH_SOURCE[0]}")/_commun.sh"

quick_output_dir="${QUICK_OUTPUT_DIR:-${OUTPUT_DIR:-$EXAMPLE_OUTPUT_ROOT/demarrage_rapide}}"
make quick \
  GEOMETRY="${GEOMETRY:-sphere}" \
  QUICK_N_PHOTONS="${QUICK_N_PHOTONS:-2000}" \
  QUICK_ANGLES="${QUICK_ANGLES:-0 60 90}" \
  QUICK_TAUS="${QUICK_TAUS:-0.1 1 10}" \
  CONFIG_DIR="$EXAMPLE_CONFIG_DIR" \
  QUICK_OUTPUT_DIR="$quick_output_dir" \
  QUICK_WEB_ASSET_PREFIX="${WEB_ASSET_PREFIX:-$EXAMPLE_WEB_ASSET_PREFIX}"
