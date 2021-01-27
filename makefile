cflags=`pkg-config --cflags gtk+-3.0`
ldflags=`pkg-config --libs gtk+-3.0`
cc=gcc

dw_gui: resources
	$(cc) $(cflags)  resources.c dw_app.c dw_app_window.c dw_gui.c $(ldflags) -o dw_gu

resources:
	glib-compile-resources --target=resources.c --generate-source gresources.xml


install:
	cp dw_gui /usr/local/bin
	cp deconwolf.desktop /usr/share/applications
	cp deconwolf.png /usr/share/icons/hicolor/48x48/apps/
