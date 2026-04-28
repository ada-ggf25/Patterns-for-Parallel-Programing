# Running the Examples on CX3

End-to-end recipe, from `ssh` to reading the output.

## One-time setup

```bash
ssh myusername@login.cx3.hpc.ic.ac.uk
git clone <this repo> ic-hpc-intro
cd ic-hpc-intro/examples
ml tools/prod GCC OpenMPI CMake
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Verify:

```bash
ls build/serial build/openmp build/mpi
# should show pi_serial, pi_openmp, pi_mpi
```

See [[Building the Examples]] for the build details.

## Submit all three

The PBS scripts live in the per-target subdirectories, but you submit from `examples/`:

```bash
cd ic-hpc-intro/examples         # important — see "submission CWD" below

qsub serial/pi_serial.pbs
qsub openmp/pi_openmp.pbs
qsub mpi/pi_mpi.pbs
```

Each `qsub` returns a job ID. Watch them:

```bash
qstat -u $USER
```

States: `Q` (queued), `R` (running), `F` (finished). See [[../pbs/Job States]].

## Read the logs

When state hits `F`:

```bash
cat pi_serial.o*
cat pi_openmp.o*
cat pi_mpi.o*
```

Expected output for the OpenMP run (with the topology block from the script):

```
... topology dump ...
n=1000000000  threads=8  pi=3.141592653589821  time=0.163s
===============================================
CPU Time used: 00:01:18
CPU Percent:   789%
Memory usage:  18mb
Walltime:      00:00:01
```

For interpretation see [[../openmp/Reading the OpenMP log]] and [[../mpi/Reading the MPI log]].

## Submission CWD — why it matters

The PBS scripts include lines like:

```bash
./build/openmp/pi_openmp
```

This is **relative to the directory you ran `qsub` from**, because that's where `cd $PBS_O_WORKDIR` puts you. If you submit from `examples/openmp/`, the path becomes `examples/openmp/build/openmp/pi_openmp` — which doesn't exist.

**Always submit from `examples/`.**

## Iterating on rank/thread counts

The PBS scripts are short — feel free to copy and edit. To run the OpenMP version with 64 threads:

```bash
cp openmp/pi_openmp.pbs openmp/pi_openmp_64.pbs
sed -i 's/ompthreads=8/ompthreads=64/' openmp/pi_openmp_64.pbs
sed -i 's/ncpus=8/ncpus=64/'           openmp/pi_openmp_64.pbs
qsub openmp/pi_openmp_64.pbs
```

(Always change `ncpus` and `ompthreads` together for OpenMP — see [[../pbs/Resource Selection]].)

## Cleaning up

After harvesting timings:

```bash
mkdir -p old_logs
mv pi_*.o* pi_*.e* old_logs/
```

Or, for a clean slate:

```bash
make examples-clean       # rm -rf examples/build
```

## Related

- [[Building the Examples]] — build details.
- [[Running Locally]] — laptop workflow.
- [[../pbs/qsub qstat qdel]] — the lifecycle commands.
- [[../pbs/Log Files]] — log-file conventions.
- [[pi_serial]] / [[pi_openmp]] / [[pi_mpi]] — per-target notes.
