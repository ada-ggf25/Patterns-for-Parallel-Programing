# Topology Inspection

To know what hardware your job actually got, run topology tools **inside the PBS job** — that way the view is restricted to the cpuset PBS allocated, not the whole compute node.

## The four tools

| Command | What you see |
|---|---|
| `lscpu` | CPU model, sockets, cores-per-socket, cache sizes, summary NUMA line |
| `numactl --hardware` | NUMA nodes, CPUs per node, memory per node, **distance matrix** |
| `lstopo-no-graphics --no-io` | Full hierarchy: package → NUMA node → L3 → L2/L1 → core → PU |
| `hwloc-info` | One-line summary (counts of each level) |

`lstopo-no-graphics` without `--no-io` also adds PCIe devices (useful if you care about which socket the InfiniBand HCA sits on).

## Where these tools come from

- `lscpu` is part of `util-linux` and is always on `PATH`.
- `numactl` lives in the `numactl` module.
- `lstopo` and `hwloc-info` live in the `hwloc` module.

Crucially, on CX3 EasyBuild loads `numactl` and `hwloc` **as dependencies of the `OpenMPI` module**. So even for a pure-OpenMP job you should:

```bash
ml tools/prod
ml GCC OpenMPI         # OpenMPI brings hwloc + numactl onto PATH
```

Loading just `GCC` is not enough. If you swap modules inside an existing shell, run `hash -r` — bash caches "command not found" lookups and will keep insisting `lstopo` doesn't exist.

## Reference: the OpenMP PBS script

The reference `examples/openmp/pi_openmp.pbs` runs all three commands before the binary, so the `.o<jobid>` log opens with a full topology block:

```bash
echo "=== lscpu (summary) ==="
lscpu | sed -n '1,25p'
echo
echo "=== numactl --hardware (NUMA view + distance matrix) ==="
numactl --hardware
echo
echo "=== lstopo-no-graphics (full hierarchy) ==="
lstopo-no-graphics --no-io
```

This pattern is a reliable habit for any benchmarking job — you get a record of the hardware alongside the timing.

## Reading the output

- Confirm the CPU model matches what you asked for via `cpu_type` ([[../pbs/cpu_type Selection]]).
- Note which cores PBS gave you. They may not be contiguous — see the "reality check" in [[AMD Rome Architecture]].
- The NUMA distance matrix tells you the SLIT-rated cost of cross-NUMA access; for measured numbers see [[NUMA Latency]].

## Related

- [[AMD Rome Architecture]] — what you're inspecting on Rome nodes.
- [[../modules/Loading Combos]] — module combos that pull these tools in.
- [[NUMA Latency]] — the corresponding measured numbers.
