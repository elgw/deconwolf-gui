#!/bin/bash
set -e

make dw_gui

# <project>_<major version>.<minor version>-<package revision>
version=0.0-1
name=dwgui_$version

mkdir -p $name/usr/local/bin/
cp dw_gui $name/usr/local/bin/
mkdir -p $name/usr/share/applications/
cp resources/deconwolf.desktop $name/usr/share/applications/
mkdir -p $name/usr/share/icons/hicolor/48x48/apps/
cp resources/deconwolf.png $name/usr/share/icons/hicolor/48x48/apps/

mkdir -p $name/DEBIAN

cat > $name/DEBIAN/control <<- EOM
Package: dwgui
Version: $version
Section: science
Priority: optional
Architecture: amd64
Maintainer: Your Name <erik.wernersson@sclilifelab.se>
Homepage: https://www.github.com/elgw/dw_gui/
Description: dwgui
 A graphical user interface for the deconvolution
 program deconwolf (dw).
EOM

date > $name/DEBIAN/changelog

dpkg-deb --build $name
rm -rf $name
