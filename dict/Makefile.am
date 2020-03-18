SUBDIRS = gb big5 ks jis
install:
	for subd in $(SUBDIRS); do\
	    $(mkinstalldirs) $(DESTDIR)/$(pkgdatadir)/dict/$$subd;\
	    $(INSTALL_DATA) $$subd/*.cit $(DESTDIR)/$(pkgdatadir)/dict/$$subd;\
	    if test -f $$subd/simple.lx; then\
		$(INSTALL_DATA) $$subd/simple.lx $(DESTDIR)/$(pkgdatadir)/dict/$$subd;\
	    fi;\
	done
