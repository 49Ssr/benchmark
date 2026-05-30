#include "timer.h"

void Timer::start() {
	//call static method, return and assign current point in time to startTime attribute
	startTime = std::chrono::steady_clock::now();
}

void Timer::stop() {
	//again call static method, return and assign
	endTime = std::chrono::steady_clock::now(); //store time end
	//subtracting 2 time_points produces double duration object
	elapsedMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
}

double Timer::getMilliseconds() const {
	//return stored elapsedMs
	return elapsedMs;
}