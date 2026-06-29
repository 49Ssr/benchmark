#pragma once
#include <string>
#include <vector>

//has common class used by all types of benchmarks, with their own run()

class Benchmark {
public:
	virtual ~Benchmark() = default; //virtual means derived class can have own implementation
	virtual void run() = 0; //0 - function must be implemented by derived classes
	virtual std::string getName() const = 0;

};