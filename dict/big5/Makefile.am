OBJECTS =\
    4Corner.cit  Boshiamy.cit  ETZY.cit     HSU.cit  Punct.cit  Simplex.cit\
    ARRAY30.cit  CangJie.cit   English.cit  PY.cit   QJ.cit     ZOZY.cit

SOURCES =\
    4Corner.tit  Boshiamy.tit  ETZY.tit     HSU.tit  Punct.tit  Simplex.tit\
    ARRAY30.tit  CangJie.tit   English.tit  PY.tit   QJ.tit     ZOZY.tit

SUFFIXES = .cit .tit
.tit.cit:
	../../utils/tit2cit $< > $@

all: Makefile $(OBJECTS)

CLEANFILES = $(OBJECTS)
EXTRA_DIST = $(SOURCES) simple.lx
