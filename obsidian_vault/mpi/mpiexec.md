# `mpiexec` — Launching MPI Jobs

`mpiexec` is the launcher for an MPI program. It starts N copies of the executable, sets each one's rank, and wires them up to the MPI runtime so they can communicate.

## Locally (on your laptop)

```bash
mpiexec -n 4 ./pi_mpi
```

- **`-n 4`** — launch 4 ranks (this is mandatory off-cluster).
- The executable path must be reachable from each rank — usually trivial locally.

## On CX3 (inside a PBS job)

```bash
mpiexec ./build/mpi/pi_mpi
```

- **No `-n`** — PBS exposes the rank count via `$PBS_NODEFILE`, and `mpiexec` reads it automatically.
- The number of ranks is whatever the `mpiprocs=N` directive said.

This is the difference that catches people: locally you must say how many ranks; under PBS you must *not* (or you'll fight with the scheduler).

## A few useful flags

```bash
mpiexec --report-bindings ./pi_mpi              # show which core each rank pinned to
mpiexec --map-by numa --bind-to core ./pi_mpi   # one rank per NUMA, pinned to its core
mpiexec --map-by ppr:1:numa:pe=16 --bind-to core ./hybrid    # 1 rank per NUMA, 16 cores per rank
```

- **`--report-bindings`** — prints one line per rank at startup showing its core(s). Goes to stderr.
- **`--map-by numa`** — distribute ranks one per NUMA domain (round-robin). On Rome that's 8 ranks per node, each on its own NUMA.
- **`--bind-to core`** — pin each rank to a single core (default for OpenMPI).
- **`--map-by ppr:1:numa:pe=16`** — "Place 1 rank per NUMA domain, reserving 16 processing elements (cores) per rank." Used for hybrid MPI+OpenMP — see [[Hybrid MPI OpenMP]].

## What "shared memory" means inside one node

Even when MPI ranks live on the same node, their messages do **not** go over the network — Open MPI uses shared memory under the hood for intra-node messaging. So a 16-rank single-node MPI run is much faster than 16 ranks across two nodes for the same total amount of work.

## How OpenMPI knows what cores it has

Inside a PBS job, OpenMPI reads `$PBS_NODEFILE` to learn:

- The list of nodes assigned to this job.
- How many slots (ranks) each node should host.

You don't manage this manually; PBS sets `$PBS_NODEFILE` and OpenMPI's `mpiexec` honours it.

## Common mistakes

- `mpiexec: command not found` → forgot `ml OpenMPI`. See [[../modules/Loading Combos]].
- Passing `-n N` inside PBS → may double-launch or fight the scheduler. Just leave it off.
- Running `./pi_mpi` directly (no `mpiexec`) → the program runs as a single rank with `size=1`. It "works" but isn't using MPI.

## Related

- [[Building MPI]] — how the binary is produced.
- [[MPI PBS Script]] — `mpiexec` in context.
- [[Hybrid MPI OpenMP]] — `--map-by ppr:` for hybrid runs.
- [[MPI Pitfalls]]
