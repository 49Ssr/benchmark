#include <iostream>
#include "../core/benchmark_runner.h"
#include "../benchmarks/particle_benchmark.h"
#include "../benchmarks/simulation_benchmark.h"

int main() {
	BenchmarkRunner runner;
	std::cout << "\nInitialized.\n";

	ParticleBenchmark benchmark1;
	betweenSeparators(benchmark1.getName());
	BenchmarkResult result1;
	for (int i = 0; i < 25; i++) {
		result1 = runner.runBenchmark(benchmark1);
		std::cout << "\n" << result1.name << "\tElapsed:" << result1.elapsedMs << " ms\n";
	}

	SimulationBenchmark benchmark2;
	betweenSeparators(benchmark2.getName());
	BenchmarkResult result2;
	for (int i = 0; i < 25; i++) {
		result2 = runner.runBenchmark(benchmark2);
		std::cout << "\n" << result2.name << "\tElapsed:" << result2.elapsedMs << " ms\n";
	}

	return 0;
}