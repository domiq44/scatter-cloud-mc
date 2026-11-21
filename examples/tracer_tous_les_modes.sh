#!/usr/bin/env bash
# tracer_tous_les_modes.sh - Génère les 5 modes de tracé à partir d'une distribution
# Produit 5 PNG (distribution, distribution-linear, distribution-mu, distribution-mu-linear, phase).
# Paramètres ajustables : N_PHOTONS, ANGLE, TAU.
#
# Partie de scatter-cloud-mc
# shellcheck source=examples/_commun.sh
set -euo pipefail
source "$(dirname -- "${BASH_SOURCE[0]}")/_commun.sh"

output_dir="${RUN_DIR:-${OUTPUT_DIR:-$(example_run_dir modes_traces)}}"
example_run "$output_dir" "Simulation de la distribution" make sweep \
  CONFIG_DIR="$EXAMPLE_CONFIG_DIR" GEOMETRY=sphere N_PHOTONS="${N_PHOTONS:-2000}" \
  ANGLES="${ANGLE:-30}" TAUS="${TAU:-1}" OUTPUT_DIR="$output_dir"
distribution=$(find "$output_dir" -type f -name '*.distribution.csv' -print -quit)
config="$(dirname "$distribution")/sweep-config.dat"
test -n "$distribution" && test -f "$config"

for plot_type in distribution distribution-linear distribution-mu distribution-mu-linear phase; do
  example_run "$output_dir" "Tracé $plot_type" make plot \
    DISTRIBUTION="$distribution" CONFIG="$config" \
    PNG="$output_dir/$plot_type.png" PLOT_TYPE="$plot_type"
done
example_summary "Modes de tracé" "$output_dir"
