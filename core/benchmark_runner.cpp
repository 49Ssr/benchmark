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

void runBenchmarkSet(BenchmarkRunner& runner, Benchmark& benchmark, int runs) {
	std::vector<BenchmarkResult> results;	//create dynamic list for benchmark results
	for (int i = 0; i < runs; i++) {
		BenchmarkResult result = runner.runBenchmark(benchmark);
		results.push_back(result);
		std::cout << result.name << "\tElapsed:" << result.elapsedMs << "ms\n";
	}
	//best, worst , average
	double total = 0.0; double best = std::numeric_limits<double>::max(); double worst = 0.0;
	for (const BenchmarkResult& result : results) {	//loop through each stored result without any copying
		total += result.elapsedMs;
		if (result.elapsedMs < best) best = result.elapsedMs;
		if (result.elapsedMs > worst) worst = result.elapsedMs;
	}
	double average = total / results.size();
	std::cout << "\nSummary:\n"; std::cout << "Runs:\t" << results.size() << "\n"; std::cout << "Average:\t" << average << "ms\n";
	std::cout << "Best:\t" << best << "ms\n"; std::cout << "Worst:\t" << worst << "ms\n";
}