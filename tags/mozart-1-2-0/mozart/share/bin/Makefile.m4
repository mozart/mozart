SHELL = /bin/sh


define(OzDir,/usr/share/gs/soft/oz)
define(OzLib,OzDir/lib)
define(OzInc,OzDir/`include')
define(OzLisp,OzDir/elisp)
define(OzBin,OzDir/bin)

define(installOz,
$1:	OzBin/$1

OzBin/$1:   $2$1.csh
	install -m 555 $< $@
)

help:
	@echo ""
	@echo "The following targets are available"
	@echo "(use 'make <target>' for installation)"
	@echo "	oz2lpr		mehl@dfki.uni-sb.de"
	@echo ""

Makefile: Makefile.m4
	m4 Makefile.m4 > Makefile


installOz(oz2lpr)

