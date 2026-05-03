#!/usr/bin/env bash
set -e
echo '--- Running benchmarks (normal mode) ---'
./main --benchmark_out=../plots/results.json --benchmark_out_format=json
