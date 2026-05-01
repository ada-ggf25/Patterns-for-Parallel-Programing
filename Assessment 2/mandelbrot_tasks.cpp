// Assignment 2 — tasks variant (STUDENT IMPLEMENTATION).
//
// Tile the NPOINTS × NPOINTS grid into TILE × TILE blocks and spawn one
// OpenMP task per tile. Because per-pixel cost is highly variable near the
// Mandelbrot boundary, task-parallelism gives better load-balancing than
// static parallel_for.
//
// Two idioms are equally acceptable:
//   (a) #pragma omp parallel + #pragma omp single + nested #pragma omp task
//   (b) #pragma omp parallel + #pragma omp taskloop grainsize(...)
//
// Aggregate the per-tile counts with a reduction or an atomic/critical.
// Implementation uses option (b): parallel + single + taskloop with a
// per-task local accumulator flushed into the shared total via atomic.
//
// This variant compiles to `./build/mandelbrot_tasks`.

#include "mandelbrot.h"

#include <cstdio>
#include <omp.h>

namespace {
struct cplx
{
    double re;
    double im;
};

inline int escape_iters(double cr, double ci)
{
    cplx z = {cr, ci};
    for (int it = 0; it < MAXITER; ++it) {
        const double ztemp = (z.re * z.re) - (z.im * z.im) + cr;
        z.im = (z.re * z.im * 2.0) + ci;
        z.re = ztemp;
        if ((z.re * z.re) + (z.im * z.im) > 4.0) {
            return it;
        }
    }
    return MAXITER;
}

// Count escape points within an upper-half tile.
long count_tile_upper(int i0, int j0, int j_half)
{
    const int i1 = (i0 + TILE < NPOINTS) ? i0 + TILE : NPOINTS;
    const int j1 = (j0 + TILE < j_half) ? j0 + TILE : j_half;
    long local = 0;
    for (int i = i0; i < i1; ++i) {
        for (int j = j0; j < j1; ++j) {
            const double cr = -2.0 + (3.0 * static_cast<double>(i) / NPOINTS);
            const double ci = -1.5 + (3.0 * static_cast<double>(j) / NPOINTS);
            if (escape_iters(cr, ci) < MAXITER) {
                ++local;
            }
        }
    }
    return local;
}
} // namespace

long mandelbrot_tasks()
{
    long outside = 0;

    // Each task processes one outer tile-row (all j-tiles for a fixed i0).
    // A task-local `tile_count` accumulates the row's contribution; a single
    // `#pragma omp atomic` at the end of the task flushes it into `outside`.
    // Using `atomic` rather than `taskloop reduction` avoids the race that
    // TSan/Archer report with the opaque runtime combiner (`.red_comb.`):
    // the combiner reads per-task private copies via internal OMP bookkeeping
    // that TSan does not track as a proper happens-before edge.
#pragma omp parallel default(none) shared(outside)
#pragma omp single
    {
#pragma omp taskloop grainsize(1) default(none) shared(outside)
        for (int i0 = 0; i0 < NPOINTS; i0 += TILE) {
            long tile_count = 0;
            for (int j0 = 0; j0 < NPOINTS / 2; j0 += TILE) {
                tile_count += 2L * count_tile_upper(i0, j0, NPOINTS / 2);
            }
#pragma omp atomic
            outside += tile_count;
        }
    } // implicit taskwait at end of single; implicit barrier at end of parallel
    if constexpr (NPOINTS % 2 == 1) {
        constexpr int j = NPOINTS / 2;
        for (int i = 0; i < NPOINTS; ++i) {
            const double cr = -2.0 + (3.0 * static_cast<double>(i) / NPOINTS);
            const double ci = -1.5 + (3.0 * static_cast<double>(j) / NPOINTS);
            if (escape_iters(cr, ci) < MAXITER) {
                ++outside;
            }
        }
    }
    return outside;
}

int main()
{
    const long outside = mandelbrot_tasks();
    const double area =
        9.0 * static_cast<double>(outside) / (static_cast<double>(NPOINTS) * NPOINTS);
    // Deterministic output — correctness channel only. Timing via hyperfine.
    std::printf("outside = %ld\n", outside);
    std::printf("area = %.6f\n", area);
    return 0;
}
