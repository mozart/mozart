OZHOME  = /usr/share/gs/soft/oz
BINDIR  = $(OZHOME)/bin
SHELL   = /bin/sh

SCRIPTS = oz oz2lpr oz+ oz2declare
LINKS   = oz- ozc oz.compiler oz.machine

help:
	@echo ""
	@echo "The following targets are available"
	@echo "	install		{scheidhr,mehl}@dfki.uni-sb.de"
	@echo ""

install:
	install -m 555 $(SCRIPTS) $(OZHOME)/bin
	@set -x; cd $(BINDIR);\
	for file in $(LINKS); \
        do\
	   rm -f $$file; \
	   ln -s oz $$file; \
        done

