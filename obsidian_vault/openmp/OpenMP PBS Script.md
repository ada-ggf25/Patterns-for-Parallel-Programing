# OpenMP PBS Script — `pi_openmp.pbs`

The full PBS script for the OpenMP example, annotated. Lives in the repo at `examples/openmp/pi_openmp.pbs`.

```bash
#!/bin/bash
#PBS -N pi_openmp
#PBS -l walltime=00:10:00
#PBS -l select=1:ncpus=8:mem=4gb:ompthreads=8:cpu_type=rome

cd $PBS_O_WORKDIR
ml tools/prod
ml GCC OpenMPI                  # OpenMPI brings hwloc + numactl onto PATH

# --- Topology block: dump the cpuset PBS gave us ---
echo "=== lscpu (summary) ==="
lscpu | sed -n '1,25p'
echo
echo "=== numactl --hardware ==="
numactl --hardware
echo
echo "=== lstopo-no-graphics ==="
lstopo-no-graphics --no-io
echo "=== end topology ==="
echo

# --- OpenMP pinning ---
export OMP_PROC_BIND=close
export OMP_PLACES=cores
export OMP_DISPLAY_AFFINITY=TRUE

./build/openmp/pi_openmp
```

## Why each line

- **`-l select=1:ncpus=8:...:ompthreads=8:cpu_type=rome`** — one chunk (one node), 8 cores, 8 OpenMP threads, on a Rome node. `ompthreads=8` automatically sets `OMP_NUM_THREADS=8` inside the job. `cpu_type=rome` ensures every student's run lands on the same hardware class.
- **`cd $PBS_O_WORKDIR`** — without it the job starts in `$HOME` and `./build/openmp/pi_openmp` doesn't exist there. See [[../pbs/PBS Environment Variables]].
- **`ml tools/prod` + `ml GCC OpenMPI`** — production catalogue + compiler. We load `OpenMPI` even on a pure-OpenMP job because it pulls in `hwloc` (`lstopo`) and `numactl` for the topology block. See [[../modules/Loading Combos]].
- **Topology block** — `lscpu` + `numactl --hardware` + `lstopo-no-graphics --no-io`. Run *inside* the job so the view is restricted to the cpuset PBS gave us. See [[../cluster/Topology Inspection]].
- **`OMP_PROC_BIND=close`, `OMP_PLACES=cores`** — pin threads to cores, packed close together. For our compute-bound π this barely changes timing, but it's the right default. See [[Thread Pinning]].
- **`OMP_DISPLAY_AFFINITY=TRUE`** — one line per thread in the `.e<jobid>` file confirming where each one ended up.

## Submitting

```bash
cd ic-hpc-intro/examples
qsub openmp/pi_openmp.pbs
qstat -u $USER
# wait until S = F:
cat pi_openmp.o*
```

Submit from `examples/`, **not** from `examples/openmp/` — the script's `./build/openmp/pi_openmp` path is relative to the submit CWD.

## What you should see

`pi_openmp.o<jobid>`:

```
=== lscpu (summary) ===
... 25 lines of CPU info ...
=== numactl --hardware ===
... NUMA layout + distance matrix ...
=== lstopo-no-graphics ===
... full hierarchy ...
=== end topology ===

n=1000000000  threads=8  pi=3.141592653589821  time=0.163s
===============================================
CPU Time used: 00:01:18
CPU Percent:   789%
Memory usage:  18mb
Walltime:      00:00:01
```

`CPU Percent ~ 800%` confirms 8 cores were active.
`pi_openmp.e<jobid>` will hold one OMP-affinity line per thread.

## Scaling experiments

Edit the `ncpus`/`ompthreads` numbers (keeping them equal) and resubmit:

| `ncpus`/`ompthreads` | Expected speedup vs serial |
|---|---|
| 1 | 1.0× (sanity check) |
| 4 | ~3.5–4× |
| 8 | ~6–7× |
| 16 | ~10–13× |
| 64 | ~30–40× |
| 128 | ~60–80× (top-out) |

See [[Amdahls Law]] for why we don't see 8× from 8 threads.

## Related

- [[../examples/pi_openmp]] — the source code.
- [[OMP Environment Variables]] — what `ompthreads` and the exports do.
- [[Thread Pinning]] — `close` vs `spread`.
- [[Reading the OpenMP log]] — interpreting the output.
- [[../pbs/Job Script Anatomy]] — the general PBS template.
