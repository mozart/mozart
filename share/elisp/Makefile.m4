SHELL = /bin/sh


define(OzDir,/usr/share/gs/soft/oz)
define(OzLib,OzDir/lib)
define(OzInc,OzDir/`include')
define(OzLisp,OzDir/elisp)

define(installOz,
$1:  OzLisp/$1.el

OzLisp/$1.el:   $2$1.el
	install -m 664 $< $@
)

help:
	@echo ""
	@echo "The following targets are available"
	@echo "(use 'make <target>' for installation)"
	@echo "	oz		mehl@dfki.uni-sb.de"
	@echo ""

Makefile: Makefile.m4
	m4 Makefile.m4 > Makefile


installOz(oz)
	echo "(byte-compile-file \"OzLisp/oz.el\")" > compile
	lemacs -batch -load compile
	rm compile
