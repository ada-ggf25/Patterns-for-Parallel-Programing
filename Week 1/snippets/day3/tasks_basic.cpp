// Minimal OpenMP-task example: post-order sum of a binary tree.
//
// A tree has no flat iteration space, so `parallel for` can't reach
// it — that's exactly the gap tasks fill. Each subtree is independent
// work; recursion fans tasks out, the runtime spreads them across the
// team. Real-world relatives: quicksort, mergesort, Barnes-Hut, sparse
// Cholesky.

#include <omp.h>

struct Node {
    long  value;
    Node* left;
    Node* right;
};

// snippet-begin: tree_sum
long tree_sum(const Node* p)
{
    if (p == nullptr) {
        return 0;
    }
    long left_sum  = 0;
    long right_sum = 0;

    // shared() overrides firstprivate-by-default — child writes are
    // visible to the parent's locals.
#pragma omp task shared(left_sum)
    left_sum  = tree_sum(p->left);
#pragma omp task shared(right_sum)
    right_sum = tree_sum(p->right);

    // taskwait blocks until direct children finish (use `taskgroup`
    // for transitive waits over grandchildren too).
#pragma omp taskwait

    return p->value + left_sum + right_sum;
}
// snippet-end: tree_sum

// Driver: one thread seeds the recursion via `parallel + single`;
// the rest of the team pick up child tasks from the queue as the
// recursion fans out.
long tree_sum_parallel(const Node* root)
{
    long total = 0;
#pragma omp parallel default(none) shared(total) firstprivate(root)
    {
#pragma omp single
        total = tree_sum(root);
    }
    return total;
}
