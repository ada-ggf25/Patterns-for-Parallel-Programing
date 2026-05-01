# Learning outcomes, Bloom coverage, and constructive alignment

This is the **pedagogical blueprint** for the 3-day OpenMP course (days 2–4 of a 5-day block; day 1 is delivered by the sibling `ppp-hpc-intro` course; day 5 is assessment iteration). Every subsequent design decision — slide content, lab exercises, rubric weights, CI mechanics — must trace back to an entry in this document.

> **Source of truth.** If an assessment component cannot be tied to a learning outcome below, it should not carry summative marks. If a learning outcome below is not practised somewhere in days 2–5, the course is under-teaching it.

## Course-level outcomes (by end of day 4)

A student who passes the course can:

### LO1 — Describe the OpenMP 5.1 execution model

Teams, implicit barriers, tasks vs worksharing, data environment, memory visibility at pragma boundaries.

*Bloom: Remember + Understand.*

### LO2 — Parallelise a loop-parallel kernel, justifying schedule and clauses

Write and correctly annotate `parallel for` with appropriate `shared` / `private` / `firstprivate` / `default(none)` and `reduction`; pick a schedule (`static` / `dynamic` / `guided`) matched to the workload profile, and *explain why* from measured timings.

*Bloom: Apply + Evaluate.*

### LO3 — Decompose an irregular workload using OpenMP tasks

Use `task`, `taskgroup` / `taskwait`, `taskloop`, and task dependences. Compare a task-parallel decomposition against a loop-parallel baseline using measured evidence.

*Bloom: Apply + Analyze + Evaluate.*

### LO4 — Diagnose NUMA and cache-coherence performance pathologies

False sharing, poor first-touch placement, cache-line contention, oversubscription. Identify the symptom from code + timings, name the cause, propose a targeted fix.

*Bloom: Analyze.*

### LO5 — Reason about performance with the roofline model

Given a kernel, compute its operational intensity; read off the achievable ceiling from STREAM bandwidth and peak FLOPs; predict which limit (compute, bandwidth, memory latency) applies. Estimate what fraction of the ceiling a given implementation achieves.

*Bloom: Analyze + Evaluate.*

### LO6 — Reason about visibility and ordering in shared-memory code

Distinguish when `critical` is necessary, when `atomic` suffices, when `flush` or acquire-release semantics apply. Explain memory-model implications of task dependences and work-sharing boundaries.

*Bloom: Understand + Evaluate.*

### LO7 — Communicate performance results with reproducibility and context

Report time-to-solution, speedup, and roofline efficiency in a way that another team could reproduce. Distinguish *time-to-solution* from *self-speedup* from *reference-relative performance* from *roofline efficiency*; choose the metric appropriate to the kernel's regime, and avoid the trap of rewarding a slow serial baseline that scales well. Give enough context (thread count, binding, problem size) for the result to be re-runnable. Produce a measurement table that is internally consistent (`speedup = T(1)/T(P)`, `efficiency = speedup/P`).

*Bloom: Evaluate + Create.*

## Target Bloom distribution across summative marks (total 100)

| Level | Evidence source | Target marks |
|---|---|---|
| Remember | MCQ concept-checks (15 per exercise; auto-graded from private keys) | ~6 |
| Understand | MCQ scenario questions + structured-header fields (e.g. A2 CHOICE) | ~6 |
| Apply | Code — build, correctness, TSan-clean, baseline implementation working | ~30 |
| Analyze | Reference-parallel-time / roofline performance metrics (require choosing the right schedule, layout, vectorisation); two-variant A2 timing delta; A3 extension soft-threshold delta | ~28 |
| Evaluate | A2 CHOICE recommendation grounded in the student's *own* measured evidence (deterministic); instructor-marked reasoning questions | ~7 |
| Create | Core implementation decisions within starter scaffolding; day-5 iteration | ~10 |
| Hygiene / communication | Build warnings, clang-format, clang-tidy, cppcheck, README readability | 10 |
| Reflection completion (pass/fail) | CI-format-check of REFLECTION.md — no content judgement | 3 |

Totals sum to **100**. Evaluate marks are deliberately lower than the reviewer's initial target because we committed to deterministic grading (no LLM judgement of free-text). That is an intentional trade-off for audit defensibility.

## Outcome → activity → evidence alignment

| Outcome | Taught in | Practised in | Assessed by |
|---|---|---|---|
| LO1 execution model | recap + day2 §2 + day3 §2 | day2 lab scaffolding | MCQ (all exercises) |
| LO2 loop parallel + schedule | day2 §4–5 + day2 lab | A1 implementation + schedule sweep | A1 code + reference-parallel-time perf + tables consistency |
| LO3 tasks + compare | day3 §5–6 + day3 lab | A2 two-variant submission | A2 code + CHOICE structured header + tables consistency |
| LO4 NUMA / false sharing | day4 §3–4 + day4 lab | A3-core + A3-extension (NUMA / false-sharing branches) | A3 code + perf + extension soft-threshold delta + reasoning question |
| LO5 roofline | **recap pre-teach + day4 §2–3 + §6** | A3 perf analysis (memory-bound roofline kernel) | A3-core roofline % metric in grader (deterministic) |
| LO6 ordering / visibility | day3 §2–3 + memory-model section (day 3, *not* day 4) | day3 lab | MCQ |
| LO7 reporting | day4 §1 retrospective + day 5 synthesis iteration | tables.csv internal-consistency + REFLECTION on every exercise | deterministic CI table scoring + reasoning question |

> **Alignment invariant**: every column in this table is non-empty for every LO. If a future edit leaves a gap, the outcome is no longer properly assessed.

## Assessment rollout (build-as-you-learn)

- **Day 2 morning**: all three briefs (A1 + A2 + A3) released with **full rubrics**. MCQ question shells (student-facing, keys private), REFLECTION.md templates, tables.csv templates, starter code.
- **End of day 2**: A1 completable.
- **End of day 3**: A2 completable.
- **End of day 4**: A3-core + chosen A3-extension completable.
- **Day 5**: lab-supported iteration. Students refine any of A1/A2/A3 in light of what they've learned.
- **End of day 5**: **final snapshot** graded — no per-day partial credit, no iterating after the deadline.

Rubric mechanics are **not staged**. Full rubrics are visible from day 2 morning. Kernel design is structured so that *the pedagogically-correct thing is also the highest-scoring thing*: A2's image region is irregular but not pathological — neither variant is rigged to win, and CHOICE.md is graded on the student's own evidence either way; A3 extensions reward actual optimisation; correctness gates performance everywhere. There's no shortcut that pays off better than learning the material.

## Anti-patterns the assessment deliberately resists

1. **"Bad serial that scales well"** — speedup alone is a misleading metric: it rewards a slow starting point. **Reference-parallel-time** scoring against an instructor-authored solution (A1, A2) and **roofline fraction** against measured STREAM (A3-core) both eliminate that advantage: scaling a slow serial just produces slow parallel.
2. **"Correct-looking output that's actually wrong"** — every perf score is correctness-gated. A wrong answer scores zero regardless of timing.
3. **"Pragma-ism"** — the A2 two-variant submission requires *both* implementations to be authored, built, and run; tokens in code don't score marks, measured behaviour does.

4. **"Tuning the kernel to make tasks always win"** — A2's image region is irregular but not pathological; neither variant is rigged to win. CHOICE.md is graded on whether the recommendation is consistent with the student's own measured evidence; either variant can be the right answer.
5. **"LLM-graded reflection"** — removed by design. Free-text reasoning marks are either deterministic (structured keyword enums, cross-check against the student's own measured data) or explicitly human-graded on tightly-bounded prompts. The LLM is never in the summative loop.
6. **"Secret rubric mechanics"** — full rubric released day 2; no hidden point allocations.

## Misconceptions to pre-empt in teaching

Every MCQ bank entry has a distractor rationale tying each incorrect answer to a specific misconception. Known targets:

- *"`atomic` and `critical` are interchangeable."* — They are not; `atomic` is narrower (specific ops) but cheaper; `critical` is a general-purpose mutex.
- *"`default(none)` is overly strict."* — It is protective; silently defaulting to `shared` is the #1 source of accidental races.
- *"More threads is always faster."* — Ignores NUMA, bandwidth saturation, and oversubscription.
- *"Speedup > P/2 means it's well-parallelised."* — Not if the serial baseline is 2× slower than the reference.
- *"Tasks and parallel-for are just different ways to write the same thing."* — Tasks shine on irregular or recursive workloads; parallel-for is better for regular iteration with known trip counts.
- *"`#pragma omp simd` is only needed when the compiler won't auto-vectorise."* — It can also document intent and enable more aggressive vectorisation under strict aliasing assumptions.
- *"Memory-bound kernels can be made compute-bound with more threads."* — Adding threads past bandwidth saturation helps nothing; the bound is structural.

## Minimum passing evidence (what "pass" looks like at ~50%)

A student who scores 50 can credibly claim LO1, LO2 (on regular workloads), partial LO3, partial LO4, partial LO5, and LO6. They may have weak Analyze/Evaluate evidence. This is an intentional pass bar: it rewards correct parallelism and reproducible reporting, not heroic optimisation.

## Change control

When this document changes:

1. The lectures and rubric should be reviewed for realignment.
2. `ppp-openmp-assessment/README.md` and `ppp-openmp-assessment/CLAUDE.md` should be checked for stale references.
3. The grader `evaluate-assessment-artifacts.py` `W` weights dict (in the private `~/projects/ppp-openmp-grader` repo) should be checked against the Bloom distribution above.

Last reviewed: <!-- update when editing -->.
