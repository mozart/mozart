# Spec file for mozart-gtk
# Author: Denys Duchier, 2003
# Copyright: Denys Duchier, 2003
Name: mozart-gtk
Version: 1.1
Release: 1
Summary: GTK 1.2 bindings for Mozart and Alice
Copyright: Free
Vendor: Mozart Consortium
URL: http://www.mozart-oz.org/
Group: Development/Languages
Packager: Denys Duchier
Source: ftp://ftp.mozart-oz.org/pub/extras/mozart-gtk-1.0.tar.gz
Prefix: /usr/lib/mozart
BuildRoot: /var/tmp/mozart-gtk

%description
This package provides GTK 1.2 bindings for Mozart and Alice

%prep
%setup

%build
: ${CFLAGS="$RPM_OPT_FLAGS"}
: ${CXXFLAGS="$RPM_OPT_FLAGS"}
export CFLAGS CXXFLAGS
./configure --prefix=$RPM_BUILD_ROOT/usr/lib/mozart
make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/lib/mozart
make install

%files
/usr/lib/mozart/cache/x-oz/system/gtk/GOZSignal.so-linux-i486
/usr/lib/mozart/cache/x-oz/system/gtk/GOZCore.ozf
/usr/lib/mozart/cache/x-oz/system/gtk/GDK.ozf
/usr/lib/mozart/cache/x-oz/system/gtk/GTK.ozf
/usr/lib/mozart/cache/x-oz/system/gtk/GTKCANVAS.ozf
/usr/lib/mozart/cache/x-oz/system/gtk/GdkNative.so-linux-i486
/usr/lib/mozart/cache/x-oz/system/gtk/GtkNative.so-linux-i486
/usr/lib/mozart/cache/x-oz/system/gtk/GdkFieldNative.so-linux-i486
/usr/lib/mozart/cache/x-oz/system/gtk/GtkFieldNative.so-linux-i486
/usr/lib/mozart/cache/x-oz/system/gtk/GtkCanvasNative.so-linux-i486
/usr/lib/mozart/cache/x-oz/system/gtk/GtkCanvasFieldNative.so-linux-i486
/usr/lib/mozart/cache/x-oz/system/gtk/parser.so-linux-i486
/usr/lib/mozart/cache/x-oz/system/gtk/Generator.ozf
/usr/lib/mozart/cache/x-oz/system/gtk/GBuilderTypes.ozf
/usr/lib/mozart/cache/x-oz/system/gtk/GBuilderWidgets.ozf
/usr/lib/mozart/cache/x-oz/system/gtk/GBuilder.ozf
/usr/lib/mozart/cache/x-oz/system/gtk/ClassNames.ozp
/usr/lib/mozart/examples/gtk/CList.oz
/usr/lib/mozart/examples/gtk/CTree.oz
/usr/lib/mozart/examples/gtk/CanvasEvents.oz
/usr/lib/mozart/examples/gtk/CanvasMove.oz
/usr/lib/mozart/examples/gtk/CanvasScramble.oz
/usr/lib/mozart/examples/gtk/FileSelection.oz
/usr/lib/mozart/examples/gtk/HelloArgs.oz
/usr/lib/mozart/examples/gtk/HelloCanvas.oz
/usr/lib/mozart/examples/gtk/HelloGTK.oz
/usr/lib/mozart/examples/gtk/HelloImage.oz
/usr/lib/mozart/examples/gtk/HelloText.oz
/usr/lib/mozart/examples/gtk/List.oz
/usr/lib/mozart/examples/gtk/GBuilderExamples.oz
/usr/lib/mozart/doc/add-ons/gtk/index.html
/usr/lib/mozart/doc/add-ons/gtk/node1.html
/usr/lib/mozart/doc/add-ons/gtk/node2.html
/usr/lib/mozart/doc/add-ons/gtk/node3.html
/usr/lib/mozart/doc/add-ons/gtk/node4.html
/usr/lib/mozart/doc/add-ons/gtk/node5.html
/usr/lib/mozart/doc/add-ons/gtk/ozdoc.css
/usr/lib/mozart/doc/add-ons/gtk/page.gif