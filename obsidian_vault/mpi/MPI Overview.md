# MPI — Overview

MPI (Message Passing Interface) is the standard for **distributed-memory** parallelism. The same executable is launched many times (once per *rank*), each rank is a separate OS process with its own memory, and ranks communicate by sending explicit messages.

## Mental model

- N processes, no shared address space.
- To share data, one rank sends; another rank receives. No silent communication.
- Each rank knows its own ID (`rank`) and the team size (`size`); it picks its slice of the data accordingly.

## Pros

- **Scales beyond one node.** Thousands of cores routinely; over a million at the high end. The course's CX3 cluster has ~600 nodes; the InfiniBand interconnect makes inter-node messaging fast.
- **No shared-memory race conditions** by construction — each rank's data is its own.
- **Mature ecosystem.** Open MPI, MPICH, Intel MPI, MVAPICH all interoperate at the source level; vendor-tuned for specific hardware.

## Cons

- **More boilerplate** than OpenMP. You explicitly initialise, partition, communicate, and finalise.
- **Communication is your problem.** Send to the wrong rank, deadlock. Forget a collective on one rank, deadlock. Pack inefficiently, slow.
- **Debugging is harder.** Bugs span multiple processes; printf is your friend; serious work needs rank-aware debuggers.

## When to reach for MPI

- Your problem doesn't fit in one node's RAM.
- You want to scale beyond ~128 cores.
- You need explicit control over data placement (which rank owns what).

## SPMD — Single Program, Multiple Data

The execution model. See [[SPMD Model]].

## The minimum working program

Six function calls cover most of what you need. See [[MPI Six Essentials]].

## Course path

1. [[SPMD Model]] — the execution model.
2. [[MPI Six Essentials]] — the API surface you really need.
3. [[MPI Init Finalize]] — bookends.
4. [[Rank and Size]] — identifying yourself.
5. [[MPI Reduce]] — collective summation.
6. [[Building MPI]] — `mpicxx` and CMake.
7. [[mpiexec]] — launching MPI jobs.
8. [[MPI PBS Script]] — putting it together on CX3.
9. [[MPI Pitfalls]] — common foot-guns.
10. [[OpenMP vs MPI]] — choosing.
11. [[Hybrid MPI OpenMP]] — combining.

## Related

- [[../openmp/OpenMP Overview]] — the shared-memory alternative.
- [[../examples/pi_mpi]] — running example.
