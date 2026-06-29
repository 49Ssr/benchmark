#pragma once
#include "benchmark.h"
#include <thread>
#include <functional>

class SimulationBenchmarkMT : public Benchmark {
	public:
		void run() override;
		std::string getName() const override;

};