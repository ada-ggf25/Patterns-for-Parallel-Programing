# Module Pitfalls

Three small foot-guns that bite everyone at least once.

## Don't bake `ml` into `~/.bashrc`

It is tempting to drop `ml GCC OpenMPI` at the bottom of `~/.bashrc` so the modules are always there.

**Don't.** Two reasons:

1. **PBS jobs source `~/.bashrc` too.** A `bashrc` that loads modules will bake those modules into every job — including jobs that wanted a different toolchain. Reproducibility goes out the window.
2. **Different projects need different combinations.** The right modules for an MPI build are wrong for a Python pipeline; you'll spend more time un-loading bashrc-imposed defaults than you save.

The correct pattern: load modules **explicitly in each PBS script**. The script is then self-documenting and reproducible.

## After swapping modules in a live shell, run `hash -r`

Bash caches the lookup of "command not found" results. If you tried `lstopo` before loading OpenMPI, then loaded OpenMPI, and tried again, bash may still tell you `lstopo: command not found` — even though it's now on `PATH`.

```bash
hash -r        # clear bash's command cache
```

Run this whenever you load a module after a failed lookup. PBS jobs start with a fresh shell so they don't hit this.

## `ml spider` finds things `ml av` doesn't

If `ml av` doesn't show a module you remember being there, it might be gated behind another module — typically [[tools-prod gateway|`tools/prod`]]. Use `ml spider <name>` to search the full catalogue including gated modules:

```bash
ml spider Python              # lists every Python, even ones gated behind tools/prod
ml spider GCC/14              # version-specific search
```

## "module not found" inside a PBS job

Almost always one of:

1. Forgot `ml tools/prod` first.
2. Typo'd the module name (Lmod is case-sensitive: `GCC` not `gcc`, `OpenMPI` not `openmpi`).
3. Loaded modules that conflict — e.g. two MPI implementations.

Check by running the same `ml` lines on a login node. If they work there, they'll work in PBS.

## Related

- [[Modules Overview]] — the system you're navigating.
- [[Lmod Commands]] — full command reference.
- [[tools-prod gateway]] — the most common cause of "module not found".
- [[../pbs/Common PBS Mistakes]] — overlapping checklist for PBS scripts.
