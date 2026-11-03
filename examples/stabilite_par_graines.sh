#!/usr/bin/env bash
# shellcheck source=examples/_commun.sh
# Objectif : comparer un resultat graphique aux residus de plusieurs graines.
# Produit : un run representatif, N_RUNS residus et leur analyse Python.
# Ajustez N_PHOTONS et N_RUNS; Python 3 est requis pour l'analyse.
set -euo pipefail
source "$(dirname -- "${BASH_SOURCE[0]}")/_commun.sh"

output_dir="${RUN_DIR:-${OUTPUT_DIR:-$(example_run_dir stabilite_graines)}}"
photon_count="${N_PHOTONS:-3000}"
config_file="$output_dir/config_stabilite.dat"
mkdir -p "$output_dir"
sed -E "s/^[[:space:]]*n_photons[[:space:]]*=.*/n_photons = $photon_count/" \
	config/config.dat > "$config_file"
example_run "$output_dir" "Simulation representative" make run-one \
	CONFIG="$config_file" NAME=representative OUTPUT_DIR="$output_dir"
example_run "$output_dir" "Résidus sur ${N_RUNS:-3} graines" make run-many \
	CONFIG="$config_file" N_RUNS="${N_RUNS:-3}" OUTPUT_DIR="$output_dir"
example_run "$output_dir" "Analyse des résidus" python3 analyse_diff.py \
	--input "$output_dir/diffs.txt" --config "$config_file"
example_summary "Stabilité par graines" "$output_dir"
