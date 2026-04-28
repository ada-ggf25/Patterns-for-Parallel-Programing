#include <cstddef>
#include <omp.h>
#include <vector>

// Demonstrates NUMA first-touch policy: a page is placed on the NUMA node
// of the thread that writes to it *first*, not the thread that allocates.
//
// `init_serial`     — only the master thread writes during init, so all
//                     pages land on one NUMA node. Later parallel access
//                     pays cross-node traffic for threads on other sockets.
// `init_first_touch`— each thread initialises the region it will later
//                     read, so pages are distributed across NUMA nodes.
//
// We cannot directly verify placement from a portable userspace test; the
// day-4 slides show timings side-by-side on Rome. Here we just sanity-check
// that both produce identical contents.

void init_serial(std::vector<double>& v)
{
    for (std::size_t i = 0; i < v.size(); ++i) {
        v[i] = static_cast<double>(i);
    }
}

// snippet-begin: parallel_init
void init_first_touch(std::vector<double>& v)
{
#pragma omp parallel for default(none) shared(v)
    for (std::size_t i = 0; i < v.size(); ++i) {
        v[i] = static_cast<double>(i);
    }
}
// snippet-end: parallel_init

// snippet-begin: consumer
double sum_parallel(const std::vector<double>& v)
{
    double s = 0.0;
#pragma omp parallel for default(none) shared(v) reduction(+ : s)
    for (std::size_t i = 0; i < v.size(); ++i) {
        s += v[i];
    }
    return s;
}
// snippet-end: consumer
