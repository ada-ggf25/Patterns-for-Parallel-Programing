# Loading Combos

The four module combinations covering everything in this course. Always load `tools/prod` first (see [[tools-prod gateway]]).

## Serial C/C++ — just GCC

```bash
ml tools/prod
ml GCC
```

`g++` lands on `PATH`, with `-fopenmp` baked into the compiler driver (so you can use OpenMP later without changing the module set, just by adding the flag to your build command).

## With CMake

```bash
ml tools/prod
ml GCC CMake
```

Necessary for the multi-target build in `examples/CMakeLists.txt`. CMake on its own — without GCC also loaded — will fall back to whatever `cc` / `c++` is on `PATH`, which is usually an old system compiler.

## OpenMP — same as above

OpenMP isn't a separate module; it ships inside `g++` as `-fopenmp`. The combo for OpenMP work:

```bash
ml tools/prod
ml GCC               # or: ml GCC CMake
```

**However** — if you also want topology tools (`lstopo`, `numactl`, `hwloc-info`), load OpenMPI even on a pure-OpenMP job:

```bash
ml tools/prod
ml GCC OpenMPI       # OpenMPI brings hwloc + numactl onto PATH
```

This is what the reference `pi_openmp.pbs` does. See [[../cluster/Topology Inspection]] for why.

## MPI — GCC + OpenMPI

```bash
ml tools/prod
ml GCC OpenMPI
```

Adds `mpicxx`, `mpicc`, `mpiexec`, `mpirun`, the MPI headers, and (as a side effect) `hwloc` and `numactl`.

Note: `mpiexec` / `mpirun` are **not** on the default PATH until OpenMPI is loaded. "command not found: mpiexec" in a job log nearly always means you forgot this line.

## Course default — everything

The combo we use for the running examples:

```bash
ml tools/prod
ml GCC OpenMPI CMake
```

Builds all three targets (`pi_serial`, `pi_openmp`, `pi_mpi`) and gives you topology tools for the OpenMP and MPI logs.

## Swapping versions

```bash
ml                                      # what's loaded?
ml switch GCC/13.3.0 GCC/14.2.0
ml                                      # confirm
```

Use `ml switch` rather than unload+load — it checks that any dependent modules (OpenMPI built against the old GCC) still make sense.

## Related

- [[tools-prod gateway]] — the prerequisite to all of these.
- [[Lmod Commands]] — discovery and inspection.
- [[Module Pitfalls]] — `hash -r`, bashrc traps.
