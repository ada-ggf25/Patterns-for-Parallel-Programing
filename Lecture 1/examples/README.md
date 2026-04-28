# Examples — Pi by numerical integration

Three implementations of the same integral, used as the running example in the PPP intro deck:

$$
\pi \;\approx\; \frac{1}{N}\sum_{i=1}^{N}\frac{4}{1 + \left(\frac{i-\tfrac{1}{2}}{N}\right)^2}
$$

| Directory | What it shows |
|-----------|---------------|
| `serial/` | plain C++ baseline |
| `openmp/` | `#pragma omp parallel for reduction(+:sum)` |
| `mpi/`    | loop split across ranks + `MPI_Reduce` |

## Build and run on CX3

```bash
ssh user@login.cx3.hpc.ic.ac.uk
git clone <this repo> ic-hpc-intro
cd ic-hpc-intro/examples

ml tools/prod
ml GCC OpenMPI CMake

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

# Submit one of the PBS scripts (all submit from examples/, not the subdir).
qsub serial/pi_serial.pbs
qsub openmp/pi_openmp.pbs
qsub mpi/pi_mpi.pbs
```

After a job finishes, check its `.o<jobid>` / `.e<jobid>` log files in the submit directory.

## Build and run locally (Mac / Linux)

```bash
cd ic-hpc-intro/examples
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

./build/serial/pi_serial
OMP_NUM_THREADS=4 ./build/openmp/pi_openmp
mpiexec -n 4 ./build/mpi/pi_mpi
```

On macOS you need Homebrew's GCC (`brew install gcc`) and a real MPI (`brew install open-mpi`); Apple Clang ships without OpenMP by default.

## Problem size

All three programs use `N = 1e9` iterations, hard-coded. Edit the `const long long n = …` line in each source if you want to make a run shorter (for class demos) or longer (for benchmarking).

## Inspecting node topology

CX3 is a mixed-hardware cluster: the login node may be an Intel Xeon while the compute node your job lands on may be an AMD EPYC with a very different cache and NUMA layout. Don't benchmark or tune blind — dump the topology *from inside* the job, so the view is restricted to the cpuset PBS actually gave you.

| Command | What you get |
|---|---|
| `lscpu` | CPU model, sockets, cores-per-socket, cache sizes, summary NUMA line |
| `numactl --hardware` | NUMA nodes, CPUs per node, memory per node, **distance matrix** |
| `lstopo-no-graphics [--no-io]` | Full hierarchy: package → NUMA node → L3 → L2/L1 → core → PU (add PCIe without `--no-io`) |
| `hwloc-info` | One-line summary (counts of each level) |

### Picking a consistent host class

CX3 is heterogeneous: compute nodes are tagged at the PBS level as either `cpu_type=rome` (AMD EPYC 7742, 128 c/node, 8 NUMA domains) or `cpu_type=icelake` (Intel Xeon 8358, 64 c/node, 2 NUMA domains). Without pinning, the scheduler places your job on whichever is free — so the same benchmark run twice can land on different hardware and produce legitimately different numbers.

See what's out there:

```bash
pbsnodes -a | awk '
  /^cx3-/{n=$1} /cpu_type/{t=$3} /^[[:space:]]*$/{print t; t=""}
' | sort | uniq -c
```

Request a specific one by adding `:cpu_type=<name>` to the `select` line:

```
#PBS -l select=1:ncpus=8:mem=4gb:ompthreads=8:cpu_type=rome
#PBS -l select=1:ncpus=8:mem=4gb:ompthreads=8:cpu_type=icelake
```

This works like other per-node PBS resources on CX3: add the resource selector to the `select` line, and PBS only considers nodes matching it.

**All three scripts in this repo pin `cpu_type=rome`** by default, because (a) there are many more free rome nodes (fast queue starts), and (b) the 8-NUMA / 128-core Zen2 layout is a richer topology for the placement discussion below. Change to `cpu_type=icelake` for a simpler 2-NUMA teaching aid, at the cost of a longer wait.

**Modules.** `lscpu` is always on `PATH` (part of util-linux). The others ship in `hwloc` and `numactl`, which are **pulled in as dependencies of `OpenMPI`** under EasyBuild. So even for an OpenMP job you should:

```bash
ml tools/prod
ml GCC OpenMPI      # brings hwloc (lstopo, hwloc-info) and numactl onto PATH
```

Loading `GCC` alone is not enough. If you swap modules inside an existing shell, run `hash -r` — bash caches "command not found" lookups.

The OpenMP PBS script (`openmp/pi_openmp.pbs`) now runs all three inspection commands before the binary, so the `.o<jobid>` log shows you what hardware the scheduler handed you.

## Using topology to guide MPI / OpenMP placement

Once you can see the topology, the two levers that matter most are **thread/rank binding** (stop the OS migrating work off-core) and **locality** (keep data and the thread that uses it in the same NUMA domain).

### OpenMP

Set two environment variables:

```bash
export OMP_PROC_BIND=close    # pack threads onto adjacent PUs
export OMP_PLACES=cores       # smallest unit = one core (not hyperthread)
export OMP_DISPLAY_AFFINITY=TRUE   # prints one line per thread at startup
```

- `close` packs threads so they share a socket / L3 / NUMA node when possible — good when threads share data or when you fit inside one socket.
- `spread` distributes threads across the machine — good for bandwidth-bound kernels that want to hit multiple memory controllers simultaneously.

Our `pi_openmp` is compute-bound (no meaningful memory traffic), so affinity barely changes the timing — but pinning is still the right default, and it's what you want in place before you try to interpret a benchmark.

**CX3 reality check.** PBS does *not* guarantee contiguous cores. On the EPYC 7742 compute nodes (8 NUMA domains × 16 cores), an `ncpus=8` allocation frequently spans multiple NUMA nodes and even both sockets. `OMP_PROC_BIND=close` still pins threads, but "close" is only close within the cpuset you were given. If this matters for your workload, ask for a whole NUMA node (`select=1:ncpus=16:ompthreads=16` matches one EPYC NUMA domain exactly) or a whole socket.

### MPI

OpenMPI binds each rank to a core by default. To see where ranks landed, add `--report-bindings` to `mpiexec`. To place ranks NUMA-aware:

```bash
mpiexec --map-by numa --bind-to core ./build/mpi/pi_mpi
```

`--map-by numa` round-robins ranks across NUMA domains, which on the EPYC nodes means each rank gets its own local memory controller and L3 slice.

### Hybrid MPI + OpenMP

The common "one rank per NUMA domain, OpenMP inside" shape on a 64-core EPYC socket (4 NUMA × 16 cores per socket):

```bash
#PBS -l select=1:ncpus=64:mpiprocs=4:ompthreads=16

export OMP_PROC_BIND=close
export OMP_PLACES=cores
mpiexec --map-by ppr:1:numa:pe=16 --bind-to core ./your_hybrid_binary
```

This launches 1 rank per NUMA node, reserves 16 processing elements per rank for its OpenMP threads, and keeps each rank's threads bound inside its own NUMA domain — no cross-socket memory traffic, no threads sharing L1/L2 caches they don't need to share.

Rule of thumb: **`mpiprocs × ompthreads == ncpus`**, and pick `mpiprocs` so each rank lines up with a natural hardware boundary (NUMA node on EPYC, socket on Xeon).
