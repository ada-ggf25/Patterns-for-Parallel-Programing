# `cpu_type` Selection

CX3 mixes two CPU architectures (see [[../cluster/CX3 Overview]]). Without pinning, PBS picks whichever is free — so the same benchmark run twice can land on different hardware.

## The problem this solves

Two students submitting the *exact same* OpenMP job can see legitimately different timings — sometimes by 2× — because one ran on a 128-core EPYC Rome and the other on a 64-core Xeon Ice Lake. That's not a bug or a measurement error; it's the heterogeneous cluster doing its job.

For consistency (in benchmarks, in coursework grading, in performance debugging), pin the CPU class with the `cpu_type` resource:

```bash
#PBS -l select=1:ncpus=8:mem=8gb:ompthreads=8:cpu_type=rome
#PBS -l select=1:ncpus=8:mem=8gb:ompthreads=8:cpu_type=icelake
```

The selector goes inside the `select=` chunk grammar like any other per-chunk resource.

## The two values

| `cpu_type` | Hardware | Cores / node | NUMA / node |
|---|---|---:|---:|
| `rome` | AMD EPYC 7742 (Zen2) | 128 | 8 |
| `icelake` | Intel Xeon Platinum 8358 | 64 | 2 |

See [[../cluster/AMD Rome Architecture]] and [[../cluster/Intel Ice Lake Architecture]] for full breakdowns.

## Why this course defaults to `rome`

- ~300 Rome nodes vs. ~80 Ice Lake → Rome queues start much faster.
- The 8-NUMA / 128-core layout is a richer topology to discuss for thread/rank placement.
- Available memory per node is generous (~1 TB).

Switch to `cpu_type=icelake` only if you specifically want the Intel tier — for the simpler 2-NUMA pedagogy, or to compare against Rome.

## Inspecting what's available right now

```bash
pbsnodes -a | awk '
  /^cx3-/      { n = $1 }
  /cpu_type/   { t = $3 }
  /^[[:space:]]*$/ { print t; t = "" }
' | sort | uniq -c
```

This prints the CPU-type tally across the cluster. See [[pbsnodes]] for more.

## Related

- [[../cluster/CX3 Overview]] — the heterogeneous-cluster context.
- [[../cluster/AMD Rome Architecture]]
- [[../cluster/Intel Ice Lake Architecture]]
- [[Resource Selection]] — where this fits in the `select=` line.
- [[pbsnodes]] — listing nodes by attribute.
