DESCRIPTION="tool for building Mozart/Oz projects and installing and managing Mozart/Oz packages"
SRC_URI="http://www.ps.uni-sb.de/~duchier/mogul/pub/pkg/ozmake-0.82"
HOMEPAGE="http://www.mozart-oz.org/mogul/info/duchier/ozmake.html"
KEYWORDS="x86 ppc sparc hppa"
RDEPEND="dev-lang/mozart-base"

S="$WORKDIR"

src_unpack() {
    cp -a $DISTDIR/$A $WORKDIR/$A
}
src_compile() { :; }
src_install() {
    into /opt/mozart
    newbin $A ozmake
}
