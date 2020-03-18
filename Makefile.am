AUTOMAKE_OPTIONS = foreign
SUBDIRS = cxterm utils dict fonts scripts

install-fonts:
	cd fonts && make install-fonts

uninstall-local:
	rm -r $(pkgdatadir)

EXTRA_DIST = Doc Xcompat
