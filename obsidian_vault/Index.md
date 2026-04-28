# PPP HPC Intro — Vault Index

This vault collects everything you need to log in to Imperial's CX3 cluster, load the right environment modules, write and submit a PBS job, and parallelise a C++ program with both OpenMP and MPI.

The companion repo is `ppp-hpc-intro` (one level up from this vault). Source code, PBS scripts, and the Quarto deck live there — these notes are the study guide.

## Suggested reading order

1. [[cluster/CX3 Overview]] — what the cluster is.
2. [[cluster/Login and Authentication]] — how to connect.
3. [[cluster/Filesystems]] — where your files live.
4. [[modules/Modules Overview]] → [[modules/Lmod Commands]] → [[modules/tools-prod gateway]] → [[modules/Loading Combos]].
5. [[pbs/PBS Overview]] → [[pbs/Job Script Anatomy]] → [[pbs/PBS Directives]] → [[pbs/Resource Selection]] → [[pbs/qsub qstat qdel]] → [[pbs/Log Files]].
6. [[examples/Pi Algorithm]] then [[examples/pi_serial]].
7. [[openmp/OpenMP Overview]] → [[openmp/parallel directive]] → [[openmp/for directive]] → [[openmp/reduction clause]] → [[openmp/Variable Scoping]] → [[openmp/Thread Pinning]] → [[examples/pi_openmp]].
8. [[mpi/MPI Overview]] → [[mpi/SPMD Model]] → [[mpi/MPI Six Essentials]] → [[mpi/MPI Reduce]] → [[examples/pi_mpi]].
9. [[mpi/OpenMP vs MPI]] and [[mpi/Hybrid MPI OpenMP]] to wrap up.

## Map by topic

- **Cluster** — `cluster/`: hardware, login, filesystems, [[cluster/AMD Rome Architecture|Rome]] / [[cluster/Intel Ice Lake Architecture|Ice Lake]] internals, [[cluster/NUMA Latency|NUMA latency]], topology tools.
- **Modules** — `modules/`: Lmod cheat sheet, the [[modules/tools-prod gateway|tools/prod]] gateway, common combos.
- **PBS** — `pbs/`: directives, resource selection, queues, lifecycle commands, the [[pbs/cpu_type Selection|cpu_type selector]], log-file conventions.
- **OpenMP** — `openmp/`: directives, scoping, environment variables, [[openmp/Thread Pinning|pinning]], [[openmp/Amdahls Law|Amdahl]].
- **MPI** — `mpi/`: SPMD, the six essential calls, [[mpi/MPI Reduce|reductions]], `mpiexec`, hybrid mode.
- **Examples** — `examples/`: walk-throughs of `pi_serial`, `pi_openmp`, `pi_mpi`, build/run instructions, the [[examples/numa_latency benchmark|NUMA-latency benchmark]].

## Quick-reference one-liners

```bash
ssh me@login.cx3.hpc.ic.ac.uk          # connect (Imperial password)
ml tools/prod GCC OpenMPI CMake        # full toolchain for this course
qsub serial/pi_serial.pbs              # submit
qstat -u $USER                         # monitor
qdel <jobid>                           # cancel
cat pi_serial.o<jobid>                 # read the log
```
