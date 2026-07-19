#pragma once
#include "../core/validation_result.h"
#include <string>

//has common class used by all types of benchmarks, with their own run()

class Benchmark {
public:
	virtual ~Benchmark() = default; //virtual means derived class can have own implementation
	virtual void prepareRun() { } //default does nothing until benchmark has its own preparation
	virtual ValidationResult validate() const { return ValidationResult{}; } //default result stays NotChecked
	virtual void run() = 0; //0 - function must be implemented by derived classes
	virtual std::string getName() const = 0;
	virtual int getWorkloadVersion() const { return 1; } //current default version, benchmarks can override later

};