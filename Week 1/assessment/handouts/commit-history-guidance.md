# Commit-history guidance (PPP-OpenMP assessment)

Adapted from the
[ESE-MSC IRP regular-commits guidance](https://ese-msc.github.io/irp/academic-integrity/regular-commits/).

## Why we look at your git history

Two reasons:

1. **It's good engineering practice.** Small, frequent commits with
   descriptive messages let you (and anyone reviewing your code) move
   forward and backward through your work, bisect bugs, and reason about
   changes one piece at a time.
2. **Academic integrity.** A submission that arrives as a single mega-commit
   on day 5 with the message `final` is hard to distinguish from work that
   wasn't done by the submitter. Your commit history is the most direct
   evidence we have that the work is yours.

## What we measure (deterministic, no marks attached)

The grader runs four checks against your `feature/eval` branch git log over
the assessment window (day-2 morning brief release → end of day 5). The
results surface in a `commit_flags` column on the per-student grade report
for instructor review. **No marks are awarded or deducted automatically.**

| Signal | "OK" | Flagged for review |
|---|---|---|
| Commit count + days touched | ≥ 8 commits across ≥ 3 distinct calendar days | < 3 commits |
| Mega-commit avoidance | Largest commit ≤ 70 % of total diff bytes | Largest commit > 90 % of diff |
| Commit message quality | < 50 % token messages (`fix`, `update`, `wip`, `temp`, `final`, `< 15 chars`) | > 80 % token messages |
| Last-day diff fraction | < 50 % of total diff bytes committed on day 5 | 100 % on day 5 |

These are integrity-triage signals, not graded constructs — but their
presence on your record means your work gets a closer look.

## Examples of healthy commit histories

```
2026-04-22  09:14  Add parallel reduction skeleton for A1
2026-04-22  10:31  Switch A1 to schedule(dynamic, 64); spike region dominates
2026-04-22  16:08  Run A1 scaling sweep, commit perf-results-a1.json
2026-04-23  09:02  A2 parallel_for variant: collapse(2) + reduction
2026-04-23  14:12  A2 tasks variant: tile size 100, taskloop grainsize(8)
2026-04-23  16:55  CHOICE.md draft based on tonight's measurements
2026-04-24  09:30  A3 core: parallelise jacobi_step + first-touch init
2026-04-24  15:48  A3 NUMA extension; before/after timings
2026-04-25  11:20  REFLECTION across all three exercises
2026-04-25  14:02  Tighten EXTENSION.md numbers after rerun on quieter node
```

Commits scattered across days, descriptive subjects, no single mega-commit.

## What raises a flag (and why)

* **Single commit on day 5 containing the entire submission.** The
  submission is technically complete, but there's no record of *how* you
  got there.
* **All commit messages are `update`, `fix`, or `wip`.** The grader can't
  tell what each commit did.
* **All work appears the day before the deadline.** Possible, but unusual
  enough that the grader looks twice.

## How to make your history look the way it should

You don't need to commit constantly. Aim for one or two commits per
working session, each with a one-line subject that explains the *intent*
(not "updated mandelbrot.cpp" — what *about* it changed?).

If you prefer working on a feature branch and squashing before merging,
that's fine **for development branches**, but the `feature/eval` branch
that gets graded should preserve the commit history. Don't `git reset
--hard` and re-add as a single commit.

If you accidentally produced a single mega-commit and want to recover, you
can do interactive rebases (`git rebase -i`) to split commits — but only on
work that hasn't been pushed and that you wrote yourself.

## What this is not

* **It is not a checklist.** A submission with a perfect-looking commit
  history but failing implementation doesn't get marks; a submission that
  passes the rubric but only has 4 commits doesn't get docked.
* **It is not a fixed cadence.** "X commits per day" is not the rule. A
  productive day might produce one well-thought commit; an exploratory
  day might produce ten.
* **No LLM is reading your commit messages.** All four signals above are
  computed deterministically from `git log`. The instructor reviews flags
  by hand if any fire.
