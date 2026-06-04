#include <iostream>
#include "benchmark_runner.h"
#include "timer.h"

//does NOT perform benchmark, just executes workloads, measures time and produces results

void betweenSeparators(const std::string& text) {
	std::cout << "\n"; for (int i = 0; i < 50; i++) std::cout << "-"; std::cout << "\n";
	std::cout << text;
	std::cout << "\n"; for (int i = 0; i < 50; i++) std::cout << "-"; std::cout << "\n";
}

void BenchmarkRunner::runBenchmark(Benchmark& benchmark) {
	Timer timer;
	timer.start();
	benchmark.run();
	timer.stop();

	std::cout << "Elapsed:\t" << timer.getMilliseconds() << " ms\n";
}