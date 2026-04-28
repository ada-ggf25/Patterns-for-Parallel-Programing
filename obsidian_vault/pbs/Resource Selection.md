# Resource Selection

Worked examples of the `-l select=...` directive for the three program shapes you'll meet in this course.

## Serial — one core, modest memory, short walltime

```bash
#PBS -l walltime=00:30:00
#PBS -l select=1:ncpus=1:mem=4gb:cpu_type=rome
```

One chunk, one CPU, 4 GB RAM, on a Rome node. Standard for `pi_serial.pbs` (which actually only asks 10 minutes).

## OpenMP — one node, eight threads

```bash
#PBS -l walltime=01:00:00
#PBS -l select=1:ncpus=8:mem=8gb:ompthreads=8:cpu_type=rome
```

OpenMP is shared-memory: all threads must live on the same node, so `select=1` (one chunk = one node). `ompthreads=8` matches `ncpus=8` so OpenMP launches 8 threads — see [[../openmp/OMP Environment Variables]].

**Rule:** for OpenMP, set `ncpus == ompthreads`. Mismatch wastes cores or over-subscribes.

## MPI — sixteen ranks on one node

```bash
#PBS -l walltime=01:00:00
#PBS -l select=1:ncpus=16:mem=8gb:mpiprocs=16:cpu_type=rome
```

`mpiprocs=16` tells PBS to place 16 MPI ranks on this chunk. `mpiexec` (with no `-n`) reads PBS's hostfile and launches exactly that many.

**Rule:** for pure MPI, set `ncpus == mpiprocs` so each rank gets one core.

## MPI — multi-node, 128 ranks

```bash
#PBS -l walltime=02:00:00
#PBS -l select=2:ncpus=64:mem=64gb:mpiprocs=64:cpu_type=rome
```

Two chunks (= two nodes on Rome), 64 ranks per node, 128 total. This routes through the `v1_capability*` queues, which queue longer than the small/medium queues. See [[Queues]].

## Hybrid MPI + OpenMP — one rank per node, OpenMP inside

```bash
#PBS -l walltime=02:00:00
#PBS -l select=2:ncpus=64:mem=64gb:mpiprocs=1:ompthreads=64
```

One MPI rank per node, each rank running 64 OpenMP threads. Saves memory (one copy of any per-rank data structure per node, not per core) and cuts MPI message volume. See [[../mpi/Hybrid MPI OpenMP]].

**Rule:** `mpiprocs × ompthreads == ncpus` per chunk.

## Sizing rules of thumb

- **Walltime:** estimate, then add 50%. Killed jobs lose all in-flight work.
- **Memory:** measure once, request 20% over the peak. Under-requesting `mem` can crash the job; over-requesting just delays it in the queue.
- **`ncpus`:** ask for what you'll use. Asking for the whole node when you only run 8 threads is unfair to other users and pushes you into a slower queue.

## What happens if you mis-size

| Mistake | Result |
|---|---|
| Walltime too short | PBS kills the job mid-run; output truncated |
| Walltime too long | Job lands in a slower queue; longer wait |
| `mem` too low | Process killed by OOM; "Killed" in `.e` file |
| `mem` too high | Eligible nodes restricted; queues longer |
| `ncpus` < threads/ranks | Wasted cores (over-subscribed cpuset) |
| `ncpus` > threads/ranks | Wasted allocation; queue penalty |

## Related

- [[PBS Directives]] — full directive grammar.
- [[cpu_type Selection]] — the architecture pin.
- [[Queues]] — which queue your `select=` ends up in.
- [[../openmp/OpenMP PBS Script]] — applied to the running OpenMP example.
- [[../mpi/MPI PBS Script]] — applied to the running MPI example.
