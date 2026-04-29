---
recommended: parallel_for
measured_tasks_s: 0.4430
measured_for_s: 0.0659
---

<!--
FIELDS (all required, parsed deterministically by the grader):

  recommended         one of: tasks | parallel_for
  measured_tasks_s    YOUR measured time (s) for the tasks variant at the highest
                      thread count, taken from your own `perf-results-a2.json`.
                      The grader cross-checks this value against your committed
                      `perf-results-a2.json` (within 5%) — NOT against the
                      canonical re-run.
  measured_for_s      YOUR measured time (s) for the parallel_for variant.
  justification_keyword   required ONLY if `recommended` is NOT the variant your
                          OWN data shows as faster. Must be one of (exact string):
                            - irregular_load_balance
                            - scales_better_at_128T
                            - simpler_to_maintain
                            - future_proof_for_dynamic_work
                          If you recommend the variant your data shows as
                          faster, leave this blank or delete the line.

The grader awards full CHOICE marks if your recommendation matches the variant
that your own committed `perf-results-a2.json` shows as faster. If you recommend
the slower variant and supply a defensible keyword, you also receive full marks.
Either variant can be the right answer — the test is whether the recommendation
follows your own measured evidence.

The canonical re-run is NOT consulted for CHOICE grading. (It is used separately
for the perf-component score against the published `T_ref` times.)
-->

## Justification (≤ 200 words)

At 128 threads on CX3 Rome (AMD EPYC 7742), `parallel_for` with `schedule(dynamic, 1)` ran in 0.0659 s versus 0.4430 s for the `taskloop` variant — a 6.7× speedup advantage in favour of `parallel_for`.

The tasks variant uses `taskloop grainsize(1)` over the 50 outer tile iterations, generating exactly 50 tasks. With 128 threads available, 78 threads receive no initial task and must steal work; once the 50 tasks are claimed the work queue is empty, so scaling plateaus beyond ~50 threads. The efficiency at 128T is only 0.11 (speedup 14.5×).

By contrast, `schedule(dynamic, 1)` divides the 5000 rows into 5000 individual chunks. Each of the 128 threads continuously picks up the next unprocessed row from the shared queue. Because the per-row cost varies dramatically near the Mandelbrot boundary, fine-grained dynamic scheduling absorbs this irregularity without leaving any thread idle. Efficiency at 128T is 0.78 (speedup 100×), near-linear.

Both variants produce identical output (`outside = 20807396`). Given the measured evidence, `parallel_for` is the clear recommendation for this workload at high thread counts.
