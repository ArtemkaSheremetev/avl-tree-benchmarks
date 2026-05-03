#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
RESULTS_DIR="${ROOT_DIR}/plots"
FLAMEGRAPH_DIR="${FLAMEGRAPH_DIR:-${HOME}/FlameGraph}"
OUTPUT_FILE="${RESULTS_DIR}/flamegraph.svg"

mkdir -p "${RESULTS_DIR}"

if ! command -v perf >/dev/null 2>&1; then
    echo "Error: perf not found. Install it with: sudo apt install linux-tools-common linux-tools-\$(uname -r)"
    exit 1
fi

if [[ ! -d "${FLAMEGRAPH_DIR}" ]]; then
    echo "Error: FlameGraph scripts not found in ${FLAMEGRAPH_DIR}"
    echo "Clone them with: git clone https://github.com/brendangregg/FlameGraph.git \"${FLAMEGRAPH_DIR}\""
    exit 1
fi

cd "${BUILD_DIR}"
rm -f perf.data perf.out perf.folded "${OUTPUT_FILE}"

echo "--- Running perf record ---"
perf record -F 999 -g -o perf.data -- ./main --benchmark_filter=BM_AVL_Find_Existing || true

echo "--- Building flamegraph ---"
sudo chown "$(whoami):$(whoami)" perf.data || true
perf script -f > perf.out
"${FLAMEGRAPH_DIR}/stackcollapse-perf.pl" perf.out > perf.folded
"${FLAMEGRAPH_DIR}/flamegraph.pl" perf.folded > "${OUTPUT_FILE}"

echo "Flamegraph saved to ${OUTPUT_FILE}"
