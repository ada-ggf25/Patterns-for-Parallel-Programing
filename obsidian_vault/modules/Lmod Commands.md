# Lmod Commands

The complete cheat sheet you need. `ml` is a short alias for `module` — they are interchangeable.

## Listing and searching

| Command | What it does |
|---|---|
| `ml` | List currently loaded modules |
| `ml av` | List every module visible *right now* |
| `ml ov` | One-line overview of available modules |
| `ml spider <kw>` | Search across **all** modules, even hidden ones gated behind another |
| `ml whatis <name>` | One-line description of a module |
| `ml help <name>` | Detailed help (includes the loaded environment changes) |

`ml av` only shows the modules that can be loaded *right now* — the catalogue depends on what is already loaded. To find modules that are gated behind `tools/prod` (for example) without loading it first, use `ml spider`.

```bash
ml spider mpi      # find every MPI ever installed
ml spider GCC      # find every GCC version
ml spider Python   # find every Python build
```

## Loading and unloading

| Command | What it does |
|---|---|
| `ml <name>` | Load a module |
| `ml load <name>` | Same; explicit form |
| `ml unload <name>` | Unload one module |
| `ml purge` | Unload **everything** |
| `ml switch A B` | Replace A with B (safer than unload+load — checks dependencies) |

You can load multiple at once by listing them: `ml GCC OpenMPI CMake`.

## Inspecting after the fact

```bash
ml                     # what's loaded?
ml list                # same thing
ml --raw show GCC      # exact env-var changes the module makes
```

`ml --raw show <name>` is the authoritative answer to "what does this actually do to my shell?".

## Recommended habits

- Run `ml` first when something doesn't work — wrong/missing modules are the #1 cause of "command not found" in PBS jobs.
- Run `ml purge` at the start of a PBS script for a known-clean environment, then load only what the job needs.
- Use `ml switch` instead of unload+load when changing versions — Lmod will preserve correct dependency relationships.
- Don't put `ml` calls in `~/.bashrc` — see [[Module Pitfalls]].

## Related

- [[Modules Overview]] — what modules are.
- [[tools-prod gateway]] — the catalogue-unlocking meta module.
- [[Loading Combos]] — the combos you'll use.
