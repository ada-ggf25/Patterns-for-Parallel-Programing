# Job States

The `S` column in `qstat` shows the job's current state.

| State | Meaning |
|---|---|
| `Q` | Queued — waiting for resources |
| `R` | Running |
| `H` | Held — by admin, dependency, or user-requested hold |
| `E` | Exiting — finishing up (writing logs, releasing resources) |
| `F` | Finished — terminal state |

## Reading the state

- **`Q` for hours.** Either the cluster is busy or your `select=` ask is large. Try `qstat -Q` to see queue lengths, or shrink the request. See [[Queues]] for queue-vs-size mapping.
- **`H`.** Most often a dependency hold (`-W depend=...`) or an admin hold. Use `qstat -f <jobid>` to see the reason.
- **`R` then suddenly gone.** Job finished; check the log files. If timing was much shorter than expected, the job may have crashed early.
- **`E` for a while.** PBS is flushing logs and releasing the cpuset; usually clears in seconds.
- **`F`.** Done. The log files are now final.

## Inspecting more closely

```bash
qstat -f 7315932.pbs-7   # everything PBS knows about the job
```

Useful fields in the `-f` output:

- `job_state` — same as the `S` column.
- `exec_host` — which compute node(s) the job ran on.
- `resources_used.cput` — total CPU-seconds consumed.
- `resources_used.walltime` — actual wall-clock time.
- `Exit_status` — exit code (0 = clean).

## Related

- [[qsub qstat qdel]] — the commands that show this.
- [[Log Files]] — what to read once a job hits `F`.
- [[Queues]] — why a `Q` job might be stuck.
