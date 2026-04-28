# Thread Pinning

Without pinning, the Linux scheduler is free to migrate a thread from one core to another whenever it sees fit. That destroys cache locality and, on NUMA systems, can move a thread far away from the memory it was using. Pinning prevents this.

## The two-line setup

```bash
export OMP_PLACES=cores            # smallest binding unit = one core
export OMP_PROC_BIND=close         # pack threads onto adjacent places
export OMP_DISPLAY_AFFINITY=TRUE   # log the binding (one line per thread)
```

This is the default for most compute-bound kernels — see [[OMP Environment Variables]] for what each variable does.

## `close` vs `spread` — picking the right policy

The two interesting `OMP_PROC_BIND` values:

| Policy | Effect | Use when... |
|---|---|---|
| `close` | Threads packed onto adjacent places (same socket / NUMA / L3 when possible) | Threads share data; or you fit inside one socket; or compute-bound |
| `spread` | Threads distributed across the machine | Memory-bandwidth-bound — you want to hit multiple memory controllers in parallel |

For our π example (compute-bound, no meaningful memory traffic), `close` is fine and pinning barely changes the timing — but having pinning *on* is the right default before you try to interpret a benchmark.

For something like a STREAM-style kernel that streams a huge array end to end, `spread` can almost double throughput by saturating memory controllers across both sockets.

## CX3 reality check

Asking PBS for `ncpus=8` on a 128-core Rome node does **not** guarantee contiguous cores. A real run from this course's example landed threads on cores `12, 30, 31, 47, 61, 62, 67, 79` — scattered across both sockets.

`OMP_PROC_BIND=close` still pins them, but "close" is only close *within the cpuset PBS gave you*. If true locality matters:

- Request a whole NUMA domain: `select=1:ncpus=16:ompthreads=16` matches one Rome NUMA exactly (16 cores).
- Request a whole socket: `select=1:ncpus=64:ompthreads=64` matches one Rome socket.

See [[../cluster/AMD Rome Architecture]] for the topology you're matching to.

## Verifying pinning happened

Set `OMP_DISPLAY_AFFINITY=TRUE` and look in the `.e<jobid>` file:

```
OMP: pid 12345 tid 12346 thread 0 bound to OS proc set {12}
OMP: pid 12345 tid 12347 thread 1 bound to OS proc set {30}
OMP: pid 12345 tid 12348 thread 2 bound to OS proc set {31}
...
```

Each line is one thread; the `OS proc set` is the set of cores it's allowed to run on. With `OMP_PLACES=cores` each set is a single core.

## Why pinning matters for NUMA

On Rome, accessing local memory is ~104 ns; cross-socket memory is ~247 ns (see [[../cluster/NUMA Latency]]). A thread that's drifted across the inter-socket link is paying 2.4× per memory access for the same work. Pinning keeps the OS from making that mistake silently.

## Related

- [[OMP Environment Variables]] — the variables involved.
- [[../cluster/AMD Rome Architecture]] — the topology you're pinning to.
- [[../cluster/NUMA Latency]] — the cost of getting it wrong.
- [[OpenMP PBS Script]] — pinning in context.
