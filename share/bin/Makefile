SHELL=/bin/sh
SCRIPTS = oz oz2lpr
LINKS = oz- oz.compiler oz.machine

BINDIR = /usr/share/gs/soft/oz/bin
#BINDIR = ./bin

install:
        install -m 555 $(SCRIPTS) $(BINDIR)
        @cd  $(BINDIR); set -x; \
        for file in $(LINKS); \
        do\
           rm -f $$file; \
           ln -s oz $$file; \
        done
