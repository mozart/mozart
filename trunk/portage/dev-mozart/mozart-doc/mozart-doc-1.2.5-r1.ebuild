DESCRIPTION="Mozart/Oz's documentation"
SRC_URI="http://www.mozart-oz.org/download/archive/1.2.5-2003-02-12/mozart-1.2.5.20030212-doc.tar.gz"
HOMEPAGE="http://www.mozart-oz.org/"
KEYWORDS="x86 ppc sparc hppa"

S="${WORKDIR}/mozart"

src_compile() { true; }
src_install() {
    dodir /opt/mozart
    cp -a . ${D}/opt/mozart
}
