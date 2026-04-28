# Reading the OpenMP log

A successful `pi_openmp` run produces two files: `pi_openmp.o<jobid>` (stdout + PBS accounting) and `pi_openmp.e<jobid>` (stderr, which is where `OMP_DISPLAY_AFFINITY` lines land).

## `pi_openmp.o<jobid>`

Two parts:

1. **Topology block** (because the script dumps it). Confirm:
   - CPU model is what you asked for (`AMD EPYC 7742` for `cpu_type=rome`).
   - Which cores PBS allocated — they may not be contiguous.
   - NUMA distance matrix — see [[../cluster/AMD Rome Architecture]].

2. **Program output and PBS accounting**:

```
n=1000000000  threads=8  pi=3.141592653589821  time=0.163s
===============================================
CPU Time used: 00:01:18
CPU Percent:   789%
Memory usage:  18mb
Walltime:      00:00:01
```

What to check:

- **`threads=8`** — confirms OpenMP picked up `OMP_NUM_THREADS=8`. If this says `threads=1`, OpenMP didn't activate (missing `-fopenmp` at build time, or `ompthreads=` wasn't honoured).
- **`pi=...`** agrees with the serial value to ~14 decimal places. Differences in last digits are floating-point reordering, not a bug.
- **`time=...`** — the program's own timing, the most reliable number.
- **`CPU Percent ≈ 800%`** — close to `100% × ncpus`. Means all cores were busy. Lower values mean cores idle (over-allocation).

For very short jobs, PBS accounting can show unhelpful values like `CPU Percent: 0%`. Trust the program's `time=` line over PBS accounting on sub-second runs.

## `pi_openmp.e<jobid>`

If you set `OMP_DISPLAY_AFFINITY=TRUE` (the reference script does), this file has one line per thread:

```
OMP: pid 12345 tid 12346 thread 0 bound to OS proc set {12}
OMP: pid 12345 tid 12347 thread 1 bound to OS proc set {30}
OMP: pid 12345 tid 12348 thread 2 bound to OS proc set {31}
OMP: pid 12345 tid 12349 thread 3 bound to OS proc set {47}
OMP: pid 12345 tid 12350 thread 4 bound to OS proc set {61}
OMP: pid 12345 tid 12351 thread 5 bound to OS proc set {62}
OMP: pid 12345 tid 12352 thread 6 bound to OS proc set {67}
OMP: pid 12345 tid 12353 thread 7 bound to OS proc set {79}
```

What to read off:

- **One line per thread** — confirms thread count.
- **`OS proc set {N}`** is the core that thread is pinned to. With `OMP_PLACES=cores` and 8 threads, you should see 8 distinct numbers.
- **Are the cores contiguous?** On Rome you'll typically see scattered cores spanning multiple NUMA domains — see [[Thread Pinning]] for why.

## Diagnostic flowchart

| Symptom | Likely cause |
|---|---|
| `threads=1` | OpenMP not activated → check `-fopenmp` and `ompthreads=` |
| `pi != 3.14159...` (off by lots) | Race condition → missing `reduction(+:sum)` |
| Time barely better than serial | One thread doing all the work (bad pinning, or hot loop is single-threaded) |
| `.o` file empty, `.e` says "command not found" | Module not loaded (`g++` or `mpiexec`) |
| `.e` shows "Killed" | OOM — bump `mem=` |

## Related

- [[OpenMP PBS Script]] — what produced this output.
- [[../pbs/Log Files]] — generic PBS log conventions.
- [[OpenMP Pitfalls]] — root causes for the symptoms above.
