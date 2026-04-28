# CX3 Rome hardware + toolchain inventory

**Status**: measured. Topology from `cx3-12-27` on 2026-04-24; **STREAM bandwidth measured in three iterations**: v1 single-binding (2026-04-24), v2 binding sweep at 400M arrays (2026-04-26), v3 sweep at **800M arrays + the AMD/VMware one-thread-per-CCX recipe** with both GCC 13.3 and Clang 18 (2026-04-26 on `cx3-12-25`). v3 closes the gap to published EPYC 7742 STREAM benchmarks.

## Results

### Topology

| Quantity | Measured |
|---|---|
| Node model | AMD EPYC 7742 |
| Sockets per node | 2 |
| Physical cores per socket | 64 |
| **Physical cores per node** | **128** |
| Threads per core (SMT) | **1** (SMT off — CX3 Rome is non-SMT) |
| **NUMA domains per node** | **8** (NPS=4 per socket, 16 cores per domain) |
| RAM per NUMA domain | ~128 GB |
| Nominal clock | 2.25 GHz |
| Max clock | 2.25 GHz |

Node-distance matrix: intra-socket distance 12, cross-socket distance 32.

### Canonical thread-count ladder (Rome)

`P ∈ {1, 16, 64, 128}` maps cleanly to the topology:

| P | Resource |
|---|---|
| 1 | serial baseline |
| 16 | one NUMA domain |
| 64 | one socket (4 NUMA domains) |
| 128 | full node (8 NUMA domains) |

The initial STREAM run used `{1, 8, 64, 128}`; future grader benchmarks should use `{1, 16, 64, 128}` so `8` doesn't sit awkwardly inside a NUMA domain.

### STREAM triad bandwidth (canonical, v3)

Measured 2026-04-26 on `cx3-12-25`, both GCC 13.3 and Clang 18:

```
-O3 -march=znver2 -mavx2 -mcmodel=medium -fopenmp
-DSTREAM_ARRAY_SIZE=800000000 -DNTIMES=20
```

(800 M doubles × 3 arrays = 19.2 GB total — matches the
[VMware vSphere reference run](https://blogs.vmware.com/cloud-foundation/2020/09/01/amd-rome-app-performance-on-vsphere-part4-stream-and-java-ee/)).

Swept across `OMP_PLACES ∈ {cores, threads, ll_caches}` and
`OMP_PROC_BIND ∈ {close, spread}`. The table below lists the best
Triad observed at each thread count, with the binding that produced
it. **GCC and Clang are tied to within 0.05 %** — compiler choice is
not the bottleneck.

| Threads | Triad (GB/s) | Copy (GB/s) | Best binding | Notes |
|---|---|---|---|---|
| 1 | **22.0** | 36.2 | close + cores | One core; one DRAM channel's worth |
| 16 | **29.0** | 39.7 | close + cores | One NUMA domain (16 cores) |
| **32** | **246.2** | **337.3** | **spread + cores** | **One thread per CCX — hardware peak** |
| 64 (full node, spread) | 236.5 | 327.0 | spread + cores | 32 threads per socket |
| 64 (one socket) | 116.0 | 158.9 | close + cores | All 64 threads on socket 0 |
| 128 (full node) | 231.5 | 314.0 | spread + cores | All 128 cores active |

**Key finding — "one thread per CCX" beats "all the cores"** by 6 %.
Rome has 32 CCXs (Core Complexes) total, 16 per socket; each CCX has
its own 16 MB L3 cache shared by 4 cores. STREAM is bandwidth-bound,
so once each CCX has one bandwidth-greedy thread the DRAM channels
are saturated; adding a second thread per CCX just contends on L3
cache without buying any more DRAM throughput. With `OMP_PROC_BIND=
spread OMP_PLACES=cores` and 32 threads, libomp spreads them one per
4 cores → exactly one per CCX → hardware ceiling.

Confirmed against the [VMware Cloud Foundation NPS-sweep](https://blogs.vmware.com/cloud-foundation/2020/09/01/amd-rome-app-performance-on-vsphere-part4-stream-and-java-ee/):
their best NPS=4 STREAM Copy on a dual EPYC 7742 = 339 GB/s. We
measure **337.3 GB/s Copy at 32 threads** — within 0.6 %. The earlier
"gap" was an artefact of running 128 threads at full-core saturation
instead of the AMD/VMware-recommended one-per-CCX configuration.

#### Footguns surfaced by the v3 sweep

- **`OMP_PLACES=ll_caches` is broken** on the libomp shipping with
  CX3's Clang 18 + GCC 13.3: enumerates only 24-72 GB/s for any
  thread count instead of the expected 200 + GB/s. We have not yet
  isolated whether the bug is in the runtime or in our `lstopo`'s
  reporting of L3 cache groups; for now, **achieve "one per CCX" via
  `spread + cores` at the right thread count, not via `ll_caches`**.
- **`numactl --interleave=all` halves bandwidth** on this hardware
  (121 GB/s at 128T). It defeats first-touch placement; first-touch
  with `spread+cores` wins.
- **Bigger arrays ≠ more bandwidth past 400 M.** v2 (400 M) and v3
  (800 M) at 128T produce 231.3 vs 231.5 GB/s — within noise. The
  effect is from binding, not array size.

#### Theoretical ceiling

Per-socket: 8 DDR4-3200 channels × 8 B × 3.2 GHz ≈ 204 GB/s. Dual-
socket: ~408 GB/s. Achieved 246.2 / 408 ≈ **60 %** of theoretical at
the hardware ceiling — typical for STREAM on DDR4 systems (50-65 %
is the published efficiency band).

#### v1 / v2 (historical, superseded)

- v1 (2026-04-24, `cx3-12-27`, 100 M arrays, single binding): **228.8
  GB/s** at 128T. Within 1.1 % of v2's same-binding result.
- v2 (2026-04-26, `cx3-15-6`, 400 M arrays, binding sweep but no 32T
  test): **231.3 GB/s** at 128T. Mistook the published 290+ GB/s
  numbers as a tuning gap; actually a thread-count-recipe gap.

Both v1 and v2 are good measurements *of what we tested*, but neither
caught the one-per-CCX peak. Raw outputs for all three runs remain in
`docs/rome-inventory-output/<stamp>/` (local; gitignored).

### HPL — compute-bound counterpart to STREAM

HPL (High-Performance Linpack) is the compute-bound benchmark that
sits at the other corner of the roofline. STREAM measures bandwidth;
HPL measures sustained DP DGEMM throughput. Together they bound the
roofline: STREAM Triad sets the bandwidth ceiling, HPL sets the
*achievable* compute ceiling (typically well below the AVX2 FMA
theoretical peak).

**Setup** (2026-04-26, `cx3-4-13`, job 2566934):

| Parameter | Value |
|---|---|
| Binary | `/rds/easybuild/icelake/apps/software/HPL/2.3-foss-2024a/bin/xhpl` |
| Toolchain | GCC 13.3 + OpenMPI 5.0.3 + OpenBLAS 0.3.27 |
| BLAS | OpenBLAS (Icelake build; not Zen-tuned — AOCL-BLIS would push higher) |
| Layout | 8 MPI ranks × 16 OMP threads = 128 cores; one rank per NUMA domain |
| Process grid | P × Q = 2 × 4 |
| Problem size | N = 80000 (≈ 51 GB matrix); NB = 232 |
| Mapping | `--map-by ppr:1:numa:pe=16` (implicit core-binding from pe=16) |
| OMP environment | `OMP_NUM_THREADS=16 OMP_PROC_BIND=close OMP_PLACES=cores` |

**Result**:

| Metric | Value |
|---|---|
| Wall time | 117.88 s |
| Achieved DP throughput | **2895.7 GFLOPs** |
| Theoretical peak (4608 GFLOPs) | **62.8 %** |
| Residual check | PASSED (≈ 1.71e-03 < 16.0) |

**Interpretation**:

- 62.8 % of theoretical peak is reasonable for a *non-Zen-tuned* HPL
  build. AMD-published HPL on EPYC 7742 with AOCL-BLIS reaches ~75 %;
  the gap is the BLAS choice, not the kernel or the binding.
- The Icelake binary has no AVX-512 path used here (Zen 2 is AVX2
  only), so the Icelake/Zen difference is a tuning gap inside
  OpenBLAS, not an ISA mismatch.
- For roofline teaching purposes, **2896 GFLOPs is a more honest
  "ceiling on real code"** than the 4608 theoretical peak. It is
  what a heavily-optimised, vendor-supplied DGEMM kernel actually
  delivers on this node. Student kernels (A1 integration, A2
  Mandelbrot) will not get within an order of magnitude — but they
  shouldn't, because they aren't DGEMM.

**Why not a Rome-specific build?** CX3 only ships HPL builds for
`/rds/easybuild/icelake/` and `/rds/easybuild/dev/`; there is no
`/rds/easybuild/rome/apps/software/HPL/`, and the
`HPL/2.3-foss-2024a` Lmod entry is not surfaced on Rome compute
nodes. A from-source build with AOCL-BLIS would close most of the
gap to 75 %, but the cost-benefit doesn't justify it for this
course's purposes — we cite both the theoretical peak (4608) and the
HPL-achieved ceiling (2896) and let students see the gap.

#### Cross-check vs published EPYC 7742 HPL (2026-04-26)

Our 2896 GFLOPs (62.8 % of theoretical 4608 at base 2.25 GHz, dual-
socket, OpenBLAS, N=80000) lines up with published HPL on this part:

- **AMD theoretical peak**: 4608 GFLOPs at base 2.25 GHz; ≈ 6963 GFLOPs at max-boost 3.4 GHz. The boost-derived number (cited as "≈ 7 TFLOPs dual-socket, ≈ 3.5 TFLOPs single-socket") is the headline number in marketing material, but Linpack rarely sustains all-core boost — base-clock peak is the honest comparison.
- **WCCFTech / Atos**: the EPYC 7H12 (higher-TDP 7742 sibling, ≈ 11 % faster all-core) reaches ≈ 4.2 TFLOPs Linpack dual-socket. Implied 7742 dual-socket Linpack with vendor-tuned BLIS ≈ 3.78 TFLOPs ≈ 82 % of base-peak (or ≈ 54 % of boost-peak).
- **AMD Zen Software Studio (NETLIB-HPL guidance)**: published configurations target ≈ 75 % of base-peak with AOCL-BLIS at N ≥ 150 000. That's the 75-80 % "tuned-BLAS ceiling" we cite.
- **Puget Systems "HPC parallel performance"** (July 2020): dual-socket 7742, N=200 000, AMD BLIS v2.0; the chart numbers are not transcribed in the text we could fetch but the configuration is consistent with the AMD-published 75 %.

Reading our 62.8 % against this:

- We use **OpenBLAS, not AOCL-BLIS** — published OpenBLAS-on-Zen HPL typically lands 10-15 percentage points below BLIS. Our gap to the AOCL ceiling (75 %) is 12 points → consistent with the BLAS gap, not a measurement error.
- We use **N=80 000, not 150-200 000**. Smaller N raises the panel-factorisation fraction relative to DGEMM and lowers efficiency by ~3-5 percentage points. Constrained on this Imperial node by 8 ranks × 6.4 GB matrix; we could push N up but it wouldn't change the teaching story.
- We use the **Icelake-built binary** running on Zen 2 via AVX2. Same ISA, no AVX-512 overhead, but the OpenBLAS kernel choices were tuned for Sunny Cove cache hierarchy, not Zen's CCX layout.

Verdict: **our number is in line with published OpenBLAS-on-Zen HPL**. To match the 75-80 % of peak that AMD/Atos publish, we would need a from-source HPL with AOCL-BLIS. We don't, and the slides are explicit about this.

Sources (reviewed 2026-04-26, no ID-1):

- [AMD Rome EPYC Processor Shatters Numeric Benchmarks (NextPlatform, 2019)](https://www.nextplatform.com/2019/09/25/amd-rome-epyc-processor-shatters-numeric-benchmarks/)
- [AMD Spack HPL Benchmark guidance](https://www.amd.com/en/developer/zen-software-studio/applications/spack/hpl-benchmark.html)
- [Puget Systems — HPC Parallel Performance for 3rd-gen Threadripper, Xeon 3265W and EPYC 7742](https://www.pugetsystems.com/labs/hpc/hpc-parallel-performance-for-3rd-gen-threadripper-xeon-3265w-and-epyc-7742-hpl-hpcg-numpy-namd-1717/)
- [WCCFTech — Atos AMD EPYC 7742 vs Xeon 8280 HPC benchmarks](https://wccftech.com/amd-hpc-epyc-7742-benchmarks-utterly-annihilates-the-intel-xeon-platinum-8280-at-roughly-1-3-the-price/)
- [Microway — Detailed Specifications of AMD EPYC "Rome" CPUs](https://www.microway.com/knowledge-center-articles/detailed-specifications-of-the-amd-epyc-rome-cpus/)

### Modules available (confirmed 2026-04-24)

On the Rome login node under `tools/prod`:

- `GCC/7.3.0` → `GCC/13.3.0` (and later via gcc/latest)
- `Clang/11.0.1` → **`Clang/18.1.8-GCCcore-13.3.0`** (default) — suitable for on-node TSan + Archer
- `LLVM/10.0.1` → `LLVM/14.0.3-GCCcore-11.3.0` (and later)
- `CMake/3.11.4` → current
- `likwid/5.2.2-GCC-12.3.0` — available for student feedback (not grading)
- `numactl/2.0.13` → `2.0.19-GCCcore-14.2.0`
- `hwloc/2.2.0` → `2.11.2-GCCcore-14.2.0`
- `Advisor/2023.2.0` — Intel tool; not usable on AMD Rome
- `hyperfine` → installable via `module load Rust && cargo install --locked hyperfine`; already present at `~/.cargo/bin/hyperfine`

No `perf` module surfaced in `module avail perf`; assume not available to unprivileged users.

### What this means for the course

- **Grader constants**:
  - `ROME_PHYSICAL_CORES = 128`
  - `ROME_NUMA_DOMAINS = 8`
  - `ROME_CORES_PER_NUMA = 16`
  - `ROME_PEAK_GFLOPS = 128 × 2.25 × 16 = 4608` (AVX2 DP FMA, theoretical)
  - `ROME_HPL_GFLOPS = 2896` (HPL on Icelake/OpenBLAS binary; 62.8 % of theoretical — the achievable compute ceiling)
  - `ROME_STREAM_BW_GBs_PEAK = 246.2` (32 threads, one-per-CCX recipe — hardware ceiling)
  - `ROME_STREAM_BW_GBs_AT_128T = 231.5` (what A3 sees with the canonical PBS env)
  - `ROME_STREAM_BW_SOCKET_GBs = 116.0` (one socket; close+cores)
- **Canonical thread counts update**: `{1, 16, 64, 128}` replaces the earlier `{1, 8, 32, 128}` in the plan.
- **NUMA teaching**: simpler than expected (8 domains not 16), but the cross-socket penalty (distance 32 vs 12) is substantial and worth emphasising for A3.
- **On-node TSan is viable**: Clang 18 + LLVM 18 libomp are available; the A3 evaluate.pbs can run TSan on the Rome node itself if wanted. GH Actions remains the primary TSan gate for A2/A3; CX3 is perf-only.
- **Roofline targets**:
  - A1 integration (compute-bound): theoretical ceiling 4608 GFLOPs; HPL-achievable ceiling 2896 GFLOPs. Student kernels are not DGEMM and won't approach either; the reference-parallel-time metric is the actual scoring path.
  - A3 Jacobi stencil (OI ≈ 0.14): hardware ceiling ≈ 0.14 × 246 ≈ 34 GFLOPs (one per CCX); 0.14 × 231 ≈ 32 GFLOPs at 128 threads full-node. Aim for ≥ 0.5 of whichever ceiling matches your thread count.

### Raw artefacts

v1 (topology + first STREAM, 2026-04-24):
- `rome-cx3-12-27-20260424-203850/lscpu.txt` — full CPU topology
- `rome-cx3-12-27-20260424-203850/numactl-hardware.txt` — NUMA layout + node distances
- `rome-cx3-12-27-20260424-203850/lstopo.txt` — hwloc topology (useful for slides)
- `rome-cx3-12-27-20260424-203850/stream-{1,8,64,128}.txt` — STREAM output per thread count (single binding)
- `rome-cx3-12-27-20260424-203850/modules-loaded.txt` — modules active at measurement time

v2 (tuned STREAM sweep, 2026-04-26):
- `rome-cx3-15-6-20260426-170728/SUMMARY.txt` — best Triad per thread count + binding
- `rome-cx3-15-6-20260426-170728/stream-<P>-bind-<bind>-places-<places>.txt` — one file per swept combination
- `rome-cx3-15-6-20260426-170728/stream-128-numactl-interleave.txt` — `numactl --interleave=all` reference

v3 (32T one-per-CCX + Clang comparison, 2026-04-26):
- `rome-cx3-12-25-20260426-172819/SUMMARY.txt` — best Triad per (thread count × compiler)
- `rome-cx3-12-25-20260426-172819/stream-{gcc,clang}-<P>-bind-<bind>-places-<places>.txt`
- `rome-cx3-12-25-20260426-172819/stream-build-{gcc,clang}.log` — build warnings/messages
- `rome-cx3-12-25-20260426-172819/stream-gcc-128-numactl-interleave.txt`

HPL run (compute-bound ceiling, 2026-04-26):
- `rome-cx3-4-13-20260426-183604-hpl/SUMMARY.txt` — achieved GFLOPs + fraction-of-peak
- `rome-cx3-4-13-20260426-183604-hpl/xhpl.log` — full HPL output incl. residual check
- `rome-cx3-4-13-20260426-183604-hpl/HPL.dat` — input parameters
- `rome-cx3-4-13-20260426-183604-hpl/xhpl-location.txt` — `ldd` + binary path

PBS scripts that produced this: `docs/rome-inventory.pbs` (STREAM) and `docs/rome-hpl.pbs` (HPL). Rerun any time to refresh.

## Who filled this in

| Date | Who | Job ID | Node |
|---|---|---|---|
| 2026-04-24 | Gerard Gorman (via Claude) | 2546897.pbs-7 | cx3-12-27 — topology + STREAM v1 |
| 2026-04-26 | Gerard Gorman (via Claude) | 2566427.pbs-7 | cx3-15-6 — STREAM v2 sweep (400M, 12-combo) |
| 2026-04-26 | Gerard Gorman (via Claude) | 2566765.pbs-7 | cx3-12-25 — STREAM v3 sweep (800M, GCC + Clang, one-per-CCX) |
| 2026-04-26 | Gerard Gorman (via Claude) | 2566934.pbs-7 | cx3-4-13 — HPL N=80000, 2896 GFLOPs (62.8 % of peak) |
