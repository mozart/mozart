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

%post
ldconfig

%postun
ldconfig

%files
./lib/libgtk-canvas.so.1.0.1
./lib/libgtk-canvas.so.1
./lib/libgtk-canvas.so
./lib/libgtk-canvas.la
./lib/libgtk-canvas.a
./include/gtk-canvas/gtk-canvas.h
./include/gtk-canvas/gtk-canvas-defs.h
./include/gtk-canvas/gtk-canvas-image.h
./include/gtk-canvas/gtk-canvas-init.h
./include/gtk-canvas/gtk-canvas-line.h
./include/gtk-canvas/gtk-canvas-load.h
./include/gtk-canvas/gtk-canvas-rect-ellipse.h
./include/gtk-canvas/gtk-canvas-polygon.h
./include/gtk-canvas/gtk-canvas-text.h
./include/gtk-canvas/gtk-canvas-types.h
./include/gtk-canvas/gtk-canvas-util.h
./include/gtk-canvas/gtk-canvas-widget.h
./include/gtk-canvas/gtk-canvastypebuiltins.h
./include/gtk-canvas.h
