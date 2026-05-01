#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <omp.h>
#include <string>
#include <vector>

int populate_thread_greetings(std::vector<std::string>& names);

TEST_CASE("parallel region populates every thread's slot")
{
    for (int p : {1, 2, 4}) {
        omp_set_num_threads(p);
        std::vector<std::string> names;
        const int n = populate_thread_greetings(names);
        CHECK(n == p);
        CHECK(static_cast<int>(names.size()) == p);
        for (int i = 0; i < p; ++i) {
            CHECK(names[i] == "hello from " + std::to_string(i));
        }
    }
}
