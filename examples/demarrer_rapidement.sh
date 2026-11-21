#!/usr/bin/env bash
# demarrer_rapidement.sh - Lancement galerie interactive sphère (premiers pas)
# Produit 9 cas (3 angles × 3 τ) avec distributions, phases et rapport HTML.
# Décharge par défaut : 2000 photons/cas; QUICK_* ajuste le profil.
#
# Partie de scatter-cloud-mc
# shellcheck source=examples/_commun.sh
set -euo pipefail
source "$(dirname -- "${BASH_SOURCE[0]}")/_commun.sh"

quick_output_dir="${QUICK_OUTPUT_DIR:-$(example_run_dir demarrage_rapide)}"
example_run "$quick_output_dir" "Sweep rapide et rapport HTML" make quick \
  GEOMETRY="${GEOMETRY:-sphere}" \
  QUICK_N_PHOTONS="${QUICK_N_PHOTONS:-2000}" \
  QUICK_ANGLES="${QUICK_ANGLES:-0 60 90}" \
  QUICK_TAUS="${QUICK_TAUS:-0.1 1 10}" \
  CONFIG_DIR="$EXAMPLE_CONFIG_DIR" \
  QUICK_OUTPUT_DIR="$quick_output_dir" \
  QUICK_WEB_ASSET_PREFIX="${WEB_ASSET_PREFIX:-$EXAMPLE_WEB_ASSET_PREFIX}"
example_summary "Démarrage rapide" "$quick_output_dir"
