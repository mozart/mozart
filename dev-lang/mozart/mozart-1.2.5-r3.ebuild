DESCRIPTION="Mozart/Oz constraint programming system"
HOMEPAGE="http://www.mozart-oz.org/"
KEYWORDS="x86 ppc sparc hppa"

IUSE="gtk"

RDEPEND="$RDEPEND
  =dev-lang/mozart-base-1.2.5-r3
  =dev-lang/mozart-stdlib-1.2.5-r1
  =dev-lang/mozart-doc-1.2.5-r1
  =dev-lang/mozart-ozmake-0.82
  gtk? ( =dev-lang/mozart-gtk-1.1 )
"
