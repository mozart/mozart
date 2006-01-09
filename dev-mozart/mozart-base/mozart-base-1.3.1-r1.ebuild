DESCRIPTION="Mozart/Oz: a multiparadigm concurrent constraint programming language"
SRC_URI="http://www.mozart-oz.org/download/mozart-ftp/store/1.3.1-2004-06-16/mozart-1.3.1.20040616-src.tar.gz"
HOMEPAGE="http://www.mozart-oz.org/"
KEYWORDS="x86 ppc sparc hppa"

DEPEND="virtual/libc
        virtual/emacs
        app-shells/bash
        dev-lang/perl
        sys-devel/make
        sys-apps/sh-utils
        sys-devel/gcc
        sys-devel/binutils
        sys-apps/sed
        >=sys-devel/flex-2.5.3
        >=sys-devel/bison-1.25
        sys-devel/m4
        dev-libs/gmp
        sys-libs/zlib
        sys-libs/gdbm
        dev-lang/tcl
        dev-lang/tk
"
RDEPEND="virtual/libc
        virtual/emacs
        app-shells/bash
        sys-apps/sh-utils
        sys-devel/gcc
        sys-devel/binutils
        dev-libs/gmp
        sys-libs/zlib
        sys-libs/gdbm
        dev-lang/tcl
        dev-lang/tk
"

S="${WORKDIR}/mozart-1.3.1.20040616"

src_compile() {
    
    ./configure --prefix=/opt/mozart || die "configure failed"
    make depend || die "make depend failed"
    make bootstrap || die "make bootstrap failed"
}

src_install() {

    # install the system itself
    make install PREFIX=${D}/opt/mozart

    # update env
    insinto /etc/env.d
    newins ${FILESDIR}/99mozart-1.3.0 99mozart

}

pkg_postinst() {
    einfo "******************************************************"
    einfo " When finished please run the following so your"
    einfo " environment gets updated."
    eerror "    /usr/sbin/env-update && source /etc/profile"
    einfo "******************************************************"
}
