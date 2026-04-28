# Tasks

Tasks are units of work that the OpenMP runtime queues and dispatches to threads. Use tasks for workloads that don't fit a regular loop: recursion, divide-and-conquer, irregular pipelines.

## Why tasks?

`parallel for` requires a known iteration count at loop entry. Tasks handle:
- Recursive divide-and-conquer (quicksort, Barnes–Hut, mergesort)
- Producer-consumer pipelines
- Graph traversal
- Per-tile workloads with variable cost (Mandelbrot — A2)

## Task lifecycle

1. Encountered by a thread: `#pragma omp task { body }` packages `body` into a task instance.
2. Queued in the team's task pool.
3. Executed by *some* thread — possibly the spawner, possibly a work-stealer.
4. Synchronised by `taskwait`, end of `taskgroup`, or end of `parallel` region.

The spawning thread does **not** wait for the spawned task. It moves on.

## The canonical pattern: `single` + recursive spawn

```cpp
long long fib_task(long long n)
{
    if (n < 2) return n;

    long long x = 0, y = 0;

#pragma omp task shared(x) firstprivate(n)
    x = fib_task(n - 1);
#pragma omp task shared(y) firstprivate(n)
    y = fib_task(n - 2);
#pragma omp taskwait           // wait for both children

    return x + y;
}

long long fib_parallel(long long n)
{
    long long result = 0;
#pragma omp parallel default(none) shared(result) firstprivate(n)
    {
#pragma omp single             // only one thread starts the tree
        result = fib_task(n);
    }
    return result;
}
```

Without `single`, each thread would spawn the full tree — P× the work.

## Task data environment

Variables captured from outer scope are **`firstprivate` by default** for tasks (unlike worksharing, where they default to `shared`):

```cpp
int outer = 42;
#pragma omp task
{ printf("%d\n", outer); }   // outer is firstprivate — captured at task creation
```

The captured value is fixed **at task creation**, not at task execution. This is a common foot-gun: if you modify `outer` after spawning and before the task runs, the task still sees the old value.

To force sharing: `shared(outer)` explicitly.

## `taskwait` — wait for direct children

```cpp
#pragma omp task { compute_a(); }
#pragma omp task { compute_b(); }
#pragma omp taskwait           // blocks until both direct children finish
use(a, b);                     // safe
```

`taskwait` waits for *direct children only* — not grandchildren.

## `taskgroup` — wait for the whole subtree

```cpp
#pragma omp taskgroup
{
#pragma omp task
    { deep_recursion_spawning_more_tasks(); }
}
// here: every descendant task has finished (transitively)
```

Use `taskgroup` when your tasks spawn their own tasks and you need to wait for the whole DAG.

## Related

- [[Task Dependences]] — depend clauses for pipeline scheduling.
- [[taskloop]] — the convenience form for task-based loops.
- [[single and masked]] — why `single` is the standard spawn wrapper.
- [[../assessment/A2 Mandelbrot]] — A2 uses tasks for irregular tile workload.
