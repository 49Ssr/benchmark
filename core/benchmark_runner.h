#pragma once
#include "../benchmarks/benchmark.h"
#include "benchmark_result.h"

void betweenSeparators(const std::string& text);

class BenchmarkRunner {
public:
	BenchmarkResult runBenchmark(Benchmark& benchmark);
	//WORK IN PROGRESS IN THE CPP FILE
};