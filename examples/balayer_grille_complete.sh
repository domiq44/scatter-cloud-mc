#!/usr/bin/env bash
# shellcheck source=examples/_commun.sh
# Objectif : couvrir toutes les geometries, angles et valeurs tau du projet.
# Produit : 182 cas a 1 000 photons par defaut, avec galerie complete.
# Sweep couteux : reduisez N_PHOTONS, ANGLES, TAUS ou GEOMETRY pour explorer.
set -euo pipefail
source "$(dirname -- "${BASH_SOURCE[0]}")/_commun.sh"

geometry="${GEOMETRY:-both}"
photon_count="${N_PHOTONS:-1000}"
angles="${ANGLES:-0 30 60 70 80 85 90}"
taus="${TAUS:-0.01 0.02 0.05 0.1 0.2 0.5 1 2 5 10 20 50 100}"
output_dir="${RUN_DIR:-${OUTPUT_DIR:-$(example_run_dir grille_complete)}}"
if [[ $# -ne 0 ]]; then
	printf 'Aucun argument requis; les paramètres se règlent avec GEOMETRY, N_PHOTONS, ANGLES et TAUS.\n' >&2
	exit 2
fi

make sweep-plan CONFIG_DIR="$EXAMPLE_CONFIG_DIR" GEOMETRY="$geometry" N_PHOTONS="$photon_count" ANGLES="$angles" TAUS="$taus" OUTPUT_DIR="$output_dir"
example_run "$output_dir" "Sweep complet" make sweep \
	CONFIG_DIR="$EXAMPLE_CONFIG_DIR" GEOMETRY="$geometry" N_PHOTONS="$photon_count" \
	ANGLES="$angles" TAUS="$taus" RESUME="${RESUME:-0}" OUTPUT_DIR="$output_dir"
example_report "$output_dir"
example_summary "Grille complète" "$output_dir"
