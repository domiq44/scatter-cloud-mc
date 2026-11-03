#!/usr/bin/env bash
# shellcheck source=examples/_commun.sh
# Objectif : mesurer l'effet de N sur les distributions et la reflectance.
# Produit : six cas par defaut (N=1000 et N=5000), avec rapport filtrable.
# Ajustez PHOTON_COUNTS, ANGLES, TAUS et GEOMETRY.
set -euo pipefail
source "$(dirname -- "${BASH_SOURCE[0]}")/_commun.sh"

output_dir="${RUN_DIR:-${OUTPUT_DIR:-$(example_run_dir tailles_echantillon)}}"
angles="${ANGLES:-0 60 90}"
taus="${TAUS:-1}"
read -r -a photon_counts <<< "${PHOTON_COUNTS:-1000 5000}"
for photon_count in "${photon_counts[@]}"; do
  make sweep CONFIG_DIR="$EXAMPLE_CONFIG_DIR" GEOMETRY="${GEOMETRY:-sphere}" N_PHOTONS="$photon_count" \
    ANGLES="$angles" TAUS="$taus" OUTPUT_DIR="$output_dir"
done
example_report "$output_dir"
example_summary "Comparaison des tailles d'échantillon" "$output_dir"
