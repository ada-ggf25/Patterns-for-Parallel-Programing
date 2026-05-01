// Standalone race-condition demo.
//
// Compile and run with:
//   clang++ -fopenmp -fsanitize=thread -g -O1 race_demo_standalone.cpp -o race_demo
//   ./race_demo
//
// ThreadSanitizer reports a data race on `counter`: the increment is an
// unsynchronised read-modify-write (RMW) and `counter` defaults to `shared`
// because the parallel region has no `default(none)` clause. The printed
// counter value will usually be less than 100000.

#include <iostream>
#include <omp.h>

int main()
{
    int counter = 0;

#pragma omp parallel for
    for (int i = 0; i < 100000; ++i) {
        ++counter;  // race: shared `counter`, no synchronisation
    }

    std::cout << "counter = " << counter << " (expected 100000)\n";
    return 0;
}
