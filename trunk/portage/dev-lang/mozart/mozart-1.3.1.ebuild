DESCRIPTION="Mozart/Oz constraint programming system"
HOMEPAGE="http://www.mozart-oz.org/"
KEYWORDS="x86 ppc sparc hppa"

IUSE="gtk"

RDEPEND="$RDEPEND
  =dev-mozart/mozart-base-1.3.1
  =dev-mozart/mozart-stdlib-1.3.1
  =dev-mozart/mozart-doc-1.3.1
  gtk? ( =dev-mozart/mozart-gtk-1.1-r1 )
"
