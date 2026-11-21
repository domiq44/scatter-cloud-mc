#!/usr/bin/env bash
# comparer_tailles_echantillon.sh - Effet du nombre de photons sur résultats
# Produit 6 cas par défaut (2 N × 3 angles) avec rapport HTML filtrable par N.
# Paramètres ajustables : PHOTON_COUNTS, ANGLES, TAUS, GEOMETRY.
#
# Partie de scatter-cloud-mc
# shellcheck source=examples/_commun.sh
set -euo pipefail
source "$(dirname -- "${BASH_SOURCE[0]}")/_commun.sh"

output_dir="${RUN_DIR:-${OUTPUT_DIR:-$(example_run_dir tailles_echantillon)}}"
angles="${ANGLES:-0 60 90}"
taus="${TAUS:-1}"
read -r -a photon_counts <<< "${PHOTON_COUNTS:-1000 5000}"
for photon_count in "${photon_counts[@]}"; do
  example_run "$output_dir" "Sweep avec N=$photon_count" make sweep \
    CONFIG_DIR="$EXAMPLE_CONFIG_DIR" GEOMETRY="${GEOMETRY:-sphere}" N_PHOTONS="$photon_count" \
    ANGLES="$angles" TAUS="$taus" OUTPUT_DIR="$output_dir"
done
example_report "$output_dir"
example_summary "Comparaison des tailles d'échantillon" "$output_dir"
