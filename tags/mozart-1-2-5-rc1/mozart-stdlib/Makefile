all:
	echo "specify a target: distrib"

VERSION=1.2.3
TODAY=$(shell date '+%Y%m%d')
TMPNAM=mozart-$(VERSION).$(TODAY)-std

distrib:
	-rm -rf $(TMPNAM)
	mkdir $(TMPNAM)
	ozmake -cp $(TMPNAM)/mozart-stdlib.pkg
	cp -a `which ozmake` $(TMPNAM)/ozmake
	cp README $(TMPNAM)/README
	tar czf $(TMPNAM).tar.gz $(TMPNAM)
	-rm -rf $(TMPNAM)
