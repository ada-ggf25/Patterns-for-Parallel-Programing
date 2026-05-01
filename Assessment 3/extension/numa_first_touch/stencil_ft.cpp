// A3 extension: numa_first_touch — parallel first-touch variant.
//
// Identical to core/stencil.cpp. init() uses two parallel loops so each thread
// faults in exactly the pages it will later read during jacobi_step(), placing
// them on the correct NUMA domain. At 128 threads on Rome (8 NUMA domains) all
// domains host a proportional share of the 2.1 GB working set, eliminating
// cross-socket bandwidth bottlenecks. This is the "after" binary; compare with
// stencil_naive to measure the NUMA first-touch delta.

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
    // Parallel first-touch: each thread writes its portion first, so the OS
    // places those pages on the thread's local NUMA domain. The traversal order
    // (flat i*NY*NZ + j*NZ + k) matches jacobi_step(), ensuring correct
    // placement for the compute phase.
#pragma omp parallel for default(none) shared(u)
    for (std::size_t i = 0; i < NX * NY * NZ; ++i) {
        u[i] = 0.0;
    }
    // Dirichlet BC on one face — drives the diffusion.
#pragma omp parallel for default(none) shared(u)
    for (std::size_t j = 0; j < NY; ++j) {
        for (std::size_t k = 0; k < NZ; ++k) {
            u[idx(0, j, k)] = 1.0;
        }
    }
}

int main()
{
    // posix_memalign returns uninitialised memory: pages are virtual-only until
    // the first write, so the parallel loops in init() are the true first-touch
    // and the OS places each page on the NUMA domain of the touching thread.
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
