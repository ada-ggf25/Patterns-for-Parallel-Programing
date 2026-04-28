# Job Script Anatomy

A PBS script is a normal bash script with a block of `#PBS` "comments" at the top that PBS reads as directives.

```bash
#!/bin/bash
#PBS -N my_job
#PBS -l walltime=00:30:00
#PBS -l select=1:ncpus=8:mem=8gb

cd $PBS_O_WORKDIR

ml tools/prod
ml GCC

./my_program
```

## The three sections

### 1. Header — `#PBS` directives

Every line beginning with `#PBS` is parsed by PBS at submission time and ignored by bash. The directives go **before any command** in the script — once a non-comment line appears, PBS stops reading directives.

The most common ones:

- `-N <jobname>` — name shown in `qstat` and used in log file names.
- `-l walltime=HH:MM:SS` — max wall-clock time before PBS kills the job.
- `-l select=...` — what to allocate; full grammar in [[PBS Directives]].
- `-o path` / `-e path` — redirect stdout/stderr (default: `<jobname>.o<jobid>` / `<jobname>.e<jobid>`).

See [[PBS Directives]] for the full set.

### 2. Setup — environment

Two universal lines:

```bash
cd $PBS_O_WORKDIR        # avoid the CWD trap (see PBS Environment Variables)
ml tools/prod            # unlock the production module catalogue
ml GCC OpenMPI CMake     # load whatever this job needs
```

Optionally, configure runtime environment:

```bash
export OMP_NUM_THREADS=8        # if you didn't set ompthreads= in the directives
export OMP_PROC_BIND=close      # see ../openmp/Thread Pinning
export OMP_PLACES=cores
```

### 3. Body — what to run

Just bash. Anything that runs on the command line works here. For multiple programs, run them one after another. For an MPI binary, prefix with `mpiexec`:

```bash
./build/serial/pi_serial
# or
mpiexec ./build/mpi/pi_mpi
# or
OMP_NUM_THREADS=8 ./build/openmp/pi_openmp
```

## Real example from the repo — `pi_openmp.pbs`

```bash
#!/bin/bash
#PBS -N pi_openmp
#PBS -l walltime=00:10:00
#PBS -l select=1:ncpus=8:mem=4gb:ompthreads=8:cpu_type=rome

cd $PBS_O_WORKDIR

ml tools/prod
ml GCC OpenMPI

# Show what the scheduler gave us:
echo "=== lscpu ===";              lscpu | sed -n '1,25p'
echo "=== numactl --hardware ==="; numactl --hardware
echo "=== lstopo --no-io ===";     lstopo-no-graphics --no-io

export OMP_PROC_BIND=close
export OMP_PLACES=cores
export OMP_DISPLAY_AFFINITY=TRUE

./build/openmp/pi_openmp
```

This is the canonical pattern: directives → CWD fix → modules → topology dump → environment for the run → the binary.

## Related

- [[PBS Directives]] — directive grammar.
- [[Resource Selection]] — `select=` worked examples.
- [[PBS Environment Variables]] — `$PBS_O_WORKDIR` and friends.
- [[Log Files]] — what the directives produce as output.
