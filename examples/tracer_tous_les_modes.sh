#!/usr/bin/env bash
# shellcheck source=examples/_commun.sh
# Objectif : generer les cinq modes de trace a partir d'une distribution neuve.
# Produit : un sweep a 2 000 photons puis cinq PNG (alpha/mu, lineaire/log, phase).
# Ajustez N_PHOTONS, ANGLE et TAU pour choisir le cas a tracer.
set -euo pipefail
source "$(dirname -- "${BASH_SOURCE[0]}")/_commun.sh"

output_dir="${OUTPUT_DIR:-$EXAMPLE_OUTPUT_ROOT/modes_traces}"
make sweep CONFIG_DIR="$EXAMPLE_CONFIG_DIR" GEOMETRY=sphere N_PHOTONS="${N_PHOTONS:-2000}" \
  ANGLES="${ANGLE:-30}" TAUS="${TAU:-1}" OUTPUT_DIR="$output_dir"
distribution=$(find "$output_dir" -type f -name '*.distribution.csv' -print -quit)
config="$(dirname "$distribution")/sweep-config.dat"
test -n "$distribution" && test -f "$config"

for plot_type in distribution distribution-linear distribution-mu distribution-mu-linear phase; do
  make plot DISTRIBUTION="$distribution" CONFIG="$config" \
    PNG="$output_dir/$plot_type.png" PLOT_TYPE="$plot_type"
done
example_summary "Modes de tracé" "$output_dir"
