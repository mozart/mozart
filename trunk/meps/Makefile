TXTFILES=$(shell echo mep-*.txt)
HTMLFILES=$(addsuffix .html,$(basename $(TXTFILES)))

all: $(HTMLFILES)

%.html:%.txt
	./docutils-mep2html.py $<
