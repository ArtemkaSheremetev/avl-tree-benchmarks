#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
RESULTS_DIR="${ROOT_DIR}/plots"

mkdir -p "${RESULTS_DIR}"

if ! command -v cpupower >/dev/null 2>&1; then
    echo "Error: cpupower not found. Install it with: sudo apt install linux-tools-common linux-tools-\$(uname -r)"
    exit 1
fi

cleanup() {
    sudo cpupower frequency-set -g powersave >/dev/null 2>&1 || true
    if [[ -f /sys/devices/system/cpu/intel_pstate/no_turbo ]]; then
        echo 0 | sudo tee /sys/devices/system/cpu/intel_pstate/no_turbo >/dev/null || true
    fi
}

trap cleanup EXIT

echo "--- Enabling performance mode and disabling turbo ---"
sudo cpupower frequency-set -g performance
if [[ -f /sys/devices/system/cpu/intel_pstate/no_turbo ]]; then
    echo 1 | sudo tee /sys/devices/system/cpu/intel_pstate/no_turbo >/dev/null
fi

echo "--- Running benchmarks ---"
"${BUILD_DIR}/main" --benchmark_out="${RESULTS_DIR}/results.json" --benchmark_out_format=json
