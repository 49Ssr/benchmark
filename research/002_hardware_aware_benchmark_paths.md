# Hardware-Aware Benchmark Paths

## Scope

This document starts the research path for ISA-aware and vendor-aware benchmark execution.

The goal is not to make one vendor look better by giving it special treatment. The goal is to separate two valid questions:

1. How fast is this processor when every processor runs the same portable workload?
2. How far can this processor be pushed when software uses the capabilities it actually provides?

Those are not the same benchmark and should never be mixed into one unexplained score.

## Core comparison model

The project should eventually expose at least two execution classes.

### Portable comparison path

Every supported CPU runs functionally equivalent code with the same algorithm, dataset, precision, stopping conditions and validation rules.

This path exists for direct comparison.

Possible baseline levels:

- scalar C++
- x86-64 SSE2 baseline on Windows x64
- a separately built ARM64 baseline later

The portable path must not silently switch algorithms based on vendor identity.

### Capability ceiling path

The benchmark detects available hardware and operating-system support, then selects a compatible optimized module.

Possible x86 modules:

- scalar
- SSE4.2
- AVX
- AVX2/FMA
- AVX-512 families where present
- AES, SHA, BMI, ADX or other task-specific extensions

Possible ARM modules later:

- NEON/Advanced SIMD
- SVE/SVE2 where supported
- architecture-specific cryptography extensions

This path measures what optimized native software could realistically extract from that machine.

Its result must be labelled by module and ISA. It must not be presented as if it were the portable result.

## Detection is not only vendor detection

Checking whether the CPU says `GenuineIntel` or `AuthenticAMD` is not enough.

Dispatch should be based mainly on actual capabilities:

- ISA feature bits from CPUID on x86/x64
- whether the operating system saves the required extended register state
- architecture and feature discovery on ARM64 later
- compiler and binary support for the selected module

Vendor and family/model information is still useful for:

- metadata
- known architecture-specific tuning
- profiler selection
- interpreting hybrid-core layouts
- recording known errata or feature differences

However, vendor name alone should not decide that an instruction exists.

Microsoft exposes `__cpuid` and `__cpuidex` through `<intrin.h>` for querying x86/x64 feature data. AVX-class execution also requires checking operating-system state support rather than trusting the hardware feature bit alone.

## Binary/module layout under consideration

A practical first design for MSVC and CMake is separate translation units or libraries compiled for separate ISA levels.

Example direction:

```text
benchmarks/
    simulation/
        simulation_common.cpp
        simulation_scalar.cpp
        simulation_avx2.cpp
        simulation_avx512.cpp

hardware/
    cpu_info.cpp
    instruction_set.cpp
    dispatch.cpp
```

Each optimized implementation should present the same interface and validation output.

The dispatch layer chooses only among modules that were built and are safe to execute.

This is preferable to applying `/arch:AVX2` to the whole executable. A whole-program AVX2 build could execute AVX2 instructions before dispatch and therefore fail on unsupported machines.

MSVC currently exposes x64 code-generation targets including SSE2, SSE4.2, AVX, AVX2 and AVX-512 through `/arch`. CMake can apply options to individual targets using `target_compile_options`.

## Fairness rules

### Same category means same work definition

A scalar and AVX2 implementation may execute different machine instructions, but they must solve the same problem.

The following must remain fixed within a comparison category:

- input data
- algorithmic result
- numerical precision policy
- iteration count or stopping condition
- validation tolerance
- timed boundaries

### Optimized paths must be validated

Every path should produce a checksum, digest, error metric or reference comparison.

A faster path that computes a different answer is not an optimization.

Floating-point vectorization may alter operation order, so validation needs an explicit tolerance where bit-identical results are not realistic.

### Scores must identify the path

Minimum metadata:

- benchmark name and version
- workload version
- portable or capability-ceiling class
- selected module
- ISA level
- compiler and version
- build flags
- thread count
- CPU vendor, family and model
- operating system

### Vendor libraries are a separate category

Intel, AMD, NVIDIA and ARM may provide tuned libraries or toolchains.

Using them may be valuable, but a result using a vendor library should not be silently compared with a hand-written portable implementation.

Possible reporting classes:

- portable source implementation
- project ISA-optimized implementation
- vendor-library implementation

## Vendor research tracks

### Intel

Research targets:

- CPUID and extended-state detection
- AVX2, FMA, AVX-512 and newer AVX10 capability groups
- hybrid P-core/E-core topology and scheduling effects
- Intel VTune or performance-counter workflows
- frequency changes under wide-vector workloads

Important caution: AVX-512 is not one single feature bit. Subsets must be detected individually.

### AMD

Research targets:

- Family/model discovery and Zen generation mapping
- AVX2/FMA behaviour across Zen generations
- AVX-512 implementation on supported Zen processors
- cache, chiplet and CCD/CCX effects
- AMD uProf, PMC, IBS, L3PMC and Data Fabric counters

AMD documents that available performance counters and supported metrics depend on processor family/model. Therefore low-level counter collection cannot initially be treated as one universal interface.

### NVIDIA

NVIDIA is not a CPU execution path for the current benchmark.

It belongs to a future heterogeneous/GPU benchmark track:

- CUDA compute modules
- host-to-device and device-to-host transfer costs
- kernel-only versus end-to-end timing
- correctness verification
- occupancy, memory bandwidth and scaling

CPU and CUDA results should remain separate categories even when they model the same workload.

### ARM

Deferred, but architecture decisions made now should avoid locking the project to x86.

Later research targets:

- Windows ARM64 and Linux ARM64 detection
- NEON/Advanced SIMD baseline
- SVE/SVE2 variable vector lengths
- big.LITTLE or other heterogeneous-core scheduling
- feature discovery mechanisms that differ from x86 CPUID

## Experimental workload directions

The following are candidates, not approved benchmarks yet.

### Compute-bound

- dense floating-point kernels
- integer multiply/add pipelines
- fused multiply-add workloads
- transcendental approximations
- small matrix kernels

### Memory and cache

- sequential streaming bandwidth
- latency-sensitive pointer chasing
- cache-sized working-set sweeps
- gather/scatter patterns
- false-sharing and contention experiments

### Branch and front end

- predictable versus unpredictable branches
- instruction-cache pressure
- dispatch-heavy state machines
- parser or compression-style control flow

### Task-specific extensions

- AES and SHA acceleration
- CRC instructions
- bit manipulation
- carry-less multiplication
- half/bfloat conversion where meaningful

### Parallel behaviour

- fixed thread counts
- physical-core versus logical-thread scaling
- work-stealing experiments
- synchronization-heavy workloads
- NUMA-aware work later

## Profiling before claiming optimization

Elapsed time alone says that something changed. It does not explain why.

The project should eventually use hardware counters and vendor profilers during development to examine:

- instructions retired
- cycles
- IPC
- cache misses
- branch mispredictions
- memory bandwidth
- effective frequency
- power and temperature where available

These counters are development evidence. They may not all be portable enough to become mandatory benchmark output.

## Immediate implementation consequences

Do not add vendor-tuned code first.

First add a hardware-information layer capable of recording:

- architecture
- vendor string
- CPU brand string
- logical processor count
- relevant ISA feature flags
- selected benchmark module

Then build one controlled experiment:

```text
same validated workload
scalar module
AVX2 module
separate results
```

That will test the dispatch architecture and fairness rules before the project grows several vendor branches.

## Open questions

- Should the portable x86 comparison use scalar code or SSE2 as its guaranteed x64 baseline?
- Should optimized modules use compiler auto-vectorization, explicit intrinsics, or both as separate tests?
- How should hybrid Intel and AMD heterogeneous-core systems be pinned or reported?
- Should AVX-512 results include effective-frequency telemetry because wide vectors may alter clock behaviour?
- How much of performance-counter collection can be implemented without privileged drivers?
- Can one workload remain genuinely equivalent across x86 SIMD, ARM SIMD and CUDA?

## Primary sources

- Microsoft `__cpuid` and `__cpuidex`: https://learn.microsoft.com/en-us/cpp/intrinsics/cpuid-cpuidex
- Microsoft x64 `/arch`: https://learn.microsoft.com/en-us/cpp/build/reference/arch-x64
- CMake `target_compile_options`: https://cmake.org/cmake/help/latest/command/target_compile_options.html
- Intel CPU dispatch documentation: https://www.intel.com/content/www/us/en/docs/dpcpp-cpp-compiler/developer-guide-reference/latest/cpu-dispatch-cpu-specific.html
- Intel AVX-512 overview: https://www.intel.com/content/www/us/en/developer/articles/technical/intel-avx-512-instructions.html
- AMD uProf documentation: https://docs.amd.com/r/en-US/57368-uProf-user-guide/uProf-User-Guide
- AMD developer optimization guides listed by uProf: https://docs.amd.com/r/en-US/57368-uProf-user-guide/Useful-URLs
- NVIDIA CUDA C++ Best Practices Guide: https://docs.nvidia.com/cuda/cuda-c-best-practices-guide/
