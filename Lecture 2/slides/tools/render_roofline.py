#!/usr/bin/env python3
"""Render the Rome roofline plot used on the recap deck.

Produces `slides/images/roofline_rome.png`. Run with `make plots`.

Plot shape:
  * Log-log axes: operational intensity (FLOPs/byte) on x, achievable
    GFLOPs on y.
  * Piecewise roof: bandwidth-bound slope `OI × BW` until it hits the
    `peak` ceiling, then horizontal at `peak`.
  * Bandwidth-bound region (left of the ridge) and compute-bound
    region (right of the ridge) are tinted distinct colours.
  * A1/A2 (high-OI, compute-bound) and A3 (OI ≈ 0.14, bandwidth-bound)
    plotted as labelled markers on the roof.

Uses Rome (dual-socket EPYC 7742) constants:
  theoretical peak DP = 4608 GFLOPs (full-node, base 2.25 GHz),
  HPL-achieved DP    = 2896 GFLOPs (foss/2024a + OpenBLAS),
  theoretical BW     = 409.6 GB/s (16 × DDR4-3200 channels × 8 B),
  measured STREAM BW = 246.2 GB/s (one-thread-per-CCX recipe).

Both ceilings are shown on each axis so the gap between physics and
real code is visible on the same plot.
"""

from __future__ import annotations

import pathlib

import matplotlib.pyplot as plt
import numpy as np

ROOT = pathlib.Path(__file__).resolve().parents[2]
OUT = ROOT / "slides" / "images" / "roofline_rome.png"

# Rome — measured constants (see docs/rome-inventory.md).
PEAK = 4608.0    # GFLOPs, double-precision, full node — theoretical
                 # (128 cores × 2.25 GHz × 16 FLOPs/cycle AVX2 FMA).
HPL = 2896.0     # GFLOPs, measured HPL on cx3-4-13 (2026-04-26):
                 # /rds/easybuild/icelake/.../HPL/2.3-foss-2024a/bin/xhpl
                 # at N=80000, NB=232, P×Q=2×4, 8 ranks × 16 threads.
                 # 62.8 % of theoretical — "ceiling on real code".
BW = 246.2       # GB/s, measured STREAM triad ceiling (cx3-12-25, 2026-04-26
                 # v3 sweep, 800M arrays, 32 threads spread+cores =
                 # one thread per CCX — AMD/VMware recommended recipe).
BW_PEAK = 409.6  # GB/s, theoretical memory bandwidth: 2 sockets × 8
                 # DDR4-3200 channels × 8 B = 409.6 GB/s. Real STREAM
                 # tops out at ~60 % of this on DDR4 systems
                 # (controller efficiency, refresh, write-allocate).

# OI sweep range covers stencils (~0.1) up to BLAS-3 land (~100).
oi = np.logspace(-2, 3, 1000)
# Theoretical roofline: physics peaks on both axes.
theory_ceiling = np.minimum(PEAK, oi * BW_PEAK)
# Achievable roofline: HPL on the compute axis, STREAM on the memory axis.
achievable_ceiling = np.minimum(HPL, oi * BW)
ridge = PEAK / BW_PEAK  # theoretical ridge — both ceilings at physics-peak
ridge_meas = HPL / BW   # measured ridge — what real code can hit

fig, ax = plt.subplots(figsize=(9, 5))
ax.set_xscale("log")
ax.set_yscale("log")

# Tinted regions — based on theoretical ridge so the regimes are
# fixed properties of the kernel, not the BLAS choice.
ax.axvspan(
    1e-2, ridge, alpha=0.10, color="tab:blue", label="bandwidth-bound region"
)
ax.axvspan(
    ridge, 1e3, alpha=0.10, color="tab:orange", label="compute-bound region"
)

# Theoretical roofline (physics ceilings on both axes).
ax.plot(
    oi,
    theory_ceiling,
    color="black",
    linewidth=2.0,
    zorder=4,
    label=f"theoretical roof (peak {PEAK:.0f} GFLOPs, BW {BW_PEAK:.0f} GB/s)",
)
# Achievable roofline (HPL ceiling + measured STREAM slope).
ax.plot(
    oi,
    achievable_ceiling,
    color="tab:purple",
    linewidth=1.8,
    linestyle="--",
    zorder=4,
    label=f"achievable roof (HPL {HPL:.0f} GFLOPs, STREAM {BW:.0f} GB/s)",
)

# Reference horizontals — both compute ceilings.
ax.axhline(PEAK, linestyle=":", color="black", alpha=0.4)
ax.text(
    1.5e-2,
    PEAK * 1.12,
    f"theoretical peak {PEAK:.0f} GFLOPs (DP, full node)",
    fontsize=9,
    color="black",
)
ax.axhline(HPL, linestyle=":", color="tab:purple", alpha=0.5)
ax.text(
    1.5e-2,
    HPL * 0.62,
    f"HPL achieved {HPL:.0f} GFLOPs",
    fontsize=9,
    color="tab:purple",
)

# Theoretical ridge (vertical reference at theoretical peak / theoretical BW).
ax.axvline(ridge, linestyle="--", color="grey", alpha=0.7)
ax.text(
    ridge * 1.08,
    0.3,
    f"ridge OI ≈ {ridge:.1f}",
    fontsize=9,
    color="grey",
)

# Kernel markers.
A3_OI = 0.14
A3_PERF = A3_OI * BW
ax.scatter(
    [A3_OI], [A3_PERF], color="tab:red", s=90, zorder=5, edgecolor="black"
)
ax.annotate(
    f"A3 Jacobi\nOI={A3_OI}, ≈ {A3_PERF:.0f} GFLOPs",
    xy=(A3_OI, A3_PERF),
    xytext=(0.02, 200),
    fontsize=9,
    color="tab:red",
    arrowprops=dict(arrowstyle="-", color="tab:red", alpha=0.5),
)

A1_OI = 100.0  # well above ridge; sits on the achievable (HPL) ceiling.
ax.scatter(
    [A1_OI], [HPL], color="tab:green", s=90, zorder=5, edgecolor="black"
)
ax.annotate(
    f"A1 / A2 ceiling\nhigh OI, HPL ≈ {HPL:.0f} GFLOPs",
    xy=(A1_OI, HPL),
    xytext=(120, 250),
    fontsize=9,
    color="tab:green",
    arrowprops=dict(arrowstyle="-", color="tab:green", alpha=0.5),
)

ax.set_xlabel("operational intensity OI [FLOPs / byte]")
ax.set_ylabel("achievable performance [GFLOPs]")
ax.set_title("Rome roofline — dual-socket EPYC 7742, full node")
ax.set_xlim(1e-2, 1e3)
ax.set_ylim(1e-1, PEAK * 2.5)
ax.grid(True, which="both", alpha=0.3)
ax.legend(loc="lower left", fontsize=9)

plt.tight_layout()
OUT.parent.mkdir(parents=True, exist_ok=True)
plt.savefig(OUT, dpi=150)
print(f"wrote {OUT.relative_to(ROOT)}")
