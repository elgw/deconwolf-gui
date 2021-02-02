# Deconwolf GUI (dwgui)

A GUI for deconwolf (to be released) written in GTK 3.24.23.

## Build and Install (on Ubuntu)
For building this should be enough:

``` shell
# sudo apt-get install libgtk-3-dev
make

```
Once it is built, the gui can be launched by `./dw_gui` To install it system wide so that it is accessible from gnome, perform:

``` shell
sudo make install
```

## Usage notes:
 - Usage should be quite straight forward:
   - Drag and drop some `tif` files on the wolf.
   - Make sure that there is a configuration for each channel.
   - Select microscope in the list, or add a new one if your favorite is missing.
   - Choose how many threads to use etc.
   - Finally, save and launch.
 - Channels and microscopes are stored as ini files in the folder `XDG_CONFIG_HOME/deconwolf/`, on Ubuntu that would be `~/.config/deconwolf/`. Remember to hit the save button if you add or change anything that you want to make persistent.

## TODO
 - [ ] Change regexp for channel identification
 - [ ] Add to batch.
 - [ ] One-file simple mode?

### Future:
 - [ ] Set up a `./configure ; make ; make install` chain.
 - [ ] Always use the C locale?
 - [ ] Group files by folder
 - [ ] Menus
 - [ ] About dialog
 - [ ] Select multiple files to delete
 - [ ] Use a svg file for system icon.
 - [ ] Refactor alot.

### Done
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
