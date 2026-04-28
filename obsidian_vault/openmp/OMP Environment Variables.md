# OpenMP Environment Variables

OpenMP behaviour is controlled at runtime by environment variables prefixed `OMP_`. The four you'll meet in this course:

| Variable | What it does |
|---|---|
| `OMP_NUM_THREADS` | Number of threads in a parallel region |
| `OMP_PLACES` | Smallest unit a thread can be pinned to |
| `OMP_PROC_BIND` | How to assign threads to places (`close` / `spread` / `master` / `false`) |
| `OMP_DISPLAY_AFFINITY` | Print one line per thread at startup showing its binding |

## `OMP_NUM_THREADS`

```bash
export OMP_NUM_THREADS=8
./pi_openmp
```

Without it, the OpenMP runtime picks its own default — usually the number of *visible* CPUs. Inside a PBS job that's the cpuset PBS gave you, but the runtime may also count hyperthreads, leading to over-subscription. **Always set it explicitly inside PBS.**

In a PBS script, the directive `ompthreads=N` is just shorthand:

```bash
#PBS -l select=1:ncpus=8:mem=4gb:ompthreads=8         # equivalent to:
# export OMP_NUM_THREADS=8                              (set automatically)
```

## `OMP_PLACES`

What's the smallest unit a thread can be pinned to?

| Value | Meaning |
|---|---|
| `cores` | One core per place. Hyperthreads on the same core form one place. |
| `threads` | One hardware thread (PU) per place — pins down to a hyperthread. |
| `sockets` | One socket per place. |
| Custom list | e.g. `{0,1,2,3},{4,5,6,7}` — fully explicit |

For compute-bound kernels (like π), `cores` is what you want — you don't gain from putting two threads on the same core's hyperthreads.

## `OMP_PROC_BIND`

How are threads distributed across places?

| Value | Behaviour |
|---|---|
| `false` | No binding (kernel may migrate threads — bad for cache/NUMA). |
| `master` | All threads bound where the master is. |
| `close` | Pack threads onto adjacent places. Good when threads share data. |
| `spread` | Distribute threads as widely as possible. Good for bandwidth-bound kernels. |

See [[Thread Pinning]] for the choice between `close` and `spread`.

## `OMP_SCHEDULE`

```bash
export OMP_SCHEDULE="dynamic,64"
./integrate
```

Controls the schedule used when a `parallel for` loop has `schedule(runtime)`. The format is `kind[,chunk]` — e.g., `"dynamic,64"`, `"guided"`, `"static,32"`.

Lets you experiment with different schedules without recompiling. Useful during A1 schedule sweep: compile once with `schedule(runtime)` and sweep `OMP_SCHEDULE` values from the shell.

## `OMP_DISPLAY_ENV`

```bash
export OMP_DISPLAY_ENV=TRUE
./integrate
```

Prints all active OpenMP environment variables and runtime settings at program start (to stderr). More comprehensive than `OMP_DISPLAY_AFFINITY` — shows `OMP_NUM_THREADS`, `OMP_PLACES`, `OMP_PROC_BIND`, `OMP_SCHEDULE`, and more. Useful as a sanity check before a benchmark run.

## `OMP_DISPLAY_AFFINITY`

```bash
export OMP_DISPLAY_AFFINITY=TRUE
```

Causes the runtime to print one line per thread at startup, showing which OS proc set (= which core) it pinned to. Goes to stderr — in a PBS job, ends up in the `.e<jobid>` file.

Useful for confirming pinning *actually* happened rather than just hoping it did.

## What the A1 PBS script sets

The A1 evaluation PBS script (`assignment-1/evaluate.pbs`) sets:

```bash
#PBS -l select=1:ncpus=128:mem=400gb:cpu_type=rome:mpiprocs=1:ompthreads=128
#PBS -l place=excl

export OMP_PROC_BIND=close
export OMP_PLACES=cores
export OMP_DISPLAY_ENV=TRUE
```

`place=excl` requests an exclusive node — no other jobs share the 128 cores. This is essential for reproducible benchmark numbers.

## Related

- [[Thread Pinning]] — the `close` vs `spread` choice.
- [[OpenMP PBS Script]] — the running example.
- [[../pbs/Resource Selection]] — the `ompthreads=` directive.
