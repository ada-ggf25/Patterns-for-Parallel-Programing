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
// per-task reduction accumulator.
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
    constexpr int J_HALF = NPOINTS / 2;

    // `taskloop` distributes the outer tile rows (50 iterations) across tasks;
    // the inner j-loop is sequential within each task, so each task processes
    // one outer-tile row × all 25 upper-half j-tiles = 25 tiles per task.
    // `reduction(+:outside)` gives every task a private accumulator
    // (initialised to 0) that the runtime merges at task completion — no
    // atomic write needed inside the task body. `firstprivate(J_HALF)` copies
    // the constant by value so there is no shared pointer in the task data,
    // which also avoids a TSan false positive from the runtime task-data pool.
#pragma omp parallel default(none) shared(outside, J_HALF)
#pragma omp single
    {
#pragma omp taskloop grainsize(1) reduction(+ : outside) default(none) firstprivate(J_HALF)
        for (int i0 = 0; i0 < NPOINTS; i0 += TILE) {
            for (int j0 = 0; j0 < J_HALF; j0 += TILE) {
                outside += 2 * count_tile_upper(i0, j0, J_HALF);
            }
        }
    } // implicit taskwait at end of single; implicit barrier at end of parallel
    if constexpr (NPOINTS % 2 == 1) {
        const int j = J_HALF;
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
