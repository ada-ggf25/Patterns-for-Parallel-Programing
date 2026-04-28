# Six Sources of Parallel Overhead

Whenever a kernel is parallelised, at least these six costs are incurred. Diagnosing poor scaling means identifying which one dominates.

## 1. Sequential code — Amdahl's law

```
serial_setup();      // 5 % of total runtime
parallel_compute();  // 95 % of total runtime
```

$$S_{\max} = \frac{1}{f_s + (1 - f_s)/P} \xrightarrow{P \to \infty} \frac{1}{f_s}$$

If 5 % is serial, the theoretical maximum speedup is 20× regardless of thread count. The way out is to attack the serial fraction first, not to add more threads.

## 2. Idle threads / load imbalance

Fast threads finish their chunk and wait at the barrier for the slowest thread. The total wall time is determined by the slowest thread.

```
Thread 0: ████ done — idle...
Thread 1: ████████████████ (has the spikes)
Thread 2: ████ done — idle...
Thread 3: ████ done — idle...
```

Countermeasure: `schedule(dynamic, C)` or `taskloop grainsize(C)`.

## 3. Synchronisation overhead

| Primitive | Cost |
|---|---|
| `barrier` | Flush write buffers + wait for all threads |
| `critical` | Global mutex acquire/release |
| `atomic` | `lock`-prefixed CPU instruction (x86) |
| `omp_set_lock` | May round-trip to kernel on contention |

Minimise by design: use `reduction`, partition data, one lock per data instance. Micro-optimising individual barriers rarely helps as much as eliminating them.

## 4. Scheduling overhead

`dynamic` and `taskloop` dispatch work from a queue. Each dispatch costs cycles. With very small chunks (chunk = 1 for an expensive dispatch), the overhead can exceed the work itself.

Trade-off: smaller chunk → better load balance; larger chunk → lower overhead. Tune chunk size by measurement.

## 5. Cache-line communication (false sharing)

Two cores writing different bytes of the same 64-byte cache line trigger the MESI protocol: each write invalidates all other copies. Cost: ~100 ns per bounce on Rome.

Countermeasure: pad per-thread accumulators to 64-byte boundaries. See [[False Sharing]].

## 6. Hardware contention

- **Memory bandwidth**: saturated at ~1 thread per CCX on Rome. More threads don't give more bandwidth.
- **Cache space**: too many threads competing for L3 → more misses.
- **SMT (hyperthreading)**: Rome runs with SMT off. On other systems, two threads share one FPU → half the compute throughput each.
- **Oversubscription**: more threads than physical cores → context-switch overhead.

## Diagnostic decision tree for A3

| Symptom | Likely cause |
|---|---|
| 1.0× speedup at any P | Pragma typo (serial), no `default(none)`, code actually serial |
| 4× at P=16, then plateau | One NUMA domain saturated; serial-init all pages on one domain |
| 8× at P=64, no more at P=128 | Single socket bandwidth saturated |
| Large variance (> 20 %) between runs | No warm-up; cold cache; cluster noise |
| 100× at P=128 | Check correctness first |

## Related

- [[../openmp/Amdahls Law]] — source 1 in detail.
- [[../openmp/Schedules]] — fix for source 2.
- [[False Sharing]] — source 5.
- [[NUMA First Touch]] — source 6 (NUMA).
- [[../assessment/A3 Jacobi]] — the assessment that exercises sources 1, 5, and 6.
