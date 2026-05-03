#!/usr/bin/env bash
set -e
echo '--- Disabling Turbo Boost and enabling performance mode ---'
if ! command -v cpupower &>/dev/null; then
  echo 'Error: cpupower not found. Install it with: sudo apt install linux-tools-common linux-tools-$(uname -r)'
  exit 1
fi
sudo cpupower frequency-set -g performance
sudo sh -c 'echo 1 > /sys/devices/system/cpu/intel_pstate/no_turbo'
echo '--- Running benchmarks ---'
./main --benchmark_out=../plots/results.json --benchmark_out_format=json
echo '--- Restoring Turbo Boost and powersave governor ---'
sudo cpupower frequency-set -g powersave
sudo sh -c 'echo 0 > /sys/devices/system/cpu/intel_pstate/no_turbo'
