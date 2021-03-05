# Deconwolf GUI (dwgui)

<img src="resources/screenshot_20210217.png">
A GUI for deconwolf (to be released) written in GTK 3.24.23.

See the [TODO](TODO.md) for current status.

## Build and Install

### On Ubuntu
For building this should be enough:

``` shell
# sudo apt-get install libgtk-3-dev
make
```
Once it is built, the gui can be launched by `./dw_gui` To install it system wide so that it is accessible from gnome, perform:

``` shell
sudo make install
# to remove
# sudo make uninstall
```

or, if you prefer to install it as a package:

``` shell
./deb_make.sh
sudo dpkg -i dwgui*deb
# to remove
# sudo dpkg -r dwgui
```

### OSX
To get the dependencies, either follow [the recommended way](https://wiki.gnome.org/action/show/Projects/GTK/OSX/Building?action=show&redirect=Projects%2FGTK%2B%2FOSX%2FBuilding) to install the Gtk3 libraries or use [brew](https://brew.sh/):
``` shell
brew update
brew upgrade
brew install gtk+3
brew install adwaita-icon-theme
```
To build, either do:
``` shell
meson builddir
cd builddir
ninja
```
or

``` shell
make -B
```

## Usage notes:
 - Usage should be quite straight forward:
   - Drag and drop some `tif` files on the wolf.
   - Make sure that there is a configuration for each channel.
   - Select microscope in the list, or add a new one if your favorite is missing.
   - Choose how many threads to use etc.
   - Finally, save and launch.
 - Channels and microscopes are stored as ini files in the folder `XDG_CONFIG_HOME/deconwolf/`, on Ubuntu that would be `~/.config/deconwolf/`. Remember to hit the save button if you add or change anything that you want to make persistent.
 - It is actually possible to use your own PSFs since the PSF generator by default does not overwrite existing files. To use your custom PSF, just be ahead of deconwolf and place your PSFs where deconwolf plans to create its own PSFs.
