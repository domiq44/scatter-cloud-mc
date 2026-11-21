#!/usr/bin/env bash
# etudier_cube_oblique.sh - Cube en incidence oblique et rasante
# Produit 8 cas par défaut (4 angles × 2 τ) avec distributions, phases, rapport.
# Paramètres ajustables : N_PHOTONS, ANGLES, TAUS.
#
# Partie de scatter-cloud-mc
# shellcheck source=examples/_commun.sh
set -euo pipefail
source "$(dirname -- "${BASH_SOURCE[0]}")/_commun.sh"

output_dir="${RUN_DIR:-${OUTPUT_DIR:-$(example_run_dir cube_oblique)}}"
example_run "$output_dir" "Sweep cube en incidence oblique" make sweep \
  CONFIG_DIR="$EXAMPLE_CONFIG_DIR" GEOMETRY=cube N_PHOTONS="${N_PHOTONS:-3000}" \
  ANGLES="${ANGLES:-30 60 80 90}" TAUS="${TAUS:-0.5 2}" OUTPUT_DIR="$output_dir"
example_report "$output_dir"
example_summary "Cube en incidence oblique" "$output_dir"
