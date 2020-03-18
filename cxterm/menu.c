/* $Id: menu.c,v 1.2 2003/05/06 07:40:19 htl10 Exp $ */
/***********************************************************************
* Copyright 1994 by Yongguang Zhang.
*
* All rights reserved.  Under the same copyright and permission term as
* the original.  Absolutely no warranties of any kinds.
************************************************************************/
/* $XConsortium: menu.c,v 1.63 94/04/17 20:23:30 gildea Exp $ */
/*

Copyright (c) 1989  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

*/

#include "ptyx.h"
#include "data.h"
#include "menu.h"
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xmu/CharSet.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>
#include <stdio.h>
#include <signal.h>

extern void FindFontSelection(char *atom_name, int justprobe);

Arg menuArgs[2] = {{ XtNleftBitmap, (XtArgVal) 0 },
		   { XtNsensitive, (XtArgVal) 0 }};

void do_hangup(Widget gw, caddr_t closure, caddr_t data);

static void do_securekbd(Widget gw, caddr_t closure, caddr_t data), do_allowsends(Widget gw, caddr_t closure, caddr_t data), do_visualbell(Widget gw, caddr_t closure, caddr_t data),
#ifdef ALLOWLOGGING
    do_logging(),
#endif
    do_redraw(Widget gw, caddr_t closure, caddr_t data), do_suspend(Widget gw, caddr_t closure, caddr_t data), do_continue(Widget gw, caddr_t closure, caddr_t data), do_interrupt(Widget gw, caddr_t closure, caddr_t data), 
    do_terminate(Widget gw, caddr_t closure, caddr_t data), do_kill(Widget gw, caddr_t closure, caddr_t data), do_quit(Widget gw, caddr_t closure, caddr_t data), do_scrollbar(Widget gw, caddr_t closure, caddr_t data), do_jumpscroll(Widget gw, caddr_t closure, caddr_t data),
    do_reversevideo(Widget gw, caddr_t closure, caddr_t data), do_autowrap(Widget gw, caddr_t closure, caddr_t data), do_reversewrap(Widget gw, caddr_t closure, caddr_t data), do_autolinefeed(Widget gw, caddr_t closure, caddr_t data),
    do_appcursor(Widget gw, caddr_t closure, caddr_t data), do_appkeypad(Widget gw, caddr_t closure, caddr_t data), do_scrollkey(Widget gw, caddr_t closure, caddr_t data), do_scrollttyoutput(Widget gw, caddr_t closure, caddr_t data),
    do_allow132(Widget gw, caddr_t closure, caddr_t data), do_cursesemul(Widget gw, caddr_t closure, caddr_t data), do_marginbell(Widget gw, caddr_t closure, caddr_t data), do_tekshow(Widget gw, caddr_t closure, caddr_t data), 
    do_altscreen(Widget gw, caddr_t closure, caddr_t data), do_softreset(Widget gw, caddr_t closure, caddr_t data), do_hardreset(Widget gw, caddr_t closure, caddr_t data), do_clearsavedlines(Widget gw, caddr_t closure, caddr_t data),
    do_tekmode(Widget gw, caddr_t closure, caddr_t data), do_vthide(Widget gw, caddr_t closure, caddr_t data), 
#ifdef HANZI
    do_cxtermconfig(Widget gw, caddr_t closure, caddr_t data),
#endif
    do_tektextlarge(Widget gw, caddr_t closure, caddr_t data), do_tektext2(Widget gw, caddr_t closure, caddr_t data), do_tektext3(Widget gw, caddr_t closure, caddr_t data), do_tektextsmall(Widget gw, caddr_t closure, caddr_t data), 
    do_tekpage(Widget gw, caddr_t closure, caddr_t data), do_tekreset(Widget gw, caddr_t closure, caddr_t data), do_tekcopy(Widget gw, caddr_t closure, caddr_t data), do_vtshow(Widget gw, caddr_t closure, caddr_t data), do_vtmode(Widget gw, caddr_t closure, caddr_t data), 
    do_tekhide(Widget gw, caddr_t closure, caddr_t data), do_vtfont(Widget gw, caddr_t closure, caddr_t data);


/*
 * The order entries MUST match the values given in menu.h
 */
MenuEntry mainMenuEntries[] = {
    { "securekbd",	do_securekbd, NULL },		/*  0 */
    { "allowsends",	do_allowsends, NULL },		/*  1 */
#ifdef ALLOWLOGGING
    { "logging",	do_logging, NULL },		/*  2 */
#endif
    { "redraw",		do_redraw, NULL },		/*  3 */
    { "line1",		NULL, NULL },			/*  4 */
    { "suspend",	do_suspend, NULL },		/*  5 */
    { "continue",	do_continue, NULL },		/*  6 */
    { "interrupt",	do_interrupt, NULL },		/*  7 */
    { "hangup",		do_hangup, NULL },		/*  8 */
    { "terminate",	do_terminate, NULL },		/*  9 */
    { "kill",		do_kill, NULL },		/* 10 */
    { "line2",		NULL, NULL },			/* 11 */
    { "quit",		do_quit, NULL }};		/* 12 */

MenuEntry vtMenuEntries[] = {
    { "scrollbar",	do_scrollbar, NULL },		/*  0 */
    { "jumpscroll",	do_jumpscroll, NULL },		/*  1 */
    { "reversevideo",	do_reversevideo, NULL },	/*  2 */
    { "autowrap",	do_autowrap, NULL },		/*  3 */
    { "reversewrap",	do_reversewrap, NULL },		/*  4 */
    { "autolinefeed",	do_autolinefeed, NULL },	/*  5 */
    { "appcursor",	do_appcursor, NULL },		/*  6 */
    { "appkeypad",	do_appkeypad, NULL },		/*  7 */
    { "scrollkey",	do_scrollkey, NULL },		/*  8 */
    { "scrollttyoutput",do_scrollttyoutput, NULL },	/*  9 */
    { "allow132",	do_allow132, NULL },		/* 10 */
    { "cursesemul",	do_cursesemul, NULL },		/* 11 */
    { "visualbell",	do_visualbell, NULL },		/* 12 */
    { "marginbell",	do_marginbell, NULL },		/* 13 */
    { "altscreen",	do_altscreen, NULL },		/* 14 */
    { "line1",		NULL, NULL },			/* 15 */
    { "softreset",	do_softreset, NULL },		/* 16 */
    { "hardreset",	do_hardreset, NULL },		/* 17 */
    { "clearsavedlines",do_clearsavedlines, NULL },	/* 18 */
    { "line2",		NULL, NULL },			/* 19 */
    { "tekshow",	do_tekshow, NULL },		/* 20 */
    { "tekmode",	do_tekmode, NULL },		/* 21 */
#ifdef HANZI
    { "vthide",		do_vthide, NULL },		/* 22 */
    { "line3",		NULL, NULL },			/* 23 */
    { "cxtermconfig",	do_cxtermconfig, NULL }};	/* 24 */
#else
    { "vthide",		do_vthide, NULL }};		/* 22 */
#endif

MenuEntry fontMenuEntries[] = {
    { "fontdefault",	do_vtfont, NULL },		/*  0 */
    { "font1",		do_vtfont, NULL },		/*  1 */
    { "font2",		do_vtfont, NULL },		/*  2 */
    { "font3",		do_vtfont, NULL },		/*  3 */
    { "font4",		do_vtfont, NULL },		/*  4 */
    { "font5",		do_vtfont, NULL },		/*  5 */
    { "font6",		do_vtfont, NULL },		/*  6 */
    { "fontescape",	do_vtfont, NULL },		/*  7 */
    { "fontsel",	do_vtfont, NULL }};		/*  8 */
    /* this should match NMENUFONTS in ptyx.h */

MenuEntry tekMenuEntries[] = {
    { "tektextlarge",	do_tektextlarge, NULL },	/*  0 */
    { "tektext2",	do_tektext2, NULL },		/*  1 */
    { "tektext3",	do_tektext3, NULL },		/*  2 */
    { "tektextsmall",	do_tektextsmall, NULL },	/*  3 */
    { "line1",		NULL, NULL },			/*  4 */
    { "tekpage",	do_tekpage, NULL },		/*  5 */
    { "tekreset",	do_tekreset, NULL },		/*  6 */
    { "tekcopy",	do_tekcopy, NULL },		/*  7 */
    { "line2",		NULL, NULL },			/*  8 */
    { "vtshow",		do_vtshow, NULL },		/*  9 */
    { "vtmode",		do_vtmode, NULL },		/* 10 */
    { "tekhide",	do_tekhide, NULL }};		/* 11 */

static Widget create_menu(XtermWidget xtw, Widget toplevelw, char *name, struct _MenuEntry *entries, int nentries);
extern Widget toplevel;


/*
 * we really want to do these dynamically
 */
#define check_width 9
#define check_height 8
static unsigned char check_bits[] = {
   0x00, 0x01, 0x80, 0x01, 0xc0, 0x00, 0x60, 0x00,
   0x31, 0x00, 0x1b, 0x00, 0x0e, 0x00, 0x04, 0x00
};


/*
 * public interfaces
 */

/* ARGSUSED */

extern void ReverseVideo (XtermWidget termw);
extern void Redraw (void);
extern void Cleanup (int code);
extern void ScrollBarOff (register TScreen *screen);
extern void ScrollBarOn (XtermWidget xw, int init, int doalloc);
extern void FlushScroll (register TScreen *screen);
extern void set_tek_visibility (Boolean on);
extern void end_tek_mode (void);
extern void Bell (void);
extern void VTReset (Boolean full);
extern void ScrollBarDrawThumb (register Widget scrollWidget);
extern void switch_modes (int tovt);
extern void hide_vt_window (void);
extern void SetVTFont (int i, int doresize, char *name1, char *name2);
extern void TekSetFontSize (int newitem);
extern void TekSimulatePageButton (int reset);
extern void TekCopy (void);
extern void set_vt_visibility (Boolean on);
extern void dorefresh (void);
extern void end_vt_mode (void);
extern void hide_tek_window (void);

static Bool domenu (Widget w, XEvent *event, String *params, Cardinal *param_count)
             
                                /* unused */
                                /* mainMenu, vtMenu, or tekMenu */
                                /* 0 or 1 */
{
    TScreen *screen = &term->screen;

    if (*param_count != 1) {
	XBell (XtDisplay(w), 0);
	return False;
    }

    switch (params[0][0]) {
      case 'm':
	if (!screen->mainMenu) {
	    screen->mainMenu = create_menu (term, toplevel, "mainMenu",
					    mainMenuEntries,
					    XtNumber(mainMenuEntries));
	    update_securekbd();
	    update_allowsends();
#ifdef ALLOWLOGGING
	    update_logging();
#endif
#ifndef SIGTSTP
	    set_sensitivity (screen->mainMenu,
			     mainMenuEntries[mainMenu_suspend].widget, FALSE);
#endif
#ifndef SIGCONT
	    set_sensitivity (screen->mainMenu, 
			     mainMenuEntries[mainMenu_continue].widget, FALSE);
#endif
	}
	break;

      case 'v':
	if (!screen->vtMenu) {
	    screen->vtMenu = create_menu (term, toplevel, "vtMenu",
					  vtMenuEntries,
					  XtNumber(vtMenuEntries));
	    /* and turn off the alternate screen entry */
	    set_altscreen_sensitivity (FALSE);
	    update_scrollbar();
	    update_jumpscroll();
	    update_reversevideo();
	    update_autowrap();
	    update_reversewrap();
	    update_autolinefeed();
	    update_appcursor();
	    update_appkeypad();
	    update_scrollkey();
	    update_scrollttyoutput();
	    update_allow132();
	    update_cursesemul();
	    update_visualbell();
	    update_marginbell();
	}
	break;

      case 'f':
	if (!screen->fontMenu) {
	    screen->fontMenu = create_menu (term, toplevel, "fontMenu",
					    fontMenuEntries,
					    NMENUFONTS);  
	    set_menu_font (True);
	    set_sensitivity (screen->fontMenu,
			     fontMenuEntries[fontMenu_fontescape].widget,
			     (screen->menu_font_names[fontMenu_fontescape]
			      ? TRUE : FALSE));
	}
	FindFontSelection (NULL, True);
	set_sensitivity (screen->fontMenu,
			 fontMenuEntries[fontMenu_fontsel].widget,
			 (screen->menu_font_names[fontMenu_fontsel]
			  ? TRUE : FALSE));
	break;

      case 't':
	if (!screen->tekMenu) {
	    screen->tekMenu = create_menu (term, toplevel, "tekMenu",
					   tekMenuEntries,
					   XtNumber(tekMenuEntries));
	    set_tekfont_menu_item (screen->cur.fontsize, TRUE);
	}
	break;

      default:
	XBell (XtDisplay(w), 0);
	return False;
    }

    return True;
}

void HandleCreateMenu (Widget w, XEvent *event, String *params, Cardinal *param_count)
             
                                /* unused */
                                /* mainMenu, vtMenu, or tekMenu */
                                /* 0 or 1 */
{
    (void) domenu (w, event, params, param_count);
}

void HandlePopupMenu (Widget w, XEvent *event, String *params, Cardinal *param_count)
             
                                /* unused */
                                /* mainMenu, vtMenu, or tekMenu */
                                /* 0 or 1 */
{
    if (domenu (w, event, params, param_count)) {
	XtCallActionProc (w, "XawPositionSimpleMenu", event, params, 1);
	XtCallActionProc (w, "MenuPopup", event, params, 1);
    }
}


/*
 * private interfaces - keep out!
 */

/*
 * create_menu - create a popup shell and stuff the menu into it.
 */

static Widget create_menu (XtermWidget xtw, Widget toplevelw, char *name, struct _MenuEntry *entries, int nentries)
{
    Widget m;
    TScreen *screen = &xtw->screen;
    static XtCallbackRec cb[2] = { { NULL, NULL }, { NULL, NULL }};
    static Arg arg = { XtNcallback, (XtArgVal) cb };

    if (screen->menu_item_bitmap == None) {
	screen->menu_item_bitmap =
	  XCreateBitmapFromData (XtDisplay(xtw),
				 RootWindowOfScreen(XtScreen(xtw)),
				 (char *)check_bits, check_width, check_height);
    }

    m = XtCreatePopupShell (name, simpleMenuWidgetClass, toplevelw, NULL, 0);

    for (; nentries > 0; nentries--, entries++) {
	cb[0].callback = (XtCallbackProc) entries->function;
	cb[0].closure = (caddr_t) entries->name;
	entries->widget = XtCreateManagedWidget (entries->name, 
						 (entries->function ?
						  smeBSBObjectClass :
						  smeLineObjectClass), m,
						 &arg, (Cardinal) 1);
    }

    /* do not realize at this point */
    return m;
}

/* ARGSUSED */
static void handle_send_signal (Widget gw, int sig)
{
    register TScreen *screen = &term->screen;

    if (screen->pid > 1) kill_process_group (screen->pid, sig);
}


/*
 * action routines
 */

/* ARGSUSED */
void DoSecureKeyboard (Time time)
{
    do_securekbd (term->screen.mainMenu, NULL, NULL);
}

static void do_securekbd (Widget gw, caddr_t closure, caddr_t data)
{
    register TScreen *screen = &term->screen;
    Time time = CurrentTime;		/* XXX - wrong */

    if (screen->grabbedKbd) {
	XUngrabKeyboard (screen->display, time);
	ReverseVideo (term);
	screen->grabbedKbd = FALSE;
    } else {
	if (XGrabKeyboard (screen->display, term->core.window,
			   True, GrabModeAsync, GrabModeAsync, time)
	    != GrabSuccess) {
	    XBell (screen->display, 100);
	} else {
	    ReverseVideo (term);
	    screen->grabbedKbd = TRUE;
	}
    }
    update_securekbd();
}


static void do_allowsends (Widget gw, caddr_t closure, caddr_t data)
{
    register TScreen *screen = &term->screen;

    screen->allowSendEvents = !screen->allowSendEvents;
    update_allowsends ();
}

static void do_visualbell (Widget gw, caddr_t closure, caddr_t data)
{
    register TScreen *screen = &term->screen;

    screen->visualbell = !screen->visualbell;
    update_visualbell();
}

#ifdef ALLOWLOGGING
static void do_logging (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    register TScreen *screen = &term->screen;

    if (screen->logging) {
	CloseLog (screen);
    } else {
	StartLog (screen);
    }
    /* update_logging done by CloseLog and StartLog */
}
#endif

static void do_redraw (Widget gw, caddr_t closure, caddr_t data)
{
    Redraw ();
}


/*
 * The following cases use the pid instead of the process group so that we
 * don't get hosed by programs that change their process group
 */


/* ARGSUSED */
static void do_suspend (Widget gw, caddr_t closure, caddr_t data)
{
#ifdef SIGTSTP
    handle_send_signal (gw, SIGTSTP);
#endif
}

/* ARGSUSED */
static void do_continue (Widget gw, caddr_t closure, caddr_t data)
{
#ifdef SIGCONT
    handle_send_signal (gw, SIGCONT);
#endif
}

/* ARGSUSED */
static void do_interrupt (Widget gw, caddr_t closure, caddr_t data)
{
    handle_send_signal (gw, SIGINT);
}

/* ARGSUSED */
void do_hangup (Widget gw, caddr_t closure, caddr_t data)
{
    handle_send_signal (gw, SIGHUP);
}

/* ARGSUSED */
static void do_terminate (Widget gw, caddr_t closure, caddr_t data)
{
    handle_send_signal (gw, SIGTERM);
}

/* ARGSUSED */
static void do_kill (Widget gw, caddr_t closure, caddr_t data)
{
    handle_send_signal (gw, SIGKILL);
}

static void do_quit (Widget gw, caddr_t closure, caddr_t data)
{
    Cleanup (0);
}



/*
 * vt menu callbacks
 */

static void do_scrollbar (Widget gw, caddr_t closure, caddr_t data)
{
    register TScreen *screen = &term->screen;

    if (screen->scrollbar) {
	ScrollBarOff (screen);
    } else {
	ScrollBarOn (term, FALSE, FALSE);
    }
    update_scrollbar();
}


static void do_jumpscroll (Widget gw, caddr_t closure, caddr_t data)
{
    register TScreen *screen = &term->screen;

    term->flags ^= SMOOTHSCROLL;
    if (term->flags & SMOOTHSCROLL) {
	screen->jumpscroll = FALSE;
	if (screen->scroll_amt) FlushScroll(screen);
    } else {
	screen->jumpscroll = TRUE;
    }
    update_jumpscroll();
}


static void do_reversevideo (Widget gw, caddr_t closure, caddr_t data)
{
    term->flags ^= REVERSE_VIDEO;
    ReverseVideo (term);
    /* update_reversevideo done in ReverseVideo */
}


static void do_autowrap (Widget gw, caddr_t closure, caddr_t data)
{
    term->flags ^= WRAPAROUND;
    update_autowrap();
}


static void do_reversewrap (Widget gw, caddr_t closure, caddr_t data)
{
    term->flags ^= REVERSEWRAP;
    update_reversewrap();
}


static void do_autolinefeed (Widget gw, caddr_t closure, caddr_t data)
{
    term->flags ^= LINEFEED;
    update_autolinefeed();
}


static void do_appcursor (Widget gw, caddr_t closure, caddr_t data)
{
    term->keyboard.flags ^= CURSOR_APL;
    update_appcursor();
}


static void do_appkeypad (Widget gw, caddr_t closure, caddr_t data)
{
    term->keyboard.flags ^= KYPD_APL;
    update_appkeypad();
}


static void do_scrollkey (Widget gw, caddr_t closure, caddr_t data)
{
    register TScreen *screen = &term->screen;

    screen->scrollkey = !screen->scrollkey;
    update_scrollkey();
}


static void do_scrollttyoutput (Widget gw, caddr_t closure, caddr_t data)
{
    register TScreen *screen = &term->screen;

    screen->scrollttyoutput = !screen->scrollttyoutput;
    update_scrollttyoutput();
}


static void do_allow132 (Widget gw, caddr_t closure, caddr_t data)
{
    register TScreen *screen = &term->screen;

    screen->c132 = !screen->c132;
    update_allow132();
}


static void do_cursesemul (Widget gw, caddr_t closure, caddr_t data)
{
    register TScreen *screen = &term->screen;

    screen->curses = !screen->curses;
    update_cursesemul();
}


static void do_marginbell (Widget gw, caddr_t closure, caddr_t data)
{
    register TScreen *screen = &term->screen;

    if (!(screen->marginbell = !screen->marginbell)) screen->bellarmed = -1;
    update_marginbell();
}


static void handle_tekshow (Widget gw, int allowswitch)
{
    register TScreen *screen = &term->screen;

    if (!screen->Tshow) {		/* not showing, turn on */
	set_tek_visibility (TRUE);
    } else if (screen->Vshow || allowswitch) {  /* is showing, turn off */
	set_tek_visibility (FALSE);
	end_tek_mode ();		/* WARNING: this does a longjmp */
    } else
      Bell();
}

/* ARGSUSED */
static void do_tekshow (Widget gw, caddr_t closure, caddr_t data)
{
    handle_tekshow (gw, True);
}

/* ARGSUSED */
static void do_tekonoff (Widget gw, caddr_t closure, caddr_t data)
{
    handle_tekshow (gw, False);
}

/* ARGSUSED */
static void do_altscreen (Widget gw, caddr_t closure, caddr_t data)
{
    /* do nothing for now; eventually, will want to flip screen */
}


static void do_softreset (Widget gw, caddr_t closure, caddr_t data)
{
    VTReset (FALSE);
}


static void do_hardreset (Widget gw, caddr_t closure, caddr_t data)
{
    VTReset (TRUE);
}


static void do_clearsavedlines (Widget gw, caddr_t closure, caddr_t data)
{
    register TScreen *screen = &term->screen;

    screen->savedlines = 0;
    ScrollBarDrawThumb(screen->scrollWidget);
    VTReset (TRUE); 
}


static void do_tekmode (Widget gw, caddr_t closure, caddr_t data)
{
    register TScreen *screen = &term->screen;

    switch_modes (screen->TekEmu);	/* switch to tek mode */
}

/* ARGSUSED */
static void do_vthide (Widget gw, caddr_t closure, caddr_t data)
{
    hide_vt_window();
}

#ifdef HANZI
/* ARGSUSED */
static void do_cxtermconfig (Widget gw, caddr_t closure, caddr_t data)
{
  String popup_name = "config";
  Cardinal num_param = 1;
  extern void HandlePopupPanel(Widget widget, XEvent *event, String *params, Cardinal *num_params);	/* HZInput.c */

    HandlePopupPanel(gw, (XEvent *)NULL, &popup_name, &num_param);
}

#endif /* HANZI */

/*
 * vtfont menu
 */

static void do_vtfont (Widget gw, caddr_t closure, caddr_t data)
{
    char *entryname = (char *) closure;
    int i;

    for (i = 0; i < NMENUFONTS; i++) {
	if (strcmp (entryname, fontMenuEntries[i].name) == 0) {
	    SetVTFont (i, True, NULL, NULL);
	    return;
	}
    }
    Bell();
}


/*
 * tek menu
 */

static void do_tektextlarge (Widget gw, caddr_t closure, caddr_t data)
{
    TekSetFontSize (tekMenu_tektextlarge);
}


static void do_tektext2 (Widget gw, caddr_t closure, caddr_t data)
{
    TekSetFontSize (tekMenu_tektext2);
}


static void do_tektext3 (Widget gw, caddr_t closure, caddr_t data)
{
    TekSetFontSize (tekMenu_tektext3);
}


static void do_tektextsmall (Widget gw, caddr_t closure, caddr_t data)
{

    TekSetFontSize (tekMenu_tektextsmall);
}


static void do_tekpage (Widget gw, caddr_t closure, caddr_t data)
{
    TekSimulatePageButton (False);
}


static void do_tekreset (Widget gw, caddr_t closure, caddr_t data)
{
    TekSimulatePageButton (True);
}


static void do_tekcopy (Widget gw, caddr_t closure, caddr_t data)
{
    TekCopy ();
}


static void handle_vtshow (Widget gw, int allowswitch)
{
    register TScreen *screen = &term->screen;

    if (!screen->Vshow) {		/* not showing, turn on */
	set_vt_visibility (TRUE);
    } else if (screen->Tshow || allowswitch) {  /* is showing, turn off */
	set_vt_visibility (FALSE);
	if (!screen->TekEmu && TekRefresh) dorefresh ();
	end_vt_mode ();
    } else 
      Bell();
}

static void do_vtshow (Widget gw, caddr_t closure, caddr_t data)
{
    handle_vtshow (gw, True);
}

static void do_vtonoff (Widget gw, caddr_t closure, caddr_t data)
{
    handle_vtshow (gw, False);
}

static void do_vtmode (Widget gw, caddr_t closure, caddr_t data)
{
    register TScreen *screen = &term->screen;

    switch_modes (screen->TekEmu);	/* switch to vt, or from */
}


/* ARGSUSED */
static void do_tekhide (Widget gw, caddr_t closure, caddr_t data)
{
    hide_tek_window();
}



/*
 * public handler routines
 */

static void handle_toggle (void (*proc) (/* ??? */), int var, String *params, Cardinal nparams, Widget w, caddr_t closure, caddr_t data)
{
    int dir = -2;

    switch (nparams) {
      case 0:
	dir = -1;
	break;
      case 1:
	if (XmuCompareISOLatin1 (params[0], "on") == 0) dir = 1;
	else if (XmuCompareISOLatin1 (params[0], "off") == 0) dir = 0;
	else if (XmuCompareISOLatin1 (params[0], "toggle") == 0) dir = -1;
	break;
    }

    switch (dir) {
      case -2:
	Bell();
	break;

      case -1:
	(*proc) (w, closure, data);
	break;

      case 0:
	if (var) (*proc) (w, closure, data);
	else Bell();
	break;

      case 1:
	if (!var) (*proc) (w, closure, data);
	else Bell();
	break;
    }
    return;
}

void HandleAllowSends(Widget w, XEvent *event, String *params, Cardinal *param_count)
{
    handle_toggle (do_allowsends, (int) term->screen.allowSendEvents,
		   params, *param_count, w, NULL, NULL);
}

void HandleSetVisualBell(Widget w, XEvent *event, String *params, Cardinal *param_count)
{
    handle_toggle (do_visualbell, (int) term->screen.visualbell,
		   params, *param_count, w, NULL, NULL);
}

#ifdef ALLOWLOGGING
void HandleLogging(w, event, params, param_count)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *param_count;
{
    handle_toggle (do_logging, (int) term->screen.logging,
		   params, *param_count, w, NULL, NULL);
}
#endif

/* ARGSUSED */
void HandleRedraw(Widget w, XEvent *event, String *params, Cardinal *param_count)
{
    do_redraw(w, NULL, NULL);
}

/* ARGSUSED */
void HandleSendSignal(Widget w, XEvent *event, String *params, Cardinal *param_count)
             
                  		/* unused */
                   
                          
{
    static struct sigtab {
	char *name;
	int sig;
    } signals[] = {
#ifdef SIGTSTP
	{ "suspend",	SIGTSTP },
	{ "tstp",	SIGTSTP },
#endif
#ifdef SIGCONT
	{ "cont",	SIGCONT },
#endif
	{ "int",	SIGINT },
	{ "hup",	SIGHUP },
	{ "quit",	SIGQUIT },
	{ "alrm",	SIGALRM },
	{ "alarm",	SIGALRM },
	{ "term",	SIGTERM },
	{ "kill",	SIGKILL },
	{ NULL, 0 },
    };

    if (*param_count == 1) {
	struct sigtab *st;

	for (st = signals; st->name; st++) {
	    if (XmuCompareISOLatin1 (st->name, params[0]) == 0) {
		handle_send_signal (w, st->sig);
		return;
	    }
	}
	/* one could allow numeric values, but that would be a security hole */
    }

    Bell();
}

/* ARGSUSED */
void HandleQuit(Widget w, XEvent *event, String *params, Cardinal *param_count)
{
    do_quit(w, NULL, NULL);
}

void HandleScrollbar(Widget w, XEvent *event, String *params, Cardinal *param_count)
{
    handle_toggle (do_scrollbar, (int) term->screen.scrollbar,
		   params, *param_count, w, NULL, NULL);
}

void HandleJumpscroll(Widget w, XEvent *event, String *params, Cardinal *param_count)
{
    handle_toggle (do_jumpscroll, (int) term->screen.jumpscroll,
		   params, *param_count, w, NULL, NULL);
}

void HandleReverseVideo(Widget w, XEvent *event, String *params, Cardinal *param_count)
{
    handle_toggle (do_reversevideo, (int) (term->flags & REVERSE_VIDEO),
		   params, *param_count, w, NULL, NULL);
}

void HandleAutoWrap(Widget w, XEvent *event, String *params, Cardinal *param_count)
{
    handle_toggle (do_autowrap, (int) (term->flags & WRAPAROUND),
		   params, *param_count, w, NULL, NULL);
}

void HandleReverseWrap(Widget w, XEvent *event, String *params, Cardinal *param_count)
{
    handle_toggle (do_reversewrap, (int) (term->flags & REVERSEWRAP),
		   params, *param_count, w, NULL, NULL);
}

void HandleAutoLineFeed(Widget w, XEvent *event, String *params, Cardinal *param_count)
{
    handle_toggle (do_autolinefeed, (int) (term->flags & LINEFEED),
		   params, *param_count, w, NULL, NULL);
}

void HandleAppCursor(Widget w, XEvent *event, String *params, Cardinal *param_count)
{
    handle_toggle (do_appcursor, (int) (term->keyboard.flags & CURSOR_APL),
		   params, *param_count, w, NULL, NULL);
}

void HandleAppKeypad(Widget w, XEvent *event, String *params, Cardinal *param_count)
{
    handle_toggle (do_appkeypad, (int) (term->keyboard.flags & KYPD_APL),
		   params, *param_count, w, NULL, NULL);
}

void HandleScrollKey(Widget w, XEvent *event, String *params, Cardinal *param_count)
{
    handle_toggle (do_scrollkey, (int) term->screen.scrollkey,
		   params, *param_count, w, NULL, NULL);
}

void HandleScrollTtyOutput(Widget w, XEvent *event, String *params, Cardinal *param_count)
{
    handle_toggle (do_scrollttyoutput, (int) term->screen.scrollttyoutput,
		   params, *param_count, w, NULL, NULL);
}

void HandleAllow132(Widget w, XEvent *event, String *params, Cardinal *param_count)
{
    handle_toggle (do_allow132, (int) term->screen.c132,
		   params, *param_count, w, NULL, NULL);
}

void HandleCursesEmul(Widget w, XEvent *event, String *params, Cardinal *param_count)
{
    handle_toggle (do_cursesemul, (int) term->screen.curses,
		   params, *param_count, w, NULL, NULL);
}

void HandleMarginBell(Widget w, XEvent *event, String *params, Cardinal *param_count)
{
    handle_toggle (do_marginbell, (int) term->screen.marginbell,
		   params, *param_count, w, NULL, NULL);
}

void HandleAltScreen(Widget w, XEvent *event, String *params, Cardinal *param_count)
{
    /* eventually want to see if sensitive or not */
    handle_toggle (do_altscreen, (int) term->screen.alternate,
		   params, *param_count, w, NULL, NULL);
}

/* ARGSUSED */
void HandleSoftReset(Widget w, XEvent *event, String *params, Cardinal *param_count)
{
    do_softreset(w, NULL, NULL);
}

/* ARGSUSED */
void HandleHardReset(Widget w, XEvent *event, String *params, Cardinal *param_count)
{
    do_hardreset(w, NULL, NULL);
}

/* ARGSUSED */
void HandleClearSavedLines(Widget w, XEvent *event, String *params, Cardinal *param_count)
{
    do_clearsavedlines(w, NULL, NULL);
}

void HandleSetTerminalType(Widget w, XEvent *event, String *params, Cardinal *param_count)
{
    if (*param_count == 1) {
	switch (params[0][0]) {
	  case 'v': case 'V':
	    if (term->screen.TekEmu) do_vtmode (w, NULL, NULL);
	    break;
	  case 't': case 'T':
	    if (!term->screen.TekEmu) do_tekmode (w, NULL, NULL);
	    break;
	  default:
	    Bell();
	}
    } else {
	Bell();
    }
}

void HandleVisibility(Widget w, XEvent *event, String *params, Cardinal *param_count)
{
    if (*param_count == 2) {
	switch (params[0][0]) {
	  case 'v': case 'V':
	    handle_toggle (do_vtonoff, (int) term->screen.Vshow,
			   params+1, (*param_count) - 1, w, NULL, NULL);
	    break;
	  case 't': case 'T':
	    handle_toggle (do_tekonoff, (int) term->screen.Tshow,
			   params+1, (*param_count) - 1, w, NULL, NULL);
	    break;
	  default:
	    Bell();
	}
    } else {
	Bell();
    }
}

/* ARGSUSED */
void HandleSetTekText(Widget w, XEvent *event, String *params, Cardinal *param_count)
{
    void (*proc)() = NULL;

    switch (*param_count) {
      case 0:
	proc = do_tektextlarge;
	break;
      case 1:
	switch (params[0][0]) {
	  case 'l': case 'L': proc = do_tektextlarge; break;
	  case '2': proc = do_tektext2; break;
	  case '3': proc = do_tektext3; break;
	  case 's': case 'S': proc = do_tektextsmall; break;
	}
	break;
    }
    if (proc) (*proc) (w, NULL, NULL);
    else Bell();
}

/* ARGSUSED */
void HandleTekPage(Widget w, XEvent *event, String *params, Cardinal *param_count)
{
    do_tekpage(w, NULL, NULL);
}

/* ARGSUSED */
void HandleTekReset(Widget w, XEvent *event, String *params, Cardinal *param_count)
{
    do_tekreset(w, NULL, NULL);
}

/* ARGSUSED */
void HandleTekCopy(Widget w, XEvent *event, String *params, Cardinal *param_count)
{
    do_tekcopy(w, NULL, NULL);
}


