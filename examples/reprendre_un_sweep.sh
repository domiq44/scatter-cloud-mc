#!/usr/bin/env bash
# reprendre_un_sweep.sh - Démonstration RESUME=1 : calcul, reprise, rapport après clean
# Produit 8 cas (2 géométries × 2 angles × 2 τ) avec snapshots conservés.
# Nettoyage ciblé : build/config de l'exemple seulement.
#
# Partie de scatter-cloud-mc
# shellcheck source=examples/_commun.sh
set -euo pipefail
source "$(dirname -- "${BASH_SOURCE[0]}")/_commun.sh"

output_dir="${RUN_DIR:-${OUTPUT_DIR:-$(example_run_dir reprise_sweep)}}"
geometry="${GEOMETRY:-both}"
photon_count="${N_PHOTONS:-3000}"
angles="${ANGLES:-0 60}"
taus="${TAUS:-0.5 1}"

make sweep-plan CONFIG_DIR="$EXAMPLE_CONFIG_DIR" GEOMETRY="$geometry" N_PHOTONS="$photon_count" ANGLES="$angles" TAUS="$taus" OUTPUT_DIR="$output_dir"
example_run "$output_dir" "Premier sweep" make sweep \
	CONFIG_DIR="$EXAMPLE_CONFIG_DIR" GEOMETRY="$geometry" N_PHOTONS="$photon_count" \
	ANGLES="$angles" TAUS="$taus" RESUME="${RESUME:-0}" OUTPUT_DIR="$output_dir"
example_run "$output_dir" "Reprise du sweep" make sweep \
	CONFIG_DIR="$EXAMPLE_CONFIG_DIR" RESUME=1 GEOMETRY="$geometry" N_PHOTONS="$photon_count" \
	ANGLES="$angles" TAUS="$taus" OUTPUT_DIR="$output_dir"
make clean CONFIG_DIR="$EXAMPLE_CONFIG_DIR" BUILD_DIR="$EXAMPLE_OUTPUT_ROOT/build"
make clean-sweep-configs CONFIG_DIR="$EXAMPLE_CONFIG_DIR"
example_report "$output_dir"
example_summary "Reprise de sweep" "$output_dir"
