DESCRIPTION="Mozart/Oz's standard library"
SRC_URI="http://www.mozart-oz.org/download/mozart-ftp/store/1.3.0-2004-04-15/mozart-1.3.0.20040413-std.tar.gz"
HOMEPAGE="http://www.mozart-oz.org/"
KEYWORDS="x86 ppc sparc hppa"
DEPEND="
  dev-mozart/mozart-base
"

S="${WORKDIR}/mozart-1.3.0.20040413-std"

src_compile() {
    ./configure --prefix=/opt/mozart || die "configure failed"
    make || die "make failed"
}

src_install() {
    make install PREFIX=${D}/opt/mozart || die "make install failed"
}
