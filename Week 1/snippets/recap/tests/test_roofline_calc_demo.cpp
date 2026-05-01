#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

struct RooflineEstimate {
    double ceiling_gflops;
    bool compute_bound;
};

RooflineEstimate roofline_ceiling(double, double, double);
RooflineEstimate rome_ceiling_for(double);

TEST_CASE("memory-bound regime returns OI x BW")
{
    // OI = 0.14 FLOPs/byte; BW = 100 GB/s; peak = 1000 GFLOPs.
    // 0.14 * 100 = 14 < 1000, so ceiling = 14 (bandwidth-bound).
    const auto r = roofline_ceiling(0.14, 1000.0, 100.0);
    CHECK(r.ceiling_gflops == doctest::Approx(14.0));
    CHECK_FALSE(r.compute_bound);
}

TEST_CASE("compute-bound regime returns peak")
{
    // OI = 50; BW = 100 → 5000 > 1000 peak, so ceiling = 1000 (compute-bound).
    const auto r = roofline_ceiling(50.0, 1000.0, 100.0);
    CHECK(r.ceiling_gflops == doctest::Approx(1000.0));
    CHECK(r.compute_bound);
}

TEST_CASE("Rome A3-Jacobi-class kernel is memory-bound at OI=0.14")
{
    const auto r = rome_ceiling_for(0.14);
    CHECK(r.ceiling_gflops == doctest::Approx(0.14 * 246.2));
    CHECK_FALSE(r.compute_bound);
}

TEST_CASE("Rome A1-integration-class kernel is compute-bound at OI=100")
{
    // Ridge OI for Rome is ~18.7 (= 4608 / 246.2). High-OI kernels above
    // that ridge are compute-bound and the ceiling caps at peak FLOPs.
    const auto r = rome_ceiling_for(100.0);
    CHECK(r.ceiling_gflops == doctest::Approx(4608.0));
    CHECK(r.compute_bound);
}
