# dwgui (GTK)

A GUI for deconwolf written in GTK 3.24.23.

## Build
``` shell
make
make install
```
## TODO
 - [ ] Parse lists to generate command list
 - [ ] Monospace font in command list.
 - [ ] Run/Progress monitor

 ## Future:
 - [ ] Group files by folder
 - [ ] Add channels
 - [ ] Add microscope window
 - [ ] Menus
 - [ ] About dialog
 - [ ] [GkeyFile](https://developer.gnome.org/glib/stable/glib-Key-value-file-parser.html#g-key-file-new) for settings.
 - [ ] Select multiple files to delete
 - [ ] Use a svg file for icon.

 ## LOG
 - [x] Add microscope icon, found one using `gtk-3-icon-browser`
 - [x] Set radix character to be '.'
 - [x] Delete items from lists
 - [x] Parser for channel name
 - [x] Make file list scrollable
 - [x] Microscope tab
 - [x] Deconwolf tab
 - [X] Use GResource to include image data in binary.
 - [x] Drag and drop. Note that drag and drop from the 'Desktop' on Gnome might crash the whole system.
 - [x] A tree-view for file list
