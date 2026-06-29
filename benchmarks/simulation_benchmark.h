#pragma once
#include "benchmark.h"

const float dt = 0.0166f;	//delta time, 60 sims/sec

struct Entity {
	float x; float y;			//position
	float vx; float vy;			//velocity
	float ax; float ay;			//acceleration
};

static void initializeEntities(std::vector<Entity>& entities);
static void updateEntities(std::vector<Entity>& entities, size_t startIndex, size_t endIndex);

class SimulationBenchmark : public Benchmark {
public:
	void run() override;
	std::string getName() const override;

};