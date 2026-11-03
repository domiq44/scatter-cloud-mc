#!/usr/bin/env bash
# shellcheck source=examples/_commun.sh
# Objectif : comparer sphere et cube aux memes angles et tau.
# Produit : six cas par defaut, CSV, PNG, snapshots et rapport HTML.
# Ajustez GEOMETRY, N_PHOTONS, ANGLES et TAUS sans modifier le script.
set -euo pipefail
source "$(dirname -- "${BASH_SOURCE[0]}")/_commun.sh"

output_dir="${OUTPUT_DIR:-$EXAMPLE_OUTPUT_ROOT/comparer_geometries}"
geometry="${GEOMETRY:-both}"
photon_count="${N_PHOTONS:-3000}"
angles="${ANGLES:-0 60 90}"
taus="${TAUS:-1}"

make sweep-plan CONFIG_DIR="$EXAMPLE_CONFIG_DIR" GEOMETRY="$geometry" N_PHOTONS="$photon_count" ANGLES="$angles" TAUS="$taus" OUTPUT_DIR="$output_dir"
make sweep CONFIG_DIR="$EXAMPLE_CONFIG_DIR" GEOMETRY="$geometry" N_PHOTONS="$photon_count" ANGLES="$angles" TAUS="$taus" OUTPUT_DIR="$output_dir"
example_report "$output_dir"
