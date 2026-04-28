# `single`, `masked`, and `nowait`

Constructs for running code on one thread inside a parallel region.

## `single` — any one thread, then barrier

```cpp
#pragma omp parallel default(none) shared(setup)
{
#pragma omp single
    {
        do_one_shot_setup(setup);   // runs on whichever thread arrives first
    }
    // implicit barrier — all threads wait here; everyone sees setup result

    do_per_thread_work_using(setup);
}
```

- Exactly one thread (any) runs the block.
- Implicit barrier at exit — the rest of the team waits.
- Use for one-shot initialisation, lazy setup, or task-spawn loops.

## `masked` (5.1) — thread 0 only, no barrier

```cpp
#pragma omp parallel
{
#pragma omp masked           // only thread 0 runs this; others skip immediately
    {
        printf("from thread 0 only\n");
    }
    // NO barrier — other threads continue immediately

    do_per_thread_work();
}
```

- Only thread 0 (by default) runs the block.
- *No* implicit barrier — others don't wait.
- 5.1's `masked` replaces the deprecated `master` directive. They are semantically equivalent; `masked` is more general (any thread can be designated).

## `single nowait` — one thread, no barrier

```cpp
#pragma omp single nowait
{
    log_one_message();    // one thread logs; rest don't wait
}
do_unrelated_work();      // runs immediately on all other threads
```

`nowait` strips the implicit barrier from `single`. Only safe when the rest of the team does **not** need to read what `single` wrote. Easy source of races if misused.

## Choosing between them

| Need | Construct |
|---|---|
| One-shot setup that all threads need to see | `single` (with implicit barrier) |
| One-shot logging / side effect, no consumers | `single nowait` |
| Thread-0-specific code, no consumer | `masked` |
| Start a task tree from one thread | `single` wrapping `#pragma omp task` spawn |

## Tasks: why `single` is required

For recursive task trees, `single` prevents every team member from redundantly spawning the same tree:

```cpp
#pragma omp parallel
{
#pragma omp single          // only one thread starts the recursion
    result = fib_task(n);   // others pick up queued tasks
}
```

Without `single`, each of P threads would spawn the full subtree — P× the work, same answer.

## Related

- [[Tasks]] — why `single` is the standard task-spawn wrapper.
- [[Barriers]] — how implicit/explicit barriers work.
- [[Memory Model]] — implicit flush at `single` exit (unless `nowait`).
