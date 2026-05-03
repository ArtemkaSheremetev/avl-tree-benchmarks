#!/usr/bin/env bash
set -e

echo '--- Проверка наличия perf и FlameGraph ---'
if ! command -v perf &>/dev/null; then
  echo 'Error: perf not found. Install it with: sudo apt install linux-tools-common linux-tools-$(uname -r)'
  exit 1
fi
if [ ! -d /home/artem/FlameGraph ]; then
  echo 'Error: FlameGraph scripts not found in /home/artem/FlameGraph'
  echo 'Clone with: git clone https://github.com/brendangregg/FlameGraph.git ~/FlameGraph'
  exit 1
fi

echo '--- Очистка старых данных perf ---'
rm -f perf.data perf.out perf.folded /home/artem/study/itmo_c++/lab1/plots/flamegraph.svg

echo '--- Запуск perf record ---'
perf record -F 999 -g -o perf.data -- ./main --benchmark_filter=BM_AVL_Find_Existing || true

echo '--- Генерация flamegraph.svg ---'
sudo chown $(whoami):$(whoami) perf.data || true
perf script -f > perf.out
/home/artem/FlameGraph/stackcollapse-perf.pl perf.out > perf.folded
/home/artem/FlameGraph/flamegraph.pl perf.folded > /home/artem/study/itmo_c++/lab1/plots/flamegraph.svg

echo '✅ Flamegraph готов: /home/artem/study/itmo_c++/lab1/plots/flamegraph.svg'
