cflags=`pkg-config --cflags gtk+-3.0` -Wall
ldflags=`pkg-config --libs gtk+-3.0`
cc=gcc

DEBUG?=0
ifeq ($(DEBUG),1)
cflags += -g3 -DDEBUG
else
cflags += -O3 -flto
endif

dw_gui: resources dw_channel
	$(cc) $(cflags)  resources.c dw_app.c dw_app_window.c dw_gui.c $(ldflags) dw_channel.o -o dw_gui

dw_channel:
	$(cc) -c $(cflags) dw_channel.c

resources:
	glib-compile-resources --target=resources.c --generate-source gresources.xml


install:
	cp dw_gui /usr/local/bin
	cp deconwolf.desktop /usr/share/applications
	cp deconwolf.png /usr/share/icons/hicolor/48x48/apps/
