# Spec file for old gtk-canvas
# Author: Denys Duchier, 2003
# Copyright: Denys Duchier, 2003
Name: gtk-canvas
Version: 0.1
Release: 1
Summary: a GTK 1.2. canvas for use by Mozart and Alice
Copyright: GPL
Vendor: Mozart Consortium
URL: http://www.mozart-oz.org/
Group: Development/Languages
Packager: Denys Duchier
Source: ftp://ftp.mozart-oz.org/pub/extras/gtk-canvas-0.1.tar.gz
Prefix: /usr/local
BuildRoot: /var/tmp/gtk-canvas

%description
This package provides a powerful canvas for GTK 1.2 which extends
the Mozart and Alice GTK interfaces.

%prep
%setup

%build
: ${CFLAGS="$RPM_OPT_FLAGS"}
: ${CXXFLAGS="$RPM_OPT_FLAGS"}
export CFLAGS CXXFLAGS
./configure --prefix=$RPM_BUILD_ROOT/usr/local
make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/local
make install

%files
/usr/local/lib/libart_lgpl.so.2.2.0
/usr/local/lib/libart_lgpl.la
/usr/local/lib/libart_lgpl.a
/usr/local/lib/libartConf.sh
/usr/local/lib/libgtk-canvas.so.1.0.1
/usr/local/lib/libgtk-canvas.la
/usr/local/lib/libgtk-canvas.a
/usr/local/bin/libart-config
/usr/local/share/aclocal/libart.m4
/usr/local/include/libart_lgpl/art_affine.h
/usr/local/include/libart_lgpl/art_alphagamma.h
/usr/local/include/libart_lgpl/art_bpath.h
/usr/local/include/libart_lgpl/art_config.h
/usr/local/include/libart_lgpl/art_filterlevel.h
/usr/local/include/libart_lgpl/art_gray_svp.h
/usr/local/include/libart_lgpl/art_misc.h
/usr/local/include/libart_lgpl/art_pathcode.h
/usr/local/include/libart_lgpl/art_pixbuf.h
/usr/local/include/libart_lgpl/art_point.h
/usr/local/include/libart_lgpl/art_rect.h
/usr/local/include/libart_lgpl/art_rect_svp.h
/usr/local/include/libart_lgpl/art_rect_uta.h
/usr/local/include/libart_lgpl/art_rgb.h
/usr/local/include/libart_lgpl/art_rgb_affine.h
/usr/local/include/libart_lgpl/art_rgb_bitmap_affine.h
/usr/local/include/libart_lgpl/art_rgb_pixbuf_affine.h
/usr/local/include/libart_lgpl/art_rgb_rgba_affine.h
/usr/local/include/libart_lgpl/art_rgb_svp.h
/usr/local/include/libart_lgpl/art_svp.h
/usr/local/include/libart_lgpl/art_svp_ops.h
/usr/local/include/libart_lgpl/art_svp_point.h
/usr/local/include/libart_lgpl/art_svp_render_aa.h
/usr/local/include/libart_lgpl/art_svp_vpath.h
/usr/local/include/libart_lgpl/art_svp_vpath_stroke.h
/usr/local/include/libart_lgpl/art_svp_wind.h
/usr/local/include/libart_lgpl/art_uta.h
/usr/local/include/libart_lgpl/art_uta_ops.h
/usr/local/include/libart_lgpl/art_uta_rect.h
/usr/local/include/libart_lgpl/art_uta_vpath.h
/usr/local/include/libart_lgpl/art_uta_svp.h
/usr/local/include/libart_lgpl/art_vpath.h
/usr/local/include/libart_lgpl/art_vpath_bpath.h
/usr/local/include/libart_lgpl/art_vpath_dash.h
/usr/local/include/libart_lgpl/art_vpath_svp.h
/usr/local/include/libart_lgpl/libart.h
/usr/local/include/libart_lgpl/libart-features.h
/usr/local/include/gtk-canvas/gtk-canvas.h
/usr/local/include/gtk-canvas/gtk-canvas-defs.h
/usr/local/include/gtk-canvas/gtk-canvas-image.h
/usr/local/include/gtk-canvas/gtk-canvas-init.h
/usr/local/include/gtk-canvas/gtk-canvas-line.h
/usr/local/include/gtk-canvas/gtk-canvas-load.h
/usr/local/include/gtk-canvas/gtk-canvas-rect-ellipse.h
/usr/local/include/gtk-canvas/gtk-canvas-polygon.h
/usr/local/include/gtk-canvas/gtk-canvas-text.h
/usr/local/include/gtk-canvas/gtk-canvas-types.h
/usr/local/include/gtk-canvas/gtk-canvas-util.h
/usr/local/include/gtk-canvas/gtk-canvas-widget.h
/usr/local/include/gtk-canvas/gtk-canvastypebuiltins.h
/usr/local/include/gtk-canvas.h
