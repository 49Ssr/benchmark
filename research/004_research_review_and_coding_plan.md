# 004 - Research Review and Coding Adjustment Plan

## Purpose

This document reviews the first three research passes against:

- the code that actually exists now
- primary vendor and platform documentation
- the project's goal of becoming an engineered benchmark rather than a pile of timed loops

It deliberately criticises earlier conclusions where they were vague, premature or not connected tightly enough to the code.

This is not a replacement for the earlier research. It records where the first pass held up, where it did not and what the code should change next.

---

## 1. Overall verdict

The first research pass found several correct directions:

- validation must come before scoring
- raw runs must be preserved
- test conditions must be recorded
- portable and hardware-optimised results must be separated
- ISA support must be detected from capabilities, not guessed from the vendor name
- local result storage should exist before any remote database

Those points still stand.

However, the research was too comfortable in several places.

It described future modules and vendor paths before fully attacking the weaknesses of the current benchmark contract. The existing interface cannot cleanly separate setup, timed work and validation. Until that changes, hardware-aware modules would only make an unreliable measurement framework more complicated.

The immediate priority is therefore not AVX2, Intel tuning, AMD tuning or SQL.

The immediate priority is making one workload:

1. clearly defined
2. correctly timed
3. impossible to silently optimise away
4. validated after execution
5. represented by a result structure that can later be exported

---

## 2. What the earlier research got right

### 2.1 A timing number without context is weak evidence

The first document correctly rejected bare values such as `172 ms`.

A useful result must identify at least:

- benchmark and workload version
- build configuration
- compiler and relevant flags
- CPU and operating system
- execution path
- thread count
- raw timing samples
- validation result

This remains a foundation of the project.

### 2.2 Validation is more important than an early score

The current simulation modifies data but destroys that data when `run()` returns. The runner only receives elapsed time.

That means the framework cannot currently prove that:

- the intended number of updates happened
- scalar and threaded paths produced equivalent output
- a future SIMD path produced an acceptable answer
- compiler optimisation did not remove or simplify work incorrectly

The earlier research was right to put validation before scoring.

### 2.3 Portable and capability-ceiling results are different questions

The distinction remains useful:

- **portable comparison:** controlled equivalent work intended for direct comparison
- **capability ceiling:** the fastest compatible implementation available on that machine

These results must remain separately labelled.

### 2.4 Raw runs should survive summary calculation

Average, median or best time should never replace the original samples.

Keeping each run allows later work to:

- recalculate statistics
- inspect drift over time
- identify interrupted runs
- compare statistical policies without rerunning old hardware

### 2.5 Hardware metadata should be session-level data

CPU information, operating-system information and build information do not need to be rediscovered by every workload.

They should be collected once into a session object and associated with all samples from that session.

---

## 3. Where the earlier research was too weak or inaccurate

### 3.1 "Scalar C++" is not automatically a scalar benchmark

The hardware-path document treated scalar C++ as an obvious portable baseline.

That is too vague.

An optimising compiler can auto-vectorise ordinary C++ loops. MSVC enables its auto-vectoriser by default and can report which loops were or were not vectorised using `/Qvec-report`.

Therefore these are separate things:

1. **reference source implementation**
   - simple and readable
   - used as a correctness oracle
   - not guaranteed to remain scalar after compilation

2. **controlled non-vector implementation**
   - vectorisation intentionally disabled for the target loop
   - useful for a deliberate scalar-versus-vector experiment

3. **compiler-native implementation**
   - ordinary optimised source
   - compiler is allowed to auto-vectorise

4. **explicit ISA implementation**
   - intrinsics or assembly for AVX2, AVX-512, NEON and similar paths

Calling all four of these "scalar versus optimised" would hide important differences.

### Coding consequence

Every module must record how it was produced, not merely which ISA the CPU supports.

Suggested implementation labels later:

```text
reference_cpp
controlled_scalar
compiler_native
explicit_avx2
explicit_avx512
vendor_library
```

### 3.2 Vendor-aware dispatch must not become vendor-name dispatch

Intel and AMD both implement overlapping x86-64 instruction sets. An AVX2 workload should normally be selected because AVX2 and operating-system state support are available, not because the vendor string says Intel or AMD.

Vendor, family and model become relevant when there is evidence for a genuinely microarchitecture-specific decision, such as:

- topology interpretation
- known instruction-throughput differences
- cache or chiplet-aware scheduling
- profiler and event selection
- an implementation tuned for a documented microarchitecture characteristic

### Coding consequence

The dispatch order should eventually be:

```text
architecture
    -> operating-system support
        -> ISA feature set
            -> built compatible modules
                -> optional microarchitecture tuning
```

Not:

```text
if Intel use module A
if AMD use module B
```

### 3.3 Hardware support and operating-system support are separate

CPUID feature bits alone are not enough for AVX-family code.

The processor may support the instructions while the operating system has not enabled the required extended register state. AVX detection needs the relevant CPUID bits and an operating-system state check using `XGETBV`/XCR0.

AVX-512 additionally consists of multiple feature groups rather than one universal capability.

### Coding consequence

A future `CpuFeatures` object should expose safe-to-execute capabilities, not raw CPUID bits only.

For example:

```text
hardwareAvx2 = true
osAvxState = true
safeAvx2 = true
```

The dispatcher should use `safeAvx2`.

### 3.4 The first result-storage design repeats too much metadata

One wide CSV row containing CPU name, compiler, operating system and every other session field for every timing sample is easy to begin with but wasteful and awkward to evolve.

CSV also handles flat data well but does not naturally represent:

- lists of ISA features
- processor groups
- efficiency classes
- per-cache topology
- nested configuration

### Revised Stage 1 proposal

Keep the spreadsheet-friendly approach, but separate session data from sample data:

```text
results/local/<session-id>/
    session.csv
    samples.csv
```

`session.csv` contains one row for the machine/build/session.

`samples.csv` contains one row per warm-up or measured run and refers to the session ID.

This maps naturally to later SQL tables without requiring SQLite immediately.

A JSON session file may become useful when topology becomes too nested for one CSV row, but it is not required for the first exporter.

### 3.5 Affinity was treated as a future tuning switch without enough separation

Thread placement can improve repeatability, but it also changes what is being measured.

Windows CPU Sets provide a softer placement mechanism intended to work with operating-system power management. Windows also exposes topology and an `EfficiencyClass` for heterogeneous systems. Processor groups matter on systems with more than 64 logical processors, and Windows 11 changed default cross-group scheduling behaviour.

Therefore there should eventually be at least two clearly separate policies:

- **default scheduler:** measures ordinary application behaviour
- **controlled placement:** measures a declared topology/affinity policy

A pinned result must not silently replace the default result.

### 3.6 The research mentioned timers without separating time domains

The project currently measures wall-clock elapsed time using `std::chrono::steady_clock`.

For MSVC on Windows, `steady_clock` wraps `QueryPerformanceCounter`, which is appropriate for elapsed intervals.

But future results need to say what kind of time was measured:

- wall time
- process CPU time
- summed thread CPU time
- hardware cycles
- GPU event time later

For a multithreaded throughput workload, wall time answers how long the job took. Summed CPU time answers a different question and can exceed wall time.

### Coding consequence

Add a timing-domain field before alternative timers are introduced.

Do not replace `steady_clock` with `RDTSC` now. A cycle counter experiment later needs serialization, migration awareness and careful interpretation.

### 3.7 The research did not attack build reproducibility hard enough

The repository currently ignores both:

```text
CMakePresets.json
CMakeUserPresets.json
```

CMake documents a different intended role for each:

- `CMakePresets.json` is project-wide and may be committed
- `CMakeUserPresets.json` is machine/user-specific and should not be committed

Ignoring the project preset removes a straightforward way to standardise the Release build used for benchmark results.

### Coding consequence

Later change `.gitignore` so only `CMakeUserPresets.json` remains ignored, then track a project Release preset.

### 3.8 Floating-point policy needs to be a first-class part of fairness

The simulation uses floating-point arithmetic. MSVC floating-point modes can change whether operations are reordered, simplified or contracted into FMA instructions.

An AVX2/FMA path may therefore differ numerically from a precise baseline even when both are reasonable implementations.

### Coding consequence

Record the floating-point policy with each module.

Possible future classes:

```text
strict/precise comparison
fast-math capability
```

Do not silently compare `/fp:fast` output with `/fp:precise` output under one unlabeled score.

---

## 4. Review of the current code

## 4.1 `Benchmark::run()` is now too small a contract

Current interface:

```cpp
virtual void run() = 0;
```

This was enough to learn inheritance and timing, but it cannot express the lifecycle now required.

The runner needs to distinguish:

```text
setup
    -> timed execution
        -> validation
            -> cleanup
```

Without that separation, each benchmark chooses its own timing boundaries invisibly.

### Planned replacement

Keep the interface understandable:

```cpp
class Benchmark {
public:
    virtual ~Benchmark() = default;

    virtual void setup() = 0;
    virtual void run() = 0;
    virtual ValidationResult validate() const = 0;

    virtual std::string getName() const = 0;
    virtual int getThreadCount() const = 0;
    virtual int getWorkloadVersion() const = 0;
};
```

`setup()` is outside the timed region.

`run()` contains only the declared measured work.

`validate()` runs after the timer stops and returns evidence that the workload completed correctly.

This is the first major code adjustment.

## 4.2 `BenchmarkResult` is not yet a real sample record

Current structure:

```cpp
struct BenchmarkResult {
    std::string name;
    double elapsedMs;
};
```

It cannot identify the run or whether it is valid.

### Planned replacement

A per-run sample should eventually contain at least:

```cpp
struct BenchmarkSample {
    std::string benchmarkName;
    int workloadVersion;
    int runIndex;
    int threadCount;
    double elapsedMs;
    bool warmup;
    bool validationPassed;
    unsigned long long checksum;
};
```

Module/path/timing-domain fields can be added when hardware dispatch arrives.

Do not put average, median and standard deviation into every sample. Those belong to a separate summary structure.

## 4.3 `runBenchmarkSet()` does too many jobs

It currently:

- runs benchmarks
- stores samples
- calculates statistics
- prints console output

That was acceptable for the prototype, but CSV export would force more unrelated logic into the same function.

### Planned split

```text
BenchmarkRunner
    executes workload lifecycle and returns samples

Statistics
    converts samples into a summary

ConsoleReporter
    prints samples and summary

CsvExporter
    writes samples and session metadata
```

This does not need to become five complicated classes immediately. The separation can begin with small free functions/files and grow only when justified.

## 4.4 The particle benchmark is not a useful particle benchmark

Current workload:

```cpp
volatile long long sum = 0;
for (...) {
    sum += i;
}
```

Problems:

- no particles exist
- the result never leaves the function
- `volatile` changes the semantics and can force repeated observable accesses
- it mostly measures a very narrow loop, volatile behaviour and compiler decisions
- its name claims more than the workload does

### Decision

Do not build future scoring around it.

Keep it temporarily as a framework smoke test or rename it later to something honest such as:

```text
Integer Accumulation Smoke Test
```

A replacement compute workload should have a defined operation count and validated output.

## 4.5 The simulation currently times allocation and initialization

`SimulationBenchmark::run()` currently:

1. creates a vector
2. resizes it
3. initializes one million entities
4. executes 100 update steps

This is a valid end-to-end job, but it is not a pure simulation-update benchmark.

The code and name do not currently state which interpretation is intended.

### Decision

The first validated version should measure **simulation updates only**:

```text
setup: allocate and initialize entities
run: execute 100 update steps
validate: calculate and compare final-state evidence
```

If end-to-end cost is valuable later, add a separately named benchmark instead of quietly moving setup into or out of the timer.

Changing the timed boundary changes the workload definition, so this should bump the workload version.

## 4.6 Simulation state must survive `run()`

The entity vector is currently local to `run()` and disappears immediately afterward.

Validation requires the final state to remain available.

### Planned change

Make the vector a member of the benchmark object.

Conceptually:

```cpp
class SimulationBenchmark : public Benchmark {
private:
    std::vector<Entity> entities;
};
```

`setup()` fills it, `run()` updates it, and `validate()` reads it.

## 4.7 The two-thread implementation has an important assumption

Each thread owns half of the entity vector for all 100 steps, and the threads join only at the end.

That is valid for the current workload because entities do not interact with each other. Each entity's future state depends only on its own previous state.

It would stop being equivalent if future simulation steps introduced:

- entity-to-entity collisions
- neighbourhood queries
- shared global forces updated per step
- cross-range dependencies

Then a barrier or different scheduling design would be required between simulation steps.

### Coding consequence

Document workload independence as part of the workload definition. Do not assume this thread structure remains valid after adding interactions.

## 4.8 The current data layout is hostile to a clean first SIMD experiment

`Entity` is an array-of-structures layout:

```text
x y vx vy ax ay | x y vx vy ax ay | ...
```

SIMD may prefer structure-of-arrays:

```text
x x x x ...
y y y y ...
vx vx vx vx ...
```

But changing layout also changes memory behaviour. That creates a fairness problem.

### Revised comparison categories

1. **fixed-layout ISA comparison**
   - same data layout
   - scalar and SIMD implementations solve the same representation

2. **native capability implementation**
   - layout may be redesigned for the hardware
   - separately labelled because both algorithm implementation and memory layout changed

The first AVX2 experiment should probably use a simpler dedicated vector-math workload instead of forcing the current branch-heavy AoS simulation to become the first SIMD test.

## 4.9 Header ownership is currently fragile

Examples:

- `benchmark.h` includes `<vector>` and `<limits>` even though the base class does not use them
- `simulation_benchmark.h` relies on those indirect includes for `std::vector`
- simulation helpers are declared `static` in a public header even though they are implementation details
- `simulation_benchmark_mt.h` includes threading headers even though the thread objects exist only in the `.cpp`

### Planned cleanup

Each header should include what it directly uses.

Move `Entity`, `dt`, `initializeEntities()` and `updateEntities()` into a private simulation workload header/source or an unnamed namespace where practical.

This cleanup should happen before ISA-specific files multiply the include problems.

## 4.10 Current statistics are only a beginning

Average, best and worst are now implemented.

Still missing:

- median
- sample standard deviation
- coefficient of variation
- warm-up handling
- run index
- drift visibility

Do not reject outliers automatically yet. First store and display them. An interrupted result is evidence about the session, and automatic removal can hide instability.

---

## 5. Revised architecture direction

A reasonable near-term structure is:

```text
benchmarks/
    benchmark.h

    simulation/
        simulation_types.h
        simulation_workload.h
        simulation_workload.cpp
        simulation_benchmark.h
        simulation_benchmark.cpp
        simulation_benchmark_mt.h
        simulation_benchmark_mt.cpp

core/
    benchmark_runner.h
    benchmark_runner.cpp
    benchmark_sample.h
    benchmark_summary.h
    statistics.h
    statistics.cpp
    timer.h
    timer.cpp

reporting/
    console_reporter.h
    console_reporter.cpp

hardware/                 later
    cpu_info.h
    cpu_info_windows.cpp
    cpu_features_x86.cpp

export/                   later
    csv_exporter.h
    csv_exporter.cpp
```

This is a direction, not an instruction to create every file immediately.

The project should add files only as the next feature makes them necessary.

---

## 6. Prioritised coding plan

## Phase 1 - Correct workload boundaries and validation

This is next.

1. Add `ValidationResult`.
2. Add `setup()` and `validate()` to the benchmark interface.
3. Move simulation entity storage into the benchmark object.
4. Move allocation/initialization into `setup()`.
5. Keep the 100 update steps in `run()`.
6. Calculate a deterministic validation value after timing.
7. Confirm single-thread and two-thread paths pass validation.
8. Bump the simulation workload version because the timed boundary changed.

### Validation warning

A simple floating-point checksum needs care.

For the first version, the same compiler and arithmetic path should produce the same deterministic final state. Later FMA, fast-math or different ISA paths may need:

- a tolerance-based comparison
- multiple aggregate values
- comparison with a reference output

Do not design the first checksum as if every future path must remain bit-identical.

## Phase 2 - Separate samples, summaries and output

1. Replace the current tiny result with `BenchmarkSample`.
2. Make the runner return samples without printing them.
3. Add a summary function for mean, median, sample standard deviation, minimum, maximum and coefficient of variation.
4. Move console formatting out of the runner.
5. Add a configurable warm-up count.
6. Keep warm-up samples identifiable even if they are not included in the measured summary.

## Phase 3 - Reproducible build configuration

1. Stop ignoring `CMakePresets.json`.
2. Continue ignoring `CMakeUserPresets.json`.
3. Add a tracked MSVC x64 Release preset.
4. Set `CMAKE_CXX_STANDARD_REQUIRED ON`.
5. Record compiler identification, compiler version and build configuration.
6. Add a clear warning level for development builds.
7. Record floating-point and ISA compile policies when specialised modules begin.

## Phase 4 - Session and hardware information

Start with Windows/x86-64 only rather than pretending the first implementation is cross-platform.

Collect:

- architecture
- CPU vendor and brand string
- logical processor count
- physical core count
- processor group count
- cache/topology information available through Windows
- heterogeneous efficiency classes where reported
- safe ISA feature set
- operating-system version

Important implementation sources:

- CPUID/XGETBV for x86 feature discovery
- `GetLogicalProcessorInformationEx` for core/cache/package topology
- `GetSystemCpuSetInformation` for CPU-set and efficiency-class data

Do not use `std::thread::hardware_concurrency()` as the only topology source. It is useful as a simple hint, not a complete topology model.

## Phase 5 - CSV session export

Create:

```text
results/local/<session-id>/session.csv
results/local/<session-id>/samples.csv
```

Then:

- add `results/local/` to `.gitignore`
- add `results/examples/` for deliberately selected tracked results
- escape CSV fields correctly
- write schema and workload versions from day one
- record failed validation rather than treating it as a normal performance result

## Phase 6 - First controlled optimisation experiment

Do not begin with vendor-specific code.

Create one simple, validated vector arithmetic experiment with fixed work.

Compare separately:

```text
reference_cpp
controlled_scalar
compiler_native
explicit_avx2
```

Development checks:

- use MSVC vectorisation reports
- inspect generated assembly when needed
- confirm equal or tolerance-valid output
- record build flags
- report operation count or throughput as well as elapsed time

Only after the dispatch and validation architecture works should the project add AVX-512 or microarchitecture-specific variants.

## Phase 7 - Topology and affinity experiments

Add optional policies, never silent defaults:

```text
default_scheduler
performance_class_only
one_thread_per_physical_core
all_logical_processors
```

Each policy must be labelled in the result.

Hybrid systems, SMT scaling and chiplet/cache locality should become explicit experiments, not hidden tuning.

## Phase 8 - Vendor and microarchitecture research

Then deepen vendor work:

### Intel

- hybrid topology behaviour
- AVX2/AVX-512/AVX10 feature groups
- vector-frequency effects
- Intel performance-monitoring events and VTune evidence

### AMD

- Zen generation mapping
- AVX execution differences by generation
- CCD/CCX/cache locality
- family/model-specific PMC and IBS support
- uProf evidence

### NVIDIA later

Keep CPU and GPU results separate.

GPU work requires separate timing definitions for:

- kernel-only time
- transfer time
- end-to-end time

CUDA operations are often asynchronous, so CPU timing without synchronization can measure submission rather than completed work.

### ARM later

Design interfaces now so x86 feature discovery is not baked directly into every benchmark, but do not delay the Windows/x86 prototype trying to implement ARM prematurely.

---

## 7. Things we should explicitly not do yet

- do not create one overall CPU score
- do not add Intel and AMD branches based only on vendor strings
- do not compile the whole executable with AVX2
- do not add AVX-512 before AVX2 dispatch is proven
- do not pin threads silently
- do not upload results to a remote service yet
- do not introduce SQLite before the local schema has real data
- do not automatically delete outliers
- do not call the current particle loop a realistic workload
- do not change simulation layout and call the result directly comparable without labelling it

---

## 8. First actual coding task after this review

The next code change should be narrow:

> Refactor the simulation benchmark into setup, timed execution and validation while preserving the current single-thread and two-thread update logic.

Success criteria:

- project builds in x64 Release
- allocation and initialization occur outside the timed update region
- final entity state survives `run()`
- single-thread and two-thread paths produce valid results
- runner reports validation failure clearly
- existing comments and learning-oriented style remain understandable

Only after this works should statistics/export/hardware layers be added.

---

## 9. Primary sources used in this review

- Google Benchmark User Guide: https://github.com/google/benchmark/blob/main/docs/user_guide.md
- Microsoft `steady_clock`: https://learn.microsoft.com/en-us/cpp/standard-library/steady-clock-struct
- Microsoft high-resolution timestamps: https://learn.microsoft.com/en-us/windows/win32/sysinfo/acquiring-high-resolution-time-stamps
- Microsoft auto-vectorisation: https://learn.microsoft.com/en-us/cpp/parallel/auto-parallelization-and-auto-vectorization
- Microsoft `/Qvec-report`: https://learn.microsoft.com/en-us/cpp/build/reference/qvec-report-auto-vectorizer-reporting-level
- Microsoft floating-point modes: https://learn.microsoft.com/en-us/cpp/build/reference/fp-specify-floating-point-behavior
- Microsoft `GetLogicalProcessorInformationEx`: https://learn.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getlogicalprocessorinformationex
- Microsoft CPU Sets: https://learn.microsoft.com/en-us/windows/win32/procthread/cpu-sets
- Microsoft `GetSystemCpuSetInformation`: https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getsystemcpusetinformation
- Microsoft `SYSTEM_CPU_SET_INFORMATION`: https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-system_cpu_set_information
- Microsoft Processor Groups: https://learn.microsoft.com/en-us/windows/win32/procthread/processor-groups
- CMake Presets: https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html
- Intel architecture and optimisation manuals: https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html
- AMD uProf and optimisation-document index: https://docs.amd.com/r/en-US/57368-uProf-user-guide/Useful-URLs
- NVIDIA CUDA C++ Best Practices Guide: https://docs.nvidia.com/cuda/cuda-c-best-practices-guide/
