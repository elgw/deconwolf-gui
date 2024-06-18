# TODO

## Planned
- [ ] Create installer for MacOS
 [.dmg installer](https://mesonbuild.com/Creating-OSX-packages.html)
 file for OSX (or alternatively a [HomeBrew Formula](https://medium.com/@tharun208/creating-a-homebrew-formula-f76da25b79e4)) etc...

- [ ] Have a way to create batch jobs (append to another job script?).
- [ ] A simpler dialog/mode for single files.

- [ ] Refuse to create channels with empty alias.
- [ ] Rescan the list of files to identify channel names again when
 the channel list is changed or the regexp is changed.
- [ ] Allow to manually set the channel of multiple selected files.
- [ ] Detect what deconwolf version is running and populate options accordingly.


## Future
- [ ] Switch to sqlite for data storage.
- [ ] When regexp fails to parse a channel, the DwFile struct should
       be fully populated.
- [ ] Group files by folder
- [ ] Select multiple files to delete
- [ ] Use a svg file for system icon.
- [ ] Refactor, write tests, error handling etc.
- [ ] Be consistent, use glib functions when possible.
- [ ] Decouple the views from the data. As it is now the values are
       truncated when parsing from the TreeViews.

# Development hints

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
