#!/usr/bin/env python3
"""Render the schedule-distribution diagram for the day-2 schedule slide.

Shows how 100 iterations of a spike workload distribute across 4 threads
under static, dynamic(64), and guided schedules. The y-axis is iteration
index; each cell is colour-coded by the assigned thread; spike iterations
(every 10th) are marked with a hatch.

Output: slides/images/schedule_distribution.png
"""

from __future__ import annotations

import pathlib

import matplotlib.pyplot as plt
import numpy as np
from matplotlib.patches import Patch

ROOT = pathlib.Path(__file__).resolve().parents[2]
OUT = ROOT / "slides" / "images" / "schedule_distribution.png"

N_ITERS = 100
N_THREADS = 4
SPIKE_EVERY = 10

THREAD_COLOURS = ["#4c72b0", "#dd8452", "#55a467", "#c44e52"]


def assign_static(n_iters: int, n_threads: int) -> np.ndarray:
    """Default static: contiguous chunks of n/p iterations per thread."""
    out = np.zeros(n_iters, dtype=int)
    chunk = n_iters // n_threads
    for tid in range(n_threads):
        start = tid * chunk
        end = (tid + 1) * chunk if tid < n_threads - 1 else n_iters
        out[start:end] = tid
    return out


def assign_dynamic(n_iters: int, n_threads: int, chunk: int = 8) -> np.ndarray:
    """Dynamic: hand out chunks on demand. Threads finishing fast pick up
    the next chunk. We simulate by making thread 0 finish each chunk
    fastest unless it has a spike (in which case the next thread grabs).
    For visualisation purposes we round-robin chunks weighted by the
    relative cost of each chunk."""
    out = np.zeros(n_iters, dtype=int)
    chunks = [(i, min(i + chunk, n_iters)) for i in range(0, n_iters, chunk)]
    # Track per-thread accumulated cost (in arbitrary units).
    thread_load = [0] * n_threads
    for start, end in chunks:
        # Cost of this chunk: 10 per cheap iter + 100 per spike.
        cost = sum(100 if i % SPIKE_EVERY == 0 else 10 for i in range(start, end))
        # Assign to least-loaded thread.
        tid = thread_load.index(min(thread_load))
        out[start:end] = tid
        thread_load[tid] += cost
    return out


def assign_guided(n_iters: int, n_threads: int) -> np.ndarray:
    """Guided: chunk size shrinks as we approach the end of the loop.
    The OpenMP spec: chunk = max(min_chunk, remaining / n_threads).
    Chunks are then handed out in round-robin to the threads."""
    out = np.zeros(n_iters, dtype=int)
    i = 0
    tid_cycle = 0
    while i < n_iters:
        remaining = n_iters - i
        chunk = max(1, remaining // n_threads)
        out[i : i + chunk] = tid_cycle % n_threads
        i += chunk
        tid_cycle += 1
    return out


def render_subplot(ax, assignment: np.ndarray, title: str) -> None:
    n_iters = len(assignment)
    # 2D grid: each iteration as a thin vertical bar.
    for i in range(n_iters):
        tid = assignment[i]
        ax.add_patch(
            plt.Rectangle(
                (i, 0), 1, 1, facecolor=THREAD_COLOURS[tid], edgecolor="white",
                linewidth=0.3,
            )
        )
        # Mark spikes with a black notch on top.
        if i % SPIKE_EVERY == 0:
            ax.add_patch(
                plt.Rectangle(
                    (i, 0.85), 1, 0.15, facecolor="black", edgecolor="none"
                )
            )
    ax.set_xlim(0, n_iters)
    ax.set_ylim(0, 1)
    ax.set_yticks([])
    ax.set_xlabel("iteration index")
    ax.set_title(title, fontsize=11)
    ax.set_aspect("auto")


fig, axes = plt.subplots(3, 1, figsize=(10, 4.5), sharex=True)
render_subplot(
    axes[0], assign_static(N_ITERS, N_THREADS), "schedule(static) — contiguous chunks"
)
render_subplot(
    axes[1], assign_dynamic(N_ITERS, N_THREADS), "schedule(dynamic, chunk) — work-stealing"
)
render_subplot(
    axes[2], assign_guided(N_ITERS, N_THREADS), "schedule(guided) — shrinking chunks"
)

# Shared legend.
thread_handles = [
    Patch(facecolor=THREAD_COLOURS[t], edgecolor="white", label=f"thread {t}")
    for t in range(N_THREADS)
]
spike_handle = Patch(facecolor="black", edgecolor="none", label="spike (10× cost)")
fig.legend(
    handles=thread_handles + [spike_handle],
    loc="lower center",
    ncols=N_THREADS + 1,
    frameon=False,
    fontsize=9,
    bbox_to_anchor=(0.5, -0.02),
)

fig.suptitle(
    f"Iteration → thread under three schedules ({N_ITERS} iters, {N_THREADS} threads, "
    "spike every 10)",
    fontsize=11,
)
plt.tight_layout(rect=(0.0, 0.04, 1.0, 0.96))
OUT.parent.mkdir(parents=True, exist_ok=True)
plt.savefig(OUT, dpi=150, bbox_inches="tight")
print(f"wrote {OUT.relative_to(ROOT)}")
