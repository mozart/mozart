DESCRIPTION="Mozart/Oz's documentation"
#SRC_URI="http://www.mozart-oz.org/download/mozart-ftp/store/1.3.0-2004-04-15/mozart-1.3.0.20040413-doc.tar.gz"
SRC_URI="http://www.sics.se/~kost/release-1-3-1/TARs/mozart-1.3.0.20040609-doc.tar.gz"
HOMEPAGE="http://www.mozart-oz.org/"
#KEYWORDS="x86 ppc sparc hppa"
KEYWORDS="-*"

S="${WORKDIR}/mozart"

src_compile() { true; }
src_install() {
    dodir /opt/mozart
    cp -a . ${D}/opt/mozart
}
