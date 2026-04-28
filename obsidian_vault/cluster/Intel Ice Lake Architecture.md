# Intel Ice Lake Architecture

CX3's smaller compute tier uses **Intel Xeon Platinum 8358** processors (Ice Lake). Compared to [[AMD Rome Architecture|Rome]] it is a more conventional 2-socket / 2-NUMA layout — useful as a pedagogical contrast.

## Per-node tally

- 2 sockets × 32 cores = **64 cores per node**.
- 2 NUMA domains per node (one per socket).
- DDR4 RAM, sized similarly to Rome on a per-core basis.

## When to choose Ice Lake

Pick `cpu_type=icelake` (see [[../pbs/cpu_type Selection]]) when:

- You want a simpler topology for teaching — only two NUMA domains, no CCX/CCD structure to reason about.
- You want to compare timings against the Rome tier as a sanity check.
- You specifically need Intel-specific instructions (e.g. AVX-512 with reasonable throughput).

The default in this course is Rome because there are roughly 4× as many Rome nodes (~300 vs ~80), so Ice Lake queues take longer to start.

## Comparison with Rome

| Aspect | Rome (EPYC 7742) | Ice Lake (Xeon 8358) |
|---|---|---|
| Sockets | 2 | 2 |
| Cores / socket | 64 | 32 |
| Cores / node | 128 | 64 |
| NUMA domains / node | 8 (NPS4) | 2 |
| Cores per NUMA | 16 | 32 |
| L3 sharing | per-CCX (4 cores) | per-socket |
| Vector width | AVX2 | AVX-512 |

For an OpenMP team that fits in one socket, Ice Lake's flat L3-per-socket is easier to reason about than Rome's CCX-fragmented L3. For wide MPI runs, Rome's higher core count per node lets you pack more ranks per allocation.

## Related

- [[AMD Rome Architecture]] — the other CX3 tier.
- [[../pbs/cpu_type Selection]] — picking which one your job lands on.
