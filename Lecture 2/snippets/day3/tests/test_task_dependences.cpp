#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <omp.h>

int run_pipeline();

TEST_CASE("task DAG produces deterministic result regardless of thread count")
{
    for (int p : {1, 2, 4}) {
        omp_set_num_threads(p);
        CHECK(run_pipeline() == 27);
    }
}
