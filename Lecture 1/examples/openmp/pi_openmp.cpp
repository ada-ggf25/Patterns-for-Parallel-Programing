// OpenMP numerical integration of pi.
// The only change from the serial version is the pragma above the loop.
//
// Build:  cmake --build build --target pi_openmp
// Run:    OMP_NUM_THREADS=8 ./build/openmp/pi_openmp

#include <chrono>
#include <cstdio>

#include <omp.h>

int main() {
    const long long n = 1'000'000'000LL;
    const double w = 1.0 / static_cast<double>(n);
    double sum = 0.0;

    const auto t0 = std::chrono::steady_clock::now();

    #pragma omp parallel for reduction(+:sum)
    for (long long i = 1; i <= n; ++i) {
        const double x = w * (static_cast<double>(i) - 0.5);
        sum += 4.0 / (1.0 + x * x);
    }

    const double pi = w * sum;
    const auto t1 = std::chrono::steady_clock::now();
    const double secs = std::chrono::duration<double>(t1 - t0).count();

    std::printf("n=%lld  threads=%d  pi=%.15f  time=%.3fs\n",
                n, omp_get_max_threads(), pi, secs);
    return 0;
}
