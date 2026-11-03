#!/usr/bin/env bash
# shellcheck source=examples/_commun.sh
# Objectif : calculer un sweep, le reprendre sans recalcul, puis reconstruire le rapport.
# Produit : huit cas par defaut, snapshots conserves sous output/examples/.
# Le nettoyage ne touche qu'au build/config de cet exemple.
set -euo pipefail
source "$(dirname -- "${BASH_SOURCE[0]}")/_commun.sh"

output_dir="${OUTPUT_DIR:-$EXAMPLE_OUTPUT_ROOT/reprise_sweep}"
geometry="${GEOMETRY:-both}"
photon_count="${N_PHOTONS:-3000}"
angles="${ANGLES:-0 60}"
taus="${TAUS:-0.5 1}"

make sweep-plan CONFIG_DIR="$EXAMPLE_CONFIG_DIR" GEOMETRY="$geometry" N_PHOTONS="$photon_count" ANGLES="$angles" TAUS="$taus" OUTPUT_DIR="$output_dir"
make sweep CONFIG_DIR="$EXAMPLE_CONFIG_DIR" GEOMETRY="$geometry" N_PHOTONS="$photon_count" ANGLES="$angles" TAUS="$taus" OUTPUT_DIR="$output_dir"
make sweep CONFIG_DIR="$EXAMPLE_CONFIG_DIR" RESUME=1 GEOMETRY="$geometry" N_PHOTONS="$photon_count" ANGLES="$angles" TAUS="$taus" OUTPUT_DIR="$output_dir"
make clean CONFIG_DIR="$EXAMPLE_CONFIG_DIR" BUILD_DIR="$EXAMPLE_OUTPUT_ROOT/build"
make clean-sweep-configs CONFIG_DIR="$EXAMPLE_CONFIG_DIR"
example_report "$output_dir"
example_summary "Reprise de sweep" "$output_dir"
