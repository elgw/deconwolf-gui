cflags=-march=x86-64 `pkg-config --cflags gtk+-3.0` -Wall
ldflags=`pkg-config --libs gtk+-3.0` -lm
cc=gcc

DEBUG?=0
ifeq ($(DEBUG),1)
cflags += -g3 -DDEBUG
else
cflags += -O3 -flto
endif

src=src/

dw_gui: resources dw_channel dw_scope dw_conf dw_file dw_colors
	$(cc) $(cflags) resources.c $(src)dw_app.c $(src)dw_app_window.c $(src)dw_gui.c dw_channel.o dw_scope.o dw_conf.o dw_file.o dw_colors.o $(ldflags) -o dw_gui

dw_channel:
	$(cc) -c $(cflags) $(src)dw_channel.c

dw_scope:
	$(cc) -c $(cflags) $(src)dw_scope.c

dw_conf:
	$(cc) -c $(cflags) $(src)dw_conf.c

dw_file:
	$(cc) -c $(cflags) $(src)dw_file.c

dw_colors:
	$(cc) -c $(cflags) $(src)dw_colors.c

resources: resources.c
	glib-compile-resources --target=resources.c --generate-source src/gresources.xml

clean:
	rm *.o
	rm dw_gui

install:
	cp dw_gui /usr/local/bin
	cp resources/deconwolf.desktop /usr/share/applications
	cp resources/deconwolf.png /usr/share/icons/hicolor/48x48/apps/

uninstall:
	rm /usr/local/bin/dw_gui
	rm /usr/share/applications/deconwolf.desktop
	rm /usr/share/icons/hicolor/48x48/apps/deconwolf.png
