DESCRIPTION="Mozart/Oz's standard library"
SRC_URI="ftp://ftp.mozart-oz.org/pub/mozart/store/1.2.5-2003-02-01/mozart-1.2.5.20030131-std.tar.gz"
HOMEPAGE="http://www.mozart-oz.org/"
KEYWORDS="x86 ppc sparc hppa"
DEPEND="
  dev-lang/mozart-base
"

S="${WORKDIR}/mozart-1.2.5.20030131-std"

src_compile() { true; }

src_install() {
    /opt/mozart/bin/ozengine ./ozmake.ozf --prefix=${D}/opt/mozart --nosavedb --install --package=mozart-stdlib.pkg || die "ozmake failed"
}
