#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
RESULTS_DIR="${ROOT_DIR}/plots"

mkdir -p "${RESULTS_DIR}"

"${BUILD_DIR}/main" --benchmark_out="${RESULTS_DIR}/results.json" --benchmark_out_format=json
