// Explicit OpenMP locks (`omp_lock_t`) give you a named, lifecycle-
// managed mutex. Reach for them when:
//
//   - You need a lock owned by a specific data structure (e.g. one
//     lock per bucket of a concurrent hash map).
//   - You need fine-grained locking that `critical` can't express
//     (`critical` is a single global mutex per *name*; named criticals
//     help but the granularity is still tied to source code, not data).
//
// In all other cases prefer `atomic` (single op) or `reduction`
// (combinable updates).

#include <cstddef>
#include <omp.h>
#include <vector>

// One lock per bucket — independent updates can proceed in parallel
// against different buckets.
class LockedBuckets {
   public:
    // snippet-begin: class_lifecycle
    explicit LockedBuckets(std::size_t n)
        : buckets_(n, 0), locks_(n)
    {
        for (auto& lk : locks_) {
            omp_init_lock(&lk);
        }
    }
    ~LockedBuckets()
    {
        for (auto& lk : locks_) {
            omp_destroy_lock(&lk);
        }
    }
    // snippet-end: class_lifecycle

    LockedBuckets(const LockedBuckets&) = delete;
    LockedBuckets& operator=(const LockedBuckets&) = delete;
    LockedBuckets(LockedBuckets&&) = delete;
    LockedBuckets& operator=(LockedBuckets&&) = delete;

    // snippet-begin: class_increment
    void increment(std::size_t bucket)
    {
        omp_set_lock(&locks_[bucket]);
        ++buckets_[bucket];
        omp_unset_lock(&locks_[bucket]);
    }
    // snippet-end: class_increment

    long total() const
    {
        long s = 0;
        for (long b : buckets_) {
            s += b;
        }
        return s;
    }

   private:
    std::vector<long> buckets_;
    std::vector<omp_lock_t> locks_;
};

// snippet-begin: driver
long bucketed_count(std::size_t n_buckets, int events_per_thread)
{
    LockedBuckets store(n_buckets);
#pragma omp parallel default(none) shared(store, n_buckets) firstprivate(events_per_thread)
    {
        const int tid = omp_get_thread_num();
        for (int i = 0; i < events_per_thread; ++i) {
            const std::size_t bucket = static_cast<std::size_t>(tid + i) % n_buckets;
            store.increment(bucket);
        }
    }
    return store.total();
}
// snippet-end: driver
