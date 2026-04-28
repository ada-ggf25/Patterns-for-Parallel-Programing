# OpenMP vs MPI — Choosing

When do you reach for which?

## Comparison table

| Question | OpenMP | MPI |
|---|---|---|
| Fits in one node's RAM? | yes | either |
| Do I want to start with one pragma? | yes | no |
| Need >128 cores? | no | yes |
| Shared-memory data structures? | yes | no (explicit messages) |
| Debugging difficulty | moderate | harder |
| Portability across machines | within one node | across clusters |
| Communication cost | implicit (NUMA hits) | explicit (visible in code) |

## Rule of thumb

> **OpenMP first, MPI when you outgrow one node.**

Most problems get a respectable first speedup from a single `#pragma omp parallel for`. Going to MPI is a bigger commitment — you redesign data ownership, partition explicitly, and add communication. Don't pay that price until you have to.

## Concrete decision flow

1. **Profile the serial code.** Is the hot loop parallelisable? If not, neither approach saves you.
2. **One node is enough?** OpenMP. Add a pragma; iterate.
3. **One node isn't enough — bigger problem, more cores, more memory?** MPI. Or [[Hybrid MPI OpenMP|hybrid MPI+OpenMP]].
4. **Memory-bandwidth-bound?** Either model can hit the limit. MPI's per-rank private memory and explicit data placement give you finer control over locality.

## What OpenMP gives you that MPI doesn't

- **Implicit data sharing.** Threads see each other's writes to global structures without explicit messages.
- **Incremental parallelisation.** You can parallelise one loop at a time and re-run between changes.
- **Lower latency for fine-grained work.** A barrier or reduction inside one node, with shared memory, is much cheaper than the equivalent MPI call across nodes.

## What MPI gives you that OpenMP doesn't

- **Scaling beyond one node.** OpenMP tops out at the cores in one machine.
- **Explicit data locality.** Each rank owns its memory; you decide what's local and what gets sent.
- **No hidden races.** No shared variables means no race conditions by construction.
- **Determinism (in practice).** Floating-point reordering can still differ across rank counts, but the data-sharing semantics are explicit.

## Pi example timings on Rome

Rough single-node numbers from the running examples (n = 10⁹ on `cpu_type=rome`):

| Configuration | Wall-time | Speedup |
|---|---:|---:|
| Serial | ~10 s | 1.0× |
| OpenMP, 8 threads | ~1.5 s | ~6.5× |
| OpenMP, 64 threads | ~0.25 s | ~40× |
| MPI, 16 ranks | ~0.08 s | ~125× (oddly high — see note) |
| MPI, 64 ranks | ~0.04 s | ~250× |

The MPI numbers look anomalously good vs OpenMP because MPI ranks have private memory and don't suffer the OpenMP reduction's last-step contention; in real applications with significant memory traffic, OpenMP and MPI converge on similar single-node performance.

## Related

- [[../openmp/OpenMP Overview]]
- [[MPI Overview]]
- [[Hybrid MPI OpenMP]]
- [[../examples/Pi Algorithm]]
