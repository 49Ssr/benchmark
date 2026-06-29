#include "simulation_benchmark.h"
#include "simulation_benchmark_mt.h"

static void initializeEntities(std::vector<Entity>& entities) { //only handles setup loop
	for (size_t i = 0; i < entities.size(); i++) {
		entities[i].x = static_cast<float>(i);		//example if i=42 then 42.0f assigned to x and y
		entities[i].y = static_cast<float>(i);

		entities[i].vx = 0.1f;		//set velocities to 0.1f
		entities[i].vy = 0.1f;

		entities[i].ax = 0.0f;		//set accelerations, y at -9.81 based on real world
		entities[i].ay = -9.81f;
	}
}

static void updateEntities(std::vector<Entity>& entities, size_t startIndex, size_t endIndex) {
	for (size_t i = startIndex; i < endIndex; i++) {	//run from startIndex till endIndex
		Entity& entity = entities[i];

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

void SimulationBenchmark::run() {
	std::vector<Entity> entities;	//create container for storing many entity objects
	entities.resize(1000000);		//resize to large number after creating a vector object

	initializeEntities(entities);

	for (int step = 0; step < 100; step++) {
		updateEntities(entities, 0, entities.size());
	}
}

void SimulationBenchmarkMT::run() {
	std::vector<Entity> entities;
	entities.resize(1000000);
	initializeEntities(entities);
	const size_t midpoint = entities.size() / 2;

	//create, for each step, thread 1, then thread 2, then join	
	std::thread thread1([&entities, midpoint]() {
		for (int step = 0; step < 100; step++) updateEntities(entities, 0, midpoint);
		});
	std::thread thread2([&entities, midpoint]() {
		for (int step = 0; step < 100; step++) updateEntities(entities, midpoint, entities.size());
		});
			

	thread1.join(); thread2.join();
}

std::string SimulationBenchmark::getName() const {
	return "Simulation Benchmark";
}

std::string SimulationBenchmarkMT::getName() const {
	return "Simulation Benchmark Multi-Threaded 2T";
}