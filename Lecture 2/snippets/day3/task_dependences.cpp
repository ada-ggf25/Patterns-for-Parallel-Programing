// `depend(in:..)` / `depend(out:..)` / `depend(inout:..)` clauses
// build a DAG of sibling tasks. The runtime infers ordering from the
// memory addresses listed in the clauses; tasks with `out` must
// finish before any subsequent `in` on the same address starts.
//
// Below: a four-task pipeline `a → b → c, parallel d`, joined into
// `result`. Sketch of the DAG:
//
//        out:a            in:a, out:b
//        ┌──────┐         ┌──────┐
//        │  A   │ ──────▶ │  B   │ ─┐
//        └──────┘         └──────┘  │
//        out:d                       ▼  in:b, in:d, out:r
//        ┌──────┐                  ┌──────┐
//        │  D   │ ────────────────▶│  R   │
//        └──────┘                  └──────┘
//
// Useful for irregular pipelines that don't fit a `taskloop`.

#include <omp.h>

// snippet-begin: chain
int run_pipeline()
{
    int a = 0;
    int b = 0;
    int d = 0;
    int result = 0;

#pragma omp parallel default(none) shared(a, b, d, result)
    {
#pragma omp single
        {
#pragma omp task depend(out : a)
            {
                a = 10;
            }
#pragma omp task depend(in : a) depend(out : b)
            {
                b = a * 2;
            }
// snippet-end: chain
// snippet-begin: join
#pragma omp task depend(out : d)
            {
                d = 7;
            }
#pragma omp task depend(in : b) depend(in : d) depend(out : result)
            {
                result = b + d;
            }
        }  // implicit barrier waits for the whole DAG
    }
    return result;  // 10 * 2 + 7 = 27
}
// snippet-end: join
