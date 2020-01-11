OBJECTS = HANGUL.cit  HANJA.cit  Punct.cit  QJ.cit

SOURCES = HANGUL.tit  HANJA.tit  Punct.tit  QJ.tit

SUFFIXES = .cit .tit
.tit.cit:
	../../utils/tit2cit $< > $@

all: Makefile $(OBJECTS)

EXTRA_DIST = $(SOURCES) simple.lx
CLEANFILES = $(OBJECTS)
