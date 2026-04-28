# Log Files

When a PBS job ends (state `F`), two log files appear in the directory you submitted from:

- `<jobname>.o<jobid>` — stdout, plus a PBS accounting summary at the bottom.
- `<jobname>.e<jobid>` — stderr.

You can override these with the `-o` and `-e` directives ([[PBS Directives]]).

## What's in `.o<jobid>`

1. Anything your program printed to stdout, in order.
2. A PBS-appended footer with accounting info:

```
n=1000000000  threads=8  pi=3.141592653589821  time=0.163s
===============================================
CPU Time used: 00:01:18
CPU Percent:   789%
Memory usage:  18mb
Walltime:      00:00:01
```

- **`CPU Percent`** — total CPU-seconds / walltime, as a percentage. ~`100% × ncpus` means all your cores were saturated; lower means cores were idle.
- **`Memory usage`** — peak resident set size. Compare against your `mem=` request.
- **`Walltime`** — actual wall-clock duration.

For very short jobs (< 1 s), PBS's accounting can show unhelpful values like `CPU Percent: 0%`. Trust your program's own timing output first; rely on PBS accounting only for jobs longer than a few seconds.

## What's in `.e<jobid>`

stderr only. For OpenMP jobs that set `OMP_DISPLAY_AFFINITY=TRUE`, this is where you'll find one line per thread showing which core it pinned to:

```
OMP: pid 12345 tid 12346 thread 0 bound to OS proc set {12}
OMP: pid 12345 tid 12347 thread 1 bound to OS proc set {30}
...
```

## Reading habits

**Read the log files even when everything looks fine.** Silent failures, OOM kills, and wrong-answer bugs usually announce themselves here:

- A program that segfaulted but exited cleanly (no error code) — the segfault is in `.e`.
- An MPI job where only some ranks worked — partial output in `.o`, error from the missing ranks in `.e`.
- An OpenMP job that ran with `threads=1` — pragmas didn't activate; check `.o` to confirm.

## Cleaning up

Log files accumulate. After harvesting the data you need, archive or delete them:

```bash
mkdir -p old_logs
mv pi_*.o* pi_*.e* old_logs/
```

## Related

- [[qsub qstat qdel]] — the lifecycle that produces these.
- [[../openmp/Reading the OpenMP log]] — what to look for in OpenMP output.
- [[../mpi/Reading the MPI log]] — what to look for in MPI output.
