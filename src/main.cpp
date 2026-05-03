#include <benchmark/benchmark.h>
#include "AVL.h"
#include <vector>
#include <random>
#include <algorithm>

static void Warmup_Cache(AVL& tree, int N) {
    for (int i = 0; i < N; i++) {
        tree.insert(i, "warmup_" + std::to_string(i));
    }  
}

static void get_balanced_order(int start, int end, std::vector<int>& result) {
    if (start > end) return;
    int mid = start + (end - start) / 2;
    result.push_back(mid);
    get_balanced_order(start, mid - 1, result);
    get_balanced_order(mid + 1, end, result);
}

static void build_balanced(AVL& tree, int start, int end) {
            if (start > end) return;
            int mid = start + (end - start) / 2;
            tree.insert(mid, "balanced_" + std::to_string(mid));
}

static void BM_AVL_Find_Leaf(benchmark::State& state) {
    AVL tree;
    std::vector<int> insertion_order;

    // Заполняем порядок вставки для сбалансированного дерева
    get_balanced_order(0, state.range(0) - 1, insertion_order);

    for (int key : insertion_order)
        tree.insert(key, "data_" + std::to_string(key));

    int key_to_find = insertion_order.back();

    std::string dummy;
    for (auto _ : state) {
        bool result = tree.contains(key_to_find);
        benchmark::DoNotOptimize(result);
    }
}


static void BM_AVL_Find_Existing(benchmark::State& state) {
    AVL tree;

    std::vector<int> insertion_order;
    get_balanced_order(0, state.range(0) - 1, insertion_order);

    // Вставка в AVL по сбалансированному порядку
    for (int key : insertion_order)
        tree.insert(key, "data_" + std::to_string(key));

    // Генератор случайных ключей для поиска
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, state.range(0) - 1);

    for (auto _ : state) {
        int key_to_find = dist(rng);
        bool result = tree.contains(key_to_find);
        benchmark::DoNotOptimize(result);
    }
}


static void BM_AVL_Build_Random_Case(benchmark::State& state) {
    std::mt19937 rng(42);
    
    for (auto _ : state) {
        AVL tree;

        std::vector<int> keys(state.range(0));
        std::iota(keys.begin(), keys.end(), 0);
        std::shuffle(keys.begin(), keys.end(), rng);
        
        for (int key : keys) {
            tree.insert(key, "random_" + std::to_string(key));
        }
    }
}

static void BM_AVL_Build_Worst_Case(benchmark::State& state) {
    for (auto _ : state) {
        AVL tree;
        for (int i = 0; i < state.range(0); i++) {
            if (i % 2 == 0) {
                tree.insert(i / 2, "worst_" + std::to_string(i / 2));
            } else {
                tree.insert(state.range(0) - i / 2 - 1, "worst_" + std::to_string(state.range(0) - i / 2 - 1));
            }
        }
    }
}

static void BM_AVL_Build_Best_Case(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        
        std::vector<int> insertion_order;
        get_balanced_order(0, state.range(0) - 1, insertion_order);
        
        AVL tree;
        state.ResumeTiming(); 
        
        for (int key : insertion_order) {
            tree.insert(key, "balanced_" + std::to_string(key));
        }
    }
}

static void BM_AVL_Insert_Single_Operation(benchmark::State& state) {
    std::mt19937 rng(42);
    
    for (auto _ : state) {
        state.PauseTiming();       
        AVL tree;
        // Подготовка дерева
        std::vector<int> keys(state.range(0) - 1);
        std::iota(keys.begin(), keys.end(), 0);
        std::shuffle(keys.begin(), keys.end(), rng);
        
        for (int key : keys) {
            tree.insert(key, "warmup_" + std::to_string(key));
        }
        
        state.ResumeTiming();  
        // Измеряем одну вставку
        tree.insert(state.range(0), "final_value");
    }
}

static void BM_AVL_Delete_Single_Operation(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();  
        AVL tree;
        Warmup_Cache(tree, state.range(0));
        int key_to_delete = state.range(0) / 2;
        state.ResumeTiming();  
        tree.remove(key_to_delete);
    }
}


BENCHMARK(BM_AVL_Build_Random_Case)
    ->RangeMultiplier(2)
    ->Range(1000, 128000)
    ->Iterations(300)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_AVL_Build_Worst_Case)
    ->RangeMultiplier(2)
    ->Range(1000, 128000)
    ->Iterations(300)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_AVL_Build_Best_Case)
    ->RangeMultiplier(2)
    ->Range(1000, 128000)
    ->Iterations(300)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_AVL_Insert_Single_Operation)
    ->RangeMultiplier(2)
    ->Range(1000, 128000)
    ->Iterations(400);

BENCHMARK(BM_AVL_Delete_Single_Operation)
    ->RangeMultiplier(2)
    ->Range(1000, 128000)
    ->Iterations(400);

BENCHMARK(BM_AVL_Find_Existing)
    ->RangeMultiplier(2)
    ->Range(1000, 128000)
    ->Iterations(800);

BENCHMARK(BM_AVL_Find_Leaf)
    ->RangeMultiplier(2)
    ->Range(1000, 128000)
    ->Iterations(800);


BENCHMARK_MAIN();
