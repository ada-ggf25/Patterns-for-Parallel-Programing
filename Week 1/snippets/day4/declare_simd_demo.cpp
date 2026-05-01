// `#pragma omp declare simd` annotates a *function* so the compiler
// emits a vectorised version. The vectorised function can then be
// called from inside a `simd` loop and the compiler will pick the
// SIMD form, avoiding a fall-back scalar call per iteration.
//
// Without `declare simd`, calling a function from a SIMD loop forces
// the compiler to either inline (if the function is visible) or
// emit a scalar call gather, defeating vectorisation.
//
// Use `inbranch` / `notinbranch` to control whether the SIMD variant
// is callable from a masked context.

#include <cmath>
#include <cstddef>
#include <omp.h>
#include <vector>

// snippet-begin: function_annotation
// Educational kernel: a transcendental we'd like vectorised inside the
// caller's SIMD loop. Without `declare simd`, the cosine call would
// scalarise the loop.
#pragma omp declare simd notinbranch
double smooth_step(double x)
{
    // Approximate; the point is to prevent the compiler from inlining
    // a known-cheap operation and to let `declare simd` matter.
    return (1.0 - std::cos(x * 3.14159265358979)) * 0.5;
}
// snippet-end: function_annotation

// snippet-begin: consumer
void apply_smooth_step(std::vector<double>& v)
{
    const std::size_t n = v.size();
#pragma omp simd
    for (std::size_t i = 0; i < n; ++i) {
        v[i] = smooth_step(v[i]);
    }
}
// snippet-end: consumer

// Threaded + SIMD. `parallel for simd` distributes iterations across
// threads, then vectorises within each thread's chunk.
void apply_smooth_step_threaded(std::vector<double>& v)
{
    const std::size_t n = v.size();
#pragma omp parallel for simd default(none) shared(v, n)
    for (std::size_t i = 0; i < n; ++i) {
        v[i] = smooth_step(v[i]);
    }
}
