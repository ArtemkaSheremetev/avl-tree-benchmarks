cmake_minimum_required(VERSION 3.14)
project(Lab1)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")  # Без оптимизаций
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-inline")  # Запрет inline
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer")  # Сохранять frame pointer

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)

include(FetchContent)

FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.0
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

FetchContent_GetProperties(benchmark)
if(NOT benchmark_POPULATED)
    message(STATUS "Downloading Google Benchmark")
    FetchContent_Populate(benchmark)
    add_subdirectory(${benchmark_SOURCE_DIR} ${benchmark_BINARY_DIR})
endif()

add_executable(main 
    src/main.cpp
    src/AVL.cpp
)

target_include_directories(main PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(main benchmark::benchmark)

# --- Скрипт для запуска бенчмарков с фиксированной частотой ---
set(BENCH_SCRIPT "${CMAKE_BINARY_DIR}/run_benchmarks_with_fixed_freq.sh")

file(WRITE ${BENCH_SCRIPT} "#!/usr/bin/env bash\n")
file(APPEND ${BENCH_SCRIPT} "set -e\n")
file(APPEND ${BENCH_SCRIPT} "echo '--- Disabling Turbo Boost and enabling performance mode ---'\n")
file(APPEND ${BENCH_SCRIPT} "if ! command -v cpupower &>/dev/null; then\n")
file(APPEND ${BENCH_SCRIPT} "  echo 'Error: cpupower not found. Install it with: sudo apt install linux-tools-common linux-tools-$(uname -r)'\n")
file(APPEND ${BENCH_SCRIPT} "  exit 1\n")
file(APPEND ${BENCH_SCRIPT} "fi\n")
file(APPEND ${BENCH_SCRIPT} "sudo cpupower frequency-set -g performance\n")
file(APPEND ${BENCH_SCRIPT} "sudo sh -c 'echo 1 > /sys/devices/system/cpu/intel_pstate/no_turbo'\n")

file(APPEND ${BENCH_SCRIPT} "echo '--- Running benchmarks ---'\n")
file(APPEND ${BENCH_SCRIPT} "./main --benchmark_out=../plots/results.json --benchmark_out_format=json\n")

file(APPEND ${BENCH_SCRIPT} "echo '--- Restoring Turbo Boost and powersave governor ---'\n")
file(APPEND ${BENCH_SCRIPT} "sudo cpupower frequency-set -g powersave\n")
file(APPEND ${BENCH_SCRIPT} "sudo sh -c 'echo 0 > /sys/devices/system/cpu/intel_pstate/no_turbo'\n")

file(COPY ${BENCH_SCRIPT} DESTINATION ${CMAKE_BINARY_DIR})
execute_process(COMMAND chmod +x ${BENCH_SCRIPT})

# Цель для запуска бенчмарков с управлением частотой
add_custom_target(run_benchmarks
    COMMAND ${BENCH_SCRIPT}
    DEPENDS main
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running benchmarks with fixed CPU frequency (no turbo)..."
)

# --- Скрипт для обычного запуска (без управления частотой) ---
set(SIMPLE_BENCH_SCRIPT "${CMAKE_BINARY_DIR}/run_benchmarks_simple.sh")

file(WRITE ${SIMPLE_BENCH_SCRIPT} "#!/usr/bin/env bash\n")
file(APPEND ${SIMPLE_BENCH_SCRIPT} "set -e\n")
file(APPEND ${SIMPLE_BENCH_SCRIPT} "echo '--- Running benchmarks (normal mode) ---'\n")
file(APPEND ${SIMPLE_BENCH_SCRIPT} "./main --benchmark_out=../plots/results.json --benchmark_out_format=json\n")

file(COPY ${SIMPLE_BENCH_SCRIPT} DESTINATION ${CMAKE_BINARY_DIR})
execute_process(COMMAND chmod +x ${SIMPLE_BENCH_SCRIPT})

# Цель для обычного запуска
add_custom_target(run_simple
    COMMAND ${SIMPLE_BENCH_SCRIPT}
    DEPENDS main
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running benchmarks in normal mode..."
)

find_package(Python3 REQUIRED COMPONENTS Interpreter)
set(RESULTS_DIR "${CMAKE_SOURCE_DIR}/plots")
file(MAKE_DIRECTORY ${RESULTS_DIR})

set(RESULTS_JSON "${RESULTS_DIR}/results.json")
set(PYTHON_SCRIPT "${CMAKE_SOURCE_DIR}/pyscript/graphics.py")

add_custom_target(analyze
    COMMAND ${CMAKE_COMMAND} -E echo "Запуск Benchmark с отключением Turbo Boost"
    COMMAND ${CMAKE_COMMAND} -E make_directory ${RESULTS_DIR}
    COMMAND ${BENCH_SCRIPT}
    COMMAND ${CMAKE_COMMAND} -E echo "Запуск Python-скрипта визуализации"
    COMMAND ${Python3_EXECUTABLE} ${PYTHON_SCRIPT} ${RESULTS_JSON} ${RESULTS_DIR}
    COMMAND ${Python3_EXECUTABLE} ${ASYMPTOTIC_SCRIPT} ${RESULTS_JSON} ${RESULTS_DIR}
    DEPENDS main
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Компиляция, запуск бенчмарков с фиксированной частотой и построение графиков"
)

# Путь к FlameGraph утилитам
set(FLAMEGRAPH_DIR "/home/artem/FlameGraph")

set(PERF_SCRIPT "${CMAKE_BINARY_DIR}/run_perf_flamegraph.sh")
set(FLAMEGRAPH_OUTPUT "${CMAKE_SOURCE_DIR}/plots/flamegraph.svg")

file(WRITE ${PERF_SCRIPT} "#!/usr/bin/env bash\n")
file(APPEND ${PERF_SCRIPT} "set -e\n\n")

file(APPEND ${PERF_SCRIPT} "echo '--- Проверка наличия perf и FlameGraph ---'\n")
file(APPEND ${PERF_SCRIPT} "if ! command -v perf &>/dev/null; then\n")
file(APPEND ${PERF_SCRIPT} "  echo 'Error: perf not found. Install it with: sudo apt install linux-tools-common linux-tools-$(uname -r)'\n")
file(APPEND ${PERF_SCRIPT} "  exit 1\n")
file(APPEND ${PERF_SCRIPT} "fi\n")
file(APPEND ${PERF_SCRIPT} "if [ ! -d ${FLAMEGRAPH_DIR} ]; then\n")
file(APPEND ${PERF_SCRIPT} "  echo 'Error: FlameGraph scripts not found in ${FLAMEGRAPH_DIR}'\n")
file(APPEND ${PERF_SCRIPT} "  echo 'Clone with: git clone https://github.com/brendangregg/FlameGraph.git ~/FlameGraph'\n")
file(APPEND ${PERF_SCRIPT} "  exit 1\n")
file(APPEND ${PERF_SCRIPT} "fi\n\n")

# Очистка предыдущих данных
file(APPEND ${PERF_SCRIPT} "echo '--- Очистка старых данных perf ---'\n")
file(APPEND ${PERF_SCRIPT} "rm -f perf.data perf.out perf.folded ${FLAMEGRAPH_OUTPUT}\n\n")

# Запуск профилирования
file(APPEND ${PERF_SCRIPT} "echo '--- Запуск perf record ---'\n")
file(APPEND ${PERF_SCRIPT} "perf record -F 999 -g -o perf.data -- ./main --benchmark_filter=BM_AVL_Find_Existing || true\n\n")

# Генерация Flamegraph
file(APPEND ${PERF_SCRIPT} "echo '--- Генерация flamegraph.svg ---'\n")
file(APPEND ${PERF_SCRIPT} "sudo chown $(whoami):$(whoami) perf.data || true\n")
file(APPEND ${PERF_SCRIPT} "perf script -f > perf.out\n")
file(APPEND ${PERF_SCRIPT} "${FLAMEGRAPH_DIR}/stackcollapse-perf.pl perf.out > perf.folded\n")
file(APPEND ${PERF_SCRIPT} "${FLAMEGRAPH_DIR}/flamegraph.pl perf.folded > ${FLAMEGRAPH_OUTPUT}\n\n")
file(APPEND ${PERF_SCRIPT} "echo '✅ Flamegraph готов: ${FLAMEGRAPH_OUTPUT}'\n")

file(COPY ${PERF_SCRIPT} DESTINATION ${CMAKE_BINARY_DIR})
execute_process(COMMAND chmod +x ${PERF_SCRIPT})

add_custom_target(flamegraph
    COMMAND ${PERF_SCRIPT}
    DEPENDS main
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Запуск perf и построение флеймграфа 🔥"
)

add_custom_target(plot_all
    COMMAND ${Python3_EXECUTABLE} ${PYTHON_SCRIPT} ${RESULTS_JSON} ${RESULTS_DIR}
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/pyscript/plot_asymptotic.py ${RESULTS_JSON} ${RESULTS_DIR}
    DEPENDS main
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Построение всех типов графиков"
)
