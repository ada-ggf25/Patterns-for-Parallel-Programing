#!/usr/bin/env python3
"""Render the false-sharing cache-line picture for day 4.

Two side-by-side cache-line diagrams:
  * BAD: 8 × 8-byte accumulators packed onto one 64-byte line. Eight
         threads write here; every write invalidates the other seven
         threads' copies of the line.
  * GOOD: each accumulator is alignas(64), so each thread owns a full
          line. No invalidation traffic.

Output: slides/images/false_sharing_cache_lines.png
"""

from __future__ import annotations

import pathlib

import matplotlib.pyplot as plt
from matplotlib.patches import FancyArrowPatch, Rectangle

ROOT = pathlib.Path(__file__).resolve().parents[2]
OUT = ROOT / "slides" / "images" / "false_sharing_cache_lines.png"

N_THREADS = 4
THREAD_COLOURS = ["#4c72b0", "#dd8452", "#55a467", "#c44e52"]


def draw_line(ax, y0: float, slot_count: int, colours: list[str], label: str) -> None:
    """Draw one 64-byte cache line as `slot_count` adjacent slots."""
    line_width = 8.0
    slot_width = line_width / slot_count
    # Outer rectangle (the cache line).
    ax.add_patch(
        Rectangle(
            (0, y0),
            line_width,
            1.0,
            facecolor="none",
            edgecolor="black",
            linewidth=1.5,
        )
    )
    # Per-slot fills.
    for s in range(slot_count):
        ax.add_patch(
            Rectangle(
                (s * slot_width, y0),
                slot_width,
                1.0,
                facecolor=colours[s % len(colours)],
                edgecolor="white",
                linewidth=0.5,
            )
        )
    ax.text(-0.4, y0 + 0.5, label, ha="right", va="center", fontsize=10)
    ax.text(
        line_width + 0.4, y0 + 0.5, "← 64 B cache line →",
        ha="left", va="center", fontsize=8, color="gray", style="italic",
    )


fig, (ax_bad, ax_good) = plt.subplots(2, 1, figsize=(9, 4.4))

# === BAD: 4 threads share one 64-byte cache line ===
ax_bad.set_xlim(-3.2, 11.2)
ax_bad.set_ylim(-0.4, 2.2)
ax_bad.axis("off")
draw_line(
    ax_bad, 0.5, N_THREADS,
    THREAD_COLOURS,
    "Packed\n8 B per slot",
)
# Arrows: every thread writes to the SAME line.
for tid in range(N_THREADS):
    slot_x = (tid + 0.5) * (8.0 / N_THREADS)
    ax_bad.annotate(
        f"thread {tid}\nwrites",
        xy=(slot_x, 1.5),
        xytext=(slot_x, 2.0),
        ha="center",
        fontsize=8,
        color=THREAD_COLOURS[tid],
        arrowprops=dict(arrowstyle="->", color=THREAD_COLOURS[tid], lw=1),
    )
ax_bad.set_title(
    "BAD: 4 threads share one cache line — every write invalidates the "
    "other three threads' copies",
    fontsize=10,
)

# === GOOD: 4 lines, one per thread (alignas(64)) ===
ax_good.set_xlim(-3.2, 11.2)
ax_good.set_ylim(-0.4, 5.0)
ax_good.axis("off")
for tid in range(N_THREADS):
    y = 0.5 + tid * 1.05
    draw_line(
        ax_good, y, 1, [THREAD_COLOURS[tid]],
        f"alignas(64)\nthread {tid}",
    )
    ax_good.annotate(
        f"thread {tid}\nwrites",
        xy=(8.4, y + 0.5),
        xytext=(10.2, y + 0.5),
        ha="left", va="center",
        fontsize=8,
        color=THREAD_COLOURS[tid],
        arrowprops=dict(arrowstyle="<-", color=THREAD_COLOURS[tid], lw=1),
    )
ax_good.set_title(
    "GOOD: each thread's accumulator on its own cache line — "
    "no invalidation traffic",
    fontsize=10,
)

plt.tight_layout()
OUT.parent.mkdir(parents=True, exist_ok=True)
plt.savefig(OUT, dpi=150, bbox_inches="tight")
print(f"wrote {OUT.relative_to(ROOT)}")
