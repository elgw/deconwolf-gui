# dwgui (GTK)

A GUI for deconwolf written in GTK 3.24.23.

## Build
``` shell
make
make install
```

## Usage notes:
 - Channels and microscopes are stored as ini files in the folder `XDG_CONFIG_HOME/deconwolf/`, on Ubuntu that would be `~/.config/deconwolf/`.

## TODO

 - [ ] Change regexp for channel identification
 - [ ] Set number of threads and tilesize (dw)

 ## Future:
 - [ ] Always use the C locale?
 - [ ] Group files by folder
 - [ ] Menus
 - [ ] About dialog
 - [ ] Select multiple files to delete
 - [ ] Use a svg file for system icon.
 - [ ] Refactor alot.

 ## Done
 - [x] Save and load microscopes.
 - [x] [GkeyFile](https://developer.gnome.org/glib/stable/glib-Key-value-file-parser.html#g-key-file-new) for settings.
 - [x] Parse lists to generate command list
 - [x] Monospace font in command list.
 - [x] Add microscope icon, found one using `gtk-3-icon-browser`
 - [x] Set radix character to be '.' in the command window
 - [x] Delete items from lists
 - [x] Parser for channel name
 - [x] Make file list scrollable
 - [x] Microscope tab
 - [x] Deconwolf tab
 - [X] Use GResource to include image data in binary.
 - [x] Drag and drop. Note that drag and drop from the 'Desktop' on Gnome might crash the whole system.
 - [x] A tree-view for file list
