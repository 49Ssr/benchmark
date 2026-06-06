#pragma once
#include "benchmark.h"

class SimulationBenchmark : public Benchmark {
public:
	void run() override;
	std::string getName() const override;

};