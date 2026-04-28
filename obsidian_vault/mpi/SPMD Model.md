# SPMD — Single Program, Multiple Data

MPI's execution model:

- You write **one program**.
- The launcher (`mpiexec`) starts **N copies** of it.
- Each copy (rank) runs the same code path until it branches based on its rank ID.
- No automatic data partitioning — you compute "what data does rank `r` own" yourself.

## What this looks like in code

A typical MPI program has a structure like:

```cpp
#include <mpi.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Every rank picks its slice of the work
    long long my_chunk = total_work / size;
    long long start = rank * my_chunk;
    long long end = (rank == size - 1) ? total_work : (rank + 1) * my_chunk;

    // Do the local work
    double local_result = do_work(start, end);

    // Combine results across all ranks
    double global_result;
    MPI_Reduce(&local_result, &global_result, 1, MPI_DOUBLE, MPI_SUM,
               /*root=*/0, MPI_COMM_WORLD);

    if (rank == 0) {
        std::cout << "answer = " << global_result << '\n';
    }

    MPI_Finalize();
    return 0;
}
```

Notice:

- All ranks run *every* line until the explicit `if (rank == 0)` branch.
- The data partitioning (`start`, `end`) is computed locally from `rank` and `size`; no shared "I own iterations [a..b]" table.
- The reduce is a *collective* — every rank in the communicator must call it. See [[MPI Reduce]].

## Why "SPMD"?

The classification distinguishes from:

- **SIMD** (Single Instruction, Multiple Data) — what GPU lanes and CPU vector instructions do.
- **MIMD** (Multiple Instruction, Multiple Data) — fully heterogeneous; rare.
- **MPMD** (Multiple Program, Multiple Data) — different binaries cooperating, e.g. coupled solvers. MPI supports this with `mpiexec -np 4 prog_a : -np 2 prog_b`, but it's uncommon.

SPMD ("the same program, different slices of the data") is by far the most common pattern.

## Block partition for a sum

The π example uses a textbook block partition. With `n` total iterations and `size` ranks, rank `r` owns iterations:

```cpp
chunk  = n / size
start  = 1 + r * chunk
finish = (r == size - 1) ? n : (r + 1) * chunk
```

The `(r == size - 1) ? n : ...` handles the case when `n % size != 0` — the last rank picks up the remainder. Without that fix, with `n = 1'000'000'001` and `size = 16`, you'd skip one iteration silently.

See [[../examples/pi_mpi]] for the full source.

## Related

- [[MPI Overview]] — bigger context.
- [[MPI Six Essentials]] — the API you'll actually call.
- [[MPI Reduce]] — combining results.
- [[../examples/pi_mpi]] — the running example.
