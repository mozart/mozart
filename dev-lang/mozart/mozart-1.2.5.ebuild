DESCRIPTION="Mozart/Oz constraint programming system"
MOZART_TAG="1.2.5.20021220"
HOMEPAGE="http://www.mozart-oz.org/"
KEYWORDS="x86 ppc sparc hppa"

RDEPEND="$RDEPEND
  dev-lang/mozart-base
  dev-lang/mozart-stdlib
  dev-lang/mozart-doc
  gtk? ( dev-lang/mozart-gtk )
"
