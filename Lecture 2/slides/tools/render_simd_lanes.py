#!/usr/bin/env python3
"""Render the AVX2 SIMD register diagram for day 4.

Shows a 256-bit AVX2 register holding 4 double-precision lanes. One
SIMD instruction operates on all 4 doubles in parallel.

Output: slides/images/simd_lanes_avx2.png
"""

from __future__ import annotations

import pathlib

import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle

ROOT = pathlib.Path(__file__).resolve().parents[2]
OUT = ROOT / "slides" / "images" / "simd_lanes_avx2.png"

LANE_COLOURS = ["#4c72b0", "#dd8452", "#55a467", "#c44e52"]
LANE_WIDTH = 2.0   # arbitrary units
LANE_HEIGHT = 1.0
N_LANES = 4

fig, ax = plt.subplots(figsize=(9, 3.4))
ax.set_xlim(-1.6, N_LANES * LANE_WIDTH + 1.6)
ax.set_ylim(-1.4, 2.6)
ax.axis("off")

# === Register A: a[i..i+3] ===
y_a = 1.4
for lane in range(N_LANES):
    x = lane * LANE_WIDTH
    ax.add_patch(
        Rectangle(
            (x, y_a),
            LANE_WIDTH,
            LANE_HEIGHT,
            facecolor=LANE_COLOURS[lane],
            edgecolor="black",
            linewidth=1.0,
        )
    )
    ax.text(
        x + LANE_WIDTH / 2,
        y_a + LANE_HEIGHT / 2,
        f"a[i+{lane}]",
        ha="center", va="center",
        color="white", fontsize=11, fontweight="bold",
    )
ax.text(-0.2, y_a + LANE_HEIGHT / 2, "ymm0", ha="right", va="center", fontsize=10)
ax.text(N_LANES * LANE_WIDTH + 0.2, y_a + LANE_HEIGHT / 2,
        "256 bits = 4 × 64-bit doubles", ha="left", va="center",
        fontsize=8, style="italic", color="gray")

# === Register B: b[i..i+3] ===
y_b = 0.0
for lane in range(N_LANES):
    x = lane * LANE_WIDTH
    ax.add_patch(
        Rectangle(
            (x, y_b),
            LANE_WIDTH,
            LANE_HEIGHT,
            facecolor="#cccccc",
            edgecolor="black",
            linewidth=1.0,
        )
    )
    ax.text(
        x + LANE_WIDTH / 2,
        y_b + LANE_HEIGHT / 2,
        f"b[i+{lane}]",
        ha="center", va="center",
        color="black", fontsize=11,
    )
ax.text(-0.2, y_b + LANE_HEIGHT / 2, "ymm1", ha="right", va="center", fontsize=10)

# Annotation: vaddpd
ax.annotate(
    "  one vaddpd instruction\n  → 4 add operations in parallel",
    xy=(N_LANES * LANE_WIDTH / 2, y_b - 0.15),
    xytext=(N_LANES * LANE_WIDTH / 2, -1.1),
    ha="center",
    fontsize=10,
    color="black",
    arrowprops=dict(arrowstyle="->", color="black", lw=1.0),
)

ax.set_title(
    "AVX2 — one register holds 4 doubles; one SIMD op acts on all 4 lanes",
    fontsize=11,
)

plt.tight_layout()
OUT.parent.mkdir(parents=True, exist_ok=True)
plt.savefig(OUT, dpi=150, bbox_inches="tight")
print(f"wrote {OUT.relative_to(ROOT)}")
