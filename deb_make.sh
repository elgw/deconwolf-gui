#!/bin/bash

# Create a debian (.deb) package.

set -e # exit on error

NAME="Erik Wernersson"
EMAIL="erik.wernersson@scilifelab.se"

if [ $EUID = 0 ]; then
    echo "WARNING: You are running as root, please abort!"
    echo "Sleeping for 10 s"
    sleep 10
fi

make dw_gui

ver_major=`sed -rn 's/^#define.*DW_GUI_VERSION_MAJOR.*([0-9]+).*$/\1/p' < src/version.h`
ver_minor=`sed -rn 's/^#define.*DW_GUI_VERSION_MINOR.*([0-9]+).*$/\1/p' < src/version.h`
ver_patch=`sed -rn 's/^#define.*DW_GUI_VERSION_PATCH.*([0-9]+).*$/\1/p' < src/version.h`
pkgver="${ver_major}.${ver_minor}.${ver_patch}"
echo "pkgver=$pkgver"

arch=$(dpkg --print-architecture)

name=dwgui_${pkgver}_${arch}

mkdir -p $name/usr/local/bin/
cp dw_gui $name/usr/local/bin/
mkdir -p $name/usr/share/applications/
cp resources/deconwolf.desktop $name/usr/share/applications/
mkdir -p $name/usr/share/icons/hicolor/48x48/apps/
cp resources/deconwolf.png $name/usr/share/icons/hicolor/48x48/apps/

mkdir -p $name/DEBIAN

cat > $name/DEBIAN/control <<- EOM
Package: dwgui
Version: $pkgver
Section: science
Priority: optional
Architecture: $arch
Depends: libgtk-3-0
Maintainer: $NAME <$EMAIL>
Homepage: https://www.github.com/elgw/deconwolf-gui/
Description: dwgui
 A graphical user interface for the deconvolution
 program deconwolf (dw).
EOM

date > $name/DEBIAN/changelog

dpkg-deb --build $name
rm -rf $name
