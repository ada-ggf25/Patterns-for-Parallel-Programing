// `collapse(N)` fuses the next N nested loops into a single iteration
// space. This matters for parallel-for when the outer loop is shorter
// than the team size:
//
//   #pragma omp parallel for          // 4 iters at the outer loop;
//   for (int i = 0; i < 4; ++i)       // a 64-thread team is mostly idle.
//     for (int j = 0; j < 1000; ++j) ...
//
//   #pragma omp parallel for collapse(2)   // 4*1000 = 4000 iters total;
//   for (int i = 0; i < 4; ++i)            // distributable evenly.
//     for (int j = 0; j < 1000; ++j) ...
//
// Caveats: collapse only works on *perfectly nested* canonical loops
// (no code between the outer loop body and the inner loop, identical
// trip counts independent of outer index).

#include <cstddef>
#include <omp.h>
#include <vector>

// snippet-begin: naive
double sum_naive(int rows, int cols)
{
    std::vector<double> matrix(static_cast<std::size_t>(rows) * cols);
    for (std::size_t i = 0; i < matrix.size(); ++i) {
        matrix[i] = 1.0;
    }

    double total = 0.0;
#pragma omp parallel for default(none) shared(matrix, rows, cols) reduction(+ : total)
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            total += matrix[(i * cols) + j];
        }
    }
    return total;
}
// snippet-end: naive

// snippet-begin: collapsed
double sum_collapsed(int rows, int cols)
{
    std::vector<double> matrix(static_cast<std::size_t>(rows) * cols);
    for (std::size_t i = 0; i < matrix.size(); ++i) {
        matrix[i] = 1.0;
    }

    double total = 0.0;
#pragma omp parallel for collapse(2) default(none) shared(matrix, rows, cols)              \
    reduction(+ : total)
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            total += matrix[(i * cols) + j];
        }
    }
    return total;
}
// snippet-end: collapsed
