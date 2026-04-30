.PHONY: all html watch clean partials plots snippets-build snippets-test snippets-tsan snippets-tsan-full snippets-perf snippets-clean lint

DECKS := slides/day2 slides/day3 slides/day4
INDEX := slides/index
HTML  := $(addsuffix .html,$(DECKS) $(INDEX))

# --- slide-include partials -------------------------------------------------
# slides/tools/build_partials.py converts each snippets/<dir>/<name>.cpp
# into a slides/_partials/<dir>/<name>.qmd (whole-file partial) AND, for
# every `// snippet-begin: <region>` / `// snippet-end: <region>` marker
# pair, a slides/_partials/<dir>/<name>__<region>.qmd partial containing
# only the body lines between the markers. Region partials let a slide
# show a single function instead of a 70-line file.
#
# Quarto's `{.cpp include="..."}` block-attribute syntax is NOT a valid
# directive — it preserves the attribute as data-include but emits an
# empty <code> element, so we generate proper partials and include them
# via the {{< include >}} shortcode.
SNIPPET_CPPS := $(shell find snippets -name '*.cpp' \
                  -not -path '*/tests/*' \
                  -not -path '*/build/*' \
                  -not -path '*/third_party/*')

# Stamp file marks the last successful run of build_partials.py. Any
# .html rule that lists this as a prerequisite will re-render when a
# snippet changes, even though the .qmd source is untouched (Quarto
# `{{< include >}}` shortcodes pull partials at render time).
PARTIALS_STAMP := slides/_partials/.stamp

$(PARTIALS_STAMP): $(SNIPPET_CPPS) slides/tools/build_partials.py
	python3 slides/tools/build_partials.py
	@mkdir -p $(dir $@) && touch $@

partials: $(PARTIALS_STAMP)

# --- generated plots --------------------------------------------------------
# Plot scripts in slides/tools/render_*.py emit committed PNGs under
# slides/images/. NOT a dependency of `make html` — matplotlib may not
# be on the CI runner, and the PNGs are checked in. Regenerate by hand
# (`make plots`) when the underlying constants change.
plots:
	python3 slides/tools/render_roofline.py
	python3 slides/tools/render_schedule_distribution.py
	python3 slides/tools/render_false_sharing.py
	python3 slides/tools/render_simd_lanes.py

all: html

# --- slides -----------------------------------------------------------------

html: $(HTML)

# Each .qmd carries its own format in its YAML header (revealjs for decks,
# html for the landing page), so we don't pass --to here. The stamp dep
# ensures partials are regenerated, AND the .html re-renders, when a
# snippet changes.
%.html: %.qmd _quarto.yml $(PARTIALS_STAMP)
	quarto render $<

# Live-preview the active deck in a browser (pass DECK=slides/day2 etc.).
DECK ?= slides/day2
watch:
	quarto preview $(DECK).qmd

# --- lecture snippets -------------------------------------------------------

snippets-build:
	cmake -S snippets -B snippets/build/release -DCMAKE_BUILD_TYPE=Release
	cmake --build snippets/build/release -j

# Release lane — correctness tests excluding race-demo (RACE_DEMO is
# only meaningful under TSan) and perf assertions (noisy without a
# dedicated runner; opt-in via `make snippets-perf`).
snippets-test: snippets-build
	ctest --test-dir snippets/build/release --output-on-failure -LE 'RACE_DEMO|perf'

snippets-tsan:
	cmake -S snippets -B snippets/build/tsan -DCMAKE_BUILD_TYPE=Debug -DENABLE_TSAN=ON
	cmake --build snippets/build/tsan -j
	# Race-demo lane only. Each race-demo test sets its own
	# TSAN_OPTIONS via the CTest `ENVIRONMENT` property, and passes
	# via PASS_REGULAR_EXPRESSION on the TSan diagnostic.
	#
	# Use --verbose (not --output-on-failure): the test "passes" on
	# the TSan diagnostic match, so plain ctest hides the report. For
	# this lane the diagnostic IS the output the user wants to see.
	#
	# Why not run *every* test under TSan here? OpenMP constructs use
	# hidden synchronisation that bare TSan reports as data races
	# (false positives on every `reduction`, barrier, etc.). The
	# Archer OMPT tool annotates those constructs so TSan ignores
	# them — but Homebrew's macOS LLVM does not ship libarcher. The
	# full-suite TSan run lives in `snippets-tsan-full` (Linux + LLVM
	# with libarcher) and is what CI runs.
	ctest --test-dir snippets/build/tsan --verbose -L RACE_DEMO

# Full-suite TSan + Archer — Linux + LLVM build that ships libarcher.
# Race-demo tests still pass via PASS_REGULAR_EXPRESSION; non-race
# tests must remain TSan-clean (Archer hides the OpenMP-internal
# pseudo-races so reductions etc. don't trip the detector).
snippets-tsan-full:
	cmake -S snippets -B snippets/build/tsan -DCMAKE_BUILD_TYPE=Debug -DENABLE_TSAN=ON
	cmake --build snippets/build/tsan -j
	@if [ -z "$$ARCHER_LIB" ]; then \
	  echo "error: set ARCHER_LIB=/path/to/libarcher.so before invoking" >&2; \
	  exit 1; \
	fi
	OMP_TOOL=enabled OMP_TOOL_LIBRARIES="$$ARCHER_LIB" \
	  TSAN_OPTIONS="halt_on_error=1:second_deadlock_stack=1:ignore_noninstrumented_modules=1" \
	  ctest --test-dir snippets/build/tsan --output-on-failure -LE perf

# Perf lane — behavioural-perf assertions (false-sharing, etc.). Opt-in
# because the assertions are noisy on small / virtualised runners.
snippets-perf: snippets-build
	ctest --test-dir snippets/build/release --output-on-failure -L perf

snippets-clean:
	rm -rf snippets/build

# --- lint / housekeeping ----------------------------------------------------

lint:
	@echo "Running clang-format --dry-run..."
	@find snippets -name '*.cpp' -o -name '*.h' | xargs clang-format --dry-run --Werror || true
	@echo "Running markdownlint..."
	@markdownlint '**/*.md' '!venv/**' '!legacy/**' '!snippets/build/**' || true
	@echo "Running lychee (link check)..."
	@lychee --config .lychee.toml '**/*.md' '**/*.qmd' || true

# --- clean ------------------------------------------------------------------

clean: snippets-clean
	rm -rf slides/*.html slides/*_files/ slides/_partials/ _freeze/
