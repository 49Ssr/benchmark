#include <iostream>
#include "../core/benchmark_runner.h"
#include "../benchmarks/particle_benchmark.h"
#include "../benchmarks/simulation_benchmark.h"

int main() {
	std::cout << "\nInitialized.\n";
	
	betweenSeparators("Particle Benchmarks");

	ParticleBenchmark benchmark1;
	BenchmarkRunner runner;
	for (int i = 0; i < 25; i++) runner.runBenchmark(benchmark1);

	betweenSeparators("Simuation Benchmarks");

	SimulationBenchmark benchmark2;
	BenchmarkRunner runner2;
	for (int i = 0; i < 25; i++) runner2.runBenchmark(benchmark2);

	return 0;
}