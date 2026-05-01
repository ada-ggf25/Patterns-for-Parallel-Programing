#include <omp.h>
#include <string>
#include <vector>

// Fill `names[tid]` with a greeting for each thread in a parallel region.
// Returns the total number of threads the team contained.
//
// Deliberately written to exercise fork-join, thread ID queries, and team
// size queries without printing — the test harness then asserts on `names`.
int populate_thread_greetings(std::vector<std::string>& names)
{
    int team_size = 0;

#pragma omp parallel default(none) shared(names, team_size)
    {
        const int tid = omp_get_thread_num();
        const int n = omp_get_num_threads();

#pragma omp single
        {
            names.assign(n, {});
            team_size = n;
        }
        // implicit barrier at `single` exit — safe to index now

        names[tid] = "hello from " + std::to_string(tid);
    }

    return team_size;
}
