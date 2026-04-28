# Rank and Size

Every MPI rank needs to know two things: who it is, and how many of them there are.

```cpp
int rank, size;
MPI_Comm_rank(MPI_COMM_WORLD, &rank);     // 0 .. size-1
MPI_Comm_size(MPI_COMM_WORLD, &size);     // total number of ranks
```

## Communicators — `MPI_COMM_WORLD`

`MPI_COMM_WORLD` is the default communicator: "everyone launched together by `mpiexec`". You can create sub-communicators (`MPI_Comm_split`) for partial groups, but for simple programs you'll stay in `MPI_COMM_WORLD`.

`rank` is your ID *within* a communicator. The same process has different ranks in different communicators.

## Convention: rank 0 is the "root"

By convention:

- Rank 0 reads input files and writes output. (One rank does I/O; others get the data via `MPI_Bcast` if needed.)
- Rank 0 is the root for `MPI_Reduce` (where the final value lands).
- Rank 0 prints things — only one rank prints, so output isn't a mess of N copies.

Nothing in MPI *enforces* this; it's just the universal idiom. Your code will read it everywhere.

## The `if (rank == 0)` branch

```cpp
if (rank == 0) {
    // Master-only work: read config, write results, print summary.
    std::cout << "result = " << global_sum << '\n';
}
```

This is the canonical SPMD branch — every rank runs it, but only rank 0's body executes. See [[SPMD Model]].

## Picking your slice of the work

Once you have `rank` and `size`, partition the data:

```cpp
const long long n = 1'000'000'000LL;
const long long chunk  = n / size;
const long long start  = 1 + rank * chunk;
const long long finish = (rank == size - 1) ? n : (rank + 1) * chunk;
```

The `(rank == size - 1) ? n : ...` clause handles the case when `n` doesn't divide evenly — the last rank picks up the remainder. Without it, with `n = 1'000'000'001` and `size = 16`, you'd silently skip one iteration.

This block-partitioning idiom comes up over and over in MPI code. See [[../examples/pi_mpi]] for the full implementation.

## Related

- [[MPI Init Finalize]] — call these *after* Init.
- [[SPMD Model]] — why each rank runs the same code.
- [[MPI Reduce]] — what rank 0 typically does with everyone's contributions.
