OZHOME=/usr/share/gs/soft/oz

SHELL=/bin/sh
SCRIPTS = oz oz2lpr oz+
LINKS = oz- oz.compiler oz.machine

help:
        @echo ""
        @echo "The following targets are available"
        @echo " install         {scheidhr,mehl}@dfki.uni-sb.de"
        @echo ""

install:
        install -m 555 $(SCRIPTS) $(OZHOME)/bin
        @cd  $(BINDIR); set -x; \
        for file in $(LINKS); \
        do\
           rm -f $$file; \
           ln -s oz $$file; \
        done
