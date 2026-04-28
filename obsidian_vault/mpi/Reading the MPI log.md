# Reading the MPI log

A successful `pi_mpi` run produces `pi_mpi.o<jobid>` (stdout + PBS accounting) and `pi_mpi.e<jobid>` (stderr).

## `pi_mpi.o<jobid>`

```
n=1000000000  ranks=16  pi=3.141592653589793  time=0.076s
===============================================
CPU Time used: 00:01:13
CPU Percent:   1581%
Memory usage:  52mb
Walltime:      00:00:01
```

Only **rank 0** prints (because the program guards its output with `if (rank == 0)`). So you'll see exactly one program-output line per run, regardless of how many ranks ran.

What to check:

- **`ranks=16`** — confirms `mpiprocs=` was honoured. If this is `1`, you ran without `mpiexec` (the program uses `MPI_COMM_WORLD` size = 1).
- **`pi=...`** matches serial to ~14 decimal places. Differences in the last digits are floating-point reordering across ranks.
- **`time=...`** — measured by rank 0 between `MPI_Wtime()` calls. Includes all communication.
- **`CPU Percent ≈ 100% × ranks`** — close to 1600% for 16 ranks means all ranks were busy. Lower = some idle.

## `pi_mpi.e<jobid>`

For a clean run, this is usually empty. If the program crashes or `mpiexec` warns about something, this is where it lands.

If you added `mpiexec --report-bindings`, the per-rank binding lines also go here:

```
[cx3-13-2:12345] MCW rank 0 bound to socket 0[core 0[hwt 0]]: [B/././...]
[cx3-13-2:12345] MCW rank 1 bound to socket 0[core 1[hwt 0]]: [./B/./...]
...
```

## Diagnostic flowchart

| Symptom | Likely cause |
|---|---|
| `ranks=1` | Ran without `mpiexec`, or `mpiprocs=1` |
| Empty `.o`, `.e` says `mpiexec: command not found` | Forgot `ml OpenMPI` |
| Job hangs forever, never produces `.o` | Collective deadlock (e.g. some ranks didn't call `MPI_Reduce`) |
| `pi` is way off | Block partition off — check `start`/`finish` arithmetic |
| Some ranks segfault | Likely a memory bug; rebuild with `-g` and run under `mpiexec gdb` interactively |

## Verifying communication actually happens

A deceptively passing run: 16 ranks each computed a local partial sum, but the Reduce wasn't called → rank 0 prints `global = 0`.

Sanity check: with `ranks=1` you should get the same answer as the serial run; with `ranks=16` you should get an answer that differs only in the last few decimals (floating-point reordering). Anything else means a logic bug.

## Related

- [[MPI PBS Script]] — what produced this output.
- [[../pbs/Log Files]] — generic PBS log conventions.
- [[MPI Pitfalls]] — root causes for the symptoms above.
