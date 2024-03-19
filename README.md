# Deconwolf GUI (dwgui)

<img src="resources/screenshot_20210217.png"> A GUI for
[deconwolf](https://github.com/elgw/deconwolf)  written in GTK 3.24.23.

Usage should be quite forward:
1. Drag and drop some `tif` files on the wolf in the first tab.

2. Head over to the 'Files' tab to check that the channel names were
   identified automatically. By default it expects that files follow
   the convention **channel_###.tif** for example **dapi_001.tif**
   would be identified as an image stained by dapi.

3. Go the the tab __Channels__ and configure each channel, i.e. set
   the number of iterations per channel and specify the emission
   maxima. Hit the save button in the bottom.

4. Create/select the microscope that was used. Set the basic
   parameters.

5. Go to the __Deconwolf__ tab to set some options.

6. Go to __Run__. Inspect the commands that will be started and hit
   the "play" button to save this as a script and run.

## Build and Install

On Linux the following should be enough:

``` shell
sudo apt-get install libgtk-3-dev # At least on Ubuntu
mkdir build ; cd build ; cmake .. ; make
sudo cmake install
```

Once it is built, the gui can be launched by `dw_gui`. At least under
gnome you will find it when you press the __Super__ key and start
typing deconwolf...

For other platforms, see [INSTALL.md](INSTALL.md).

## Notes

Most likely there are bugs and they can only be fixed when they are known.
Please open a [new ticket](https://github.com/elgw/deconwolf/issues) if you
have any issues with the program.

Some changes are planned for the future, see the [TODO.md](TODO.md).

The gui plays well with [nd2tool](https://www.github.com/elgw/nd2tool)
which converts nd2 files (Nikon) to tif files. Can also generate
scripts for deconvolution with dw.
