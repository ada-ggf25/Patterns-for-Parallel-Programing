# CX3 Overview

CX3 ("compute cluster 3") is Imperial College London's general-purpose HPC cluster, run by Research Computing Services (RCS). It is a shared multi-user system with a hard separation between login nodes (where you edit, build, and submit) and compute nodes (where your jobs actually run).

## At a glance

- ~600 compute nodes plus a handful of login nodes.
- Two CPU architectures mixed in one cluster:
  - **AMD EPYC 7742** (Rome / Zen2) — 128 cores per node — see [[AMD Rome Architecture]].
  - **Intel Xeon Platinum 8358** (Ice Lake) — 64 cores per node — see [[Intel Ice Lake Architecture]].
- One shared parallel filesystem (RDS) visible from every login *and* compute node — see [[Filesystems]].
- Job scheduler: **PBS Professional** — see [[../pbs/PBS Overview|PBS Overview]].

## The shared-resource contract

The login nodes are shared with everyone else. Heavy work there will be killed by admins. The only legitimate path to do real computation is:

```
log in  →  write a PBS script  →  qsub  →  read the log file
```

You cannot ssh into compute nodes directly; the scheduler allocates them on your behalf when a PBS job starts.

## Why two CPU architectures matter

Because CX3 is heterogeneous, the *same* benchmark can land on a Rome node one day and an Ice Lake node the next, producing legitimately different numbers (~15–20% spread, sometimes more). For comparable timings, pin your job to a specific CPU class with `cpu_type=rome` or `cpu_type=icelake` — see [[../pbs/cpu_type Selection|cpu_type Selection]].

For this course we default to `cpu_type=rome`: there are roughly 300 Rome nodes versus 80 Ice Lake, so queues start quickly even with a full class submitting.

## Authoritative documentation

- RCS service page: <https://www.imperial.ac.uk/admin-services/ict/self-service/research-support/rcs/service-offering/hpc/>
- RCS user guide: <https://icl-rcs-user-guide.readthedocs.io/>
- Cluster spec: <https://icl-rcs-user-guide.readthedocs.io/en/latest/hpc/cluster-specification/>

## Related

- [[Login and Authentication]] — how to connect.
- [[Login Shards]] — which login host you land on.
- [[Topology Inspection]] — see what the scheduler gave you.
