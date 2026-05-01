#!/usr/bin/env python3
"""Generate Quarto include-partials from snippet .cpp files.

For every `snippets/<dir>/<name>.cpp` (excluding tests, build artefacts,
and third_party), emit:

  * `slides/_partials/<dir>/<name>.qmd` — whole-file partial (a fenced
    cpp block, line-numbered).
  * For every `// snippet-begin: <region>` / `// snippet-end: <region>`
    marker pair found in the source file:
    `slides/_partials/<dir>/<name>__<region>.qmd` — a partial containing
    only the body lines between the markers (the markers themselves are
    stripped).

Slides include whichever partial fits the slide-budget. The .cpp file
stays compilable + tested; per-region partials let a slide show just
the function under discussion without dragging in headers, helpers, or
multi-line comment blocks.

Usage: invoked from `make partials`. Runs in well under a second.
"""

from __future__ import annotations

import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
SNIP_DIR = ROOT / "snippets"
PART_DIR = ROOT / "slides" / "_partials"

# Match an entire region in one go. Names allow [A-Za-z0-9_-].
_REGION_RE = re.compile(
    r"^[ \t]*//\s*snippet-begin:\s*([\w-]+)\s*\n"
    r"(.*?)"
    r"^[ \t]*//\s*snippet-end:\s*\1\s*$",
    re.MULTILINE | re.DOTALL,
)

_EXCLUDE_PARTS = {"tests", "build", "third_party"}


def _emit(path: pathlib.Path, body: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        '```{.cpp code-line-numbers="true"}\n' + body.rstrip("\n") + "\n```\n"
    )


def main() -> int:
    n_full = 0
    n_region = 0
    for cpp in sorted(SNIP_DIR.rglob("*.cpp")):
        rel = cpp.relative_to(SNIP_DIR)
        if any(part in _EXCLUDE_PARTS for part in rel.parts):
            continue
        src = cpp.read_text()

        # Whole-file partial (kept for slides that include the full snippet).
        _emit(PART_DIR / rel.with_suffix(".qmd"), src)
        n_full += 1

        # Per-region partials, if any markers exist.
        for match in _REGION_RE.finditer(src):
            region_name, body = match.group(1), match.group(2)
            region_path = PART_DIR / rel.with_name(
                f"{rel.stem}__{region_name}.qmd"
            )
            _emit(region_path, body)
            n_region += 1

    print(f"partials: {n_full} whole-file + {n_region} region")
    return 0


if __name__ == "__main__":
    sys.exit(main())
