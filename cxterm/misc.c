/* $Id: misc.c,v 1.2 2003/05/06 07:40:19 htl10 Exp $ */
/***********************************************************************
* Copyright 1994 by Yongguang Zhang.
* Copyright 1990, 1991, 1992 by Yongguang Zhang and Pong Man-Chi.
*
* All rights reserved.  Under the same copyright and permission term as
* the original.  Absolutely no warranties of any kinds.
************************************************************************/

/*
 *	$XConsortium: misc.c,v 1.102 94/03/28 18:27:08 gildea Exp $
 */

/*
 * Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.
 *
 *                         All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 *
 *
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#include "ptyx.h"		/* X headers included here. */

#include <X11/Xos.h>
#ifdef CXTERM_COMPATIBLE
# if X11RELEASE <= 5		/* X11R6 =>  <X11/Xos.h> */
#  if defined(SVR4) || defined(VMS) || defined(WIN32)
#   define X_GETTIMEOFDAY(t) gettimeofday(t)
#  else
#   define X_GETTIMEOFDAY(t) gettimeofday(t, (struct timezone*)0)
#  endif
# endif
#endif
#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <ctype.h>
#include <pwd.h>
#include <errno.h>
#include <string.h>

#include <X11/Xatom.h>
#include <X11/cursorfont.h>

#include <X11/Shell.h>
#include <X11/Xmu/Error.h>
#include <X11/Xmu/SysUtil.h>
#include <X11/Xmu/WinUtil.h>

#include "data.h"
#include "error.h"
#include "menu.h"

extern jmp_buf Tekend;
extern jmp_buf VTend;

#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#else
extern char *malloc();
extern char *getenv();
#endif

static void DoSpecialEnterNotify(register XEnterWindowEvent *ev);
static void DoSpecialLeaveNotify(register XEnterWindowEvent *ev);

void selectwindow (register TScreen *screen, register int flag);
void unselectwindow (register TScreen *screen, register int flag);
void VisualBell (void);
void Changename (register char *name);
void Changetitle (register char *name);
void Cleanup (int code);



extern void FlushScroll (register TScreen *screen);
extern void recolor_cursor (Cursor cursor, long unsigned int fg, long unsigned int bg);
extern void Input (register TKeyboard *keyboard, register TScreen *screen, register XKeyEvent *event, int eightbit);
extern void StringInput (register TScreen *screen, register char *string, int nbytes);
extern void ReverseVideo (XtermWidget termw);
extern void TCursorToggle (int toggle);
extern void HideCursor (void);
extern void ShowCursor (void);
extern void TekExpose (Widget w, XEvent *event, Region region);
extern void SetVTFont (int i, int doresize, char *name1, char *name2);
extern void SetHZinDir (XtermWidget xw, String dir);
extern void SetHZinMethod (XtermWidget xw, String name, int doredraw);
extern void SetHZinParam (char *str);
extern void GetColors (XtermWidget term, ScrnColors *pColors);
extern void ChangeColors (XtermWidget term, ScrnColors *pNew);
extern void Exit (int n);
extern int VTInit (void);
extern int TekInit (void);
extern void dorefresh (void);

void xevents(void)
{
	XEvent event;
	register TScreen *screen = &term->screen;
	extern XtAppContext app_con;

	if(screen->scroll_amt)
		FlushScroll(screen);
	if (!XPending (screen->display))
	    /* protect against events/errors being swallowed by us or Xlib */
	    return;
	do {
		if (waitingForTrackInfo)
			return;
		XNextEvent (screen->display, &event);
		/*
		 * Hack to get around problems with the toolkit throwing away
		 * eventing during the exclusive grab of the menu popup.  By
		 * looking at the event ourselves we make sure that we can
		 * do the right thing.
		 */
		if(event.type == EnterNotify &&
		   (event.xcrossing.window == XtWindow(XtParent(term)) ||
		    (tekWidget &&
		     event.xcrossing.window == XtWindow(XtParent(tekWidget)))))
		  DoSpecialEnterNotify (&event.xcrossing);
		else 
		if(event.type == LeaveNotify &&
		   (event.xcrossing.window == XtWindow(XtParent(term)) ||
		    (tekWidget &&
		     event.xcrossing.window == XtWindow(XtParent(tekWidget)))))
		  DoSpecialLeaveNotify (&event.xcrossing);

		if (!event.xany.send_event ||
		    screen->allowSendEvents ||
		    ((event.xany.type != KeyPress) &&
		     (event.xany.type != KeyRelease) &&
		     (event.xany.type != ButtonPress) &&
		     (event.xany.type != ButtonRelease)))
		    XtDispatchEvent(&event);
	} while (QLength(screen->display) > 0);
}


Cursor make_colored_cursor (int cursorindex, long unsigned int fg, long unsigned int bg)
	                			/* index into font */
	                     			/* pixel value */
{
	register TScreen *screen = &term->screen;
	Cursor c;
	register Display *dpy = screen->display;
	
	c = XCreateFontCursor (dpy, cursorindex);
	if (c == (Cursor) 0) return (c);

	recolor_cursor (c, fg, bg);
	return (c);
}

/* ARGSUSED */
void HandleKeyPressed(Widget w, XEvent *event, String *params, Cardinal *nparams)
{
    register TScreen *screen = &term->screen;

#ifdef ACTIVEWINDOWINPUTONLY
    if (w == (screen->TekEmu ? (Widget)tekWidget : (Widget)term))
#endif
	Input (&term->keyboard, screen, &event->xkey, False);
}
/* ARGSUSED */
void HandleEightBitKeyPressed(Widget w, XEvent *event, String *params, Cardinal *nparams)
{
    register TScreen *screen = &term->screen;

#ifdef ACTIVEWINDOWINPUTONLY
    if (w == (screen->TekEmu ? (Widget)tekWidget : (Widget)term))
#endif
	Input (&term->keyboard, screen, &event->xkey, True);
}

/* ARGSUSED */
void HandleStringEvent(Widget w, XEvent *event, String *params, Cardinal *nparams)
{
    register TScreen *screen = &term->screen;

#ifdef ACTIVEWINDOWINPUTONLY
    if (w != (screen->TekEmu ? (Widget)tekWidget : (Widget)term)) return;
#endif

    if (*nparams != 1) return;

    if ((*params)[0] == '0' && (*params)[1] == 'x' && (*params)[2] != '\0') {
	char c, *p, hexval[2];
	hexval[0] = hexval[1] = 0;
	for (p = *params+2; (c = *p); p++) {
	    hexval[0] *= 16;
	    if (isupper(c)) c = tolower(c);
	    if (c >= '0' && c <= '9')
		hexval[0] += c - '0';
	    else if (c >= 'a' && c <= 'f')
		hexval[0] += c - 'a' + 10;
	    else break;
	}
	if (c == '\0')
	    StringInput (screen, hexval, 1);
    }
    else {
	StringInput (screen, *params, strlen(*params));
    }
}

static void DoSpecialEnterNotify (register XEnterWindowEvent *ev)
{
    register TScreen *screen = &term->screen;

#ifdef ACTIVEWINDOWINPUTONLY
    if (ev->window == XtWindow(XtParent(screen->TekEmu ?
					(Widget)tekWidget : (Widget)term)))
#endif
      if (((ev->detail) != NotifyInferior) &&
	  ev->focus &&
	  !(screen->select & FOCUS))
	selectwindow(screen, INWINDOW);
}

/*ARGSUSED*/
void HandleEnterWindow(Widget w, caddr_t eventdata, register XEnterWindowEvent *event)
{
    /* NOP since we handled it above */
}


static void DoSpecialLeaveNotify (register XEnterWindowEvent *ev)
{
    register TScreen *screen = &term->screen;

#ifdef ACTIVEWINDOWINPUTONLY
    if (ev->window == XtWindow(XtParent(screen->TekEmu ?
					(Widget)tekWidget : (Widget)term)))
#endif
      if (((ev->detail) != NotifyInferior) &&
	  ev->focus &&
	  !(screen->select & FOCUS))
	unselectwindow(screen, INWINDOW);
}


/*ARGSUSED*/
void HandleLeaveWindow(Widget w, caddr_t eventdata, register XEnterWindowEvent *event)
{
    /* NOP since we handled it above */
}


/*ARGSUSED*/
void HandleFocusChange(Widget w, caddr_t eventdata, register XFocusChangeEvent *event)
{
        register TScreen *screen = &term->screen;

        if(event->type == FocusIn)
                selectwindow(screen,
			     (event->detail == NotifyPointer) ? INWINDOW :
								FOCUS);
        else {
                unselectwindow(screen,
			       (event->detail == NotifyPointer) ? INWINDOW :
								  FOCUS);
		if (screen->grabbedKbd && (event->mode == NotifyUngrab)) {
		    XBell(screen->display, 100);
		    ReverseVideo(term);
		    screen->grabbedKbd = FALSE;
		    update_securekbd();
		}
	}
}



void selectwindow(register TScreen *screen, register int flag)
{
	if(screen->TekEmu) {
		if(!Ttoggled)
			TCursorToggle(TOGGLE);
		screen->select |= flag;
		if(!Ttoggled)
			TCursorToggle(TOGGLE);
		return;
	} else {
		if(screen->cursor_state &&
		   (screen->cursor_col != screen->cur_col ||
		    screen->cursor_row != screen->cur_row))
		    HideCursor();
		screen->select |= flag;
		if(screen->cursor_state)
			ShowCursor();
		return;
	}
}

void unselectwindow(register TScreen *screen, register int flag)
{
    if (screen->always_highlight) return;

    if(screen->TekEmu) {
	if(!Ttoggled) TCursorToggle(TOGGLE);
	screen->select &= ~flag;
	if(!Ttoggled) TCursorToggle(TOGGLE);
    } else {
	screen->select &= ~flag;
	if(screen->cursor_state &&
	   (screen->cursor_col != screen->cur_col ||
	    screen->cursor_row != screen->cur_row))
	      HideCursor();
	if(screen->cursor_state)
	  ShowCursor();
    }
}

static long lastBellTime;	/* in milliseconds */

void Bell(void)
{
    extern XtermWidget term;
    register TScreen *screen = &term->screen;
    struct timeval curtime;
    long now_msecs;

    /* has enough time gone by that we are allowed to ring
       the bell again? */
    if(screen->bellSuppressTime) {
	if(screen->bellInProgress) {
	    if (QLength(screen->display) > 0 ||
		GetBytesAvailable (ConnectionNumber(screen->display)) > 0)
		xevents();
	    if(screen->bellInProgress) { /* even after new events? */
		return;
	    }
	}
#if defined(sun)
	gettimeofday(&curtime, NULL);
#else
	X_GETTIMEOFDAY(&curtime);
#endif
	now_msecs = 1000*curtime.tv_sec + curtime.tv_usec/1000;
	if(lastBellTime != 0  &&  now_msecs - lastBellTime >= 0  &&
	   now_msecs - lastBellTime < screen->bellSuppressTime) {
	    return;
	}
	lastBellTime = now_msecs;
    }

    if (screen->visualbell)
	VisualBell();
    else
	XBell(screen->display, 0);

    if(screen->bellSuppressTime) {
	/* now we change a property and wait for the notify event to come
	   back.  If the server is suspending operations while the bell
	   is being emitted (problematic for audio bell), this lets us
	   know when the previous bell has finished */
	Widget w = screen->TekEmu ? (Widget) tekWidget : (Widget) term;
	XChangeProperty(XtDisplay(w), XtWindow(w),
			XA_NOTICE, XA_NOTICE, 8, PropModeAppend, NULL, 0);
	screen->bellInProgress = TRUE;
    }
}


void VisualBell(void)
{
    extern XtermWidget term;
    register TScreen *screen = &term->screen;
    register Pixel xorPixel = screen->foreground ^ term->core.background_pixel;
    XGCValues gcval;
    GC visualGC;

    gcval.function = GXxor;
    gcval.foreground = xorPixel;
    visualGC = XtGetGC((Widget)term, GCFunction+GCForeground, &gcval);
    if(screen->TekEmu) {
	XFillRectangle(
		       screen->display,
		       TWindow(screen), 
		       visualGC,
		       0, 0,
		       (unsigned) TFullWidth(screen),
		       (unsigned) TFullHeight(screen));
	XFlush(screen->display);
	XFillRectangle(
		       screen->display,
		       TWindow(screen), 
		       visualGC,
		       0, 0,
		       (unsigned) TFullWidth(screen),
		       (unsigned) TFullHeight(screen));
    } else {
	XFillRectangle(
		       screen->display,
		       VWindow(screen), 
		       visualGC,
		       0, 0,
		       (unsigned) FullWidth(screen),
		       (unsigned) FullHeight(screen));
	XFlush(screen->display);
	XFillRectangle(
		       screen->display,
		       VWindow(screen), 
		       visualGC,
		       0, 0,
		       (unsigned) FullWidth(screen),
		       (unsigned) FullHeight(screen));
    }
}

/* ARGSUSED */
void HandleBellPropertyChange(Widget w, XtPointer data, XEvent *ev, Boolean *more)
{
    register TScreen *screen = &term->screen;

    if (ev->xproperty.atom == XA_NOTICE) {
	screen->bellInProgress = FALSE;
    }
}

void Redraw(void)
{
	extern XtermWidget term;
	register TScreen *screen = &term->screen;
	XExposeEvent event;

	event.type = Expose;
	event.display = screen->display;
	event.x = 0;
	event.y = 0;
	event.count = 0; 
	
	if(VWindow(screen)) {
	        event.window = VWindow(screen);
		event.width = term->core.width;
		event.height = term->core.height;
		(*term->core.widget_class->core_class.expose)((Widget)term, (XEvent *)&event, NULL);
		if(screen->scrollbar) 
			(*screen->scrollWidget->core.widget_class->core_class.expose)(screen->scrollWidget, (XEvent *)&event, NULL);
		}

	if(TWindow(screen) && screen->Tshow) {
	        event.window = TWindow(screen);
		event.width = tekWidget->core.width;
		event.height = tekWidget->core.height;
		TekExpose ((Widget)tekWidget,(XEvent *)&event, NULL);
	}
}

#if defined(ALLOWLOGGING) || defined(DEBUG)

#ifndef X_NOT_POSIX
#define HAS_WAITPID
#endif

/*
 * create a file only if we could with the permissions of the real user id.
 * We could emulate this with careful use of access() and following
 * symbolic links, but that is messy and has race conditions.
 * Forking is messy, too, but we can't count on setreuid() or saved set-uids
 * being available.
 */
void
creat_as(uid, gid, pathname, mode)
    int uid;
    int gid;
    char *pathname;
    int mode;
{
    int fd;
    int waited;
    int pid;
#ifndef HAS_WAITPID
    int (*chldfunc)();

    chldfunc = signal(SIGCHLD, SIG_DFL);
#endif
    pid = fork();
    switch (pid)
    {
    case 0:			/* child */
	setgid(gid);
	setuid(uid);
	fd = open(pathname, O_WRONLY|O_CREAT|O_APPEND, mode);
	if (fd >= 0) {
	    close(fd);
	    _exit(0);
	} else
	    _exit(1);
    case -1:			/* error */
	return;
    default:			/* parent */
#ifdef HAS_WAITPID
	waitpid(pid, NULL, 0);
#else
	waited = wait(NULL);
	signal(SIGCHLD, chldfunc);
	/*
	  Since we had the signal handler uninstalled for a while,
	  we might have missed the termination of our screen child.
	  If we can check for this possibility without hanging, do so.
	*/
	do
	    if (waited == term->screen.pid)
		Cleanup(0);
	while ( (waited=nonblocking_wait()) > 0);
#endif
    }
}
#endif

#ifdef ALLOWLOGGING
/*
 * logging is a security hole, since it allows a setuid program to
 * write arbitrary data to an arbitrary file.  So it is disabled
 * by default.
 */ 

StartLog(screen)
register TScreen *screen;
{
	register char *cp;
	register int i;
	static char *log_default;
#ifdef ALLOWLOGFILEEXEC
	void logpipe();
#ifdef SYSV
	/* SYSV has another pointer which should be part of the
	** FILE structure but is actually a separate array.
	*/
	unsigned char *old_bufend;
#endif	/* SYSV */
#endif /* ALLOWLOGFILEEXEC */

	if(screen->logging || (screen->inhibit & I_LOG))
		return;
	if(screen->logfile == NULL || *screen->logfile == 0) {
		if(screen->logfile)
			free(screen->logfile);
		if(log_default == NULL)
			log_default = log_def_name;
			mktemp(log_default);
		if((screen->logfile = (char *)malloc((unsigned)strlen(log_default) + 1)) == NULL)
			return;
		strcpy(screen->logfile, log_default);
	}
	if(*screen->logfile == '|') {	/* exec command */
#ifdef ALLOWLOGFILEEXEC
		/*
		 * Warning, enabling this "feature" allows arbitrary programs
		 * to be run.  If ALLOWLOGFILECHANGES is enabled, this can be
		 * done through escape sequences....  You have been warned.
		 */
		int p[2];
		static char *shell;

		if(pipe(p) < 0 || (i = fork()) < 0)
			return;
		if(i == 0) {	/* child */
			close(p[1]);
			dup2(p[0], 0);
			close(p[0]);
			dup2(fileno(stderr), 1);
			dup2(fileno(stderr), 2);
#ifdef SYSV
			old_bufend = _bufend(stderr);
#endif	/* SYSV */
			close(fileno(stderr));
			stderr->_file = 2;
#ifdef SYSV
			_bufend(stderr) = old_bufend;
#endif	/* SYSV */
			close(ConnectionNumber(screen->display));
			close(screen->respond);
			if(!shell) {
				register struct passwd *pw;
				struct passwd *getpwuid();

				if(((cp = getenv("SHELL")) == NULL || *cp == 0)
				 && ((pw = getpwuid(screen->uid)) == NULL ||
				 *(cp = pw->pw_shell) == 0) ||
				 (shell = (char *)malloc((unsigned) strlen(cp) + 1)) == NULL)
					shell = "/bin/sh";
				else
					strcpy(shell, cp);
			}
			signal(SIGHUP, SIG_DFL);
			signal(SIGCHLD, SIG_DFL);
			setgid(screen->gid);
			setuid(screen->uid);
			execl(shell, shell, "-c", &screen->logfile[1], 0);
			fprintf(stderr, "%s: Can't exec `%s'\n", xterm_name,
			 &screen->logfile[1]);
			exit(ERROR_LOGEXEC);
		}
		close(p[0]);
		screen->logfd = p[1];
		signal(SIGPIPE, logpipe);
#else
		Bell();
		Bell();
		return;
#endif
	} else {
		if(access(screen->logfile, F_OK) != 0) {
		    if (errno == ENOENT)
			creat_as(screen->uid, screen->gid,
				 screen->logfile, 0644);
		    else
			return;
		}

		if(access(screen->logfile, F_OK) != 0
		   || access(screen->logfile, W_OK) != 0)
		    return;
		if((screen->logfd = open(screen->logfile, O_WRONLY | O_APPEND,
					 0644)) < 0)
			return;
	}
	screen->logstart = screen->TekEmu ? Tbptr : bptr;
	screen->logging = TRUE;
	update_logging();
}

CloseLog(screen)
register TScreen *screen;
{
	if(!screen->logging || (screen->inhibit & I_LOG))
		return;
	FlushLog(screen);
	close(screen->logfd);
	screen->logging = FALSE;
	update_logging();
}

FlushLog(screen)
register TScreen *screen;
{
	register Char *cp;
	register int i;

	cp = screen->TekEmu ? Tbptr : bptr;
	if((i = cp - screen->logstart) > 0)
		write(screen->logfd, (char *)screen->logstart, i);
	screen->logstart = screen->TekEmu ? Tbuffer : buffer;
}

#ifdef ALLOWLOGFILEEXEC
void logpipe()
{
	register TScreen *screen = &term->screen;

#ifdef SYSV
	(void) signal(SIGPIPE, SIG_IGN);
#endif	/* SYSV */
	if(screen->logging)
		CloseLog(screen);
}
#endif /* ALLOWLOGFILEEXEC */
#endif /* ALLOWLOGGING */


void do_osc(int (*func) (/* ??? */))
{
	register int mode, c;
	register char *cp;
	char buf[512];
	char *bufend = &buf[(sizeof buf) - 1];	/* leave room for null */
	Bool okay = True;

	/* 
	 * lines should be of the form <ESC> ] number ; string <BEL>
	 *
	 * where number is one of 0, 1, 2, or 46
	 */
	mode = 0;
	while(isdigit(c = (*func)()))
		mode = 10 * mode + (c - '0');
	if (c != ';') okay = False;
	cp = buf;
	while(isprint((c = (*func)()) & 0x7f) && cp < bufend)
		*cp++ = c;
	if (c != 7) okay = False;
	*cp = 0;
	if (okay) switch(mode) {
	 case 0:	/* new icon name and title*/
		Changename(buf);
		Changetitle(buf);
		break;
	 case 10:	case 11:	case 12:
	 case 13:	case 14:	case 15:
	 case 16:
		{
		    extern Boolean ChangeColorsRequest(XtermWidget pTerm, int start, register char *names);
		    if (term->misc.dynamicColors)
			ChangeColorsRequest(term,mode-10,buf);
		}
		break;

	 case 1:	/* new icon name only */
		Changename(buf);
		break;

	 case 2:	/* new title only */
		Changetitle(buf);
		break;

#ifdef ALLOWLOGGING
	 case 46:	/* new log file */
#ifdef ALLOWLOGFILECHANGES
		/*
		 * Warning, enabling this feature allows people to overwrite
		 * arbitrary files accessible to the person running xterm.
		 */
		if((cp = (char *)malloc((unsigned)strlen(buf) + 1)) == NULL)
			break;
		strcpy(cp, buf);
		if(term->screen.logfile)
			free(term->screen.logfile);
		term->screen.logfile = cp;
#else
		Bell();
		Bell();
#endif
		break;
#endif /* ALLOWLOGGING */

	case 50:
		SetVTFont (fontMenu_fontescape, True, buf, NULL);
		break;

#ifdef HANZI
	case 160:	/* set HANZI input method directory */
		SetHZinDir (term, buf);
		break;

	case 161:	/* set HANZI input method */
		SetHZinMethod (term, buf, True);
		break;

	case 162:	/* set HANZI input conversion parameter */
		SetHZinParam (buf);
		break;
#endif /* HANZI */
	/*
	 * One could write code to send back the display and host names,
	 * but that could potentially open a fairly nasty security hole.
	 */
	}
}

static void ChangeGroup(String attribute, XtArgVal value)
{
	extern Widget toplevel;
	Arg args[1];

	XtSetArg( args[0], attribute, value );
	XtSetValues( toplevel, args, 1 );
}

void Changename(register char *name)
{
    ChangeGroup( XtNiconName, (XtArgVal)name );
}

void Changetitle(register char *name)
{
    ChangeGroup( XtNtitle, (XtArgVal)name );
}

/***====================================================================***/

ScrnColors	*pOldColors= NULL;

Boolean
GetOldColors(XtermWidget pTerm)
{
int	i;
    if (pOldColors==NULL) {
	pOldColors=	(ScrnColors *)XtMalloc(sizeof(ScrnColors));
	if (pOldColors==NULL) {
	    fprintf(stderr,"allocation failure in GetOldColors\n");
	    return(FALSE);
	}
	pOldColors->which=	0;
	for (i=0;i<NCOLORS;i++) {
	    pOldColors->colors[i]=	0;
	    pOldColors->names[i]=	NULL;
	}
	GetColors(pTerm,pOldColors);
    }
    return(TRUE);
}

Boolean
UpdateOldColors(XtermWidget pTerm, ScrnColors *pNew)
{
int	i;

    /* if we were going to free old colors, this would be the place to
     * do it.   I've decided not to (for now), because it seems likely
     * that we'd have a small set of colors we use over and over, and that
     * we could save some overhead this way.   The only case in which this
     * (clearly) fails is if someone is trying a boatload of colors, in
     * which case they can restart xterm
     */
    for (i=0;i<NCOLORS;i++) {
	if (COLOR_DEFINED(pNew,i)) {
	    if (pOldColors->names[i]!=NULL) {
		XtFree(pOldColors->names[i]);
		pOldColors->names[i]= NULL;
	    }
	    if (pNew->names[i]) {
		pOldColors->names[i]= pNew->names[i];
	    }
	    pOldColors->colors[i]=	pNew->colors[i];
	}
    }
    return(TRUE);
}

void
ReverseOldColors(void)
{
register ScrnColors	*pOld= pOldColors;
Pixel	 tmpPix;
char	*tmpName;

    if (pOld) {
	/* change text cursor, if necesary */
	if (pOld->colors[TEXT_CURSOR]==pOld->colors[TEXT_FG]) {
	    pOld->colors[TEXT_CURSOR]=	pOld->colors[TEXT_BG];
	    if (pOld->names[TEXT_CURSOR]) {
		XtFree(pOldColors->names[TEXT_CURSOR]);
		pOld->names[TEXT_CURSOR]= NULL;
	    }
	    if (pOld->names[TEXT_BG]) {
		tmpName= XtMalloc(strlen(pOld->names[TEXT_BG])+1);
		if (tmpName) {
		    strcpy(tmpName,pOld->names[TEXT_BG]);
		    pOld->names[TEXT_CURSOR]= tmpName;
		}
	    }
	}

	/* swap text FG and BG */
	tmpPix=		pOld->colors[TEXT_FG];
	tmpName=	pOld->names[TEXT_FG];
	pOld->colors[TEXT_FG]=	pOld->colors[TEXT_BG];
	pOld->names[TEXT_FG]=	pOld->names[TEXT_BG];
	pOld->colors[TEXT_BG]=	tmpPix;
	pOld->names[TEXT_BG]=	tmpName;

	/* swap mouse FG and BG */
	tmpPix=		pOld->colors[MOUSE_FG];
	tmpName=	pOld->names[MOUSE_FG];
	pOld->colors[MOUSE_FG]=	pOld->colors[MOUSE_BG];
	pOld->names[MOUSE_FG]=	pOld->names[MOUSE_BG];
	pOld->colors[MOUSE_BG]=	tmpPix;
	pOld->names[MOUSE_BG]=	tmpName;

	/* swap Tek FG and BG */
	tmpPix=		pOld->colors[TEK_FG];
	tmpName=	pOld->names[TEK_FG];
	pOld->colors[TEK_FG]=	pOld->colors[TEK_BG];
	pOld->names[TEK_FG]=	pOld->names[TEK_BG];
	pOld->colors[TEK_BG]=	tmpPix;
	pOld->names[TEK_BG]=	tmpName;
    }
    return;
}

Boolean
AllocateColor(XtermWidget pTerm, ScrnColors *pNew, int ndx, char *name)
{
XColor			 def;
register TScreen	*screen=	&pTerm->screen;
Colormap		 cmap=		pTerm->core.colormap;
char			*newName;

    if ((XParseColor(screen->display,cmap,name,&def))&&
	(XAllocColor(screen->display,cmap,&def))) {
	SET_COLOR_VALUE(pNew,ndx,def.pixel);
	newName= XtMalloc(strlen(name)+1);
	if (newName) {
	    strcpy(newName,name);
	    SET_COLOR_NAME(pNew,ndx,newName);
	}
	return(TRUE);
    }
    return(FALSE);
}

Boolean
ChangeColorsRequest(XtermWidget pTerm, int start, register char *names)
{
char		*thisName;
ScrnColors	newColors;
int		i,ndx;

    if ((pOldColors==NULL)&&(!GetOldColors(pTerm))) {
	return(FALSE);
    }
    newColors.which=	0;
    for (i=0;i<NCOLORS;i++) {
	newColors.names[i]=	NULL;
    }
    for (i=start;i<NCOLORS;i++) {
	if (term->misc.re_verse)	ndx=	OPPOSITE_COLOR(i);
	else				ndx=	i;
	if ((names==NULL)||(names[0]=='\0')) {
	    newColors.names[ndx]=	NULL;
	}
	else {
	    if (names[0]==';') 
		 thisName=	NULL;
	    else thisName=	names;
	    names=	index(names,';');
	    if (names!=NULL) {
		*names=	'\0';
		names++;
	    }
	    if ((!pOldColors->names[ndx])||
		(thisName&&(strcmp(thisName,pOldColors->names[ndx])))) {
		AllocateColor(pTerm,&newColors,ndx,thisName);
	    }
	}
    }

    if (newColors.which==0)
	return(TRUE);

    ChangeColors(pTerm,&newColors);
    UpdateOldColors(pTerm,&newColors);
    return(TRUE);
}

/***====================================================================***/

#ifndef DEBUG
/* ARGSUSED */
#endif
void Panic(char *s, int a)
{
#ifdef DEBUG
	if(debug) {
		fprintf(stderr, "%s: PANIC!	", xterm_name);
		fprintf(stderr, s, a);
		fputs("\r\n", stderr);
		fflush(stderr);
	}
#endif	/* DEBUG */
}

char *SysErrorMsg (int n)
{
    char *err;

    err = strerror (n);
    if (! err) {
        err = "unknown error";
    }
    return (err);
}


void SysError (int i)
{
	int oerrno;

	oerrno = errno;
	/* perror(3) write(2)s to file descriptor 2 */
	fprintf (stderr, "%s: Error %d, errno %d: ", xterm_name, i, oerrno);
	fprintf (stderr, "%s\n", SysErrorMsg (oerrno));
	Cleanup(i);
}

void Error (int i)
{
	fprintf (stderr, "%s: Error %d\n", xterm_name, i);
	Cleanup(i);
}


/*
 * cleanup by sending SIGHUP to client processes
 */
void Cleanup (int code)
{
	extern XtermWidget term;
	register TScreen *screen;

	screen = &term->screen;
	if (screen->pid > 1) {
	    (void) kill_process_group (screen->pid, SIGHUP);
	}
	Exit (code);
}

/*
 * sets the value of var to be arg in the Unix 4.2 BSD environment env.
 * Var should end with '=' (bindings are of the form "var=value").
 * This procedure assumes the memory for the first level of environ
 * was allocated using calloc, with enough extra room at the end so not
 * to have to do a realloc().
 */
void Setenv (register char *var, register char *value)
{
	extern char **environ;
	register int envindex = 0;
	register int len = strlen(var);

	while (environ [envindex] != NULL) {
	    if (strncmp (environ [envindex], var, len) == 0) {
		/* found it */
		environ[envindex] = (char *)malloc ((unsigned)len + strlen (value) + 1);
		strcpy (environ [envindex], var);
		strcat (environ [envindex], value);
		return;
	    }
	    envindex ++;
	}

#ifdef DEBUG
	if (debug) fputs ("expanding env\n", stderr);
#endif	/* DEBUG */

	environ [envindex] = (char *) malloc ((unsigned)len + strlen (value) + 1);
	(void) strcpy (environ [envindex], var);
	strcat (environ [envindex], value);
	environ [++envindex] = NULL;
}

/*
 * returns a pointer to the first occurrence of s2 in s1,
 * or NULL if there are none.
 */
char *strindex (register char *s1, register char *s2)
{
	register char	*s3;
	int s2len = strlen (s2);

	while ((s3=strchr(s1, *s2)) != NULL) {
		if (strncmp(s3, s2, s2len) == 0)
			return (s3);
		s1 = ++s3;
	}
	return (NULL);
}

/*ARGSUSED*/
void xerror(Display *d, register XErrorEvent *ev)
{
    fprintf (stderr, "%s:  warning, error event receieved:\n", xterm_name);
    (void) XmuPrintDefaultErrorMessage (d, ev, stderr);
    Exit (ERROR_XERROR);
}

/*ARGSUSED*/
void xioerror(Display *dpy)
{
    (void) fprintf (stderr, 
		    "%s:  fatal IO error %d (%s) or KillClient on X server \"%s\"\r\n",
		    xterm_name, errno, SysErrorMsg (errno),
		    DisplayString (dpy));

    Exit(ERROR_XIOERROR);
}

void xt_error(String message)
{
    extern char *ProgramName;

    (void) fprintf (stderr, "%s Xt error: %s\n", ProgramName, message);
    exit(1);
}

int XStrCmp(char *s1, char *s2)
{
  if (s1 && s2) return(strcmp(s1, s2));
  if (s1 && *s1) return(1);
  if (s2 && *s2) return(-1);
  return(0);
}

static void withdraw_window (Display *dpy, Window w, int scr)
{
    (void) XmuUpdateMapHints (dpy, w, NULL);
    XWithdrawWindow (dpy, w, scr);
    return;
}


void set_vt_visibility (Boolean on)
{
    register TScreen *screen = &term->screen;

    if (on) {
	if (!screen->Vshow && term) {
	    VTInit ();
	    XtMapWidget (term->core.parent);
	    screen->Vshow = TRUE;
	}
    } else {
	if (screen->Vshow && term) {
	    withdraw_window (XtDisplay (term), 
			     XtWindow(XtParent(term)),
			     XScreenNumberOfScreen(XtScreen(term)));
	    screen->Vshow = FALSE;
	}
    }
    set_vthide_sensitivity();
    set_tekhide_sensitivity();
    update_vttekmode();
    update_tekshow();
    update_vtshow();
    return;
}
				
extern Atom wm_delete_window;	/* for ICCCM delete window */

void set_tek_visibility (Boolean on)
{
    register TScreen *screen = &term->screen;
    if (on) {
	if (!screen->Tshow && (tekWidget || TekInit())) {
	    Widget tekParent = tekWidget->core.parent;
	    XtRealizeWidget (tekParent);
	    XtMapWidget (tekParent);
	    XtOverrideTranslations(tekParent,
				   XtParseTranslationTable
				   ("<Message>WM_PROTOCOLS: DeleteWindow()"));
	    (void) XSetWMProtocols (XtDisplay(tekParent), 
				    XtWindow(tekParent),
				    &wm_delete_window, 1);
	    screen->Tshow = TRUE;
	}
    } else {
	if (screen->Tshow && tekWidget) {
	    withdraw_window (XtDisplay (tekWidget), 
			     XtWindow(XtParent(tekWidget)),
			     XScreenNumberOfScreen(XtScreen(tekWidget)));
	    screen->Tshow = FALSE;
	}
    }
    set_tekhide_sensitivity();
    set_vthide_sensitivity();
    update_vtshow();
    update_tekshow();
    update_vttekmode();
    return;
}

void end_tek_mode (void)
{
    register TScreen *screen = &term->screen;

    if (screen->TekEmu) {
#ifdef ALLOWLOGGING
	if (screen->logging) {
	    FlushLog (screen);
	    screen->logstart = buffer;
	}
#endif
	longjmp(Tekend, 1);
    } 
    return;
}

void end_vt_mode (void)
{
    register TScreen *screen = &term->screen;

    if (!screen->TekEmu) {
#ifdef ALLOWLOGGING
	if(screen->logging) {
	    FlushLog(screen);
	    screen->logstart = Tbuffer;
	}
#endif
	screen->TekEmu = TRUE;
	longjmp(VTend, 1);
    } 
    return;
}

void switch_modes (int tovt)
              				/* if true, then become vt mode */
{
    if (tovt) {
	if (TekRefresh) dorefresh();
	end_tek_mode ();		/* WARNING: this does a longjmp... */
    } else {
	end_vt_mode ();			/* WARNING: this does a longjmp... */
    }
}

void hide_vt_window (void)
{
    register TScreen *screen = &term->screen;

    set_vt_visibility (FALSE);
    if (!screen->TekEmu) switch_modes (False);	/* switch to tek mode */
}

void hide_tek_window (void)
{
    register TScreen *screen = &term->screen;

    set_tek_visibility (FALSE);
    TekRefresh = (TekLink *)0;
    if (screen->TekEmu) switch_modes (True);	/* does longjmp to vt mode */
}
