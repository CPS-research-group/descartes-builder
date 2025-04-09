all: *.tex
	latexmk -pdf -outdir=bld ./BUILDER_DOC.tex
	cp bld/BUILDER_DOC.pdf .

clean:
	latexmk -outdir=bld -C
