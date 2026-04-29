// Assignment 2 — parallel_for variant (STUDENT IMPLEMENTATION).
//
// Parallelise using #pragma omp parallel for. This is the baseline against
// which your tasks variant in mandelbrot_tasks.cpp is compared.
// The per-pixel (or per-tile) cost is highly variable, so a naive
// schedule(static) may scale poorly; consider schedule(dynamic) or a
// tiled-and-dynamically-scheduled layout.
//
// The main() driver for this variant compiles to `./build/mandelbrot_for`.

#include "mandelbrot.h"

#include <cstdio>
#include <omp.h>

namespace {
struct cplx {
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
}  // namespace

long mandelbrot_parallel_for()
{
    long outside = 0;
    constexpr int J_HALF = NPOINTS / 2;     // upper half by symmetry

    // TODO(student): parallelise the outer-or-collapsed loop. Reduction on
    // `outside`. Pick a schedule. Serial fallback below so the scaffolding
    // builds on day 2.
    //
    // The loop body counts each grid point in the upper half (j ∈ [0, J_HALF))
    // and adds 2 to `outside` for each escape (mirror in the lower half via
    // Mandelbrot symmetry mandel(c) = mandel(c̄)).
    for (int i = 0; i < NPOINTS; ++i) {
        for (int j = 0; j < J_HALF; ++j) {
            const double cr = -2.0 + (3.0 * static_cast<double>(i) / NPOINTS);
            const double ci = -1.5 + (3.0 * static_cast<double>(j) / NPOINTS);
            if (escape_iters(cr, ci) < MAXITER) {
                outside += 2;
            }
        }
    }
    if constexpr (NPOINTS % 2 == 1) {
        // Centre row (only when NPOINTS is odd) — counted exactly once.
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
    const long outside = mandelbrot_parallel_for();
    const double area = 9.0 * static_cast<double>(outside)
                        / (static_cast<double>(NPOINTS) * NPOINTS);
    // Deterministic output — correctness channel only. Timing via hyperfine.
    std::printf("outside = %ld\n", outside);
    std::printf("area = %.6f\n", area);
    return 0;
}
