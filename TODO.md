## TODO
 - [ ] Have a way to create batch jobs (append to another job script?).
 - [ ] A simpler dialog/mode for single files.
 - [ ] Create installers. [.dmg installer](https://mesonbuild.com/Creating-OSX-packages.html) file for OSX (or alternatively a [HomeBrew Formula](https://medium.com/@tharun208/creating-a-homebrew-formula-f76da25b79e4)) etc...
 - [ ] Refuse to create channels with empty alias.
 - [ ] Rescan the list of files to identify channel names again when the channel list is changed or the regexp is changed.
 - [ ] Allow to manually set the channel of multiple selected files.

### Future:
 - [ ] When regexp fails to parse a channel, the DwFile struct should be fully pupulated.
 - [ ] Update `meson.build` to prepare for system wide installation.
 - [ ] Group files by folder
 - [ ] Select multiple files to delete
 - [ ] Use a svg file for system icon.
 - [ ] Refactor, write tests, error handling etc.
 - [ ] Be consistent, use glib functions when possible.
 - [ ] Decouple the views from the data. As it is now the values are truncated when parsing from the TreeViews.

### Done
 - [x] Tries to figure out the channel name from the available channel aliases when the regexp does not match.
 - [x] Allow custom regexp for channel identification. However this setting is just temporary.
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

Debug on linux
``` shell
make DEBUG=1 -B
G_DEBUG=fatal_warnings ./dw_gui
coredumpctl gdb -1
# then bt or whatever
```

Debug on mac

``` shell
make DEBUG=1 -B
lldb ./dw_gui
# in lldb, set up break points ...
b malloc_error_break
run
# and if it crashes
bf
```
