DESCRIPTION="Mozart/Oz's documentation"
SRC_URI="http://www.mozart-oz.org/download/mozart-ftp/store/1.3.1-2004-06-09/mozart-1.3.1.20040609-doc.tar.gz"
HOMEPAGE="http://www.mozart-oz.org/"
KEYWORDS="x86 ppc sparc hppa"

S="${WORKDIR}/mozart"

src_compile() { true; }
src_install() {
    dodir /opt/mozart
    cp -a . ${D}/opt/mozart
}
