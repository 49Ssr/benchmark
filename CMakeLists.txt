cmake_minimum_required(VERSION 3.20)

project(benchmark)

set(CMAKE_CXX_STANDARD 20)

add_executable(benchmark
    src/main.cpp

    core/timer.cpp
    core/benchmark_runner.cpp

    benchmarks/particle_benchmark.cpp
    benchmarks/simulation_benchmark.cpp
 "core/validation_result.h")