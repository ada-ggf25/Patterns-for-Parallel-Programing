# Pulling Template Updates

Your assignment repo started as a copy of the canonical template (`ese-ada-lovelace-2025/ppp-openmp-assessment-<N>`). GitHub Classroom does **not** auto-sync the template into your repo — once you accept the assignment, your fork is independent. If the instructors publish a fix after you've started, you need to pull it in by hand.

## Do NOT `git rebase template/main`

The natural-looking command `git rebase template/main` **does not work cleanly**. GitHub Classroom creates per-student repos as fresh repos with a synthetic "Initial commit" that has the template's *content* but none of its *history*. So:

- `git fetch template` shows `warning: no common commits`.
- The first commit of your repo (the GH Classroom "Initial commit") conflicts with `template/main` on every file whose content has changed since you accepted.
- Resolving the conflicts replays a series of empty / near-empty commits and forces you to rewrite history.

Use the file-checkout approach below instead — it's a single fast-forward commit, no rebase, no force-push.

## The file-checkout approach (use this)

Replace `<N>` with your assignment number (`1`, `2`, or `3`).

```bash
# 1. Add the canonical template as a new remote (one-off setup).
git remote add template git@github.com:ese-ada-lovelace-2025/ppp-openmp-assessment-<N>.git

# 2. Fetch the latest template state.
git fetch template

# 3. Bring the template-managed files up to date in your working tree.
git checkout template/main -- README.md .github/workflows/ci.yml

# 4. Commit and push as a normal commit.
git add README.md .github/workflows/ci.yml
git commit -m "Sync README + CI from template"
git push origin main
```

No force push, no SHA rewriting, no conflict resolution.

## Verifying the sync

```bash
# Should show nothing (= identical content).
git diff template/main -- README.md .github/workflows/ci.yml

# Your work in the editable files should be untouched.
git status
```

## Template-managed files (do not modify these)

| Path | Purpose |
|---|---|
| `README.md` | Assignment brief |
| `.github/workflows/ci.yml` | Formative CI workflow |
| `.github/scripts/check_language.py` | English-only language check |
| `.github/actions/collect-untracked/action.yml` | Artifact upload helper |
| `.gitignore` | Build-artefact exclusions |
| `.clang-format` | Lint config |
| `.clang-tidy` | Lint config |
| `bin/smart_diff.py`, `bin/hyperfine_min_time.py`, `bin/run_and_check.sh` | Harness scripts |
| `expected_output.txt` | Reference output |
| `evaluate.pbs` | Rome benchmark harness |
| `mandelbrot.h` (A2) | Shared header — do not edit |
| `questions.md` | MCQ bank — do not edit |
| `CMakeLists.txt` | Build system |
| `LICENSE` | License file |
| `requirements.txt` | Python deps for CI |

**Files you own and should freely modify**: your C++ source (`mandelbrot_for.cpp`, `mandelbrot_tasks.cpp`, `stencil.cpp`, etc.), `CHOICE.md` / `EXTENSION.md`, `REFLECTION.md`, `answers.csv`, `tables.csv`.

## If you accidentally modified a template-managed file

```bash
# Save your version first.
cp README.md README.md.mine

# Overwrite with template version.
git checkout template/main -- README.md

# Manually re-apply any allowed change from README.md.mine on top.
```

## When to sync

Sync only when an instructor announcement says the template was updated and the change is one you want. Common reasons:

- A CI bug fix (e.g., TSan symbolisation fix giving source line numbers in race reports).
- A clarification in the README that affects your understanding of the rubric.
- A correction to `expected_output.txt` (rare; only if announced).

There is **no requirement** to track the template — students who never sync will still be graded on the rubric they accepted.

## Related

- [[Assessment Overview]] — branch naming, CI jobs, grading snapshot.
- [[A3 Progress]] — active assessment; sync if a CI fix is announced.
