#pragma once
#include <chrono> //high-res timing, steady_clock and time_point

class Timer {
public:
	void start();
	void stop(); //also calcs the elapsed time via subtraction

	double getMilliseconds() const; //returns previous calc

private:
	std::chrono::steady_clock::time_point startTime; //store timestamp
	std::chrono::steady_clock::time_point endTime; //same thing
	double elapsedMs = 0.0; //final duration value;

};