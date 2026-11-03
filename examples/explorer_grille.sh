#!/usr/bin/env bash
# shellcheck source=examples/_commun.sh
# Objectif : explorer une petite grille angles x tau pour une geometrie.
# Produit : douze cas par defaut, toutes les distributions et un rapport.
# Ajustez GEOMETRY, N_PHOTONS, ANGLES et TAUS dans l'environnement.
set -euo pipefail
source "$(dirname -- "${BASH_SOURCE[0]}")/_commun.sh"

output_dir="${OUTPUT_DIR:-$EXAMPLE_OUTPUT_ROOT/explorer_grille}"
geometry="${GEOMETRY:-sphere}"
photon_count="${N_PHOTONS:-3000}"
angles="${ANGLES:-0 30 60 90}"
taus="${TAUS:-0.1 1 10}"

make sweep-plan CONFIG_DIR="$EXAMPLE_CONFIG_DIR" GEOMETRY="$geometry" N_PHOTONS="$photon_count" ANGLES="$angles" TAUS="$taus" OUTPUT_DIR="$output_dir"
make sweep CONFIG_DIR="$EXAMPLE_CONFIG_DIR" GEOMETRY="$geometry" N_PHOTONS="$photon_count" ANGLES="$angles" TAUS="$taus" OUTPUT_DIR="$output_dir"
example_report "$output_dir"
