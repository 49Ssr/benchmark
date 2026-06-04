#include "particle_benchmark.h"

void ParticleBenchmark::run() {
	volatile long long sum = 0;
	//when this method finishes, it returns 0 and time to run this is measured
	for (int i = 0; i < 100000000; i++) {
		sum += i;
	}
}