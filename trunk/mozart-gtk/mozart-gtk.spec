# Spec file for mozart-gtk
# Author: Denys Duchier, 2003
# Copyright: Denys Duchier, 2003
Name: mozart-gtk
Version: 1.0
Release: 1
Summary: GTK 1.2 bindings for Mozart and Alice
Copyright: Free
Vendor: Mozart Consortium
URL: http://www.mozart-oz.org/
Group: Development/Languages
Packager: Denys Duchier
Source: ftp://ftp.mozart-oz.org/pub/extras/mozart-gtk-1.0.tar.gz
Prefix: /usr/local/oz
BuildRoot: /var/tmp/mozart-gtk

%description
This package provides GTK 1.2 bindings for Mozart and Alice

%prep
%setup

%build
: ${CFLAGS="$RPM_OPT_FLAGS"}
: ${CXXFLAGS="$RPM_OPT_FLAGS"}
export CFLAGS CXXFLAGS
./configure --prefix=$RPM_BUILD_ROOT/usr/local/oz
make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/local/oz
make install

%files
/usr/local/cache/x-oz/system/gtk/GOZSignal.so-linux-i486
/usr/local/cache/x-oz/system/gtk/GOZCore.ozf
/usr/local/cache/x-oz/system/gtk/GDK.ozf
/usr/local/cache/x-oz/system/gtk/GTK.ozf
/usr/local/cache/x-oz/system/gtk/GTKCANVAS.ozf
/usr/local/cache/x-oz/system/gtk/GdkNative.so-linux-i486
/usr/local/cache/x-oz/system/gtk/GtkNative.so-linux-i486
/usr/local/cache/x-oz/system/gtk/GdkFieldNative.so-linux-i486
/usr/local/cache/x-oz/system/gtk/GtkFieldNative.so-linux-i486
/usr/local/cache/x-oz/system/gtk/GtkCanvasNative.so-linux-i486
/usr/local/cache/x-oz/system/gtk/GtkCanvasFieldNative.so-linux-i486
/usr/local/cache/x-oz/system/gtk/Generator.ozf
/usr/local/cache/x-oz/system/gtk/GBuilderTypes.ozf
/usr/local/cache/x-oz/system/gtk/GBuilderWidgets.ozf
/usr/local/cache/x-oz/system/gtk/GBuilder.ozf
/usr/local/cache/x-oz/system/gtk/ClassNames.ozp
/usr/local/examples/gtk/CList.oz
/usr/local/examples/gtk/CTree.oz
/usr/local/examples/gtk/CanvasEvents.oz
/usr/local/examples/gtk/CanvasMove.oz
/usr/local/examples/gtk/CanvasScramble.oz
/usr/local/examples/gtk/FileSelection.oz
/usr/local/examples/gtk/HelloArgs.oz
/usr/local/examples/gtk/HelloCanvas.oz
/usr/local/examples/gtk/HelloGTK.oz
/usr/local/examples/gtk/HelloImage.oz
/usr/local/examples/gtk/HelloText.oz
/usr/local/examples/gtk/List.oz
/usr/local/examples/gtk/GBuilderExamples.oz
/usr/local/doc/add-ons/gtk/index.html
/usr/local/doc/add-ons/gtk/node1.html
/usr/local/doc/add-ons/gtk/node2.html
/usr/local/doc/add-ons/gtk/node3.html
/usr/local/doc/add-ons/gtk/node4.html
/usr/local/doc/add-ons/gtk/node5.html
