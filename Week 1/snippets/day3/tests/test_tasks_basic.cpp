#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <omp.h>

struct Node {
    long  value;
    Node* left;
    Node* right;
};

long tree_sum_parallel(const Node*);

TEST_CASE("tree_sum_parallel returns the post-order tree sum")
{
    // Balanced tree of depth 3, values 1..7. Sum = 28.
    Node n4{4, nullptr, nullptr};
    Node n5{5, nullptr, nullptr};
    Node n6{6, nullptr, nullptr};
    Node n7{7, nullptr, nullptr};
    Node n2{2, &n4, &n5};
    Node n3{3, &n6, &n7};
    Node n1{1, &n2, &n3};

    for (int p : {1, 2, 4}) {
        omp_set_num_threads(p);
        CHECK(tree_sum_parallel(&n1) == 28);
    }
}
