#!/usr/bin/env bash
# shellcheck source=examples/_commun.sh
# Objectif : comparer absorption et fonctions HG/double-HG.
# Produit : six simulations et leurs CSV/PNG; albedos 0, 0.5, 1 par defaut.
# Ajustez ALBEDOS, PHASE_FUNCTIONS, N_PHOTONS et BASE_CONFIG.
set -euo pipefail
source "$(dirname -- "${BASH_SOURCE[0]}")/_commun.sh"

output_dir="${OUTPUT_DIR:-$EXAMPLE_OUTPUT_ROOT/albedo_phase}"
config_dir="$EXAMPLE_CONFIG_DIR/albedo_phase"
source_config="${BASE_CONFIG:-config/config.dat}"
photon_count="${N_PHOTONS:-2000}"
read -r -a albedos <<< "${ALBEDOS:-0 0.5 1}"
read -r -a phase_functions <<< "${PHASE_FUNCTIONS:-double_hg hg}"
mkdir -p "$config_dir"

write_variant() {
  local name=$1
  local albedo=$2
  local phase=$3
  local config_file="$config_dir/$name.dat"

  if [[ "$phase" == "hg" ]]; then
    sed -E \
      -e "s/^[[:space:]]*n_photons[[:space:]]*=.*/n_photons = $photon_count/" \
      -e "s/^[[:space:]]*albedo[[:space:]]*=.*/albedo = $albedo/" \
      -e 's/^[[:space:]]*phase_function[[:space:]]*=.*/phase_function = hg/' \
      -e '/^[[:space:]]*g2[[:space:]]*=/d' \
      -e '/^[[:space:]]*hg_fraction[[:space:]]*=/d' \
      "$source_config" > "$config_file"
  else
    sed -E \
      -e "s/^[[:space:]]*n_photons[[:space:]]*=.*/n_photons = $photon_count/" \
      -e "s/^[[:space:]]*albedo[[:space:]]*=.*/albedo = $albedo/" \
      "$source_config" > "$config_file"
  fi
  make run-one CONFIG="$config_file" NAME="$name" OUTPUT_DIR="$output_dir"
}

for phase in "${phase_functions[@]}"; do
  for albedo in "${albedos[@]}"; do
    name="${phase}-albedo${albedo//./p}"
    write_variant "$name" "$albedo" "$phase"
  done
done
example_summary "Albédo et fonction de phase" "$output_dir"
