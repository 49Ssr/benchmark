# 001 - What Makes a CPU Benchmark Trustworthy?

This is the first research pass for the benchmark project.

The goal is not to copy an existing benchmark. The goal is to identify the conditions that make benchmark results worth believing, then turn those conditions into requirements for this project.

## Current Project Position

The prototype currently has:

- a high-resolution timer based on `std::chrono::steady_clock`
- a benchmark abstraction layer
- a particle workload
- a physics-inspired single-thread workload
- a two-thread version of the same simulation workload
- repeated runs with average, best and worst timing output

The prototype can produce numbers. That does not yet mean those numbers are reliable, comparable or useful.

## 1. A Result Is More Than a Number

SPEC treats a published benchmark result as an observation made under stated conditions, not as a universal fact about a processor.

That distinction matters.

A timing result is affected by more than the CPU model:

- compiler and compiler flags
- build configuration
- operating system and scheduler state
- power mode
- boost behaviour
- temperature
- memory configuration
- background processes
- workload inputs
- benchmark version

### Project consequence

A future saved result must include its test conditions. A bare value such as `172 ms` is not enough for comparison.

At minimum, future result metadata should include:

- benchmark and workload version
- compiler and build type
- CPU identity
- operating system
- thread count
- number of repetitions
- timing method
- relevant test settings

## 2. Correctness Must Be Checked

SPEC validates benchmark output against expected results. Fast execution is meaningless if the program produced the wrong answer or if the compiler removed useful work.

Our simulation currently changes a vector of entities, but the benchmark runner does not verify the final state. This leaves two problems:

1. a bug could make the workload cheaper while still producing a timing result
2. future compiler optimisation could remove work whose result is never observed

### Project consequence

Every serious workload needs a cheap validation value after execution, such as a checksum derived from final entity state.

The checksum should not dominate the timed work, but the result must eventually escape the workload so the computation remains observable.

This is a higher priority than adding a score.

## 3. Repetition Is Necessary, but Average/Best/Worst Is Not Enough

Google Benchmark repeats tests because a single run may not represent normal behaviour. It reports mean, median, standard deviation and coefficient of variation.

SPEC CPU 2017 uses either:

- three runs and the median, or
- two runs and the slower result

These are different policies because they serve different goals.

- Mean uses every result but is sensitive to outliers.
- Best shows the fastest observed case but can hide instability.
- Worst exposes interruptions but may be dominated by unrelated system activity.
- Median represents the middle run and resists isolated outliers.
- Standard deviation and coefficient of variation describe stability.

### Project consequence

Keep average, best and worst because they are easy to understand, but add:

- median
- standard deviation
- coefficient of variation

Do not invent a single benchmark score until the raw timing distribution is properly reported.

## 4. Warm-up and Run Order Matter

The first run may behave differently because of:

- cold instruction and data caches
- page faults and memory commitment
- library initialization
- CPU frequency ramp-up
- branch predictor state

Google Benchmark supports warm-up time and random interleaving of benchmark repetitions. Random interleaving exists to reduce bias from gradual system-state changes.

Our current program runs all particle tests, then all single-thread simulation tests, then all two-thread tests. This means temperature, boost state and background activity can systematically favour or punish whichever benchmark runs later.

### Project consequence

We need two separate concepts:

- warm-up runs: executed but not recorded
- measured runs: included in statistics

Later, benchmark order should be interleaved or randomized rather than always grouped by workload.

## 5. Timer Choice and Workload Duration

Microsoft recommends `QueryPerformanceCounter` for high-resolution interval measurement on Windows. It is monotonic and normally consistent across processors. Microsoft also warns that timer resolution and timer access cost place limits on very short measurements.

`std::chrono::steady_clock` is the correct C++ abstraction for elapsed time because it is monotonic. On Windows implementations it is normally backed by the platform performance counter, but this should be confirmed for the compiler/runtime used by the project rather than assumed forever.

The particle workload has previously produced results around only a few tens of milliseconds. At that scale, scheduler interruptions and CPU frequency changes can form a noticeable fraction of the result.

### Project consequence

- Keep `steady_clock` for now.
- Document the actual clock period returned by `steady_clock`.
- Add a timer-overhead experiment.
- Prefer workloads long enough that timer access cost is negligible.
- Do not simply make every workload extremely long; runtime and heat can introduce new bias.

## 6. Low Noise Is Not the Same as No Bias

LLVM's benchmarking guidance makes an important distinction: reducing noise is necessary, but it does not prove that a benchmark is unbiased.

A benchmark can be very repeatable and still measure the wrong thing.

Examples:

- a workload may mostly measure memory bandwidth while being labelled a CPU arithmetic test
- compiler optimisation may transform a synthetic loop into something unlike real software
- a workload may fit entirely in cache and fail to represent larger applications
- fixed thread placement may favour one CPU topology
- one chosen input may create unusually predictable branches

### Project consequence

Every workload needs a written statement of what it is intended to stress and what it does not measure.

For the current simulation workload, we do not yet know how much time is spent on:

- floating-point arithmetic
- memory access
- branch handling
- allocation and initialization
- thread scheduling

That must be measured rather than guessed.

## 7. Reproducibility Requires Configuration Disclosure

SPEC requires performance-relevant hardware, software and tuning information to be disclosed. Its configuration files exist partly so another tester can recreate the build and runtime conditions.

### Project consequence

The benchmark should eventually print and export a machine-readable result record instead of only console text.

Likely future format:

```text
result
  benchmark name
  workload version
  elapsed samples
  statistics
  validation checksum
  compiler
  build type
  operating system
  CPU information
  thread count
  timestamp
  test configuration
```

JSON is an obvious candidate, but the schema should be designed only after we know which metadata is essential.

## 8. Immediate Engineering Priorities

Based on this first research pass, the next work should be:

1. Add a validation/checksum path so benchmark work cannot silently disappear or become incorrect.
2. Add median, standard deviation and coefficient of variation.
3. Add unrecorded warm-up runs.
4. Record build and test configuration.
5. Separate workload initialization time from update time experimentally.
6. Measure timer overhead and inspect `steady_clock` resolution.
7. Investigate randomized or interleaved run order.

Adding more thread counts before these steps would produce more numbers without improving trustworthiness.

## Open Questions

- Should initialization and simulation updates be separate benchmark modes?
- What final-state checksum is cheap, deterministic and difficult to optimize away?
- Should the main reported value be median rather than average?
- How long should each workload run on both fast and slow CPUs?
- Should the program detect thermal or frequency drift during a test?
- How should hybrid CPUs with performance and efficiency cores be handled?
- When should thread affinity be used, and when would it distort normal behaviour?
- How can workload inputs be varied without making results incomparable?

## Sources

- SPEC CPU 2017 Run and Reporting Rules: https://www.spec.org/cpu2017/Docs/runrules.html
- SPEC CPU 2017 Overview: https://www.spec.org/cpu2017/Docs/overview.html
- SPEC CPU 2017 Configuration Files: https://www.spec.org/cpu2017/Docs/config.html
- Google Benchmark User Guide: https://github.com/google/benchmark/blob/main/docs/user_guide.md
- Microsoft, Acquiring high-resolution time stamps: https://learn.microsoft.com/en-us/windows/win32/sysinfo/acquiring-high-resolution-time-stamps
- LLVM Benchmarking Tips: https://llvm.org/docs/Benchmarking.html
