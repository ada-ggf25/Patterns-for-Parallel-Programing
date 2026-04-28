#include <cstddef>
#include <omp.h>
#include <vector>

// snippet-begin: function
// Sum a vector in parallel via reduction(+:sum).
// Used in the A1 slide deck to motivate composite integration.
double reduction_sum(const std::vector<double>& a)
{
    double sum = 0.0;
    const std::size_t n = a.size();

#pragma omp parallel for default(none) shared(a, n) reduction(+ : sum)
    for (std::size_t i = 0; i < n; ++i) {
        sum += a[i];
    }
    return sum;
}
// snippet-end: function
