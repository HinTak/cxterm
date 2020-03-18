/*
 *	$Id: hzimctrl.c,v 1.2 2003/05/06 07:40:20 htl10 Exp $
 */

/***********************************************************
Copyright 1991,1994 by Yongguang Zhang.  All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of the authors not
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

THE AUTHORS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

******************************************************************/

#ifdef CXTERM_COMPATIBLE
# if (X11RELEASE <= 4)
#  if defined(__STDC__) || defined(_STDC_)
#   undef NOSTDHDRS
#  else
#   define NOSTDHDRS
#  endif
#  include "../Xcompat/Xosdefs.h"	/* for the Xos.h */
# endif
#endif

#ifdef __CYGWIN32__
#define linux
#define FOPEN_W		"wb"
#else
#define FOPEN_W		"w"
#endif


#ifdef __CYGWIN32__
#include <sys/types.h>
#else
#include <X11/Xos.h>
#endif

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>

#if defined(att) || (defined(SYSV) && defined(SYSV386))
#define ATT
#endif

#ifdef SVR4
#ifndef SYSV
#define SYSV
#endif
#define ATT
#endif

#ifdef ATT
#define USE_USG_PTYS
#endif

#ifdef APOLLO_SR9
#define CANT_OPEN_DEV_TTY
#endif

#ifdef linux
#define SYSV
#endif

#ifdef macII
#define USE_SYSV_TERMIO
#undef SYSV				/* pretend to be bsd */
#endif /* macII */

#ifdef SYSV
#define USE_SYSV_TERMIO
#endif /* SYSV */

#include <sys/ioctl.h>
#ifdef USE_SYSV_TERMIO
#ifdef linux
#include <sys/termios.h>
#else
#include <sys/termio.h>
#endif
#else /* else not USE_SYSV_TERMIO */
#include <sgtty.h>
#endif	/* USE_SYSV_TERMIO */

#ifdef SYSV
#ifdef USE_USG_PTYS
#include <sys/stream.h>
#ifndef SVR4
#include <sys/ptem.h>
#endif
#endif
#endif

#include <signal.h>

#ifdef SIGNALRETURNSINT
#define SIGNAL_T int
#else
#define SIGNAL_T void
#endif

#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#else
char *getenv();
#endif

#ifdef USE_SYSV_TERMIO
struct termio tioorig;
#else /* not USE_SYSV_TERMIO */
struct sgttyb sgorig;
#endif /* USE_SYSV_TERMIO */
int tty;
FILE *ttyfp;

SIGNAL_T onintr(int sig);

char *myname;
char *setdirs = "\033]160;%s\007";	/* esc seq for set hzinputdir */
char *setim = "\033]161;%s\007";	/* esc seq for set hz input method */
char *setparam = "\033]162;%s\007";	/* esc seq for set input param */

static void Usage(void);

/*
   set input method 
 */

int main (int argc, char **argv)
{
	register char *ptr;
#ifdef USE_SYSV_TERMIO
	struct termio tio;
#else /* not USE_SYSV_TERMIO */
	struct sgttyb sg;
#endif /* USE_SYSV_TERMIO */
	char buf[BUFSIZ];
	char *name_of_tty;
#ifdef CANT_OPEN_DEV_TTY
	extern char *ttyname();
#endif

myname = argv[0] ;
	ptr = strrchr(myname, '/');
	if (ptr)
	    myname = ptr + 1;

	buf[0] = '\0';
	argv++; argc--;
	if (argc < 1)
	    Usage();
	while (*argv) {
	    char str[BUFSIZ];
	    if (strcmp(*argv, "-d") == 0) {		/* hzinputdir */
		if (argc < 2)
		    Usage();	/* Never returns */
		argv++; argc--;
		sprintf (str, setdirs, *argv);
		strcat (buf, str);
	    } else if (strcmp(*argv, "-m") == 0) {	/* input method */
		if (argc < 2)
		    Usage();	/* Never returns */
		argv++; argc--;
		sprintf (str, setim, *argv++);
		strcat (buf, str);
	    } else if (strcmp(*argv, "-p") == 0) {	/* input param */
		if (argc < 2)
		    Usage();	/* Never returns */
		argv++; argc--;
		sprintf (str, setparam, *argv++);
		strcat (buf, str);
	    } else {
		Usage();	/* Never returns */
	    }
	    argv++; argc--;
	}

#ifdef CANT_OPEN_DEV_TTY
	if ((name_of_tty = ttyname(fileno(stderr))) == NULL) 
#endif
	  name_of_tty = "/dev/tty";

	if ((ttyfp = fopen (name_of_tty, FOPEN_W)) == NULL) {
	    fprintf (stderr, "%s:  can't open terminal %s\n",
		     myname, name_of_tty);
	    exit (1);
	}
	tty = fileno(ttyfp);

#ifdef USE_SYSV_TERMIO
	ioctl (tty, TCGETA, &tioorig);
	tio = tioorig;
	tio.c_iflag &= ~(ICRNL | IUCLC);
	tio.c_lflag &= ~(ICANON | ECHO);
	tio.c_cflag |= CS8;
	tio.c_cc[VMIN] = 6;
	tio.c_cc[VTIME] = 1;
#else	/* else not USE_SYSV_TERMIO */
 	ioctl (tty, TIOCGETP, &sgorig);
	sg = sgorig;
	sg.sg_flags |= RAW;
	sg.sg_flags &= ~ECHO;
#endif	/* USE_SYSV_TERMIO */
	signal(SIGINT, onintr);
	signal(SIGQUIT, onintr);
	signal(SIGTERM, onintr);
#ifdef USE_SYSV_TERMIO
	ioctl (tty, TCSETAW, &tio);
#else	/* not USE_SYSV_TERMIO */
	ioctl (tty, TIOCSETP, &sg);
#endif	/* USE_SYSV_TERMIO */

	write(tty, buf, strlen(buf));

#ifdef USE_SYSV_TERMIO
	ioctl (tty, TCSETAW, &tioorig);
#else	/* not USE_SYSV_TERMIO */
	ioctl (tty, TIOCSETP, &sgorig);
#endif	/* USE_SYSV_TERMIO */
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);

	exit(0);
}

static void Usage(void)
{
	fprintf(stderr,"Usage: %s [ -m input_method | -d HZINPUTDIR | -p parameters ]\n",
		myname);
	exit(1);
}

/* ARGSUSED */
SIGNAL_T
onintr(int sig)
{
#ifdef USE_SYSV_TERMIO
	ioctl (tty, TCSETAW, &tioorig);
#else	/* not USE_SYSV_TERMIO */
	ioctl (tty, TIOCSETP, &sgorig);
#endif	/* USE_SYSV_TERMIO */
	exit(1);
}
