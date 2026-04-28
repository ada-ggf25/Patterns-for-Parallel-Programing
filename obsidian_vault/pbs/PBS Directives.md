# PBS Directives

The `#PBS` directives at the top of a job script are PBS's only input from you about *what* to run. Get them right and PBS does the rest.

## The directives you'll actually use

| Directive | Meaning |
|---|---|
| `-N <jobname>` | Name shown in `qstat`; used as log-file prefix |
| `-l walltime=HH:MM:SS` | Max wall-clock time before PBS kills the job |
| `-l select=N:ncpus=C:mem=Xgb` | Resource request — N "chunks" (≈ nodes), each with C CPUs and X GB RAM |
| `-l select=N:...:ompthreads=C` | Extra: also set `OMP_NUM_THREADS=C` inside the job |
| `-l select=N:...:mpiprocs=C` | Extra: number of MPI ranks per chunk |
| `-l select=N:...:cpu_type=rome` | Extra: pin to Rome (or `icelake`) — see [[cpu_type Selection]] |
| `-o <path>` | Redirect stdout to a file (default: `<jobname>.o<jobid>`) |
| `-e <path>` | Redirect stderr to a file (default: `<jobname>.e<jobid>`) |

## The two `-l` directives — `walltime` and `select`

These two appear on every script.

### `-l walltime=HH:MM:SS`

The maximum wall-clock time PBS will let your job run. If you exceed it, PBS kills the job. Set it generously — a job killed at 9:59 of a 10-minute estimate has wasted compute and produced nothing.

But not *too* generously: longer walltime requests sit in slower queues. For our `pi_*` examples, 10 minutes is more than enough.

### `-l select=...`

The resource line. Grammar:

```
select=<N>:ncpus=<C>:mem=<X>gb[:ompthreads=<T>][:mpiprocs=<R>][:cpu_type=<...>]
```

- **`N`** — number of chunks. A chunk is roughly "a piece of one node": PBS will fit each chunk entirely on a single physical node.
- **`ncpus`** — CPUs per chunk.
- **`mem`** — memory per chunk.
- **`ompthreads`** — *optional*. If set, PBS exports `OMP_NUM_THREADS=<value>` inside the job.
- **`mpiprocs`** — *optional*. Number of MPI ranks placed on this chunk; consumed by `mpiexec`.
- **`cpu_type`** — *optional*. Pin chunks to nodes with the matching `cpu_type` resource. See [[cpu_type Selection]].

## Per-resource defaults

If you omit `ompthreads`, OpenMP picks its default thread count (usually = visible CPUs, often *not* what you want inside a constrained PBS allocation). If you omit `mpiprocs`, `mpiexec` defaults to one rank per chunk.

Best practice: **always set them explicitly** for OpenMP and MPI jobs.

## Multiple `-l` lines stack

You can split into separate lines for readability — PBS concatenates them:

```bash
#PBS -l walltime=01:00:00
#PBS -l select=1:ncpus=8:mem=8gb:ompthreads=8:cpu_type=rome
```

This is the same as putting both on one line.

## Where directives go

All `#PBS` lines must be at the **top** of the script, before any command. Once PBS sees a non-comment line, it stops parsing directives.

```bash
#!/bin/bash
#PBS -N good                # ✓ parsed
#PBS -l walltime=00:10:00   # ✓ parsed
echo "hello"
#PBS -l select=1:ncpus=1    # ✗ ignored — too late
```

## Related

- [[Job Script Anatomy]] — where these live.
- [[Resource Selection]] — worked examples.
- [[cpu_type Selection]] — pinning the architecture.
- [[Log Files]] — what `-o` and `-e` produce.
