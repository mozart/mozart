OZDOC     = ozdoc

SGMLSRCS  = gump.sgml intro.sgml parser.sgml scanner.sgml syntax.sgml
OZGSRCS   = LambdaParser.ozg LambdaScanner.ozg TestParser.ozg TestScanner.ozg
MISCSRCS  = Lambda.in

all: gump-html

gump-html: $(OZGSRCS) $(SGMLSRCS) $(MISCSRCS)
	$(OZDOC) --in=gump.sgml --out=gump-html --type=html-color

gump-tex: $(OZGSRCS) $(SGMLSRCS) $(MISCSRCS)
	$(OZDOC) --in=gump.sgml --out=gump-tex --type=latex

install: 
	@echo 'Not yet implemented.'

clean:
	-rm -rf gump-html
	-rm -rf gump-tex
	-rm *~
