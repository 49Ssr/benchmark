#pragma once
#include "benchmark.h"

class ParticleBenchmark : public Benchmark {
public:
	void run() override; //provide own version of benchmark function
	std::string getName() const override;

};