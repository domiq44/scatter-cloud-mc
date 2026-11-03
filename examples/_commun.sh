#!/usr/bin/env bash
set -euo pipefail

EXAMPLES_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd -- "$EXAMPLES_DIR/.." && pwd)"
cd "$PROJECT_ROOT"

EXAMPLE_OUTPUT_ROOT="${EXAMPLE_OUTPUT_ROOT:-output/examples}"
EXAMPLE_CONFIG_DIR="${EXAMPLE_CONFIG_DIR:-$EXAMPLE_OUTPUT_ROOT/configs}"
EXAMPLE_WEB_ASSET_PREFIX="${EXAMPLE_WEB_ASSET_PREFIX:-../../../web}"
EXAMPLE_SCRIPT_SOURCE="${BASH_SOURCE[1]:-${BASH_SOURCE[0]}}"
EXAMPLE_SCRIPT_NAME="$(basename -- "$EXAMPLE_SCRIPT_SOURCE" .sh)"
mkdir -p "$EXAMPLE_CONFIG_DIR"
if [[ ! -f "$EXAMPLE_CONFIG_DIR/config.template.dat" ]]; then
  cp config/config.template.dat "$EXAMPLE_CONFIG_DIR/config.template.dat"
fi

example_run_dir() {
  local name=$1
  if [[ -n "${RUN_DIR:-}" ]]; then
    printf '%s' "$RUN_DIR"
  elif [[ -n "${OUTPUT_DIR:-}" ]]; then
    printf '%s' "$OUTPUT_DIR"
  else
    printf '%s/%s-%s-%s' "$EXAMPLE_OUTPUT_ROOT" "$name" "$(date +%Y%m%d-%H%M%S)" "$$"
  fi
}

example_run() {
  local output_dir=$1
  local step_name=$2
  local command_status=0
  shift 2
  local log_file="$output_dir/$EXAMPLE_SCRIPT_NAME.log"

  mkdir -p "$output_dir"
  printf 'Étape : %s\n' "$step_name"
  if [[ "${VERBOSE:-0}" == 1 ]]; then
    "$@" 2>&1 | tee -a "$log_file" || command_status=$?
  else
    "$@" >> "$log_file" 2>&1 || command_status=$?
  fi

  if [[ "$command_status" -ne 0 ]]; then
    printf "Echec de l'etape '%s' (code %s). Journal : %s\n" \
      "$step_name" "$command_status" "$log_file" >&2
    tail -n 40 "$log_file" >&2 || true
    return "$command_status"
  fi
}

example_report() {
  local output_dir=$1
  local web_asset_prefix=${WEB_ASSET_PREFIX:-$EXAMPLE_WEB_ASSET_PREFIX}
  example_run "$output_dir" "Générer le rapport HTML" make report \
    OUTPUT_DIR="$output_dir" CONFIG_DIR="$EXAMPLE_CONFIG_DIR" \
    WEB_ASSET_PREFIX="$web_asset_prefix"
}

example_summary() {
  local example_name=$1
  local output_dir=$2
  local report_file="$output_dir/report_interactif.html"
  local csv_count png_count

  if [[ ! -d "$output_dir" ]]; then
    printf 'Erreur : aucun dossier de résultats pour %s (%s).\n' "$example_name" "$output_dir" >&2
    return 1
  fi

  csv_count=$(find "$output_dir" -type f -name '*.csv' -print | wc -l | tr -d '[:space:]')
  png_count=$(find "$output_dir" -type f -name '*.png' -print | wc -l | tr -d '[:space:]')
  if [[ "$csv_count" -eq 0 && "$png_count" -eq 0 && ! -f "$report_file" ]]; then
    printf 'Erreur : aucun artefact produit pour %s dans %s.\n' "$example_name" "$output_dir" >&2
    return 1
  fi

  printf '\nTerminé : %s\nRésultats : %s\nArtefacts : %s CSV, %s PNG\nJournal : %s\n' \
    "$example_name" "$output_dir" "$csv_count" "$png_count" \
    "$output_dir/$EXAMPLE_SCRIPT_NAME.log"
  if [[ -f "$report_file" ]]; then
    printf 'Rapport HTML : %s\n' "$report_file"
  else
    printf 'Rapport HTML : non généré par ce parcours\n'
  fi
}
