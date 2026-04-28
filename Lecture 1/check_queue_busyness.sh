#!/usr/bin/env bash
# check_queue_busyness.sh — CPU-queue busyness snapshot for a PBS cluster.
#
# Run directly on the login node (no SSH).
#
# Usage:
#   check_queue_busyness.sh               # all CPU queues
#   check_queue_busyness.sh --all         # include GPU / Jupyter / admin queues
#
# Scope (default): filtered to queues relevant for a CPU / OpenMP / MPI intro
# course. Excluded: GPU queues (v1_gpu72, v1_a100, v1_jupytergpu),
# Jupyter/OnDemand (v1_jupyter, v1_ood),
# admin / ACL'd queues (system_queue, carbon, v1_limited),
# private reservations (R*).

set -euo pipefail

SHOW_ALL=false

while [[ $# -gt 0 ]]; do
  case "$1" in
    --all)  SHOW_ALL=true; shift ;;
    -h|--help) sed -n '2,16p' "$0"; exit 0 ;;
    *) echo "Unknown arg: $1" >&2; exit 2 ;;
  esac
done

if ! command -v qstat &>/dev/null; then
    echo "Error: qstat not found. This script must run on a PBS login node." >&2
    exit 1
fi

# Queue-name prefixes that matter for a CPU / OpenMP / MPI course.
QUEUE_RE='^(cx|hx|v1_(small|medium|large|capability|interactive|4nodes))'

echo "============================================================"
echo " PBS Queue Busyness — $(hostname) — $(date '+%Y-%m-%d %H:%M:%S')"
echo "============================================================"
echo

# --- Per-queue job counts ---
echo "--- Queue Summary ---"
if $SHOW_ALL; then
    qstat -Q 2>&1
else
    qstat -Q 2>&1 | awk -v re="$QUEUE_RE" '
        /^Queue[ ]/ || /^---/ { print; next }
        $1 ~ re               { print }
    '
fi

echo
echo "--- CPU Node Utilisation ---"
pbsnodes -av 2>/dev/null | awk '
  function flush() {
    if (node != "" && (ngpus+0) == 0) counts[state]++
  }
  BEGIN { node=""; state="unknown"; ngpus=0 }
  /^[^ \t]/ { flush(); node=$1; state="unknown"; ngpus=0 }
  /^ *state =/                    { state=$3 }
  /resources_available\.ngpus =/  { ngpus=$3 }
  END {
    flush()
    tot=0
    printf "  %-20s %s\n", "State", "Nodes"
    printf "  %-20s %s\n", "--------------------", "-----"
    for (s in counts) { printf "  %-20s %d\n", s, counts[s]; tot+=counts[s] }
    printf "  %-20s %d\n", "TOTAL (CPU nodes)", tot
  }
'

echo
echo "--- Job Summary ---"
qstat -t 2>/dev/null | awk '
    NR <= 2 { next }
    { counts[$5]++ ; total++ }
    END {
        printf "  %-10s %s\n", "State", "Jobs"
        printf "  %-10s %s\n", "----------", "----"
        for (s in counts) printf "  %-10s %d\n", s, counts[s]
        printf "  %-10s %d\n", "TOTAL", total
    }
'
