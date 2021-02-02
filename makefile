cflags=`pkg-config --cflags gtk+-3.0` -Wall
ldflags=`pkg-config --libs gtk+-3.0` -lm
cc=gcc

DEBUG?=0
ifeq ($(DEBUG),1)
cflags += -g3 -DDEBUG
else
cflags += -O3 -flto
endif

src=src/

dw_gui: resources dw_channel dw_scope dw_conf
	$(cc) $(cflags) resources.c $(src)dw_app.c $(src)dw_app_window.c $(src)dw_gui.c $(ldflags) dw_channel.o dw_scope.o dw_conf.o -o dw_gui

dw_channel:
	$(cc) -c $(cflags) $(src)dw_channel.c

dw_scope:
	$(cc) -c $(cflags) $(src)dw_scope.c

dw_conf:
	$(cc) -c $(cflags) $(src)dw_conf.c

resources:
	glib-compile-resources --target=resources.c --generate-source src/gresources.xml


install:
	cp dw_gui /usr/local/bin
	cp resources/deconwolf.desktop /usr/share/applications
	cp resources/deconwolf.png /usr/share/icons/hicolor/48x48/apps/
