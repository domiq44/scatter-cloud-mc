#!/usr/bin/env bash
set -euo pipefail

EXAMPLES_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd -- "$EXAMPLES_DIR/.." && pwd)"
cd "$PROJECT_ROOT"

EXAMPLE_OUTPUT_ROOT="${EXAMPLE_OUTPUT_ROOT:-output/examples}"
EXAMPLE_CONFIG_DIR="${EXAMPLE_CONFIG_DIR:-$EXAMPLE_OUTPUT_ROOT/configs}"
EXAMPLE_WEB_ASSET_PREFIX="${EXAMPLE_WEB_ASSET_PREFIX:-../../../web}"
mkdir -p "$EXAMPLE_CONFIG_DIR"
if [[ ! -f "$EXAMPLE_CONFIG_DIR/config.template.dat" ]]; then
  cp config/config.template.dat "$EXAMPLE_CONFIG_DIR/config.template.dat"
fi

example_report() {
  local output_dir=$1
  local web_asset_prefix=${WEB_ASSET_PREFIX:-$EXAMPLE_WEB_ASSET_PREFIX}
  make report OUTPUT_DIR="$output_dir" CONFIG_DIR="$EXAMPLE_CONFIG_DIR" WEB_ASSET_PREFIX="$web_asset_prefix"
}
