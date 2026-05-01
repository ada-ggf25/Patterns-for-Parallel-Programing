// Assignment 1 — reference serial implementation (DO NOT EDIT).
//
// Computes ∫[a,b] f(x) dx via composite trapezoid rule with N subintervals.
// f(x) is deliberately non-uniform in cost so schedule choice in the parallel
// version (integrate.cpp) is genuinely informative.
//
// Build:
//   cmake -B build -S . && cmake --build build --target integrate
//   OMP_NUM_THREADS=4 ./build/integrate
//
// The grader compares ./build/integrate's stdout against expected_output.txt
// via bin/smart_diff.py (float-tolerant).

#include "integrate.h"

#include <cmath>

// Deliberately non-uniform: the "spike" region has heavier per-evaluation
// cost. Chosen so that static scheduling leaves one thread doing 10× the
// work of its peers when the domain straddles the spike.
double f(double x)
{
    // Heavy region: ten nested sqrt-of-sqrt ops when x ∈ [0.3, 0.4].
    const bool heavy = (x > 0.3) && (x < 0.4);
    double y = std::sin(x) * std::cos(x);
    if (heavy) {
        for (int k = 0; k < 10; ++k) {
            y = std::sqrt(std::abs(y) + 1.0);
        }
    }
    return y + (x * x);
}

double integrate_serial(double a, double b, long n)
{
    const double h = (b - a) / static_cast<double>(n);
    double sum = 0.5 * (f(a) + f(b));
    for (long i = 1; i < n; ++i) {
        const double x = a + (static_cast<double>(i) * h);
        sum += f(x);
    }
    return sum * h;
}
