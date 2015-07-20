
appicondir = $(datadir)/pixmaps
appicon_DATA = gseptica.png

desktopdir = $(datadir)/applications
desktop_in_files=gseptica.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

man_MANS = gseptica.1


cardimagedir = $(datadir)/gseptica
cardimage_DATA = cards.png

helpdir = $(datadir)/gseptica/help
help_DATA = help.html

EXTRA_DIST = \
	$(desktop_in_files) \
	$(appicon_DATA) \
	$(man_MANS) \
	$(cardimage_DATA) \
	$(help_DATA)
	
CLEANFILES = \
	$(desktop_DATA)

uninstall-local:
	-rm -r $(helpdir)
	-rm -r $(cardimagedir)

