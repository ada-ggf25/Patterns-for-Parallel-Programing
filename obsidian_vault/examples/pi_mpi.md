# `pi_mpi.cpp` — The MPI Version

The MPI version is the most invasive of the three: it adds the MPI bookends ([[../mpi/MPI Init Finalize]]), a [[../mpi/SPMD Model|block partition]], and a [[../mpi/MPI Reduce|collective reduce]].

## The full file

```cpp
#include <cstdio>
#include <mpi.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank = 0;
    int size = 1;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const long long n = 1'000'000'000LL;
    const double w = 1.0 / static_cast<double>(n);

    // Block partition: rank r owns iterations [start, finish].
    const long long chunk  = n / size;
    const long long start  = 1 + static_cast<long long>(rank) * chunk;
    const long long finish = (rank == size - 1)
                               ? n
                               : static_cast<long long>(rank + 1) * chunk;

    const double t0 = MPI_Wtime();

    double local_sum = 0.0;
    for (long long i = start; i <= finish; ++i) {
        const double x = w * (static_cast<double>(i) - 0.5);
        local_sum += 4.0 / (1.0 + x * x);
    }

    double global_sum = 0.0;
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM,
               /*root=*/0, MPI_COMM_WORLD);

    const double t1 = MPI_Wtime();

    if (rank == 0) {
        const double pi = w * global_sum;
        std::printf("n=%lld  ranks=%d  pi=%.15f  time=%.3fs\n",
                    n, size, pi, t1 - t0);
    }

    MPI_Finalize();
    return 0;
}
```

## Walk-through

### Bookends

```cpp
MPI_Init(&argc, &argv);
... 
MPI_Finalize();
```

See [[../mpi/MPI Init Finalize]].

### Identifying the rank

```cpp
int rank, size;
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
MPI_Comm_size(MPI_COMM_WORLD, &size);
```

See [[../mpi/Rank and Size]]. Defaults of `rank=0, size=1` are belt-and-braces; both calls always succeed and overwrite them.

### Block partition

```cpp
const long long chunk  = n / size;
const long long start  = 1 + rank * chunk;
const long long finish = (rank == size - 1) ? n : (rank + 1) * chunk;
```

Standard block partition. With `n = 10⁹` and `size = 16`:

| Rank | start | finish |
|---|---|---|
| 0 | 1 | 62500000 |
| 1 | 62500001 | 125000000 |
| ... | ... | ... |
| 15 | 937500001 | 1000000000 |

The `(rank == size - 1) ? n : ...` clause handles uneven divisions: if `n % size != 0`, the last rank picks up the remainder. Without it, with `n = 1'000'000'001`, you'd silently skip an iteration.

### Local accumulation

```cpp
double local_sum = 0.0;
for (long long i = start; i <= finish; ++i) {
    const double x = w * (i - 0.5);
    local_sum += 4.0 / (1.0 + x * x);
}
```

Same loop body as serial — every rank just runs over its own slice.

### Collective reduce

```cpp
double global_sum = 0.0;
MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM,
           /*root=*/0, MPI_COMM_WORLD);
```

Every rank contributes `local_sum`; the result lands on rank 0 in `global_sum`. **Every rank must call this** — see [[../mpi/MPI Reduce]] and [[../mpi/MPI Pitfalls]].

### Rank-0-only output

```cpp
if (rank == 0) {
    const double pi = w * global_sum;
    std::printf("...");
}
```

Only rank 0 has a meaningful `global_sum`, and printing from every rank would produce a mess. Standard SPMD pattern.

## Why `MPI_Wtime()` instead of `std::chrono`

`MPI_Wtime()` is portable across MPI implementations and machines; it's guaranteed to be monotonic and synchronised on each rank. Practically equivalent to `std::chrono::steady_clock` for our purposes.

## Building

```bash
mpicxx -O3 pi_mpi.cpp -o pi_mpi
```

Or with CMake:

```bash
cd ic-hpc-intro/examples
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target pi_mpi
mpiexec -n 4 ./build/mpi/pi_mpi
```

See [[../mpi/Building MPI]].

## Running on CX3

```bash
qsub mpi/pi_mpi.pbs
```

For the PBS script and what to expect from the output, see [[../mpi/MPI PBS Script]] and [[../mpi/Reading the MPI log]].

## Expected scaling

| Ranks | Wall-time (Rome, 1 node) |
|---|---:|
| 1 | ~10 s |
| 4 | ~2.7 s |
| 8 | ~1.4 s |
| 16 | ~0.7 s |
| 32 | ~0.4 s |
| 64 | ~0.2 s |

Near-linear up to 32 ranks, flattening as fork-overhead and the reduce step's logarithmic cost catch up. See [[../openmp/Amdahls Law]].

## Related

- [[Pi Algorithm]]
- [[pi_serial]]
- [[pi_openmp]]
- [[../mpi/MPI Six Essentials]]
- [[../mpi/MPI PBS Script]]
- [[../mpi/Reading the MPI log]]
