cmake_minimum_required(VERSION 3.20)

project(benchmark)

set(CMAKE_CXX_STANDARD 20)

add_executable(benchmark
    src/main.cpp
    core/timer.cpp
    core/benchmark_runner.cpp
)