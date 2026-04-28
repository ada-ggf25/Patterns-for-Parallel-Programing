# PBS Overview

CX3's job scheduler is **PBS Professional** (often "PBS Pro" or just "PBS"). PBS does five things:

1. Tracks which compute nodes are free.
2. Sorts incoming requests into **queues** by size and time.
3. Starts your job when matching resources become free.
4. Monitors it while it runs.
5. Captures stdout and stderr into log files.

Your half of the contract is to tell PBS **exactly** what you need — cores, memory, walltime — and to write a script PBS can run.

## Login vs compute — the rule

- **Login nodes** are shared. The MOTD says no heavy work; admins enforce it. "Heavy" = uses multiple cores for more than a minute, or more than ~4 GB memory, or hammers the disk.
- **Compute nodes** are allocated by PBS. You cannot ssh to them directly.
- The right path is always: log in → write a PBS script → submit with `qsub`.

Why this matters: 200 students all compiling at once on the same login node means nobody gets anything done, and the admins start killing processes.

## The minimum useful PBS script

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

Five things to notice:

1. **Shebang** — a normal bash script.
2. **`#PBS` directives** at the top tell PBS what resources to allocate. They look like comments to bash; PBS reads them before launching the job.
3. **`cd $PBS_O_WORKDIR`** — without this you start in `$HOME` (see [[PBS Environment Variables]]).
4. **`ml ...`** — load modules explicitly inside the job (see [[../modules/Module Pitfalls]]).
5. **The actual command** to run.

See [[Job Script Anatomy]] for a fuller walk-through.

## Authoritative documentation

- OpenPBS upstream: <https://www.openpbs.org/>
- RCS PBS guide: <https://icl-rcs-user-guide.readthedocs.io/>
- Queue picker: <https://icl-rcs-user-guide.readthedocs.io/en/latest/hpc/queues/job-sizing-guidance/>

A common alternative scheduler on other HPC systems is SLURM (<https://github.com/schedmd/slurm>) — most concepts transfer, the syntax differs.

## Related

- [[Job Script Anatomy]]
- [[PBS Directives]]
- [[Resource Selection]]
- [[qsub qstat qdel]]
- [[Log Files]]
