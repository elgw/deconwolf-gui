cc=gcc

cflags= -std=c11 -Wfatal-errors
ldflags=-flto


DEBUG?=0
ifeq ($(DEBUG),1)
cflags += -g3
else
cflags += -O3
endif

# math library
ldflags=-lm

# GTK3.0
#cflags+=`pkg-config --cflags gtk+-3.0`
#ldflags+=`pkg-config --libs gtk+-3.0`

# GTK 4
cflags+=`pkg-config --cflags gtk4`
ldflags+=`pkg-config --libs gtk4`


dw_gui_files=src/dw_gui.c \
dw_app.o \
dw_app_window.o \
resources.c \
dw_channel.o \
dw_scope.o \
dw_conf.o \
dw_file.o \
dw_colors.o \
dw_app_runner.o \
common.o

dw_gui: $(dw_gui_files)
	$(cc) $(cflags) $(dw_gui_files) $(ldflags) -o dw_gui

%.o: src/%.c
	$(cc) -c $(cflags) $<

resources.c:
	glib-compile-resources --target=resources.c --generate-source src/gresources.xml

clean:
	rm *.o
	rm dw_gui
	rm resources.c

install_linux:
	cp dw_gui /usr/local/bin
	cp resources/deconwolf.desktop /usr/share/applications/
	cp resources/deconwolf.png /usr/share/icons/hicolor/48x48/apps/

uninstall_linux:
	rm /usr/local/bin/dw_gui
	rm /usr/share/applications/deconwolf.desktop
	rm /usr/share/icons/hicolor/48x48/apps/deconwolf.png
