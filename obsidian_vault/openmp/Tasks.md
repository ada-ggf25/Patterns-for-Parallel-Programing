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

Post-order tree sum — a tree has no flat iteration space so `parallel for` can't reach it; tasks fan out naturally:

```cpp
long tree_sum(const Node* p)
{
    if (p == nullptr) return 0;
    long left_sum = 0, right_sum = 0;

#pragma omp task shared(left_sum)
    left_sum  = tree_sum(p->left);
#pragma omp task shared(right_sum)
    right_sum = tree_sum(p->right);

#pragma omp taskwait           // wait for both direct children
    return p->value + left_sum + right_sum;
}

long tree_sum_parallel(const Node* root)
{
    long total = 0;
#pragma omp parallel default(none) shared(total) firstprivate(root)
    {
#pragma omp single             // one thread seeds the recursion
        total = tree_sum(root);
    }
    return total;
}
```

`shared()` overrides the `firstprivate`-by-default rule so child writes are visible to the parent's locals. Without `single`, each of P threads would spawn the full subtree — P× the work, same answer.

Real-world relatives: quicksort, mergesort, Barnes–Hut tree walks, sparse Cholesky.

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
- [[taskloop]] — task-based loops; when to prefer it over `parallel for`.
- [[single and masked]] — why `single` is the standard spawn wrapper.
- [[../assessment/A2 Mandelbrot]] — A2 requires both a `taskloop` and a `parallel for` variant.
