// Explicit OpenMP locks (`omp_lock_t`) give you a separate, named,
// lifecycle-managed mutex per data structure instance. Reach for them
// when `critical` is too coarse — `critical` is one mutex per source
// name, so you can't get per-record / per-bucket / per-node
// granularity out of it.
//
// In all other cases prefer `atomic` (single op) or `reduction`
// (combinable updates).
//
// Lifecycle: `omp_init_lock` ... { `omp_set_lock` / `omp_unset_lock`
// in the hot region } ... `omp_destroy_lock`. Locks are *not* RAII,
// so missed paths can leak; wrap `set` / `unset` in a small RAII
// guard in production code.

#include <cstddef>
#include <numeric>
#include <omp.h>
#include <vector>

// snippet-begin: bucketed_count
long bucketed_count(std::size_t n_buckets, int events_per_thread)
{
    std::vector<long>       buckets(n_buckets, 0);
    std::vector<omp_lock_t> locks(n_buckets);
    for (auto& lk : locks) {
        omp_init_lock(&lk);
    }

#pragma omp parallel default(none) \
        shared(buckets, locks, n_buckets) firstprivate(events_per_thread)
    {
        const int tid = omp_get_thread_num();
        for (int i = 0; i < events_per_thread; ++i) {
            const std::size_t b = (tid + i) % n_buckets;
            omp_set_lock(&locks[b]);     // acquire (blocks)
            ++buckets[b];                // protected update
            omp_unset_lock(&locks[b]);   // release
        }
    }

    for (auto& lk : locks) {
        omp_destroy_lock(&lk);
    }
    return std::accumulate(buckets.begin(), buckets.end(), 0L);
}
// snippet-end: bucketed_count
