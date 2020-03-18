OBJECTS = Punct.cit QJ.cit ROMKANA.cit TCODE.cit

SOURCES = Punct.tit QJ.tit ROMKANA.tit TCODE.tit

SUFFIXES = .cit .tit
.tit.cit:
	../../utils/tit2cit $< > $@

all: Makefile $(OBJECTS)

CLEANFILES = $(OBJECTS)
EXTRA_DIST = $(SOURCES)
