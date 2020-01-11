/*
 *	$Id: HZInput.c,v 1.2 2003/05/06 07:40:19 htl10 Exp $
 */

/***********************************************************
Copyright 1994,1995 by Yongguang Zhang.  All Rights Reserved

Permission to retain, use, modify, copy, and distribute CXTERM 5.0
in source or binary and its documentation (hereafter, the Software)
for non-commercial purpose is hereby granted to you without a fee,
provided that this entire copyright and permission notice appear in
all such copies, that no charge be associated with such copies,
that distribution of derivative works (including value-added
distributions such as with additional input dictionaries or fonts)
include clarification that such added or derived parts are not from
the original Software, and that the names of the author(s) not be
used to endorse or promote such works.

THE AUTHOR(S) DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
THE AUTHOR(S) BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

******************************************************************/

/* HZInput.c
 *
 * This file defines the interface between xterm and cxterm input module.
 *
 * The following external function are called by xterm.
 *
 *	function		called by
 * -------------------		----------
 * CreateCXtermInput()		charproc.c 	(to create the input module)
 * DestroyCXtermInput()		charproc.c 	(to destroy the input module)
 * RealizeCXtermInput()		charproc.c 	(when xterm widget realizes)
 * RefreshCXtermInput()		charproc.c util.c
 *				(when xterm refreshes the screen, e.g. expose)
 * ResetCXtermInput()		charproc.c	(when xterm is full-reset)
 * ResizeCXtermInput()		charproc.c screen.c scrollbar.c
 *				(when xterm resizes the screen)
 * HandleSwitchHZMode()		charproc.c	(switch-HZ-mode Xt action)
 * HandleSetHZParam()		charproc.c	(set-HZ-parameter Xt action)
 * HandleClickHZinArea()	charproc.c	(click-HZ-area Xt action)
 * HandlePopupPanel()		charproc.c menu.c
 *				(popup-panel Xt action/cxtermconfig menu item)
 * SetHZinDir()			misc.c		(esc seq to set HZINPUTDIR)
 * SetHZinMethod()		misc.c		(esc seq to switch HZ mode)
 * SetHZinParam()		misc.c		(esc seq to set parameters)
 * HZParseInput()		input.c		(keyseq => HZ)
 *
 * The following external function are called by cxterm.
 *	function		called by
 * -------------------		----------
 * XtermWritePty()		HZinArea.c 	(to write data to pty)
 *
 */

#include "HZinput.h"		/* X headers included here. */

#include "data.h"

/* could be moved to ptyx.h */
extern void CreateCXtermInput(XtermWidget xw);
extern void DestroyCXtermInput(XtermWidget xw);
extern void RealizeCXtermInput(XtermWidget xw);
extern void ResizeCXtermInput(TScreen *screen);
extern void RefreshCXtermInput(TScreen *screen);
extern void ResetCXtermInput(XtermWidget xw);
extern void HandleSwitchHZMode(Widget w, XEvent *event, String *params, Cardinal *nparams);
extern void HandleSetHZParam(Widget w, XEvent *event, String *params, Cardinal *nparams);
extern void HandleClickHZinArea(Widget w, XEvent *event, String *params, Cardinal *nparams);
extern void HandlePopupPanel(Widget widget, XEvent *event, String *params, Cardinal *num_params);
extern void SetHZinDir(XtermWidget xw, String dir);
extern void SetHZinMethod(XtermWidget xw, String name, int doredraw);
extern void SetHZAssoc(char *inputdir, String name);
extern void SetHZinParam(char *str);
extern  int HZParseInput(TScreen *screen, int nbytes, char *strbuf);
extern  int v_write(int f, char *d, int len);	    /* charproc.c */

/* The only global variable in the input module: _THE_ CXterm Input Module */
CXtermInputModule cxtermInput;


extern void HZCreatePopup (CXtermInputModule *cxin);

void CreateCXtermInput(XtermWidget xw)
{
  TScreen *screen = &xw->screen;
  char *cp = (char *) getenv ("HZINPUTDIR");

    cxtermInput.realized = FALSE;
    screen->hzIwin.rows = ROWSINPUTAREA;	/* num of rows in Input Area */
    if ((xw->misc.it_dirs == NULL) && cp) {
	xw->misc.it_dirs = (char *) malloc (strlen(cp) + 1);
	if (xw->misc.it_dirs)
	    strcpy (xw->misc.it_dirs, cp);
    }
    HZimInit (&cxtermInput, xw, screen);
    HZiaInit (&cxtermInput.hzia, &xw->screen);

    HZCreatePopup(&cxtermInput);
}

/*ARGSUSED*/
void DestroyCXtermInput(XtermWidget xw)
{
    HZimCleanUp (&cxtermInput);
    HZiaCleanUp (&cxtermInput.hzia);
}

void RealizeCXtermInput(XtermWidget xw)
{
    cxtermInput.realized = TRUE;

    /* association list must be loaded before the first input method */ 
    if (xw->misc.assoc_files)
	SetHZAssoc (xw->misc.it_dirs, xw->misc.assoc_files);

    if (xw->misc.hz_mode)
	SetHZinMethod (xw, xw->misc.hz_mode, False);
    else
	HZswitchModeByNum (&cxtermInput, 0);	/* initially, ASCII mode */
    /* don't draw it now, leave it to the Expose event */
}

void ResizeCXtermInput(TScreen *screen)
{
    screen->hzIwin.rule_x = screen->scrollbar;
    screen->hzIwin.rule_y = Height(screen) + screen->border;
    screen->hzIwin.rule_length = Width(screen) + 2 * screen->border;
    screen->hzIwin.x = screen->scrollbar + screen->border;
    screen->hzIwin.y = screen->hzIwin.rule_y + 1;
    screen->hzIwin.width = Width(screen);
    screen->hzIwin.height = FontHeight(screen) * screen->hzIwin.rows;

    HZiaResize (&cxtermInput.hzia, screen, &cxtermInput);
    /* don't redraw, leave it to the Expose event */
}

void RefreshCXtermInput(TScreen *screen)
{
    HZiaRedraw (&cxtermInput.hzia, screen);
}

void ResetCXtermInput(XtermWidget xw)
{
  TScreen *screen = &xw->screen;

    HZimCleanUp (&cxtermInput);
    HZimInit (&cxtermInput, xw, screen);
    if (xw->misc.assoc_files)
	SetHZAssoc (xw->misc.it_dirs, xw->misc.assoc_files);
    HZswitchModeByNum (&cxtermInput, 0);	/* back to ASCII mode */
    RefreshCXtermInput(screen);
}

/*ARGSUSED*/
void HandleSwitchHZMode(Widget w, XEvent *event, String *params, Cardinal *nparams)
{
    if (*nparams > 0) {
	/* set the hanzi mode and do redraw */
	SetHZinMethod (cxtermInput.xterm, *params, True);
//	showLatestHZKeysInNewMethod(cxtermInput);
    }
}

/*ARGSUSED*/
void HandleSetHZParam(Widget w, XEvent *event, String *params, Cardinal *nparams)
{
  Cardinal n = *nparams;

    while (n-- > 0) {
	SetHZinParam (*params++);
    }
}

/*ARGSUSED*/
void HandleClickHZinArea(Widget w, XEvent *event, String *params, Cardinal *nparams)
             
                  		/* must be XButtonEvent */
                   
                      
{
    HZiaClickAction( &(cxtermInput.hzia), &cxtermInput,
		     event->xbutton.x, event->xbutton.y );
}

/*ARGSUSED*/
void HandlePopupPanel (Widget widget, XEvent *event, String *params, Cardinal *num_params)
{
    if (*num_params != 1) {
	XBell (XtDisplay(widget), 0);
	return;
    }

    switch (params[0][0]) {
      case 'c':
	HZPopupConfig(&cxtermInput);
	break;
/*
      case 'h':
	HZPopupHelp();
	break;
*/
      default:
	XBell (XtDisplay(widget), 0);
	return;
    }
}

void SetHZinDir (XtermWidget xw, String dir)
{
  char *cp = (char *)malloc((unsigned)strlen(dir) + 1);

    if (cp == NULL)
	return;
    strcpy(cp, dir);
    if(xw->misc.it_dirs)
	free(xw->misc.it_dirs);
    xw->misc.it_dirs = cp;
}

void SetHZinMethod (XtermWidget xw, String name, int doredraw)
{
  CXtermInputModule *cxin = &cxtermInput;
  int i;

    if (!name || !(*name))
	return;		/* nothing to switch */

    for (i = 0; i < cxin->numHZim; i++) {
	if (! strcmp(name, cxin->imtbl[i].name)) {
	    /* already loaded */
	    break;
	}
    }
    if (i == cxin->numHZim) {	/* not loaded */
	if (cxin->numHZim == MAX_HZIM) {	/* too many input methods */
	    HZprintfMsg("Too many input methods (max = %d)", MAX_HZIM);
	    return;
	} else {
	    if (HZLoadInputMethod (cxin, name, xw->misc.it_dirs,
			cxin->xterm->misc.hz_encoding, &(cxin->imtbl[i]) ))
	    {
		/* non-zero return: loading unsuccessfully */
		return;
	    }
	    cxin->numHZim++ ;
	}
    }
    HZswitchModeByNum (cxin, i);

    /* enable it if temporarily disable. */
    if (cxtermInput.temp_disable) {
	cxtermInput.temp_disable = False;
	HZiaSetSensitive (&cxtermInput.hzia, True);
    }
    if (doredraw)
	RefreshCXtermInput (&xw->screen);
}

void SetHZAssoc (char *inputdir, String name)
{
  CXtermInputModule *cxin = &cxtermInput;

    cxin->hzal = LoadAssocList (name, inputdir, cxin->xterm->misc.hz_encoding);
    if (! cxin->hzal)
	cxin->xterm->misc.assoc_files = NULL;
    else {
	cxin->global.do_auto_segment = True;
	cxin->global.do_ps_assoc = True;
    }
}

void SetHZinParam (char *str)
{
    HZimSetParam (&cxtermInput, str);
}

int HZParseInput (TScreen *screen, int nbytes, char *strbuf)
{
    if (cxtermInput.mode == 0)	/* ASCII */
	return (nbytes);
    if (cxtermInput.temp_disable)	/* temporarily disable  */
	return (nbytes);

    /* optimize the simply cases */
    if (nbytes == 1) {
	return ((* cxtermInput.chzif) (&cxtermInput, strbuf[0], strbuf));
    } else if (nbytes == 0) {
	return (0);
    } else {
	char istr[100];	/* match the STRBUFSIZE in input.c */
	int i, nbytesOut = 0;

	strncpy (istr, strbuf, nbytes);
	for (i = 0; i < nbytes; i++) {
	    nbytesOut += (* cxtermInput.chzif) (&cxtermInput,
				istr[i], &strbuf[nbytesOut]);
	}
	return (nbytesOut);
    }
}

void XtermWritePty(char *d, int len)
{

    if (d && (len > 0))
	v_write (cxtermInput.xterm->screen.respond, d, len);
}


#include <stdio.h>

/*
 * HZprintfMsg(...)	printf an error message
 */
/*VARARGS2*/
void HZprintfMsg(char *str, char *s1, char *s2, char *s3, char *s4, char *s5, char *s6)
{
  CXtermInputModule *cxin = &cxtermInput;

    if (cxin->realized) {
	char tmpstr[128];

	(void) sprintf(tmpstr, str, s1, s2, s3, s4, s5, s6);
	HZiaShowMesg (&(cxin->hzia), cxin->screen, tmpstr);
    } else {
	(void) fprintf(stderr, str, s1, s2, s3, s4, s5, s6);
	(void) fprintf(stderr, "\n");
    }
}
