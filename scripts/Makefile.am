bin_SCRIPTS = cxterm
$(bin_SCRIPTS): cxterm.tpl genCxterm
	./genCxterm $(prefix) $(pkgdatadir)

cxtermrc:   cxtermrc.tpl genCxterm
	./genCxterm $(prefix) $(pkgdatadir)

install-data-local:
	$(INSTALL_DATA) cxtermrc $(DESTDIR)/$(pkgdatadir)

CLEANFILES = cxterm cxtermrc
EXTRA_DIST = cxtermrc.tpl cxterm.tpl genCxterm
