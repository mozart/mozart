DESCRIPTION="GTK 1.2 bindings for Mozart and Alice"
SRC_URI="ftp://ftp.mozart-oz.org/pub/mozart/store/1.2.5-2003-01-31/mozart-gtk-1.0.tar.gz"
HOMEPAGE="http://www.mozart-oz.org/"
KEYWORDS="x86 ppc sparc hppa"
DEPEND="
  =x11-libs/gtk+-1.2*
  dev-lang/mozart-base
  dev-lang/mozart-gtk-canvas
"
S="${WORKDIR}/mozart-gtk-1.0"

src_compile() {
    PATH="/opt/mozart/bin:$PATH"
    ./configure --prefix=/opt/mozart || die "configure failed"
    make || die "make failed"
}

src_install() {
    make install PREFIX=${D}/opt/mozart || die "make install failed"
}
