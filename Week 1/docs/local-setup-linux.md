# Local setup — Linux / WSL

Laptop setup on Ubuntu 22.04 / 24.04 and Windows-Subsystem-for-Linux. The toolchain is identical; Ubuntu ships Clang 18+ which is close enough to Rome's Clang for TSan work.

## Install the toolchain

```bash
sudo apt update
sudo apt install -y clang-18 clang-tidy-18 clang-format-18 \
                    libomp-18-dev libomp-dev \
                    cmake ninja-build \
                    cppcheck \
                    python3-pip

# hyperfine (timer used by CX3 benchmark scripts; handy locally too)
sudo apt install -y hyperfine

# Quarto for rendering slide decks
curl -fsSL https://quarto.org/docs/download/_download.json | \
    jq -r '.download[] | select(.asset | test("linux-amd64.deb")) | .asset' | \
    xargs -I{} wget -O /tmp/quarto.deb "https://quarto.org{}"
sudo dpkg -i /tmp/quarto.deb

pip install --user pre-commit
```

## Point CMake at Clang-18

```bash
export CC=clang-18
export CXX=clang++-18
```

(If you use a newer Clang, fine — anything Clang ≥ 16 supports `-fopenmp-version=51`.)

## Fetch doctest (required once after cloning)

```bash
curl -fsSL https://raw.githubusercontent.com/doctest/doctest/v2.4.11/doctest/doctest.h \
  -o snippets/third_party/doctest.h
```

## Build and test

```bash
make snippets-test
make snippets-tsan
```

## WSL-specific notes

- Make sure you're inside the WSL filesystem (e.g. `~/projects`), **not** the mounted Windows `/mnt/c/...`. Compile times on the mounted filesystem are ~10× slower.
- TSan works fine under WSL2; under WSL1 it doesn't.
- If `clang-18` isn't available via `apt`, add LLVM's apt repo: see <https://apt.llvm.org>.

## Pre-commit hooks (recommended)

```bash
pre-commit install
```

## Troubleshooting

- **`libomp.so.5: not found` at run time**: `sudo apt install libomp-dev libomp5` — some distros split the shared lib out.
- **TSan runs pass but miss OpenMP races**: you're likely using GCC's libgomp. TSan instrumentation in libgomp is incomplete. Switch to Clang + LLVM libomp.
- **`Archer` tool not loading**: `export OMP_TOOL=archer` and verify `libarcher.so` is on `LD_LIBRARY_PATH` (`/usr/lib/llvm-18/lib`).
