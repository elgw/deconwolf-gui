# Installation notes

## Linux

The cmake file should work on most platforms. If not, please open a
[new ticket](https://github.com/elgw/deconwolf/issues).

As an alternative installation procedure, packages can be built for
most common package managers, see `cpack --help`. Example:

``` shell
sudo apt-get install libgtk-3-dev # At least on Ubuntu
mkdir build ; cd build ; cmake .. ; make
cpack -G DEB # To create a debian .deb file
```

## OSX
To get the dependencies, either follow [the recommended
way](https://wiki.gnome.org/action/show/Projects/GTK/OSX/Building?action=show&redirect=Projects%2FGTK%2B%2FOSX%2FBuilding)
to install the Gtk3 libraries or use [brew](https://brew.sh/):

``` shell
brew update
brew upgrade
brew install gtk+3
brew install adwaita-icon-theme
```

Then follow the same procedure as on linux.

## Windows
> [!TIP]
! No need to do this. Download an installer from [releases](https://! github.com/elgw/deconwolf-gui/releases).

First, set up Visual Studio with clang and get cmake.

Download the latest release of [gsvbuild](https://github.com/wingtk/gvsbuild)
and unpack to `c:\gtk`. Add the following environmental variables:

``` shell
$env:Path = "C:\gtk\bin;" + $env:Path
$env:LIB = "C:\gtk\lib;" + $env:LIB
$env:INCLUDE = "C:\gtk\include;C:\gtk\include\cairo;C:\gtk\include\glib-2.0;C:\gtk\include\gobject-introspection-1.0;C:\gtk\lib\glib-2.0\include;" + $env:INCLUDE
```

Clone the repository and create a visual studio project
``` shell
mkdir build
cd build
cmake .. -T ClangCL -A x64
```

The program will only start when the files gtk are placed int the same folder.

To create an installer, first set up the file structure according to [https://www.datatable.online/en/blog/002-how-to-deploy-gtk-app-on-windows.html#background]. Then copy `src/create_dwgui_installer.nsis` into that folder and compile the script with NSIS.

