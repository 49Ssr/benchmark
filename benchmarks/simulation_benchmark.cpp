#include "simulation_benchmark.h"
#include <vector>

const float dt = 0.0166f;	//delta time, 60 sims/sec

struct Entity {
	float x; float y;			//position
	float vx; float vy;			//velocity
	float ax; float ay;			//acceleration
};

void SimulationBenchmark::run() {
	std::vector<Entity> entities;	//create container for storing many entity objects
	entities.resize(1000000);		//resize to large number after creating a vector object

	for (size_t i = 0; i < entities.size(); i++) {	//give 0-9999999 to x and y to i+1 entity
		entities[i].x = static_cast<float>(i);		//example if i=42 then 42.0f assigned to x and y
		entities[i].y = static_cast<float>(i);

		entities[i].vx = 0.1f;		//set velocities to 0.1f
		entities[i].vy = 0.1f;

		entities[i].ax = 0.0f;		//set accelerations, y at -9.81 based on real world
		entities[i].ay = -9.81f;
	}

	for (int step = 0; step < 100; step++) {	//run 100 times
		for (auto& entity : entities) {			//visit every entity stored in the vector, compiler deduces the type Entity, giving reference to real object
			
			entity.vx += entity.ax * dt;		//assign speed gain (acceleration*tick_time)
			entity.vy += entity.ay * dt;
			
			entity.x += entity.vx * dt;			//add current velocity value
			entity.y += entity.vy * dt;

			entity.vx *= 0.999f;				//simulate small amounts of drag
			entity.vy *= 0.999f;

			if (entity.x > 10000.0f) {			//start moving backwards after hitting 10000.0f
				entity.vx = -entity.vx;
			}

			if (entity.y < 0.0f) {
				
				entity.y = 0.0f;					//collision
				entity.vy = -entity.vy * 0.9f;		//lose a 10th of the energy on bounce
			}
		}
	}
}

std::string SimulationBenchmark::getName() const {
	return "Simulation Benchmark";
}