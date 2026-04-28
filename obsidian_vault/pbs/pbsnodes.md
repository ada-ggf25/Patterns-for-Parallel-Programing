# `pbsnodes` — listing nodes

While `qstat` shows you jobs, `pbsnodes` shows you the nodes themselves: which exist, what state they're in, what resources they have.

## Basic usage

```bash
pbsnodes -a              # one record per node
pbsnodes -av             # verbose form (more attributes)
pbsnodes -l              # only nodes that are down/offline/busy
```

Each record looks something like:

```
cx3-13-2
     Mom = cx3-13-2.cx3.hpc.ic.ac.uk
     ntype = PBS
     state = free
     resources_available.ncpus = 128
     resources_available.mem = 1056gb
     resources_available.cpu_type = rome
     ...
```

## Useful queries

**How many free nodes of each CPU type are available right now?**

```bash
pbsnodes -a | awk '
  /^$/{f=0}
  /state = free/{f=1}
  /cpu_type/ && f {print $3; f=0}
' | sort | uniq -c
```

**List nodes by their CPU type**

```bash
pbsnodes -a | awk '
  /^cx3-/      { n = $1 }
  /cpu_type/   { t = $3 }
  /^[[:space:]]*$/ { print n, t; n = ""; t = "" }
'
```

## When to use it

- Before submitting a large job, check whether the resources you want are even available — saves a long `Q` wait.
- Diagnosing why a job won't start: maybe most matching nodes are `down` or `offline`.
- Getting a feel for cluster utilisation. Combined with `qstat -Q` you can see where the pressure is.

## Related

- [[qsub qstat qdel]] — the job-side counterpart.
- [[Queues]] — queue-level view.
- [[check queue busyness]] — wraps both into a single snapshot.
- [[cpu_type Selection]] — what those `cpu_type` lines feed.
