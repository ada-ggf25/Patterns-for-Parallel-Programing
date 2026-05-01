// Assignment 2 — shared declarations for the two-variant Mandelbrot kernel.
#pragma once

#include <cstddef>

// Per-tile cost varies substantially: the tile-grid straddles the
// Mandelbrot-set boundary, where divergence tests take wildly different
// iteration counts. The image region is chosen so that neither variant
// is overwhelmingly favoured — your CHOICE.md is graded on whether your
// recommendation matches your own measured evidence (see CHOICE.md
// template), not on a predetermined right answer.
//
// **Symmetry**: the Mandelbrot set is symmetric about the real axis
// (mandel(c) = mandel(c̄)), so we iterate only the upper half of the
// imaginary axis (j ∈ [0, NPOINTS/2)) and double the count. This is
// ~2× cheaper than the naive doubly-nested implementation. The serial
// reference and both student variants follow this convention; your
// `expected_output.txt` is generated against it.
//
// Grid is NPOINTS × NPOINTS; area estimated by counting escaped points.
constexpr int NPOINTS = 5000;
constexpr int MAXITER = 1000;
constexpr int TILE = 100; // 50 × 25 upper-half tile grid

// Reference serial — do not modify. Defined in mandelbrot_serial.cpp.
long mandelbrot_serial();

// Student-authored, two variants. Both MUST return the same count.
// - mandelbrot_parallel_for: use #pragma omp parallel for (with collapse if wanted).
// - mandelbrot_tasks:       use #pragma omp task or #pragma omp taskloop.
long mandelbrot_parallel_for();
long mandelbrot_tasks();
