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
/usr/local/oz/cache/x-oz/system/gtk/GOZSignal.so-linux-i486
/usr/local/oz/cache/x-oz/system/gtk/GOZCore.ozf
/usr/local/oz/cache/x-oz/system/gtk/GDK.ozf
/usr/local/oz/cache/x-oz/system/gtk/GTK.ozf
/usr/local/oz/cache/x-oz/system/gtk/GTKCANVAS.ozf
/usr/local/oz/cache/x-oz/system/gtk/GdkNative.so-linux-i486
/usr/local/oz/cache/x-oz/system/gtk/GtkNative.so-linux-i486
/usr/local/oz/cache/x-oz/system/gtk/GdkFieldNative.so-linux-i486
/usr/local/oz/cache/x-oz/system/gtk/GtkFieldNative.so-linux-i486
/usr/local/oz/cache/x-oz/system/gtk/GtkCanvasNative.so-linux-i486
/usr/local/oz/cache/x-oz/system/gtk/GtkCanvasFieldNative.so-linux-i486
/usr/local/oz/cache/x-oz/system/gtk/Generator.ozf
/usr/local/oz/cache/x-oz/system/gtk/GBuilderTypes.ozf
/usr/local/oz/cache/x-oz/system/gtk/GBuilderWidgets.ozf
/usr/local/oz/cache/x-oz/system/gtk/GBuilder.ozf
/usr/local/oz/cache/x-oz/system/gtk/ClassNames.ozp
/usr/local/oz/examples/gtk/CList.oz
/usr/local/oz/examples/gtk/CTree.oz
/usr/local/oz/examples/gtk/CanvasEvents.oz
/usr/local/oz/examples/gtk/CanvasMove.oz
/usr/local/oz/examples/gtk/CanvasScramble.oz
/usr/local/oz/examples/gtk/FileSelection.oz
/usr/local/oz/examples/gtk/HelloArgs.oz
/usr/local/oz/examples/gtk/HelloCanvas.oz
/usr/local/oz/examples/gtk/HelloGTK.oz
/usr/local/oz/examples/gtk/HelloImage.oz
/usr/local/oz/examples/gtk/HelloText.oz
/usr/local/oz/examples/gtk/List.oz
/usr/local/oz/examples/gtk/GBuilderExamples.oz
