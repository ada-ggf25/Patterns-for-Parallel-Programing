.PHONY: all pdf html watch clean examples-build examples-clean

all: pdf html

pdf: cx3-hpc-intro.pdf
cx3-hpc-intro.pdf: cx3-hpc-intro.qmd _quarto.yml
	quarto render cx3-hpc-intro.qmd --to beamer

html: cx3-hpc-intro.html
cx3-hpc-intro.html: cx3-hpc-intro.qmd _quarto.yml
	quarto render cx3-hpc-intro.qmd --to revealjs

# Live-preview the HTML deck in a browser.
watch:
	quarto preview cx3-hpc-intro.qmd --to revealjs

# Build the companion examples locally (needs GCC with OpenMP + MPI).
examples-build:
	cmake -S examples -B examples/build -DCMAKE_BUILD_TYPE=Release
	cmake --build examples/build -j

examples-clean:
	rm -rf examples/build

clean: examples-clean
	rm -rf cx3-hpc-intro.pdf cx3-hpc-intro.html cx3-hpc-intro_files/ \
	       slides.pdf slides.html slides_files/ _freeze/ \
	       *.tex *.log *.aux *.out *.nav *.snm *.toc *.vrb
