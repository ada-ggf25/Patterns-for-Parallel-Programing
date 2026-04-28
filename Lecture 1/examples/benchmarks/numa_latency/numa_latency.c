// numa_latency.c — NUMA-aware pointer-chase latency probe.
//
// Allocates a buffer on the NUMA node passed as argv[1] and runs a
// serialised pointer chase over it; the average dereference time is
// printed as lat_ns=<ns>.
//
// Run via numactl so the *CPU* side of the measurement is pinned:
//     numactl --cpunodebind=<src> ./numa_latency <tgt> [MB] [iters]
// Default buffer size is 256 MB (well beyond the 16 MB per-CCX L3 on
// Rome) and 50M iterations, giving a few seconds per measurement.

#define _GNU_SOURCE
#include <numa.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static double now_ns(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return 1e9 * (double)t.tv_sec + (double)t.tv_nsec;
}

int main(int argc, char **argv) {
    if (numa_available() < 0) {
        fprintf(stderr, "numa unavailable\n");
        return 1;
    }
    if (argc < 2) {
        fprintf(stderr, "usage: %s <tgt_node> [MB=256] [iters=50000000]\n", argv[0]);
        return 1;
    }
    int tgt = atoi(argv[1]);
    size_t mb = (argc > 2) ? (size_t)atol(argv[2]) : 256;
    long iters = (argc > 3) ? atol(argv[3]) : 50000000L;

    size_t nbytes = mb << 20;
    size_t n = nbytes / sizeof(size_t);

    size_t *buf = (size_t *)numa_alloc_onnode(nbytes, tgt);
    if (!buf) {
        fprintf(stderr, "numa_alloc_onnode(%zu bytes, node %d) failed\n", nbytes, tgt);
        return 1;
    }

    // Sattolo's algorithm: start from the identity and swap buf[i] with a
    // strictly-smaller index. The result is a single-cycle permutation, so
    // p = buf[p] visits every slot once before returning to the start.
    for (size_t i = 0; i < n; ++i) buf[i] = i;
    srand(0xC0FFEE);
    for (size_t i = n - 1; i > 0; --i) {
        size_t j = (size_t)rand() % i;
        size_t t = buf[i]; buf[i] = buf[j]; buf[j] = t;
    }

    // Warm up: fault in the pages (already bound to tgt) and settle the chase.
    size_t p = 0;
    for (long k = 0; k < 2000000L; ++k) p = buf[p];
    __asm__ __volatile__("" : : "r"(p) : "memory");

    const double t0 = now_ns();
    for (long k = 0; k < iters; ++k) p = buf[p];
    const double t1 = now_ns();
    __asm__ __volatile__("" : : "r"(p) : "memory");

    const double ns_per = (t1 - t0) / (double)iters;
    printf("tgt=%d MB=%zu iters=%ld lat_ns=%.2f\n", tgt, mb, iters, ns_per);
    numa_free(buf, nbytes);
    return 0;
}
