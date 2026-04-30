# Pulling template updates into your assessment fork

Your assignment repo started life as a copy of the canonical template
(`ese-ada-lovelace-2025/ppp-openmp-assessment-<N>` for assignment N).
GitHub Classroom **does not** auto-sync the template into your repo:
once you accept the assignment, your fork is independent. If we publish
a fix to the template after you've started — for example, a CI tweak
that makes ThreadSanitizer print source line numbers — you need to
pull it in by hand.

## Important: do *not* `git rebase template/main`

The natural-looking command `git rebase template/main` **does not work
cleanly** for this setup. GitHub Classroom creates per-student repos as
fresh repos with a synthetic "Initial commit" that has the template's
*content* but none of its *history*. So:

- `git fetch template` shows `warning: no common commits`.
- The first commit of your repo (the GH Classroom "Initial commit")
  conflicts with `template/main` on every file whose content has
  changed in the template since you accepted (typically `README.md`
  and `.github/workflows/ci.yml`).
- Resolving the conflicts replays a series of empty / near-empty
  commits and forces you to rewrite your commit history.

**Use the file-checkout approach below instead.** It's a single
fast-forward commit, no rebase, no force-push.

## The cherry-pick approach (use this)

Replace `<N>` with your assignment number (`1`, `2`, or `3`).

```sh
# 1. Add the canonical template as a new remote (one-off setup).
git remote add template git@github.com:ese-ada-lovelace-2025/ppp-openmp-assessment-<N>.git

# 2. Fetch the latest template state.
git fetch template

# 3. Bring the template-managed files up to date in your working tree.
#    (Add other template-managed files to this list if a fix is announced
#    that changes them, e.g. `core/stencil.h` for assignment 3.)
git checkout template/main -- README.md .github/workflows/ci.yml

# 4. (If your assignment uses any of the other template-managed files —
#    typically you don't need to touch these, but if a fix is announced
#    that includes them, add their paths to the checkout above.)

# 5. Commit and push as a normal commit.
git add README.md .github/workflows/ci.yml
git commit -m "Sync README + CI from template"
git push origin main
```

That's it — no force push, no SHA rewriting, no conflict resolution.

## Which files are template-managed

Anything in this list is **owned by the template** and you should not
modify it in your fork. When the template changes any of these, the
cherry-pick command above can pick up the update without conflict:

| Path | Purpose |
|---|---|
| `README.md` | Assignment brief |
| `.github/workflows/ci.yml` | Formative CI workflow |
| `.github/scripts/check_language.py` | English-only language check |
| `.github/actions/collect-untracked/action.yml` | Artifact upload helper |
| `.gitignore` | Build-artefact exclusions |
| `.clang-format` | Lint config (kept in lockstep with lectures repo) |
| `.clang-tidy` | Lint config |
| `bin/smart_diff.py`, `bin/hyperfine_min_time.py`, `bin/run_and_check.sh` | Harness scripts |
| `expected_output.txt` | Reference output for `smart_diff.py` |
| `evaluate.pbs` | Rome benchmark harness |
| `mandelbrot.h` (A2) | Shared header — do not edit |
| `questions.md` | MCQ bank — do not edit (you fill `answers.csv`) |
| `CMakeLists.txt` | Build system |
| `LICENSE` | License file |
| `requirements.txt` | Python deps for CI |

The files **you** own and should freely modify are everything else:
the C++ source you write (`mandelbrot_for.cpp`, `mandelbrot_tasks.cpp`,
`stencil.cpp`, etc.), `CHOICE.md` / `EXTENSION.md`, `REFLECTION.md`,
`answers.csv`, and `tables.csv`.

## What if I *did* modify a template-managed file?

You weren't supposed to (the brief says so), but if you did:

- `git checkout template/main -- <file>` will overwrite your changes
  silently. Before running it, save your version somewhere first:
  `cp <file> <file>.mine`.
- After the checkout, manually re-apply whatever change you needed
  from `<file>.mine` on top of the new template version.
- If your change was something the brief allowed — e.g., adding a
  personal note at the top of the README — keep that change as a
  separate small commit on top of the template sync.

## Verifying the sync

```sh
# Should now show the same content as template/main for the synced files.
git diff template/main -- README.md .github/workflows/ci.yml
# Output: nothing (= identical)

# Your work in the editable files should be untouched.
git status
# Output: clean working tree.
```

## When to do this

Sync only when an instructor announcement says the template was
updated and the change is one you want. There's **no requirement**
to track the template — students who never sync will still be graded
on the rubric they accepted. Common reasons to sync:

- A CI bug fix (e.g., the TSan symbolisation fix that gives source
  line numbers in race reports).
- A clarification in the README that affects your understanding of
  the rubric.
- A correction to `expected_output.txt` (rare; only if announced).

## What this is not

- **It is not "the template is now your upstream".** You're not
  forking from the template in the git sense; you're occasionally
  pulling specific file content from it. Your repo's history stays
  yours.
- **It is not a substitute for actually working on the assignment.**
  Syncing brings nothing into your editable files — those are your
  work. Sync solves a different problem (template-side fixes after
  you started).
- **It is not a tool for backing out your own work.** If you used
  `git checkout` and lost a local change, recover from the reflog
  (`git reflog`) — don't try to undo with another sync.
