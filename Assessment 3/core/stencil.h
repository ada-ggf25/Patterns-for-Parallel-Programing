// A3 — shared declarations for the 3D Jacobi stencil.
#pragma once

#include <cstddef>

#ifdef CI_BUILD
// CI grid: TSan adds ~8× memory + ~10× slowdown; the full Rome size
// doesn't fit on a GitHub-hosted runner (16 GB RAM, 4 vCPU, 25-min job
// timeout). Race detection is grid-size invariant — the parallel
// constructs are the same. Correctness on the full grid is verified by
// the canonical Rome run, not the CI.
constexpr std::size_t NX = 64;
constexpr std::size_t NY = 64;
constexpr std::size_t NZ = 64;
constexpr int NSTEPS = 5;
#else
// Grid sized so the working set (2 × NX*NY*NZ × 8B = 2.1 GB) is solidly
// larger than Rome's aggregate L3 (256 MB), so the kernel stays
// DRAM-bandwidth-bound even at 128 threads with first-touch. Calibration
// at NX=256 (working set 256 MB ≈ aggregate L3) showed the kernel goes
// cache-resident — the swap pattern keeps the read source hot in L3 and
// effective bandwidth exceeds STREAM, breaking the memory-bound framing.
// At NX=512 the working set forces real DRAM streaming.
constexpr std::size_t NX = 512;
constexpr std::size_t NY = 512;
constexpr std::size_t NZ = 512;
constexpr int NSTEPS = 100;
#endif

// Student-authored stencil step. Reads from `u`, writes to `u_next`.
void jacobi_step(const double* u, double* u_next);

// Reference checksum — used by the grader to verify the final field.
double checksum(const double* u);
