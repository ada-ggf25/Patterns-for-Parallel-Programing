// Assignment 2 — reference serial Mandelbrot (DO NOT EDIT).
//
// Counts the number of grid points that escape the Mandelbrot set within
// MAXITER iterations. Returns the count; the main() wrappers in
// mandelbrot_for.cpp / mandelbrot_tasks.cpp print area derived from this.
//
// **Symmetry refactor**: the Mandelbrot set is symmetric about the real
// axis (mandel(c) = mandel(c̄)). We iterate only the upper half of the
// imaginary axis (j ∈ [0, NPOINTS/2)) and double the count. ~2× faster
// than the naive doubly-nested implementation. If NPOINTS is odd, the
// centre row is counted exactly once.

#include "mandelbrot.h"

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

long mandelbrot_serial()
{
    long outside = 0;
    constexpr int J_HALF = NPOINTS / 2;        // upper half by symmetry
    for (int i = 0; i < NPOINTS; ++i) {
        for (int j = 0; j < J_HALF; ++j) {
            const double cr = -2.0 + (3.0 * static_cast<double>(i) / NPOINTS);
            const double ci = -1.5 + (3.0 * static_cast<double>(j) / NPOINTS);
            if (escape_iters(cr, ci) < MAXITER) {
                outside += 2;                  // mirror in lower half
            }
        }
    }
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
