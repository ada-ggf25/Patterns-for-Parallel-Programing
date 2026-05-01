// A3 extension: numa_first_touch — naive (serial init) variant.
//
// Identical to core/stencil.cpp except that init() uses plain serial loops.
// All pages in both grids are faulted in by the master thread and therefore
// land on socket 0 (NUMA domain 0). At 128 threads on Rome (8 NUMA domains),
// 7/8 of threads must fetch their working data across the inter-socket xGMI
// link at roughly 3× the latency of local DRAM. This is the "before" binary;
// compare with stencil_ft to measure the NUMA first-touch delta.

#include "../../core/stencil.h"

#include <cstdio>
#include <cstdlib>
#include <omp.h>

inline std::size_t idx(std::size_t i, std::size_t j, std::size_t k)
{
    return (i * NY * NZ) + (j * NZ) + k;
}

void jacobi_step(const double* u, double* u_next)
{
#pragma omp parallel for collapse(3) default(none) shared(u, u_next)
    for (std::size_t i = 1; i < NX - 1; ++i) {
        for (std::size_t j = 1; j < NY - 1; ++j) {
            for (std::size_t k = 1; k < NZ - 1; ++k) {
                u_next[idx(i, j, k)] =
                    (u[idx(i - 1, j, k)] + u[idx(i + 1, j, k)] + u[idx(i, j - 1, k)] +
                     u[idx(i, j + 1, k)] + u[idx(i, j, k - 1)] + u[idx(i, j, k + 1)]) /
                    6.0;
            }
        }
    }
}

double checksum(const double* u)
{
    double s = 0.0;
    for (std::size_t i = 0; i < NX * NY * NZ; ++i) {
        s += u[i];
    }
    return s;
}

static void init(double* u)
{
    // Serial zeroing — all pages land on the master thread's NUMA node (socket 0).
    for (std::size_t i = 0; i < NX * NY * NZ; ++i) {
        u[i] = 0.0;
    }
    // Serial BC face — consistent with serial page placement.
    for (std::size_t j = 0; j < NY; ++j) {
        for (std::size_t k = 0; k < NZ; ++k) {
            u[idx(0, j, k)] = 1.0;
        }
    }
}

int main()
{
    // posix_memalign returns uninitialised memory so the serial init() below is
    // the true first-touch — all pages land on the master thread's NUMA node
    // (socket 0 / domain 0), the intentional "bad" placement this variant
    // exists to demonstrate.
    void* raw_a = nullptr;
    void* raw_b = nullptr;
    posix_memalign(&raw_a, 64, NX * NY * NZ * sizeof(double));
    posix_memalign(&raw_b, 64, NX * NY * NZ * sizeof(double));
    double* a = static_cast<double*>(raw_a);
    double* b = static_cast<double*>(raw_b);

    init(a);
    init(b);

    double* cur = a;
    double* nxt = b;
    for (int s = 0; s < NSTEPS; ++s) {
        jacobi_step(cur, nxt);
        double* tmp = cur;
        cur = nxt;
        nxt = tmp;
    }
    // Deterministic output — correctness channel only. Timing via hyperfine.
    std::printf("checksum = %.6e\n", checksum(cur));
    free(a);
    free(b);
    return 0;
}
