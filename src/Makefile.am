## Process this file with automake to produce Makefile.in

## Created by Anjuta

glade_images = gseptica-icon.png

gladedir = $(datadir)/gseptica/glade
glade_DATA = $(glade_images) gseptica.glade

AM_CPPFLAGS = \
	-DLOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DSRC_DIR=\""$(srcdir)"\" \
	-DDATA_DIR=\""$(datadir)"\" \
	$(GSEPTICA_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = gseptica

gseptica_SOURCES = \
	main.cpp \
	mainwindow.h \
	mainwindow.cpp \
	game.cpp \
	game.h \
	ai.cpp \
	ai.h \
	defines.h

gseptica_LDFLAGS = 

if WIN32
gseptica_LDFLAGS += \
	-Wl,--enable-runtime-pseudo-reloc -mwindows resource.res
else
gseptica_LDFLAGS += \
	-Wl,--export-dynamic
endif


if WIN32
resource.res: resource.rc
	$(WINDRES) -i resource.rc --input-format=rc -o resource.res -O coff
BUILT_SOURCES = resource.res
endif


gseptica_LDADD = $(GSEPTICA_LIBS)

EXTRA_DIST = $(glade_DATA) \
	resource.rc \
	gseptica-icon.ico


# Remove ui directory on uninstall
uninstall-local:
	-rm -r $(gladedir)
