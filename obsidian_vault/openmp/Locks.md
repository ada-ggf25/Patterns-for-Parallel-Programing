# `omp_lock_t` — Explicit Locks

Use explicit locks when you need data-driven granularity — one mutex per data structure instance, not one per source-code region.

## When `critical` isn't enough

`critical` ties granularity to *source code*: named critical sections give you a fixed number of global mutexes. If you have 1024 hash-map buckets, you can't have 1024 named critical sections.

Examples requiring explicit locks:
- One mutex per bucket of a hash map
- One mutex per node of a graph
- One mutex per row of a sparse matrix

Independent operations against different instances proceed in parallel.

## Lifecycle

```cpp
omp_lock_t lk;
omp_init_lock(&lk);        // must be called before use

omp_set_lock(&lk);         // acquire — blocks until lock is free
// ... critical section ...
omp_unset_lock(&lk);       // release

omp_test_lock(&lk);        // non-blocking try; returns true if acquired

omp_destroy_lock(&lk);     // must be called when done
```

## RAII wrapper (recommended)

An exception thrown inside the critical section will skip `omp_unset_lock`, leaking the lock. Wrap it:

```cpp
struct LockGuard {
    omp_lock_t& lk;
    explicit LockGuard(omp_lock_t& l) : lk(l) { omp_set_lock(&lk); }
    ~LockGuard() { omp_unset_lock(&lk); }
};

// Usage:
{
    LockGuard g(lk);           // acquire on construction
    update_protected_data();
}                              // release on scope exit — exception-safe
```

## Example: per-bucket locked hash map

```cpp
struct BucketMap {
    static constexpr int N = 64;
    std::array<omp_lock_t, N> locks;
    std::array<std::vector<int>, N> data;

    BucketMap()  { for (auto& l : locks) omp_init_lock(&l); }
    ~BucketMap() { for (auto& l : locks) omp_destroy_lock(&l); }

    void insert(int key) {
        const int b = key % N;
        omp_set_lock(&locks[b]);
        data[b].push_back(key);
        omp_unset_lock(&locks[b]);
    }
};

#pragma omp parallel for default(none) shared(map, keys, n)
for (size_t i = 0; i < n; ++i) {
    map.insert(keys[i]);   // threads on different buckets run in parallel
}
```

## Nested locks (`omp_nest_lock_t`)

`omp_nest_lock_t` allows the same thread to acquire the lock multiple times. Use only for re-entrant code; prefer `omp_lock_t` + careful design otherwise.

## Related

- [[critical and atomic]] — simpler; always try these first.
- [[Memory Model]] — `omp_set_lock` / `omp_unset_lock` provide implicit flush.
