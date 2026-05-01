# A1 Progress — Numerical Integration

**Overall status:** `COMPLETE — all phases done, all CI green`
**Branch:** `ggf25`
**Graded snapshot:** end of day 5 (instructor re-runs on CX3 Rome)
**Points available:** 20 / 100

---

## Scoring tracker

| Component                                    | Pts    | Status                                            |
| -------------------------------------------- | ------ | ------------------------------------------------- |
| Build + TSan clean                           | 2      | ✅ CI `Build & TSan correctness` green (run 25059877065) |
| Correctness at {1, 16, 64, 128}              | 6      | 🟨 local correctness validated; full set pending   |
| Reference-parallel-time at 128T              | 5      | 🟨 T(128)=0.0446s (guided); T_ref not yet published |
| `tables.csv` internal consistency            | 1      | ✅ filled from CX3 Rome run; guided schedule       |
| Style (clang-format / clang-tidy / cppcheck) | 2      | ✅ CI static analysis & style green (run 25059877065) |
| MCQ (15 questions)                           | 2      | ✅ all 15 answered in answers.csv                  |
| REFLECTION format + completion               | 1      | ✅ CI `REFLECTION.md format` green                 |
| Reasoning question                           | 1      | 🟨 written (≤100 words); pending instructor mark    |
| **Total**                                    | **20** | **In progress**                                   |

---

## Phase 1 — Parallelise `integrate_parallel()` ✅

**File to edit:** `integrate.cpp`

The function already exists as a serial fallback. Add one `#pragma omp parallel for` with `reduction(+:sum)` and `default(none)` around the inner loop. The two endpoint terms (`0.5*(f(a)+f(b))`) live outside the parallel region — do not include them in the reduction.

Key clauses needed:
- `default(none)` — forces explicit declaration of all shared/private variables
- `shared(a, h, n)` — these three are read-only inside the loop
- `reduction(+:sum)` — each thread accumulates privately; results are combined at the end
- `schedule(???)` — start with `schedule(dynamic, 64)` and tune in Phase 2

### Phase 1 checklist

- [x] Edit `integrate.cpp` — add `#pragma omp parallel for` to `integrate_parallel()`
- [x] Add `default(none)` with explicit `shared()` and `reduction(+:sum)`
- [x] Choose a starting schedule (recommend `dynamic, 64` as baseline)
- [x] Build: `cmake -B build -S . -DCMAKE_CXX_COMPILER=clang++-18 && cmake --build build -j`
- [x] Quick correctness check: `OMP_NUM_THREADS=4 ./build/integrate > out.txt` (local output matched expected exactly)
- [x] TSan sanity check: built with `-fsanitize=thread`; local warning observed only inside `libomp` runtime internals (known false positive without Archer OMPT), no user-code race evidence
- [x] Push to `ggf25` and confirm CI correctness + TSan jobs are green

**Phase 1 status note (2026-04-28):** implementation pushed on `ggf25`; CI `Build & TSan correctness` is green.

**Build command (TSan variant):**
```bash
clang++ -fopenmp -fsanitize=thread -g -O1 \
    integrate.cpp integrate_serial.cpp -o integrate_tsan
OMP_NUM_THREADS=4 ./integrate_tsan
```

**Expected stdout (check against `expected_output.txt`):**
```
integral = 0.817014911522
```

---

## Phase 2 — Schedule sweep + `tables.csv` ✅

**File to edit:** `tables.csv`
**Requires:** Phase 1 complete and CI green

Three schedules to test: `static`, `dynamic, 64`, `guided`. Measure each at `{1, 16, 64, 128}` threads on CX3 Rome to see which wins and why.

### Step 1 — Submit benchmark on CX3

```bash
# Login to CX3
ssh ggf25@login.cx3.hpc.ic.ac.uk

# From the repo root:
qsub evaluate.pbs

# Monitor
qstat -u ggf25
```

The PBS script (`evaluate.pbs`) runs `hyperfine --warmup 1 --min-runs 3` at each thread count and writes `integrate-{1,16,64,128}.json`. Use the `min` value from each file for `tables.csv`.

**Important — CX3 uses GCC, not Clang-18:**
The PBS script loads `GCC/13.3.0` and builds with `-O3 -march=znver2 -mavx2`. This is different from the local/CI Clang-18 build. The GCC Rome build is the one that counts for performance scoring — the Clang-18 CI build is only for TSan correctness. Your `integrate.cpp` must compile cleanly under both.

**Extracting min times from hyperfine JSON:**
```bash
# After the PBS job finishes, extract the min time for each thread count:
python3 bin/hyperfine_min_time.py integrate-1.json    # T(1)
python3 bin/hyperfine_min_time.py integrate-16.json   # T(16)
python3 bin/hyperfine_min_time.py integrate-64.json   # T(64)
python3 bin/hyperfine_min_time.py integrate-128.json  # T(128)
```

To also compare schedules manually, temporarily swap the schedule in `integrate.cpp`, rebuild, and re-run. Record all three schedules' times before committing your final choice.

### Step 2 — Fill `tables.csv`

```
thread_count,measured_time_s,measured_speedup,measured_efficiency
1,<T1>,1.00,1.00
16,<T16>,<T1/T16>,<(T1/T16)/16>
64,<T64>,<T1/T64>,<(T1/T64)/64>
128,<T128>,<T1/T128>,<(T1/T128)/128>
```

Arithmetic rules (grader enforces within 2 %):
- `speedup(P) = T(1) / T(P)`
- `efficiency(P) = speedup(P) / P`
- Row for P=1: speedup = 1.00, efficiency = 1.00 (exact)

### Phase 2 benchmark summary (CX3 Rome, 2026-04-29)

Winner: **`schedule(guided)`** — T(128) = 0.0446 s, speedup 42.49×, efficiency 33.2 %.

| Schedule | T(1) s | T(16) s | T(64) s | T(128) s | Speedup@128 | Eff@128 |
|---|---|---|---|---|---|---|
| `static` | 1.8939 | 0.3376 | 0.0961 | 0.0566 | 33.46× | 0.261 |
| `dynamic,64` | 1.9048 | 0.1401 | 0.0697 | 0.0987 | 19.31× | 0.151 |
| `guided` | 1.8961 | 0.2297 | 0.0719 | 0.0446 | 42.49× | 0.332 |

Notable: `dynamic,64` regresses at 128T (slower than at 64T) — chunk=64 causes dispatcher contention at high thread counts.

Full analysis and raw JSON values: [[A1 Benchmark Results]].

### Phase 2 checklist

- [x] Login to CX3 and check CX3 job access works
- [x] Push latest `integrate.cpp` to `ggf25` and pull on CX3
- [x] Submit `evaluate.pbs` with `qsub evaluate.pbs`
- [x] Wait for job to finish; check `qstat` output
- [x] Record `T(1)` from `integrate-1.json` (use `min`)
- [x] Record `T(16)`, `T(64)`, `T(128)` from their respective JSON files
- [x] Compute speedup and efficiency for each row
- [x] Fill all four rows in `tables.csv`
- [x] Verify internal consistency: `speedup = T(1)/T(P)` within 2 %
- [x] Test additional schedules (`static`, `guided`) locally or on CX3 for REFLECTION.md evidence
- [x] **Replace `schedule(runtime)` with `schedule(guided)` in `integrate.cpp`** — done; grader env has no `OMP_SCHEDULE` set
- [x] Commit `tables.csv` + updated `integrate.cpp` with message like `bench: fill tables.csv from CX3 Rome run; hardcode winning schedule`

---

## Phase 3 — MCQ `answers.csv` ✅

**File to edit:** `answers.csv`
**Can be done independently of Phases 1–2**

Format is a two-column CSV — fill the `answer` column with A, B, C, or D:

```
qid,answer
q01,<letter>
q02,<letter>
...
q15,<letter>
```

### Study guide per question

Open `questions.md` and consult these vault notes:

| Question topic | Vault note |
|---|---|
| `default(none)` clause | [[../openmp/Variable Scoping]] |
| `reduction(+:sum)` mechanism | [[../openmp/reduction clause]] |
| Schedule for uniform-cost loop | [[../openmp/Schedules]] |
| Schedule for non-uniform cost | [[../openmp/Schedules]] |
| `omp_get_num_threads()` return value | [[../openmp/parallel directive]] |
| Silent no-op pragma (typo in `#pragma`) | [[../openmp/Building OpenMP]], [[../openmp/OpenMP Pitfalls]] |
| Roofline regime (compute vs bandwidth) | [[../performance/Roofline Model]] |
| Roofline fraction — theoretical vs HPL ceiling | [[../performance/Roofline Model]], [[../performance/STREAM and HPL]] |
| Speedup comparison between two students | [[../performance/Performance Metrics]] |
| `firstprivate` clause | [[../openmp/Variable Scoping]] |
| `default(none)` + missing array → compile error | [[../openmp/Variable Scoping]], [[../openmp/Data Races and TSan]] |
| User-defined reduction for a struct | [[../openmp/User-Defined Reductions]] |
| Speedup plateau at high thread counts | [[../openmp/Amdahls Law]], [[../performance/Six Sources of Overhead]] |
| `#pragma omp parallel for` equivalence | [[../openmp/for directive]] |
| `_OPENMP` macro value for OpenMP 5.1 | [[../openmp/_OPENMP Macro]] |

### Phase 3 checklist

- [x] Read all 15 questions in `questions.md`
- [x] Consult vault notes for each topic (table above)
- [x] Fill `answers.csv` — one letter per row, all 15 rows
- [x] Commit: `mcq: answer all 15 questions in answers.csv`

---

## Phase 4 — `REFLECTION.md` ✅

**File to edit:** `REFLECTION.md`
**Requires:** Phase 2 complete (need measured numbers to ground claims)

CI checks: all five `## Section` headers present + each of Sections 1–4 has ≥ 50 words. HTML comments (`<!-- ... -->`) are stripped before word count.

### Section 1 — Schedule choice and why (≥ 50 words)

Must cover:
1. Name the winning schedule + chunk size
2. Explain f(x) spike structure (x ∈ (0.3, 0.4), ~10× cost)
3. Name at least one schedule tried and discarded, with measured evidence
4. Explain why winner is better: load rebalancing distributes the spike

Grounding: use your measured `tables.csv` times (e.g., "static gave X s at 128 threads vs dynamic(64) Y s").

### Section 2 — Scaling behaviour (≥ 50 words)

Must cover:
1. Where speedup departs from linear (e.g., "near-linear to 16T, then flattens")
2. Efficiency values from `tables.csv` cited explicitly
3. At least one cause: Amdahl / NUMA / bandwidth / fork-join overhead

Reference: [[../performance/Six Sources of Overhead]], [[../openmp/Amdahls Law]]

### Section 3 — Roofline position (≥ 50 words)

Must cover:
1. State A1 is compute-bound (OI >> 18.7 FLOPs/byte ridge)
2. Estimate achieved GFLOPs at your best thread count
3. State roofline fraction vs. 4608 GFLOPs theoretical AND vs. 2896 GFLOPs HPL-achievable
4. Explain why fraction is low (scalar execution, no SIMD, spike branch)

Rome constants: peak 4608 GFLOPs, HPL 2896 GFLOPs, STREAM **246 GB/s** (canonical per assignment brief), ridge ≈ 18.7 FLOPs/byte.
Reference: [[../performance/Roofline Model]], [[../performance/STREAM and HPL]]

Achieved GFLOPs calculation:
```
estimated_flops_per_iter = (weighted avg of cheap and spike regions)
total_flops = estimated_flops_per_iter × N
achieved_GFLOPs = total_flops / T(128) / 1e9
```

### Section 4 — What you'd try next (≥ 50 words)

Pick one concrete change and predict its effect. Good options:
- `#pragma omp simd` on f(x) to vectorise (needs branch → blend-mask conversion for spike region)
- Chunk-size tuning: profile dynamic(32) vs dynamic(128) at 128T
- Increase N to rule out L3 cache effects

### Reasoning question (≤ 100 words, instructor-marked)

"In at most 100 words, explain why your chosen schedule is appropriate for the cost structure of this particular f(x)."

Be concise: state the cost non-uniformity, why static fails (all spikes on one thread), why dynamic/guided helps (demand-driven pull distributes the spike). Cite one measured data point.

### Phase 4 checklist

- [x] Section 1 written (≥ 50 words, cites measured times) — 219 words
- [x] Section 2 written (≥ 50 words, cites efficiency values from tables.csv) — 179 words
- [x] Section 3 written (≥ 50 words, computes achieved GFLOPs + roofline fraction) — 283 words
- [x] Section 4 written (≥ 50 words, concrete change + predicted effect) — 171 words
- [x] Reasoning question written (≤ 100 words) — ~90 words
- [x] Run CI reflection-format check: push and verify `REFLECTION.md format` CI job is green
- [x] Commit: `reflection: complete all four sections and reasoning question`

---

## Phase 5 — Lint & CI hygiene ✅

CI runs on every push. All five jobs must be green before the graded snapshot.

### Lint commands (run locally before pushing)

```bash
# clang-format check (dry run — shows what would change without modifying)
clang-format-18 --dry-run --Werror integrate.cpp integrate.h

# Auto-fix formatting:
clang-format-18 -i integrate.cpp integrate.h

# clang-tidy (needs build/ with compile_commands.json):
cmake -B build -S . -DCMAKE_CXX_COMPILER=clang++-18
clang-tidy-18 -p build integrate.cpp

# cppcheck:
cppcheck --enable=performance,warning,style --quiet \
    --project=build/compile_commands.json
```

### CI jobs checklist

- [x] `Build & TSan correctness` — green (threads 1,2,4,8,16 all produce correct output, no TSan warnings)
- [x] `Static analysis & style` — green (clang-format, clang-tidy, cppcheck all pass)
- [x] `REFLECTION.md format` — green (all section headers present, ≥ 50 words each)
- [x] `Language check` — green (English only in .md files and C++ comments)
- [x] `Prevent committing build artifacts` — green (no `build/` or compiled files committed)

### Phase 5 checklist

- [x] Run `clang-format-18 -i integrate.cpp integrate.h` and commit any changes
- [x] Run `clang-tidy-18 -p build integrate.cpp` and fix any warnings
- [x] Confirm no `.o`, executables, or `build/` directory is tracked: `git ls-files build/`
- [x] Check all Markdown comments and code comments are in English
- [x] Push and confirm all 5 CI jobs green

---

## Commit history guide

Aim for ≥ 8 commits across ≥ 3 calendar days with descriptive messages:

```
omp: add parallel for + reduction(+:sum) to integrate_parallel
omp: verify TSan clean at 4 and 16 threads
bench: run evaluate.pbs on CX3 Rome, record raw times
csv: fill tables.csv with speedup and efficiency
mcq: answer all 15 questions in answers.csv
reflection: complete sections 1 and 2
reflection: complete sections 3 and 4 + reasoning question
style: apply clang-format to integrate.cpp and integrate.h
```

Do NOT: single mega-commit on day 5, token messages (`fix`, `wip`), 100 % of diff on the last day.

---

## Key file constraints

- **Do not edit** `integrate_serial.cpp`, `integrate.h`, `main()` in `integrate.cpp`, `bin/smart_diff.py`, or `.github/workflows/`
- **Do not add** new headers, dependencies, or third-party libraries
- **Do not rename** source files or public function signatures
- **Do not commit** the `build/` directory or any compiled artifacts

---

## Related

- [[A1 Integration]] — full technical reference: kernel details, schedule theory, REFLECTION guide, common mistakes.
- [[../openmp/Schedules]] — static / dynamic / guided decision guide and chunk-size tuning.
- [[../openmp/reduction clause]] — accumulator pattern with `reduction(+:sum)`.
- [[../openmp/Variable Scoping]] — `default(none)` and explicit sharing.
- [[../openmp/Data Races and TSan]] — understanding and fixing data races.
- [[../performance/Performance Metrics]] — speedup, efficiency, reference-parallel-time formulas.
- [[../performance/Roofline Model]] — OI computation, compute vs bandwidth, Rome ceiling values.
- [[../performance/Timing omp_get_wtime]] — warm-up idiom, min-of-k, hyperfine usage.
- [[../performance/Six Sources of Overhead]] — why parallel speedup is never perfect.
- [[Assessment Overview]] — 100-pt rubric, grading branch, commit history rules.
