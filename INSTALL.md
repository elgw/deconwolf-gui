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
Not tested yet. If you want it, or knows how to build it, please [let
the world know](https://github.com/elgw/deconwolf/issues).
