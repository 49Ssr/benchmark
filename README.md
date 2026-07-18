# PROTOTYPE Benchmark V1

Experimental CPU benchmarking framework written in C++.

## Current Features

- High-resolution timing
- Benchmark abstraction layer
- Particle simulation workload (very abstracted concept, useful nonetheless)
- Physics-inspired simulation workload (very abstracted concept, useful nonetheless)
- Single-threaded and two-thread simulation tests
- Benchmark result structure
- Repeated runs with average, best and worst timing output

## Research

The project research is stored alongside the code so design decisions can be traced back to evidence rather than chat history.

- [001 - What Makes a CPU Benchmark Trustworthy?](research/001_trustworthy_cpu_benchmarking.md)

## Work in Progress

- Result validation/checksums
- Better statistics and warm-up handling
- Cleaner benchmark set execution

## Planned Features

- Hardware detection
- Expanded multi-thread benchmarking
- Database integration

## Status

**Work in Progress — very early stage**
