# `qsub`, `qstat`, `qdel` — the lifecycle commands

The three commands you'll use every day.

## `qsub` — submit

```bash
$ qsub my_job.pbs
7315932.pbs-7
```

The output is the **job ID**. You'll need it for `qstat` and `qdel`. PBS records the working directory at submission time and exposes it to the job as [[PBS Environment Variables|`$PBS_O_WORKDIR`]].

You can also pass directives on the command line, overriding the script:

```bash
qsub -N quick_test -l walltime=00:05:00 my_job.pbs
```

But for reproducibility, keep directives in the script.

## `qstat` — monitor

```bash
$ qstat -u $USER             # only your jobs
$ qstat 7315932.pbs-7        # one specific job
$ qstat -f 7315932.pbs-7     # full record (very verbose)
$ qstat -t                   # all jobs, all users (cluster-wide)
```

The columns to watch are **`S`** (state — see [[Job States]]), **`Req'd Memory`**, **`Req'd Time`**, and **`Elap Time`**.

For a more visual view, RCS provides a self-service portal: <https://selfservice.rcs.imperial.ac.uk/>.

## `qdel` — cancel

```bash
$ qdel 7315932.pbs-7
```

Cancels a queued or running job. The job's log files are still produced and may contain partial output. There is no undo.

## Typical workflow

```bash
$ qsub my_job.pbs
7315932.pbs-7

$ qstat -u $USER
Job ID            Username  Queue       Jobname    SessID NDS TSK Memory Time S Time
----------------- --------- ----------- ---------- ------ --- --- ------ ---- - ----
7315932.pbs-7     me        v1_small72  my_job        --    1   8   8gb  00:30 Q   --

# ... wait, then refresh ...

$ qstat -u $USER
Job ID            ...                                                S Time
7315932.pbs-7     ...                                                R 00:01

# ... wait until state = F ...

$ ls *.o7315932* *.e7315932*
my_job.o7315932    my_job.e7315932

$ cat my_job.o7315932
```

See [[Log Files]] for what those output files contain.

## Related

- [[Job States]] — the `S` column values.
- [[Log Files]] — the post-mortem.
- [[Interactive Jobs]] — `qsub -I` for live debugging.
- [[pbsnodes]] — cluster-side view (nodes rather than jobs).
