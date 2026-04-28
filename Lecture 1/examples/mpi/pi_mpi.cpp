// MPI numerical integration of pi.
// Each rank sums over its own slice of the N iterations; rank 0 reduces.
//
// Build:  cmake --build build --target pi_mpi
// Run:    mpiexec -n 4 ./build/mpi/pi_mpi

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
