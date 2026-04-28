# Common PBS Mistakes

A checklist for when a job fails. Each of these has bitten everyone at least once.

## "file not found" at runtime

**Cause:** missing `cd $PBS_O_WORKDIR`. Job started in `$HOME`, where your input files don't exist.

**Fix:** every PBS script begins with:

```bash
cd $PBS_O_WORKDIR
```

See [[PBS Environment Variables]].

## `g++: command not found`

**Cause:** missing `ml GCC`. Login nodes don't have GCC on the default PATH; you must load it.

**Fix:**

```bash
ml tools/prod
ml GCC
```

Note that `tools/prod` is required first — see [[../modules/tools-prod gateway]].

## `mpiexec: command not found`

**Cause:** missing `ml OpenMPI` (or you forgot it's separate from GCC).

**Fix:**

```bash
ml tools/prod
ml GCC OpenMPI
```

## "module not found" inside the job

**Cause:** missing `ml tools/prod` first, or a typo in the module name (Lmod is case-sensitive: `OpenMPI` not `openmpi`).

**Fix:** double-check by running the exact same `ml` lines on a login node.

## OpenMP runs with `threads=1`

**Cause:** missing `ompthreads=N` in the `select=` line, or the binary was compiled without `-fopenmp`.

**Fix:**

- Set `ompthreads=N` to match `ncpus=N` in the directive.
- Or set `export OMP_NUM_THREADS=N` in the script body.
- Verify the build used `-fopenmp` (CMake's `find_package(OpenMP)` arranges this).

See [[../openmp/OMP Environment Variables]] and [[../openmp/Building OpenMP]].

## Mismatched `ncpus` and `mpiprocs` / `ompthreads`

**Cause:** asked for 16 CPUs but only set 8 ranks/threads — wasting half the allocation.

**Fix:** keep them equal for pure MPI / pure OpenMP. For hybrid: `mpiprocs × ompthreads == ncpus`.

## Job killed mid-run by walltime

**Cause:** `-l walltime=` set too low.

**Fix:** estimate, then add 50%. The cost of an over-estimate is a slightly slower start; the cost of an under-estimate is losing all in-flight work.

## OOM kill / `Killed` in `.e` file

**Cause:** `-l mem=` too low, process exceeded the request.

**Fix:** measure peak RSS from a successful smaller run, request 20% more. Note `mem=` is **per chunk**, not total — for `select=2:mem=4gb`, total ask is 8 GB.

## Job stuck in `Q` for hours

**Cause:** the `select=` line asks for resources the cluster doesn't have free right now (e.g. multi-node with `cpu_type=icelake` during a busy period), or a queue is saturated.

**Fix:** check with `qstat -Q` and `pbsnodes -a`. Try shrinking the request, swapping `cpu_type`, or staying on one node. See [[Queues]] and [[check queue busyness]].

## Related

- [[Job Script Anatomy]] — the canonical correct shape.
- [[../modules/Module Pitfalls]] — overlapping module-side checklist.
