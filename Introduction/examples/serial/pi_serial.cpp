// Serial numerical integration of  pi = integral_0^1  4 / (1 + x^2) dx,
// evaluated at the midpoint of N equal-width sub-intervals.
//
// Build:  cmake --build build --target pi_serial
// Run:    ./build/serial/pi_serial

#include <chrono>
#include <cstdio>

int main() {
    const long long n = 1'000'000'000LL;
    const double w = 1.0 / static_cast<double>(n);
    double sum = 0.0;

    const auto t0 = std::chrono::steady_clock::now();

    for (long long i = 1; i <= n; ++i) {
        const double x = w * (static_cast<double>(i) - 0.5);
        sum += 4.0 / (1.0 + x * x);
    }

    const double pi = w * sum;
    const auto t1 = std::chrono::steady_clock::now();
    const double secs = std::chrono::duration<double>(t1 - t0).count();

    std::printf("n=%lld  pi=%.15f  time=%.3fs\n", n, pi, secs);
    return 0;
}
