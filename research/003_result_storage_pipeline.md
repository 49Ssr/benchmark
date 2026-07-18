# Result Storage Pipeline

## Problem

Console output is temporary and difficult to compare across machines or versions.

The benchmark needs a result format that is:

- easy for the C++ program to write
- readable without special tools
- importable into spreadsheets
- structured enough for later SQL analysis
- versioned so future fields do not silently break old data

## Recommended progression

### Stage 1: CSV export

Start with CSV because it is simple, inspectable and easy to open in Excel, LibreOffice, Python, R or a database import tool.

Each benchmark run should write one row rather than only one final summary row.

Example:

```csv
schema_version,timestamp_utc,benchmark,workload_version,run_index,elapsed_ms,thread_count,path,module,isa,cpu_vendor,cpu_name,compiler,build_type,validation
1,2026-07-18T18:20:00Z,Simulation,1,0,168.25,2,capability,simulation_avx2,AVX2,GenuineIntel,Example CPU,MSVC 19.xx,Release,pass
```

Why store every run:

- median and deviation can be recomputed later
- outliers remain visible
- statistical methods can improve without rerunning old hardware
- raw evidence is not destroyed by early summarisation

A separate summary file is optional. It should be derived from raw rows, not replace them.

### Stage 2: SQLite

Move to SQLite once CSV limitations become real.

SQLite is useful because it is:

- a single local database file
- serverless
- usable directly from C/C++
- capable of indexed queries and relationships
- exportable to CSV or other analysis tools

Possible tables:

```text
systems
benchmark_sessions
benchmark_runs
workload_versions
```

Sketch:

```sql
CREATE TABLE benchmark_runs (
    id INTEGER PRIMARY KEY,
    session_id INTEGER NOT NULL,
    benchmark_name TEXT NOT NULL,
    workload_version INTEGER NOT NULL,
    run_index INTEGER NOT NULL,
    elapsed_ms REAL NOT NULL,
    thread_count INTEGER NOT NULL,
    path TEXT NOT NULL,
    module TEXT NOT NULL,
    isa TEXT NOT NULL,
    validation TEXT NOT NULL
);
```

Do not begin with a remote SQL server. That adds authentication, networking, schema migration and privacy problems before local results are even trustworthy.

### Stage 3: central service

A central result service becomes reasonable after the local schema is stable.

Likely flow:

```text
benchmark executable
    -> local CSV or SQLite
    -> user reviews submission
    -> client sends structured JSON
    -> API validates schema and benchmark version
    -> central SQL database
```

The local file remains useful even if upload fails.

## Repository policy

Generated personal benchmark results should normally not be committed to the main source branch.

Reasons:

- repeated runs create noisy diffs
- machine names and hardware details may be personal metadata
- a growing result file bloats the repository
- results from different benchmark versions can be mixed accidentally

Recommended paths:

```text
results/local/
results/examples/
```

Policy:

- `results/local/` ignored by Git
- `results/examples/` tracked and contains small sanitised samples
- schema documentation tracked under `research/` or later `docs/`

The user mentioned making results visible to the assistant through GitHub. A better compromise is to explicitly copy selected runs into `results/examples/` or a dated tracked experiment file rather than committing every local execution automatically.

## Minimum fields

### Identity

- schema version
- session UUID or other unique ID
- UTC timestamp
- benchmark program version or Git commit
- workload name and version

### Measurement

- run index
- elapsed time
- optional cycles later
- warm-up or measured-run flag
- validation status

### Execution path

- portable or capability-ceiling class
- module name
- ISA level
- thread count
- affinity policy if used

### Environment

- CPU vendor and brand
- architecture
- logical and physical core counts when available
- operating system and version
- compiler and version
- build type and important flags
- power mode or plugged state where detectable/reported

### Statistical summary

These can be stored in a session summary or derived later:

- minimum
- maximum
- mean
- median
- standard deviation
- coefficient of variation
- number of accepted and rejected runs

## CSV implementation notes

The first implementation can use `std::ofstream`.

Do not assume text fields contain no commas or quotes. CSV escaping is required:

- wrap a field in double quotes when needed
- represent a double quote inside a field as two double quotes

Do not use the CPU brand string directly as an unescaped field.

Write the header only when creating a new empty file.

Flush or close the file after a benchmark session so results survive a later crash.

## Schema versioning

Add `schema_version` from the first implementation.

Without it, a later column rename can make old and new rows look compatible while meaning different things.

Workload version is separate from schema version:

- schema version describes the file/database structure
- workload version describes what computation was performed

A result from Simulation workload version 1 should not be directly compared with version 2 unless the change is proven not to affect the measured work.

## Data integrity

Each row should record whether workload validation passed.

Later options:

- checksum of important output
- session-level hash
- rejected-run reason
- immutable raw run table with summaries regenerated separately

Do not upload or aggregate failed validation results as normal performance data.

## Spreadsheet workflow

CSV is the simplest bridge.

A useful analysis sheet can:

- filter by benchmark and workload version
- compare portable versus optimized modules
- chart elapsed time over run index
- calculate median and coefficient of variation
- detect thermal drift across a long session
- group by CPU model, compiler or ISA

The spreadsheet should consume raw exported data. It should not become the source of truth for benchmark logic.

## Immediate implementation target

First result-storage feature:

```text
--output results/local/<timestamp>.csv
```

The exporter should receive `BenchmarkResult` plus session metadata and append one row for every measured run.

Before coding it, extend the in-memory result model enough to identify:

- benchmark/workload version
- run index
- thread count
- module/path
- validation status

Do not bolt hardware metadata directly into every benchmark class. Collect session/system metadata once and associate each run with it.

## Open questions

- one CSV per session or one append-only file?
- should rejected outliers remain in the raw file with a rejection flag?
- should Git commit detection happen at build time or runtime?
- what hardware metadata is reliable on Windows without administrator privileges?
- when should SQLite be introduced: after the first exporter or only when central upload begins?
- should central submissions be anonymous by default?

## Primary sources

- SQLite C/C++ interface introduction: https://www.sqlite.org/cintro.html
- SQLite `CREATE TABLE`: https://www.sqlite.org/lang_createtable.html
- SQLite CSV command-line import/export: https://www.sqlite.org/cli.html
