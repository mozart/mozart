DESCRIPTION="A GTK canvas for Mozart and Alice"
SRC_URI="http://www.mozart-oz.org/download/archive/1.2.5-2003-02-12/gtk-canvas-0.1-20030212.tar.gz"
HOMEPAGE="http://www.mozart-oz.org/"
KEYWORDS="x86 ppc sparc hppa"
DEPEND="
  =x11-libs/gtk+-1.2*
  gnome-base/gnome-libs
"
RDEPEND="
  =x11-libs/gtk+-1.2*
"
S="${WORKDIR}/gtk-canvas-0.1"

src_compile() {
    ./configure --prefix=/usr/local || die "configure failed"
    make || die "make failed"
}

src_install() {
    make install prefix=${D}/usr/local || die "make install failed"
}
