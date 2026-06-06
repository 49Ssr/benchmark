#pragma once
#include <string>

//stores outcome of a runner's run
// BenchmarkRunner creates it via returning from a function, main.cpp consumes it

struct BenchmarkResult {
	std::string name;
	double elapsedMs;

};