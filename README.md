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
- [002 - Hardware-Aware Benchmark Paths](research/002_hardware_aware_benchmark_paths.md)
- [003 - Result Storage Pipeline](research/003_result_storage_pipeline.md)
- [004 - Research Review and Coding Adjustment Plan](research/004_research_review_and_coding_plan.md)
- [005 - Lifecycle and Validation Plan Cross-Check](research/005_lifecycle_validation_crosscheck.md)

## Work in Progress

- Result validation/checksums
- Better statistics and warm-up handling
- Cleaner benchmark set execution

## Planned Features

- Hardware detection and ISA-aware module dispatch
- Expanded multi-thread benchmarking
- CSV export and later SQLite/database integration
- Future GPU benchmarking research

## Status

**Work in Progress — very early stage**