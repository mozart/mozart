DESCRIPTION="Mozart/Oz's documentation"
MOZART_TAG="1.2.5.20030123"
SRC_URI="ftp://ftp.mozart-oz.org/pub/mozart/${MOZART_TAG}/tar/mozart-${MOZART_TAG}-doc.tar.gz"
HOMEPAGE="http://www.mozart-oz.org/"
KEYWORDS="x86 ppc sparc hppa"

S="${WORKDIR}/mozart"

src_compile() { true; }
src_install() {
    dodir /opt/mozart
    cp -a . ${D}/opt/mozart
}
