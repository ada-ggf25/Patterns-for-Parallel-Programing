# Benchmarking on CX3 Rome

How to submit an A1/A2/A3 perf run to Imperial CX3's Rome queue, get the JSON timings back, and read them.

## Prerequisites

- An Imperial RCS account with HPC access. See <https://icl-rcs-user-guide.readthedocs.io/>.
- SSH into `login.cx3.hpc.ic.ac.uk` (VPN / Zscaler required off-campus).
- Your clone of `ppp-openmp-assessment` on the CX3 filesystem (under `$HOME` or `$EPHEMERAL`; the latter is larger but wipes after 30 days).

## One-time: install hyperfine

Existing assessment PBS scripts use `hyperfine` for robust min-time measurement. Install once via Rust:

```bash
module load Rust
cargo install --locked hyperfine
# Adds to ~/.cargo/bin/hyperfine; your PBS script exports PATH=$HOME/.cargo/bin:$PATH.
```

## Submit A1 to Rome

From the assessment repo root on CX3:

```bash
qsub assignment-1/evaluate.pbs
qstat -u $USER                  # monitor queue
```

The PBS script requests:

```
#PBS -l select=1:ncpus=128:mem=400gb:cpu_type=rome:mpiprocs=1:ompthreads=128
#PBS -l place=excl
```

i.e. a full Rome node (dual-socket EPYC 7742, 128 physical cores, 8 NUMA domains) exclusive to the job. Walltime is 10 minutes by default; adjust if your kernel is slow.

Inside the script, hyperfine runs at thread counts `[1, 8, 64, 128]` and writes one JSON per thread count:

```
assignment-1/integrate-1.json
assignment-1/integrate-8.json
assignment-1/integrate-64.json
assignment-1/integrate-128.json
```

Plus a combined `perf-results.json` that the grader reads.

## Reading the output

```bash
python bin/hyperfine_min_time.py assignment-1/integrate-64.json
# → prints minimum wall-clock seconds across runs
```

The grader consumes the same JSONs. Your own `REFLECTION.md` and `tables.csv` must cite numbers from these files — CI cross-checks within 10%.

## What NOT to do

- **Don't `qsub` on a login-ai / login-bi node** if you built with Rome-specific intrinsics; binaries may mismatch. The assessment workflows build fresh on the compute node for this reason.
- **Don't run the benchmark on the login node.** PBS will detect it and kill the process; even if it didn't, login nodes are shared and noisy.
- **Don't fake the numbers.** The grader pulls `perf-results.json` from the CI artifact, *not* from your own commit, so there's no way to substitute doctored data.

## Troubleshooting

- **Job stays `Q`'d for hours**: Rome queue is busy; try `qstat -q` for queue status. Your assessment deadline is end-of-day-5, so the job doesn't need to land the same session.
- **`smart_diff.py` says "tolerance exceeded"**: your parallel output drifted numerically. For reductions, tiny float-order differences are normal but should be within `smart_diff.py`'s default tolerance. If it's failing, check for actual races.
- **Node out-of-memory**: `mem=400gb` should be plenty; if you're hitting the limit, your problem size is too large. Check the starter's grid dimensions against the assignment README.

## Self-hosted GitHub Actions runner

The assessment CI pipeline (`assessment.yml`) also targets a self-hosted Rome runner with label `self-hosted, rome-128`. If the runner is up, you don't need to `qsub` yourself — just push and CI will submit the job on your behalf and upload the artefacts. See `docs/cx3-runner-setup.md`.
