#pragma once

class Timer {
public:
	void start();
	void stop();

	double getMilliseconds() const;

private:
	double elapsedMs = 0.0;

};