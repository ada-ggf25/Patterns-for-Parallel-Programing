# Filesystems

CX3 mounts a single shared parallel filesystem (RDS) under `/rds/general/user/$USER/`, plus a per-job local scratch space. Knowing which space to use is part of the cluster-hygiene contract.

## RDS — visible from every node

| Env var | Path | Quota | Retention |
|---|---|---|---|
| `$HOME` | `/rds/general/user/$USER/home` | 1 TB | permanent |
| `$EPHEMERAL` | `/rds/general/user/$USER/ephemeral` | 10 TB | **wiped after 30 days** |

- `$HOME` and `$EPHEMERAL` are the same files on every login node *and* every compute node — your job sees what you committed before submission.
- Use `$EPHEMERAL` for big intermediate results (raw outputs, scratch checkpoints) you don't want counting against `$HOME`'s 1 TB quota.
- Anything in `$EPHEMERAL` older than 30 days will be deleted without warning.

Check usage:

```bash
quota -s
```

## `$TMPDIR` — local scratch on the compute node

- Set automatically when your PBS job starts; vanishes when it ends.
- Lives on the compute node's fast local disk — much higher IOPS than RDS for small-file or random-access workloads.
- Pattern: stage inputs in at the start of the job, work in `$TMPDIR`, copy results back to `$HOME` / `$EPHEMERAL` at the end.

For the course's compute-bound π examples this is irrelevant — they do no meaningful I/O.

## Practical notes

- `$HOME` is the right place for source code, PBS scripts, the built binaries from `cmake --build`. Everything in this course's repo lives in `$HOME`.
- `$EPHEMERAL` is the right place for, e.g., a 100 GB simulation output you want to post-process and then trim.
- `$TMPDIR` is the right place for files that exist only for the lifetime of one job.

## Related

- [[CX3 Overview]] — the cluster context.
- [[Login and Authentication]] — getting onto the filesystem.
- [[../pbs/PBS Environment Variables]] — `$PBS_O_WORKDIR`, the other PBS-set path.
