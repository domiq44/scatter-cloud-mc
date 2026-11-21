#!/usr/bin/env bash
# comparer_geometries.sh - Comparaison sphère vs cube aux mêmes conditions
# Produit 6 cas par défaut (2 géométries × 3 angles) avec CSV, PNG, snapshots, rapport HTML.
# Paramètres ajustables : GEOMETRY, N_PHOTONS, ANGLES, TAUS.
#
# Partie de scatter-cloud-mc
# shellcheck source=examples/_commun.sh
set -euo pipefail
source "$(dirname -- "${BASH_SOURCE[0]}")/_commun.sh"

output_dir="${RUN_DIR:-${OUTPUT_DIR:-$(example_run_dir comparer_geometries)}}"
geometry="${GEOMETRY:-both}"
photon_count="${N_PHOTONS:-3000}"
angles="${ANGLES:-0 60 90}"
taus="${TAUS:-1}"

make sweep-plan CONFIG_DIR="$EXAMPLE_CONFIG_DIR" GEOMETRY="$geometry" N_PHOTONS="$photon_count" ANGLES="$angles" TAUS="$taus" OUTPUT_DIR="$output_dir"
example_run "$output_dir" "Sweep de comparaison des géométries" make sweep \
	CONFIG_DIR="$EXAMPLE_CONFIG_DIR" GEOMETRY="$geometry" N_PHOTONS="$photon_count" \
	ANGLES="$angles" TAUS="$taus" OUTPUT_DIR="$output_dir"
example_report "$output_dir"
example_summary "Comparaison des géométries" "$output_dir"
