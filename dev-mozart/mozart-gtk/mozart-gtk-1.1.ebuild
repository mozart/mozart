DESCRIPTION="GTK 1.2 bindings for Mozart and Alice"
SRC_URI="http://www.mozart-oz.org/download/archive/1.2.5.2003-04-28/mozart-gtk-1.1.tar.gz"
HOMEPAGE="http://www.mozart-oz.org/"
KEYWORDS="x86 ppc sparc hppa"
DEPEND="
  =x11-libs/gtk+-1.2*
  dev-lang/mozart-base
  dev-lang/mozart-gtk-canvas
  app-text/openjade
"
S="${WORKDIR}/mozart-gtk-1.1"

src_compile() {
    PATH="/opt/mozart/bin:$PATH"
    ./configure --prefix=/opt/mozart || die "configure failed"
    make || die "make failed"
}

src_install() {
    make install PREFIX=${D}/opt/mozart || die "make install failed"
}
