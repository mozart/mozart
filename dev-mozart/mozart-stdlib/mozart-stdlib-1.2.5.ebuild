DESCRIPTION="Mozart/Oz's standard library"
MOZART_TAG="1.2.5.20030123"
SRC_URI="ftp://ftp.mozart-oz.org/pub/mozart/${MOZART_TAG}/tar/mozart-${MOZART_TAG}-std.tar.gz"
HOMEPAGE="http://www.mozart-oz.org/"
KEYWORDS="x86 ppc sparc hppa"
DEPEND="
  dev-lang/mozart-base
"

S="${WORKDIR}/mozart-${MOZART_TAG}-std"

src_compile() { true; }

src_install() {
    /opt/mozart/bin/ozengine ./ozmake.ozf --prefix=${D}/opt/mozart --nosavedb --install --package=mozart-stdlib.pkg || die "ozmake failed"
}
