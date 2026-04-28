# MPI PBS Script — `pi_mpi.pbs`

The full PBS script for the MPI example, annotated. Lives in the repo at `examples/mpi/pi_mpi.pbs`.

```bash
#!/bin/bash
#PBS -N pi_mpi
#PBS -l walltime=00:10:00
#PBS -l select=1:ncpus=16:mem=4gb:mpiprocs=16:cpu_type=rome

cd $PBS_O_WORKDIR

ml tools/prod
ml GCC OpenMPI

mpiexec ./build/mpi/pi_mpi
```

## Why each line

- **`-l select=1:ncpus=16:mem=4gb:mpiprocs=16:cpu_type=rome`** — one chunk (one node), 16 cores, 16 MPI ranks, on a Rome node. `mpiprocs=16` tells PBS to place 16 ranks on this chunk; `mpiexec` (no `-n`) reads the count from PBS automatically.
- **`cpu_type=rome`** — every student's run lands on the same hardware class, so timings are comparable.
- **`cd $PBS_O_WORKDIR`** — see [[../pbs/PBS Environment Variables]].
- **`ml tools/prod` + `ml GCC OpenMPI`** — production catalogue, GCC compiler, Open MPI runtime. Without OpenMPI loaded, `mpiexec` is "command not found".
- **`mpiexec ./build/mpi/pi_mpi`** — launch all 16 ranks; PBS's hostfile supplies the rank count.

## Submitting

```bash
cd ic-hpc-intro/examples
qsub mpi/pi_mpi.pbs
qstat -u $USER
# wait until S = F:
cat pi_mpi.o*
```

Submit from `examples/`, **not** from `examples/mpi/` — the script's `./build/mpi/pi_mpi` path is relative to the submit CWD.

## Sweeping rank counts

Edit the directive (keeping `ncpus == mpiprocs`) and resubmit. Plot wall-time against `mpiprocs`:

| `mpiprocs` | Expected wall-time (relative) |
|---|---|
| 1 | 1.00 (serial baseline) |
| 4 | ~0.27 |
| 8 | ~0.14 |
| 16 | ~0.075 |
| 32 | ~0.045 |
| 64 | ~0.035 (flattening) |

The flattening past ~32 ranks reflects fork-join overhead, the reduce step's logarithmic cost, and CPU frequency drop under load. Pi is small; real codes flatten earlier.

## Multi-node variant

For 128 ranks across 2 nodes:

```bash
#PBS -l select=2:ncpus=64:mem=64gb:mpiprocs=64:cpu_type=rome
```

Two chunks → two nodes; 64 ranks per node × 2 = 128 total. This routes through the `v1_capability*` queues — heavier backlog, longer wait. For an intro demo, stay on one node.

See [[../pbs/Queues]] for queue mapping and [[../pbs/Resource Selection]] for the directive grammar.

## What you should see

`pi_mpi.o<jobid>`:

```
n=1000000000  ranks=16  pi=3.141592653589793  time=0.076s
===============================================
CPU Time used: 00:01:13
CPU Percent:   1581%
Memory usage:  52mb
```

`CPU Percent ~ 1600%` confirms 16 ranks were busy. Only rank 0 prints, so you'll see exactly one program-output line.

See [[Reading the MPI log]] for diagnostics.

## Related

- [[../examples/pi_mpi]] — the source code.
- [[mpiexec]] — the launcher.
- [[MPI Reduce]] — the collective in this program.
- [[Reading the MPI log]] — interpreting the output.
- [[../pbs/Job Script Anatomy]] — the general PBS template.
