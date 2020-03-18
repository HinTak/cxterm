bin_PROGRAMS = cit2tit tit2cit hzimctrl

INCLUDES = -I. -I.. -I../cxterm
cit2tit$(EXEEXT): cit2tit.c ../cxterm/HZutil.o
	gcc $(INCLUDES) -c cit2tit.c
	gcc $(LIBS) -o $@ cit2tit.o ../cxterm/HZutil.o

tit2cit$(EXEEXT): tit2cit.c ../cxterm/HZutil.o
	gcc $(INCLUDES) -c tit2cit.c
	gcc $(LIBS) -o $@ tit2cit.o ../cxterm/HZutil.o

hzimctrl_SOURCES = hzimctrl.c
man_MANS = cit2tit.1 tit2cit.1 hzimctrl.1
EXTRA_DIST = cit2tit.1 tit2cit.1 hzimctrl.1

