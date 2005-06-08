TXTFILES=$(shell echo mep-*.txt)
HTMLFILES=$(addsuffix .html,$(basename $(TXTFILES)))

all: $(HTMLFILES)

%.html:%.txt
	./docutils-mep2html.py $<

THERE = www.mozart-oz.org:/services/mozart/httpd/html/meps

update:
	rsync -aPC --exclude '*~' . $(THERE)

clean:
	-rm *~
