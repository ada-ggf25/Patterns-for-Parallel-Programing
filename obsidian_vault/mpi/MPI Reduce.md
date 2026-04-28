# `MPI_Reduce` — Collective Summation

Combines a value from every rank into a single result on the *root* rank.

```cpp
double local  = /* this rank's partial sum */;
double global = 0.0;

MPI_Reduce(&local, &global, /*count=*/1, MPI_DOUBLE, MPI_SUM,
           /*root=*/0, MPI_COMM_WORLD);

if (rank == 0) {
    std::cout << "global sum = " << global << '\n';
}
```

## Anatomy of the call

```c
int MPI_Reduce(const void* sendbuf,    // every rank: pointer to the value(s) to contribute
               void*       recvbuf,    // root only: pointer to the buffer for the result
               int         count,      // number of elements (here 1)
               MPI_Datatype datatype,  // MPI_DOUBLE, MPI_INT, MPI_FLOAT, ...
               MPI_Op      op,         // MPI_SUM, MPI_MAX, MPI_MIN, MPI_PROD, ...
               int         root,       // which rank receives the combined result
               MPI_Comm    comm);      // usually MPI_COMM_WORLD
```

## What "collective" means

**Every rank in the communicator must call `MPI_Reduce`** — even ranks whose contribution is irrelevant. The function blocks until everyone has called it; then MPI combines the values and delivers the result to `root`.

If only some ranks call it, the others sit forever waiting → deadlock. See [[MPI Pitfalls]].

## Operators

| Op | Meaning |
|---|---|
| `MPI_SUM` | + |
| `MPI_PROD` | × |
| `MPI_MAX` / `MPI_MIN` | max / min |
| `MPI_LAND` / `MPI_LOR` | logical AND / OR |
| `MPI_BAND` / `MPI_BOR` | bitwise AND / OR |
| `MPI_MAXLOC` / `MPI_MINLOC` | location-aware (returns value + rank index) |

## `MPI_Reduce` vs `MPI_Allreduce`

- **`MPI_Reduce`** delivers the result to **one** rank (the `root`).
- **`MPI_Allreduce`** delivers the result to **every** rank — same signature, no `root` argument.

Use `MPI_Reduce` when only one rank needs the answer (e.g. for printing). Use `MPI_Allreduce` when every rank needs it (e.g. for an iterative algorithm where everyone needs the new global norm).

## In the running example

The π MPI program has every rank compute a local partial sum, then exactly one `MPI_Reduce` combines them:

```cpp
double local_sum = 0.0;
for (long long i = start; i <= finish; ++i) {
    const double x = w * (i - 0.5);
    local_sum += 4.0 / (1.0 + x * x);
}

double global_sum = 0.0;
MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

if (rank == 0) {
    const double pi = w * global_sum;
    std::cout << "pi = " << pi << '\n';
}
```

See [[../examples/pi_mpi]] for the full file.

## Note on result on non-root ranks

After `MPI_Reduce` returns, `recvbuf` is **only valid on the root rank**. On other ranks it's untouched (or undefined for some implementations). That's why we initialise `global_sum = 0.0` and read it inside `if (rank == 0)`.

## Related

- [[MPI Six Essentials]]
- [[SPMD Model]] — why every rank calls collectives.
- [[MPI Pitfalls]] — collective deadlocks.
- [[../examples/pi_mpi]]
