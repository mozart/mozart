CGIHOME = /usr/lib/mozart/bin

OZC      = ozc
OZCFLAGS = 
OZL      = ozl
OZLFLAGS = -z 9 --execpath=$(CGIHOME)/ozengine -x

COMPILE = $(OZC) -c $(OZCFLAGS)
LINK    = $(OZL) $(OZLFLAGS)

FUNCTORS = \
	Abstractions.ozf Book.ozf HTML.ozf ProcessBook.ozf \
	Server.ozf
SERVLETS = \
	lmf.cgi lmf-book.cgi lmfs


all: $(FUNCTORS) $(SERVLETS)

lmf.cgi: $(FUNCTORS)
	$(LINK) Book.ozf -o lmf.cgi

lmf-book.cgi: $(FUNCTORS)
	$(LINK) ProcessBook.ozf -o lmf-book.cgi

lmfs: $(FUNCTORS)
	$(LINK) Server.ozf -o lmfs

%.ozf: %.oz
	$(COMPILE)  $< -o $@

.PHONY: install


WWW = /home/chris/public_html/demo/

INSTFILES = \
	$(WWW)/lmf.cgi $(WWW)/lmf-book.cgi $(HOME)/berlin/lmfs


install: $(INSTFILES)

$(WWW)/%.cgi: %.cgi
	install $< $@

$(HOME)/berlin/lmfs: lmfs
	install $< $@

.PHONY: clean veryclean

clean:
	-rm -f $(FUNCTORS) *~

veryclean: clean
	-rm -f $(SERVLETS) 

