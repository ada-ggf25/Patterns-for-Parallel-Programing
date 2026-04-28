# Environment Modules — Overview

A cluster like CX3 has hundreds of compilers, libraries, MPI implementations, and Python versions installed at the same time. Putting them all on `$PATH` would create unworkable name collisions (which `g++`? which `python3`? which `mpicxx`?).

The solution is the **environment modules** system — on CX3 this is **Lmod**. Each piece of software is packaged as a module; `module load` adjusts your `PATH`, `LD_LIBRARY_PATH`, `MANPATH`, and a few other variables so that exactly the version you want is visible. `module unload` reverses the change.

## Mental model

> `module load X` = "add X to my shell environment for this session."

Modules stack: `ml GCC OpenMPI` first sets up GCC, then layers OpenMPI on top of it. The order matters because OpenMPI was *built* against a specific GCC and announces that dependency in its module file.

## Why this matters in practice

- The right toolchain for one project is the wrong toolchain for another. Modules let you switch quickly.
- Compilers and MPIs are the most session-sensitive: a binary built with one MPI runtime won't run under a different one.
- Login nodes ship with very few modules pre-loaded — by design, so that nothing on the default PATH conflicts with what you `ml` next.

## Two aliases — `module` and `ml`

The full command is `module ...`. The `ml` shortcut behaves identically and is faster to type:

```bash
module load GCC      # fully spelled out
ml GCC               # same thing
```

Most CX3 documentation uses `ml`. So do these notes.

## Related

- [[Lmod Commands]] — the cheat sheet.
- [[tools-prod gateway]] — the meta-module that unlocks the rest.
- [[Loading Combos]] — the recipes you'll actually use in this course.
- [[Module Pitfalls]] — the things that bite people.
