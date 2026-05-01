# Pulling template updates into your assessment fork

Your assignment repo started life as a copy (via GitHub Classroom) of
the canonical template at `ese-ada-lovelace-2025/ppp-openmp-assessment-<N>`.
If the instructor publishes a fix to the template after you've started —
for example, a CI tweak or a clarification to the brief — there are
two ways to pull it into your fork.

There is **no requirement** to track the template. Students who never
sync are still graded on the rubric they accepted. Sync only when the
instructor announces an update worth pulling in.

## Easy path: the "Sync Assignment" pull request

When the instructor pushes a fix and triggers the GH Classroom sync,
GitHub opens a pull request on **your** fork titled something like
`GitHub Classroom: Sync Assignment` (you'll see it on github.com under
your fork's "Pull requests" tab and via the usual GitHub email
notification).

To pull the update in:

1. Open the PR on github.com.
2. Read the diff — it'll show the template-managed file changes only.
3. If GitHub shows **"Able to merge"**: click "Merge pull request" → "Confirm".
4. If GitHub shows **"Conflicts"**: jump to the conflict-resolution section below.
5. Locally, `git pull origin main` to refresh your working copy.

Done — your fork now has the template fix, and your own commits are
preserved as part of the merged history.

## Manual alternative: pull from `upstream`

If you'd rather work from the command line, the same end state is
reachable directly. Your fork already has the `upstream` remote set
to the GH Classroom intermediate (this is automatic for `gh repo clone`
of a GitHub fork). Verify and pull:

```sh
git remote -v          # should list `upstream` pointing at
                       # ese-ada-lovelace-2025/ese-ada-lovelace-2025-classroom-openmp-…

git fetch upstream
git pull upstream main # 3-way merge; preserves your work
git push origin main
```

(For a linear history, use `git rebase upstream/main` and
`git push --force-with-lease origin main` instead. The PR-based path
on github.com gets you the same end state without thinking about
rebase vs merge.)

## Conflict resolution

Conflicts on a Sync Assignment PR or `git pull upstream main` mean
your edits and the template fix touched overlapping lines. This is
unusual on assessment templates because the template-managed file set
is small and disjoint from the student-edit surface (`integrate.cpp`
for A1, `mandelbrot_*.cpp` for A2, the body of `jacobi_step()` for
A3, and your own extension sources).

If you do hit one:

- Did you edit `main()`, `init()`, the harness scripts, or the README?
  Those are template-managed (per the brief) and the conflict signals
  the overlap.
- Resolve in favour of the template's version unless your edit is
  intentional and within scope per the brief — in which case re-apply
  it on top of the merged file.
- For PR-based merges, GitHub's web editor handles this. For
  command-line merges, `git status` lists the conflicted files; edit,
  `git add`, then `git commit`.

## Template-managed files (don't edit these)

| Path | Purpose |
|---|---|
| `README.md` | Assignment brief |
| `.github/workflows/ci.yml` | Formative CI workflow |
| `.github/scripts/check_language.py` | English-only check |
| `.github/actions/collect-untracked/action.yml` | Artefact upload helper |
| `.gitignore` | Build-artefact exclusions |
| `.clang-format`, `.clang-tidy` | Lint config (lockstepped with lectures repo) |
| `bin/smart_diff.py`, `bin/hyperfine_min_time.py`, `bin/run_and_check.sh` | Harness scripts |
| `expected_output.txt` | Reference output for `smart_diff.py` |
| `evaluate.pbs` | Rome perf harness |
| `CMakeLists.txt` | Build system |
| `LICENSE`, `requirements.txt` | Boilerplate |
| `mandelbrot.h` (A2) | Shared header |
| `core/stencil.h` (A3) | Stencil-grid declarations |
| `core/stencil.cpp` (A3) | Stencil scaffolding (`main()`, `init()`, `checksum()`); your work is the body of `jacobi_step()` only |
| `questions.md` | MCQ bank (you fill `answers.csv`) |
| `EXTENSION.md` (A3) | Structured-header template; you fill in values, not template fields |

The files **you** own and freely modify are everything else: the C++
source you write (`integrate.cpp`, `mandelbrot_for.cpp`,
`mandelbrot_tasks.cpp`, the body of `jacobi_step()`, your extension
sources), `CHOICE.md`, `EXTENSION.md` (values, not the template
fields), `REFLECTION.md`, `answers.csv`, `tables.csv`.

## Advanced fallback: cherry-pick directly from the canonical template

If the Sync PR didn't appear and `git pull upstream main` doesn't work
either (e.g., the `upstream` remote got removed), you can pull
specific files directly from the canonical template:

```sh
git remote add template git@github.com:ese-ada-lovelace-2025/ppp-openmp-assessment-<N>.git
git fetch template
git checkout template/main -- README.md .github/workflows/ci.yml
git add README.md .github/workflows/ci.yml
git commit -m "Pull from template"
git push origin main
```

Don't try `git rebase template/main` — the canonical template and
your fork have unrelated git histories (GH Classroom creates fresh
intermediates), so the rebase produces add/add conflicts on every
file. The intermediate (`upstream`) is the right anchor; the
canonical template is only for emergencies.

## What this is not

- **It is not "the template is now your upstream code base".** Your
  repo's history stays yours. Syncing brings updates to template-
  managed files only.
- **It is not a substitute for working on the assignment.** Syncing
  changes nothing in your editable files.
- **It is not a tool for backing out your own work.** If you used
  `git pull` or `git checkout` and lost a local change, recover from
  the reflog (`git reflog`) — don't try to undo with another sync.
