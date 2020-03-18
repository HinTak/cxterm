SOURCES = \
    CCDOSPY.tit CTLau.tit CangJie.tit English.tit HIRAGANA.tit KATAKANA.tit\
    PY.tit Punct.tit QJ.tit QianMa.tit SW.tit TONEPY.tit\
    TeleCode.tit WuBi.tit UCDOSPY.tit

OBJECTS =\
    CCDOSPY.cit CTLau.cit CangJie.cit English.cit HIRAGANA.cit KATAKANA.cit\
    PY.cit Punct.cit QJ.cit QianMa.cit SW.cit TONEPY.cit\
    TeleCode.cit WuBi.cit UCDOSPY.cit

SUFFIXES = .cit .tit
.tit.cit:
	../../utils/tit2cit $< > $@

all: Makefile $(OBJECTS)

CLEANFILES = $(OBJECTS)
EXTRA_DIST = $(SOURCES) simple.lx

