#include <iostream>
#include "benchmark_runner.h"
#include "timer.h"

//does NOT perform benchmark, just executes workloads, measures time and produces results

void betweenSeparators(const std::string& text) {
	std::cout << "\n"; for (int i = 0; i < 50; i++) std::cout << "-"; std::cout << "\n";
	std::cout << text;
	std::cout << "\n"; for (int i = 0; i < 50; i++) std::cout << "-"; std::cout << "\n";
}

BenchmarkResult BenchmarkRunner::runBenchmark(Benchmark& benchmark) { //returns BenchmarkResult struct
	Timer timer;
	timer.start();
	benchmark.run();
	timer.stop();

	BenchmarkResult result;	//create struct object
	result.name = benchmark.getName();
	result.elapsedMs = timer.getMilliseconds();

	return result;

	//std::cout << "Elapsed:\t" << timer.getMilliseconds() << " ms\n";
}