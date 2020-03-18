OBJECTS =  gb16st.pcf.gz hku16et.pcf.gz gb16fs.pcf.gz

SUFFIXES = .pcf.gz .bdf
all: $(OBJECTS)

.bdf.pcf.gz:
	bdftopcf -t $< | gzip > $@
install-fonts:
	$(mkinstalldirs) $(DESTDIR)/$(pkgdatadir)/fonts
	$(INSTALL_DATA)  $(OBJECTS) fonts.dir fonts.alias $(DESTDIR)/$(pkgdatadir)/fonts

CLEANFILES = $(OBJECTS)
EXTRA_DIST = fonts.dir fonts.alias gb16st.bdf hku16et.bdf gb16fs.bdf
