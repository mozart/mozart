DESCRIPTION="Mozart/Oz's documentation"
MOZART_TAG="1.2.5.20021220"
SRC_URI="ftp://ftp.mozart-oz.org/pub/mozart/${MOZART_TAG}/tar/mozart-${MOZART_TAG}-doc.tar.gz"
HOMEPAGE="http://www.mozart-oz.org/"
KEYWORDS="x86 ppc sparc hppa"

S="${WORKDIR}/mozart"

src_compile() { true; }
src_install() {
    cp -a . ${D}/opt/mozart
}
