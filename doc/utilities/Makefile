OZFLAGS = --noallowdeprecated
OZC = ozc $(OZFLAGS)
EMACS = emacs

FUNCTORS = \
	SGML.ozf AuthorDB.ozf BibliographyDB.ozf Fontifier.ozf \
	OzDocToHTML.ozf
SYSLET = ozdoc
ELCS = ozdoc.elc

all: $(FUNCTORS) $(SYSLET) $(ELCS)

ozdoc: MakeOzDoc.oz
	$(OZC) -x MakeOzDoc.oz -o ozdoc

%.ozf: %.oz
	$(OZC) -c $< -o $@

%.elc: %.el
	$(EMACS) -q -batch -f batch-byte-compile $<

clean:
	rm -rf $(FUNCTORS) $(SYSLET) $(ELCS)

veryclean: clean
