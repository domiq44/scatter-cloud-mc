#!/usr/bin/env bash
# shellcheck source=examples/_commun.sh
# Objectif : observer le cube en incidence oblique et rasante.
# Produit : huit cas par defaut, distributions, phases et rapport.
# Ajustez N_PHOTONS, ANGLES et TAUS pour les autres regimes.
set -euo pipefail
source "$(dirname -- "${BASH_SOURCE[0]}")/_commun.sh"

output_dir="${OUTPUT_DIR:-$EXAMPLE_OUTPUT_ROOT/cube_oblique}"
make sweep CONFIG_DIR="$EXAMPLE_CONFIG_DIR" GEOMETRY=cube N_PHOTONS="${N_PHOTONS:-3000}" \
  ANGLES="${ANGLES:-30 60 80 90}" TAUS="${TAUS:-0.5 2}" OUTPUT_DIR="$output_dir"
example_report "$output_dir"
