# Queues

PBS routes your job to a queue based on the `-l` line — you don't pick the queue yourself. Different queues have different limits on cores, memory, walltime, and start priority.

## Queues you'll meet in this course

| Demo | Queue you'll land in | Notes |
|---|---|---|
| Serial Pi (1 core, 10 min) | `v1_small24` / `v1_small72` | Fast for short jobs |
| OpenMP on 1 node (≤64c) | `v1_medium72` | Moves quickly |
| MPI on 1 node (16–64c) | `v1_medium72` | Avoid capability queues until needed |
| Interactive shell | `v1_interactive` | Usually empty |
| Multi-node MPI (2+ nodes) | `v1_capability24` / `v1_capability48` / `v1_capability72` | Heavier backlog; longer wait |

The numeric suffix on each queue is the maximum walltime in hours.

## Why staying on one node helps

The single-node queues (`v1_small*`, `v1_medium*`) are sized for the bulk of cluster traffic and start quickly. The multi-node `v1_capability*` queues have fewer slots and larger backlogs — only request multi-node when you actually need it.

For our `pi_mpi` example, even with 64 ranks we stay on one node:

```bash
#PBS -l select=1:ncpus=64:mem=4gb:mpiprocs=64:cpu_type=rome
```

vs. the multi-node form (which would queue much longer):

```bash
#PBS -l select=2:ncpus=64:mem=64gb:mpiprocs=64:cpu_type=rome
```

## Inspecting queue state

```bash
qstat -Q                        # one line per queue: total, queued, running
qstat -Q v1_medium72            # focus on one
```

Output columns: `Tot`, `Que`, `Run`, `Hld`, `Wat`, `Trn`, `Ext`, plus the queue's enabled/started flags. If `Que` is huge and `Run` is small, the queue is saturated.

The `check_queue_busyness.sh` script in the repo prints a tidy summary — see [[check queue busyness]].

## Authoritative documentation

- Queue picker / job sizing: <https://icl-rcs-user-guide.readthedocs.io/en/latest/hpc/queues/job-sizing-guidance/>

## Related

- [[Resource Selection]] — what your `select=` line means in queue terms.
- [[pbsnodes]] — node-side view (where queues source resources).
- [[check queue busyness]] — the bundled queue-snapshot script.
