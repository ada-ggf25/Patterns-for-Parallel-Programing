# `check_queue_busyness.sh`

A small bash script in the repo root that prints a tidy snapshot of the cluster from the perspective of a CPU / OpenMP / MPI course. Run it directly on a CX3 login node — it shells out to `qstat` and `pbsnodes`.

## Usage

```bash
./check_queue_busyness.sh           # CPU-relevant queues only (default)
./check_queue_busyness.sh --all     # include GPU / Jupyter / admin queues
```

By default the script filters to queues prefixed `cx`, `hx`, or `v1_(small|medium|large|capability|interactive|4nodes)` — the ones a CPU-only course actually cares about. The `--all` flag drops the filter.

## What it prints

Three sections:

1. **Queue Summary** — `qstat -Q` output, optionally filtered.
   Columns: `Tot`, `Que`, `Run`, `Hld`, `Wat`, `Trn`, `Ext` per queue.

2. **CPU Node Utilisation** — counts of CPU nodes per state (free / job-busy / down / offline / etc.). Built from `pbsnodes -av` and limited to nodes with `ngpus = 0`.

3. **Job Summary** — counts of jobs per state across the whole cluster, from `qstat -t`. Useful to see overall pressure.

## When it's useful

- Before submitting a big job: are there enough free Rome nodes right now?
- Deciding between `v1_medium72` and `v1_capability24` — which one is less backlogged?
- Sanity check after submitting: did your job land in the queue you expected?

## Related

- [[Queues]] — what the queue summary shows.
- [[pbsnodes]] — the underlying node-state source.
- [[qsub qstat qdel]] — direct queries for finer control.
