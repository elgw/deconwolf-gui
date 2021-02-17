# Deconwolf GUI (dwgui)

![screenshot](resources/screenshot_20210217.png)
A GUI for deconwolf (to be released) written in GTK 3.24.23.

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

## TODO
 - [ ] Allow custom regexp for channel identification
 - [ ] Have a way to create batch jobs (append to another job script?).
 - [ ] A simpler dialog/mode for single files.
 - [ ] Create installers. [.dmg installer](https://mesonbuild.com/Creating-OSX-packages.html) file for OSX (or alternatively a [HomeBrew Formula](https://medium.com/@tharun208/creating-a-homebrew-formula-f76da25b79e4)) etc...

### Future:
 - [ ] When regexp fails to parse a channel, the DwFile struct should be fully pupulated.
 - [ ] Update `meson.build` to prepare for system wide installation.
 - [ ] Group files by folder
 - [ ] Select multiple files to delete
 - [ ] Use a svg file for system icon.
 - [ ] Refactor, write tests, error handling etc.
 - [ ] Be consistent, use glib functions when possible.

### Done
 - [x] 'Add files' - button and 'Remove file' - button.
 - [x] Warn when `dw` can't be found.
 - [x] About dialog.
 - [x] Enable drag and drop also on the file list.
 - [x] Suggest to save the command file in the folder of the first image.
 - [x] Warn when channel is missing.
 - [x] Sets the c-locale (makes sense since the GUI is en English)
 - [x] Set number of threads and tilesize (dw)
 - [x] Save and load microscopes.
 - [x] [GkeyFile](https://developer.gnome.org/glib/stable/glib-Key-value-file-parser.html#g-key-file-new) for settings.
 - [x] Parse lists to generate command list
 - [x] Monospace font in command list.
 - [x] Add microscope icon, found one using `gtk3-icon-browser`
 - [x] Set radix character to be '.' in the command window
 - [x] Delete items from lists
 - [x] Parser for channel name
 - [x] Make file list scrollable
 - [x] Microscope tab
 - [x] Deconwolf tab
 - [X] Use GResource to include image data in binary.
 - [x] Drag and drop. Note that drag and drop from the 'Desktop' on Gnome might crash the whole system.
 - [x] A tree-view for file list

## Useful:

Interactive debugging with gtk
``` shell
GTK_DEBUG=interactive ./dw_gui
```
