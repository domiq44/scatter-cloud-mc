#!/usr/bin/env bash
# explorer_grille.sh - Exploration grille angles × tau pour une géométrie
# Produit 12 cas par défaut (4 angles × 3 τ) avec distributions et rapport HTML.
# Paramètres ajustables : GEOMETRY, N_PHOTONS, ANGLES, TAUS.
#
# Partie de scatter-cloud-mc
# shellcheck source=examples/_commun.sh
set -euo pipefail
source "$(dirname -- "${BASH_SOURCE[0]}")/_commun.sh"

output_dir="${RUN_DIR:-${OUTPUT_DIR:-$(example_run_dir explorer_grille)}}"
geometry="${GEOMETRY:-sphere}"
photon_count="${N_PHOTONS:-3000}"
angles="${ANGLES:-0 30 60 90}"
taus="${TAUS:-0.1 1 10}"

make sweep-plan CONFIG_DIR="$EXAMPLE_CONFIG_DIR" GEOMETRY="$geometry" N_PHOTONS="$photon_count" ANGLES="$angles" TAUS="$taus" OUTPUT_DIR="$output_dir"
example_run "$output_dir" "Sweep angles/tau" make sweep \
	CONFIG_DIR="$EXAMPLE_CONFIG_DIR" GEOMETRY="$geometry" N_PHOTONS="$photon_count" \
	ANGLES="$angles" TAUS="$taus" OUTPUT_DIR="$output_dir"
example_report "$output_dir"
example_summary "Exploration angles/tau" "$output_dir"
