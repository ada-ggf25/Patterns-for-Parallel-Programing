// Roofline ceiling helper. ceiling = min(peak, OI × bandwidth).

struct RooflineEstimate {
    double ceiling_gflops;
    bool compute_bound;
};

// snippet-begin: ceiling_function
RooflineEstimate roofline_ceiling(double operational_intensity,
                                  double peak_gflops,
                                  double bandwidth_gbs)
{
    const double bw_ceiling = operational_intensity * bandwidth_gbs;
    if (bw_ceiling < peak_gflops) {
        return {bw_ceiling, false};  // memory-bound
    }
    return {peak_gflops, true};      // compute-bound
}
// snippet-end: ceiling_function

// Pre-baked Rome ceiling: peak 4608 GFLOPs, STREAM 246.2 GB/s
// (hardware ceiling — one thread per CCX). 128-thread full-node
// is 231.5; 64-thread one-socket is 116.0. See docs/rome-inventory.md.
RooflineEstimate rome_ceiling_for(double operational_intensity)
{
    return roofline_ceiling(operational_intensity,
                            /*peak_gflops=*/4608.0,
                            /*bandwidth_gbs=*/246.2);
}
