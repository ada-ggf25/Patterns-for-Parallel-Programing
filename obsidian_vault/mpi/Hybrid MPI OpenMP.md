# Hybrid MPI + OpenMP

The combination: **one MPI rank per NUMA domain (or per node), running OpenMP threads inside each rank.**

```bash
#PBS -l select=2:ncpus=64:mem=64gb:mpiprocs=1:ompthreads=64
```

This means: 2 chunks (= 2 nodes), 64 cores per node, 1 MPI rank per node, 64 OpenMP threads per rank.

## Why bother

Pure MPI with 128 ranks on 2 nodes works fine for most problems. Hybrid trades MPI rank count for OpenMP thread count within each rank. The benefits:

- **One copy of read-only data per node, not per core.** If your application has a 50 GB lookup table that doesn't change, hybrid uses 50 GB total per node instead of 50 GB × 64 ranks = 3.2 TB (which won't fit).
- **Fewer MPI messages.** 2 ranks talking to each other beats 128 ranks talking to each other when the message pattern is collective.
- **Better locality knobs.** Each rank's OpenMP threads stay in one NUMA domain; cross-NUMA traffic only happens at MPI message boundaries, not within the rank.

## When it doesn't help

- **Pure-compute workloads with no shared data.** No memory savings to win; you've just added a layer of complexity.
- **Workloads where MPI scales fine.** If the simple "one rank per core" works, don't bother.
- **Embarrassingly parallel problems.** Like π — adding OpenMP inside MPI ranks gains nothing here.

## Picking the per-rank shape

The natural hardware boundaries to align with:

- **One rank per NUMA domain.** On Rome that's 8 ranks per node, each with 16 cores. Natural fit because each rank's threads stay in one NUMA domain.
- **One rank per socket.** On Rome that's 2 ranks per node, each with 64 cores. Coarser but still NUMA-aware (each socket has 4 NUMAs internally; OpenMP must cope).
- **One rank per node.** Maximum data sharing; maximum thread count per rank.

Rule: **`mpiprocs × ompthreads == ncpus`** per chunk, and pick `mpiprocs` so each rank lines up with a hardware boundary.

## PBS shape examples (on Rome, NPS4)

| Layout | `select=` line |
|---|---|
| 1 node, 1 rank/NUMA, 16 threads/rank | `1:ncpus=128:mpiprocs=8:ompthreads=16` |
| 1 node, 1 rank/socket, 64 threads/rank | `1:ncpus=128:mpiprocs=2:ompthreads=64` |
| 2 nodes, 1 rank/node, 64 threads/rank | `2:ncpus=64:mpiprocs=1:ompthreads=64` |
| 2 nodes, 1 rank/NUMA, 16 threads/rank | `2:ncpus=128:mpiprocs=8:ompthreads=16` (16 ranks total) |

## `mpiexec` for hybrid

```bash
export OMP_PROC_BIND=close
export OMP_PLACES=cores
mpiexec --map-by ppr:1:numa:pe=16 --bind-to core ./your_hybrid_binary
```

- `--map-by ppr:1:numa:pe=16` — "Place 1 rank per NUMA domain, reserving 16 processing elements per rank for its OpenMP threads."
- `--bind-to core` — keeps each thread bound to its core within the rank's reservation.

This is the cleanest layout: each rank's 16 threads stay together in one NUMA domain; cross-NUMA traffic only crosses an MPI boundary.

## Out of scope for this course

The intro deck mentions hybrid as a teaser. It's covered properly in week 2–3 alongside MPI itself. For week 1, **stick with pure OpenMP** for the OpenMP assessment.

## Related

- [[OpenMP vs MPI]]
- [[../openmp/Thread Pinning]]
- [[../cluster/AMD Rome Architecture]] — NUMA layout to map to.
- [[mpiexec]] — the `--map-by` flag.
- [[../pbs/Resource Selection]] — the `select=` grammar.
