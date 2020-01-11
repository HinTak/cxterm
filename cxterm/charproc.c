/* $Id: charproc.c,v 1.2.2.2 2004/08/14 02:43:03 htl10 Exp $ */
/***********************************************************************
* Copyright 1994 by Yongguang Zhang.
* Copyright 1990, 1991, 1992 by Yongguang Zhang and Pong Man-Chi.
*
* All rights reserved.  Under the same copyright and permission term as
* the original.  Absolutely no warranties of any kinds.
************************************************************************/

/*
 * $XConsortium: charproc.c,v 1.182 94/08/10 21:53:24 gildea Exp $
 */

/*
 
Copyright (c) 1988  X Consortium

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

/* charproc.c */

#include "ptyx.h"
#include "VTparse.h"
#include "data.h"
#include "error.h"
#include "menu.h"
#include "main.h"
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/StringDefs.h>
#include <X11/Xmu/Atoms.h>
#include <X11/Xmu/CharSet.h>
#include <X11/Xmu/Converters.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <setjmp.h>
#include <ctype.h>

#ifdef CXTERM_COMPATIBLE
# if (X11RELEASE <= 4)
#  define XtRGravity		"Gravity"
#  define XtAddConverter(a,b,c,d,e)	/* only place here, as nothing */
#  define XtGravity int			/* avoid typedef */
# endif
#endif

/*
 * Check for both EAGAIN and EWOULDBLOCK, because some supposedly POSIX
 * systems are broken and return EWOULDBLOCK when they should return EAGAIN.
 * Note that this macro may evaluate its argument more than once.
 */
#if defined(EAGAIN) && defined(EWOULDBLOCK)
#define E_TEST(err) ((err) == EAGAIN || (err) == EWOULDBLOCK)
#else
#ifdef EAGAIN
#define E_TEST(err) ((err) == EAGAIN)
#else
#define E_TEST(err) ((err) == EWOULDBLOCK)
#endif
#endif

extern jmp_buf VTend;

extern XtAppContext app_con;

extern Widget toplevel;

static void VTallocbuf(void);
static int finput(void);
static void dotext(register TScreen *screen, unsigned int flags, unsigned int fg, unsigned int bg, char charset, char *buf, char *ptr);
static void WriteText(register TScreen *screen, register char *str, register int len, unsigned int flags, unsigned int fg, unsigned int bg);
static void ToAlternate(register TScreen *screen);
static void FromAlternate(register TScreen *screen);
static void update_font_info(TScreen *screen, int doresize);

static void bitset(unsigned int *p, int mask), bitclr(unsigned int *p, int mask);
    
#define	DEFAULT		-1
#define	TEXT_BUF_SIZE	256
#define TRACKTIMESEC	4L
#define TRACKTIMEUSEC	0L
#define BELLSUPPRESSMSEC 200

#define XtNalwaysHighlight "alwaysHighlight"
#define XtNappcursorDefault "appcursorDefault"
#define XtNappkeypadDefault "appkeypadDefault"
#define XtNbellSuppressTime "bellSuppressTime"
#define XtNboldFont "boldFont"
#define XtNhilight "hilight"
#define XtNc132 "c132"
#define XtNcharClass "charClass"
#define XtNcurses "curses"
#define XtNhpLowerleftBugCompat "hpLowerleftBugCompat"
#define XtNcursorColor "cursorColor"

#define XtFcolor0 "fcolor0"
#define XtFcolor1 "fcolor1"
#define XtFcolor2 "fcolor2"
#define XtFcolor3 "fcolor3"
#define XtFcolor4 "fcolor4"
#define XtFcolor5 "fcolor5"
#define XtFcolor6 "fcolor6"
#define XtFcolor7 "fcolor7"
#define XtFcolorH0 "fcolorh0"
#define XtFcolorH1 "fcolorh1"
#define XtFcolorH2 "fcolorh2"
#define XtFcolorH3 "fcolorh3"
#define XtFcolorH4 "fcolorh4"
#define XtFcolorH5 "fcolorh5"
#define XtFcolorH6 "fcolorh6"
#define XtFcolorH7 "fcolorh7"

#define XtBcolor0 "bcolor0"
#define XtBcolor1 "bcolor1"
#define XtBcolor2 "bcolor2"
#define XtBcolor3 "bcolor3"
#define XtBcolor4 "bcolor4"
#define XtBcolor5 "bcolor5"
#define XtBcolor6 "bcolor6"
#define XtBcolor7 "bcolor7"

#define XtBBS       "bbs"
#define XtUseBold   "usebold"

#define XtNcutNewline "cutNewline"
#define XtNcutToBeginningOfLine "cutToBeginningOfLine"
#define XtNeightBitInput "eightBitInput"
#define XtNeightBitOutput "eightBitOutput"
#define XtNgeometry "geometry"
#define XtNtekGeometry "tekGeometry"
#ifdef HANZI
#define XtNhanziBoldFont "hanziBoldFont"
#define XtNhanziEncoding "hanziEncoding"
#define XtNhanziFont "hanziFont"
#define XtNhanziAssociation "hanziAssociation"
#define XtNhanziInputDir "hanziInputDir"
#define XtNhanziLineSpacing "hanziLineSpacing"
#define XtNhanziMode "hanziMode"
#endif /* HANZI */
#define XtNinternalBorder "internalBorder"
#define XtNjumpScroll "jumpScroll"
#ifdef ALLOWLOGGING
#define XtNlogFile "logFile"
#define XtNlogging "logging"
#define XtNlogInhibit "logInhibit"
#endif
#define XtNloginShell "loginShell"
#define XtNmarginBell "marginBell"
#define XtNpointerColor "pointerColor"
#define XtNpointerColorBackground "pointerColorBackground"
#define XtNpointerShape "pointerShape"
#define XtNmultiClickTime "multiClickTime"
#define XtNmultiScroll "multiScroll"
#define XtNnMarginBell "nMarginBell"
#define XtNresizeGravity "resizeGravity"
#define XtNreverseWrap "reverseWrap"
#define XtNautoWrap "autoWrap"
#define XtNsaveLines "saveLines"
#define XtNscrollBar "scrollBar"
#define XtNscrollTtyOutput "scrollTtyOutput"
#define XtNscrollKey "scrollKey"
#define XtNscrollLines "scrollLines"
#define XtNscrollPos "scrollPos"
#define XtNsignalInhibit "signalInhibit"
#define XtNtekInhibit "tekInhibit"
#define XtNtekSmall "tekSmall"
#define XtNtekStartup "tekStartup"
#define XtNtiteInhibit "titeInhibit"
#define XtNvisualBell "visualBell"
#define XtNallowSendEvents "allowSendEvents"

#define XtCAlwaysHighlight "AlwaysHighlight"
#define XtCAppcursorDefault "AppcursorDefault"
#define XtCAppkeypadDefault "AppkeypadDefault"
#define XtCBellSuppressTime "BellSuppressTime"
#define XtCBoldFont "BoldFont"
#define XtCC132 "C132"
#define XtCCharClass "CharClass"
#define XtCCurses "Curses"
#define XtCHpLowerleftBugCompat "HpLowerleftBugCompat"
#define XtCCutNewline "CutNewline"
#define XtCCutToBeginningOfLine "CutToBeginningOfLine"
#define XtCEightBitInput "EightBitInput"
#define XtCEightBitOutput "EightBitOutput"
#define XtCGeometry "Geometry"
#ifdef HANZI
#define XtCHanziBoldFont "HanziBoldFont"
#define XtCHanziEncoding "HanziEncoding"
#define XtCHanziFont "HanziFont"
#define XtCHanziAssociation "HanziAssociation"
#define XtCHanziInputDir "HanziInputDir"
#define XtCHanziLineSpacing "HanziLineSpacing"
#define XtCHanziMode "HanziMode"
#endif /* HANZI */
#define XtCJumpScroll "JumpScroll"
#ifdef ALLOWLOGGING
#define XtCLogfile "Logfile"
#define XtCLogging "Logging"
#define XtCLogInhibit "LogInhibit"
#endif
#define XtCLoginShell "LoginShell"
#define XtCMarginBell "MarginBell"
#define XtCMultiClickTime "MultiClickTime"
#define XtCMultiScroll "MultiScroll"
#define XtCColumn "Column"
#define XtCResizeGravity "ResizeGravity"
#define XtCReverseWrap "ReverseWrap"
#define XtCAutoWrap "AutoWrap"
#define XtCSaveLines "SaveLines"
#define XtCScrollBar "ScrollBar"
#define XtCScrollLines "ScrollLines"
#define XtCScrollPos "ScrollPos"
#define XtCScrollCond "ScrollCond"
#define XtCSignalInhibit "SignalInhibit"
#define XtCTekInhibit "TekInhibit"
#define XtCTekSmall "TekSmall"
#define XtCTekStartup "TekStartup"
#define XtCTiteInhibit "TiteInhibit"
#define XtCVisualBell "VisualBell"
#define XtCAllowSendEvents "AllowSendEvents"


static int nparam;
static ANSI reply;
static int param[NPARAM];

static unsigned long ctotal;
static unsigned long ntotal;
static jmp_buf vtjmpbuf;

extern int groundtable[];
extern int csitable[];
extern int dectable[];
extern int eigtable[];
extern int esctable[];
extern int iestable[];
extern int igntable[];
extern int scrtable[];
extern int scstable[];


/* event handlers */
extern void HandleKeyPressed(Widget w, XEvent *event, String *params, Cardinal *nparams), HandleEightBitKeyPressed(Widget w, XEvent *event, String *params, Cardinal *nparams);
extern void HandleStringEvent(Widget w, XEvent *event, String *params, Cardinal *nparams);
extern void HandleEnterWindow(Widget w, caddr_t eventdata, register XEnterWindowEvent *event);
extern void HandleLeaveWindow(Widget w, caddr_t eventdata, register XEnterWindowEvent *event);
extern void HandleBellPropertyChange(Widget w, XtPointer data, XEvent *ev, Boolean *more);
extern void HandleFocusChange(Widget w, caddr_t eventdata, register XFocusChangeEvent *event);
static void HandleKeymapChange(Widget w, XEvent *event, String *params, Cardinal *param_count);
extern void HandleInsertSelection(Widget w, XEvent *event, String *params, Cardinal *num_params);
extern void HandleSelectStart(Widget w, XEvent *event, String *params, Cardinal *num_params), HandleKeyboardSelectStart(Widget w, XEvent *event, String *params, Cardinal *num_params);
extern void HandleSelectExtend(Widget w, XEvent *event, String *params, Cardinal *num_params), HandleSelectSet(Widget w, XEvent *event, String *params, Cardinal *num_params);
extern void HandleSelectEnd(Widget w, XEvent *event, String *params, Cardinal *num_params), HandleKeyboardSelectEnd(Widget w, XEvent *event, String *params, Cardinal *num_params);
extern void HandleStartExtend(Widget w, XEvent *event, String *params, Cardinal *num_params), HandleKeyboardStartExtend(Widget w, XEvent *event, String *params, Cardinal *num_params);
static void HandleBell(Widget w, XEvent *event, String *params, Cardinal *param_count);
static void HandleVisualBell(Widget w, XEvent *event, String *params, Cardinal *param_count);
static void HandleIgnore(Widget w, XEvent *event, String *params, Cardinal *param_count);
extern void HandleSecure(Widget w, XEvent *event, String *params, Cardinal *param_count);
extern void HandleScrollForward(Widget gw, XEvent *event, String *params, Cardinal *nparams);
extern void HandleScrollBack(Widget gw, XEvent *event, String *params, Cardinal *nparams);
extern void HandleCreateMenu(Widget w, XEvent *event, String *params, Cardinal *param_count), HandlePopupMenu(Widget w, XEvent *event, String *params, Cardinal *param_count);
extern void HandleSetFont(Widget w, XEvent *event, String *params, Cardinal *param_count);
#ifdef HANZI
extern void HandleSwitchHZMode(Widget w, XEvent *event, String *params, Cardinal *nparams);
extern void HandleSetHZParam(Widget w, XEvent *event, String *params, Cardinal *nparams);
extern void HandleClickHZinArea(Widget w, XEvent *event, String *params, Cardinal *nparams);
extern void HandlePopupPanel(Widget widget, XEvent *event, String *params, Cardinal *num_params);
#endif /* HANZI */
extern void SetVTFont(int i, int doresize, char *name1, char *name2);
extern void ScrnBlinkRefresh (register TScreen *screen, int toprow, int leftcol, int nrows, int ncols);

extern Boolean SendMousePosition(Widget w, XEvent *event);
#ifdef HANZI
extern int HZ_XDrawImString16(Display *display, Drawable d, GC gc, int fgcs, int x, int y, XChar2b *string, int length);		/* defined in screen.c */
extern int HZ_XDrawString16(Display *display, Drawable d, GC gc, int fgcs, int x, int y, XChar2b *string, int length);		/* defined in screen.c */
#endif /* HANZI */

/*
 * NOTE: VTInitialize zeros out the entire ".screen" component of the 
 * XtermWidget, so make sure to add an assignment statement in VTInitialize() 
 * for each new ".screen" field added to this resource list.
 */

/* Defaults */
static  Boolean	defaultFALSE	   = FALSE;
static  Boolean	defaultTRUE	   = TRUE;
static  int	defaultIntBorder   = DEFBORDER;
static  int	defaultSaveLines   = SAVELINES;
static	int	defaultScrollLines = SCROLLLINES;
static  int	defaultNMarginBell = N_MARGINBELL;
static  int	defaultMultiClickTime = MULTICLICKTIME;
static  int	defaultBellSuppressTime = BELLSUPPRESSMSEC;
#ifdef HANZI
static  int	defaultLineSpacing = DEFLINESPACING;
#endif /* HANZI */
static	char *	_Font_Selected_ = "yes";  /* string is arbitrary */

/*
 * Warning, the following must be kept under 1024 bytes or else some 
 * compilers (particularly AT&T 6386 SVR3.2) will barf).  Workaround is to
 * declare a static buffer and copy in at run time (the the Athena text widget
 * does).  Yuck.
 */
#ifdef HANZI
/*
 * I cannot copy all the default translation here.
 * Otherwise it would be longer than 1024 bytes.  :-(	-ygz
 * The missing three lines are:
 *
 *   !Lock Ctrl <Btn1Down>:popup-menu(mainMenu) \n\
 *   !Lock Ctrl <Btn2Down>:popup-menu(vtMenu) \n\
 *   !Lock Ctrl <Btn3Down>:popup-menu(fontMenu) \n\
 */
static char defaultTranslations[] =
"\
 Shift <KeyPress> Prior:scroll-back(1,halfpage) \n\
  Shift <KeyPress> Next:scroll-forw(1,halfpage) \n\
Shift <KeyPress> Select:select-cursor-start() select-cursor-end(PRIMARY, CUT_BUFFER0) \n\
Shift <KeyPress> Insert:insert-selection(PRIMARY, CUT_BUFFER0) \n\
          <KeyPress> F1:switch-HZ-mode(ASCII) \n\
          <KeyPress> F2:switch-HZ-mode(IC) \n\
       ~Meta <KeyPress>:insert-seven-bit() \n\
        Meta <KeyPress>:insert-eight-bit() \n\
       !Ctrl <Btn1Down>:popup-menu(mainMenu) \n\
       ~Meta <Btn1Down>:select-start() \n\
     ~Meta <Btn1Motion>:select-extend() \n\
       !Ctrl <Btn2Down>:popup-menu(vtMenu) \n\
 ~Ctrl ~Meta <Btn2Down>:ignore() \n\
   ~Ctrl ~Meta <Btn2Up>:insert-selection(PRIMARY, CUT_BUFFER0) \n\
       !Ctrl <Btn3Down>:popup-menu(fontMenu) \n\
 ~Ctrl ~Meta <Btn3Down>:start-extend() \n\
     ~Meta <Btn3Motion>:select-extend()	\n\
                <BtnUp>:select-end(PRIMARY, CUT_BUFFER0) \n\
	      <BtnDown>:bell(0) \
";
#else  /* HANZI */
static char defaultTranslations[] =
"\
 Shift <KeyPress> Prior:scroll-back(1,halfpage) \n\
  Shift <KeyPress> Next:scroll-forw(1,halfpage) \n\
Shift <KeyPress> Select:select-cursor-start() select-cursor-end(PRIMARY, CUT_BUFFER0) \n\
Shift <KeyPress> Insert:insert-selection(PRIMARY, CUT_BUFFER0) \n\
       ~Meta <KeyPress>:insert-seven-bit() \n\
        Meta <KeyPress>:insert-eight-bit() \n\
       !Ctrl <Btn1Down>:popup-menu(mainMenu) \n\
  !Lock Ctrl <Btn1Down>:popup-menu(mainMenu) \n\
       ~Meta <Btn1Down>:select-start() \n\
     ~Meta <Btn1Motion>:select-extend() \n\
       !Ctrl <Btn2Down>:popup-menu(vtMenu) \n\
  !Lock Ctrl <Btn2Down>:popup-menu(vtMenu) \n\
 ~Ctrl ~Meta <Btn2Down>:ignore() \n\
   ~Ctrl ~Meta <Btn2Up>:insert-selection(PRIMARY, CUT_BUFFER0) \n\
       !Ctrl <Btn3Down>:popup-menu(fontMenu) \n\
  !Lock Ctrl <Btn3Down>:popup-menu(fontMenu) \n\
 ~Ctrl ~Meta <Btn3Down>:start-extend() \n\
     ~Meta <Btn3Motion>:select-extend()	\n\
                <BtnUp>:select-end(PRIMARY, CUT_BUFFER0) \n\
	      <BtnDown>:bell(0) \
";
#endif /* HANZI */

static XtActionsRec actionsList[] = { 
    { "bell",		  HandleBell },
    { "create-menu",	  HandleCreateMenu },
    { "ignore",		  HandleIgnore },
    { "insert",		  HandleKeyPressed },  /* alias for insert-seven-bit */
    { "insert-seven-bit", HandleKeyPressed },
    { "insert-eight-bit", HandleEightBitKeyPressed },
    { "insert-selection", HandleInsertSelection },
    { "keymap", 	  HandleKeymapChange },
    { "popup-menu",	  HandlePopupMenu },
    { "secure",		  HandleSecure },
    { "select-start",	  HandleSelectStart },
    { "select-extend",	  HandleSelectExtend },
    { "select-end",	  HandleSelectEnd },
    { "select-set",	  HandleSelectSet },
    { "select-cursor-start",	  HandleKeyboardSelectStart },
    { "select-cursor-end",	  HandleKeyboardSelectEnd },
    { "set-vt-font",	  HandleSetFont },
    { "start-extend",	  HandleStartExtend },
    { "start-cursor-extend",	  HandleKeyboardStartExtend },
    { "string",		  HandleStringEvent },
    { "scroll-forw",	  HandleScrollForward },
    { "scroll-back",	  HandleScrollBack },
#ifdef HANZI
    { "switch-HZ-mode",		HandleSwitchHZMode },
    { "set-HZ-parameter",	HandleSetHZParam },
    { "click-HZ-area",		HandleClickHZinArea },
    { "popup-panel",		HandlePopupPanel },
#endif /* HANZI */
    /* menu actions */
    { "allow-send-events",	HandleAllowSends },
    { "set-visual-bell",	HandleSetVisualBell },
#ifdef ALLOWLOGGING
    { "set-logging",		HandleLogging },
#endif
    { "redraw",			HandleRedraw },
    { "send-signal",		HandleSendSignal },
    { "quit",			HandleQuit },
    { "set-scrollbar",		HandleScrollbar },
    { "set-jumpscroll",		HandleJumpscroll },
    { "set-reverse-video",	HandleReverseVideo },
    { "set-autowrap",		HandleAutoWrap },
    { "set-reversewrap",	HandleReverseWrap },
    { "set-autolinefeed",	HandleAutoLineFeed },
    { "set-appcursor",		HandleAppCursor },
    { "set-appkeypad",		HandleAppKeypad },
    { "set-scroll-on-key",	HandleScrollKey },
    { "set-scroll-on-tty-output",	HandleScrollTtyOutput },
    { "set-allow132",		HandleAllow132 },
    { "set-cursesemul",		HandleCursesEmul },
    { "set-marginbell",		HandleMarginBell },
    { "set-altscreen",		HandleAltScreen },
    { "soft-reset",		HandleSoftReset },
    { "hard-reset",		HandleHardReset },
    { "clear-saved-lines",	HandleClearSavedLines },
    { "set-terminal-type",	HandleSetTerminalType },
    { "set-visibility",		HandleVisibility },
    { "set-tek-text",		HandleSetTekText },
    { "tek-page",		HandleTekPage },
    { "tek-reset",		HandleTekReset },
    { "tek-copy",		HandleTekCopy },
    { "visual-bell",		HandleVisualBell },
};

static XtResource resources[] = {
{XtNfont, XtCFont, XtRString, sizeof(char *),
	XtOffsetOf(XtermWidgetRec, misc.f_n), XtRString,
	DEFFONT},
{XtNboldFont, XtCBoldFont, XtRString, sizeof(char *),
	XtOffsetOf(XtermWidgetRec, misc.f_b), XtRString,
	DEFBOLDFONT},
#ifdef HANZI
{XtNhanziFont, XtCHanziFont, XtRString, sizeof(char *),
	XtOffsetOf(XtermWidgetRec, misc.f_hn), XtRString,
	DEFHZFONT},
{XtNhanziBoldFont, XtCHanziBoldFont, XtRString, sizeof(char *),
	XtOffsetOf(XtermWidgetRec, misc.f_hb), XtRString,
	DEFHZBOLDFONT},
{XtNhanziMode, XtCHanziMode, XtRString, sizeof(char *),
	XtOffsetOf(XtermWidgetRec, misc.hz_mode), XtRString,
	DEFHZMODE},
{XtNhanziEncoding, XtCHanziEncoding, XtRString, sizeof(char *),
	XtOffsetOf(XtermWidgetRec, misc.hz_encoding), XtRString,
	DEFHZENCODE},
{XtNhanziInputDir, XtCHanziInputDir, XtRString, sizeof(char *),
	XtOffsetOf(XtermWidgetRec, misc.it_dirs), XtRString,
	(XtPointer) NULL},
{XtNhanziAssociation, XtCHanziAssociation, XtRString, sizeof(char *),
	XtOffsetOf(XtermWidgetRec, misc.assoc_files), XtRString,
	(XtPointer) NULL},
{XtNhanziLineSpacing, XtCHanziLineSpacing, XtRInt, sizeof(int),
	XtOffsetOf(XtermWidgetRec, screen.lspacing),
	XtRInt, (XtPointer) &defaultLineSpacing},
#endif /* HANZI */
{XtNc132, XtCC132, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.c132),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNcharClass, XtCCharClass, XtRString, sizeof(char *),
	XtOffsetOf(XtermWidgetRec, screen.charClass),
	XtRString, (XtPointer) NULL},
{XtNcurses, XtCCurses, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.curses),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNhpLowerleftBugCompat, XtCHpLowerleftBugCompat, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.hp_ll_bc),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNcutNewline, XtCCutNewline, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.cutNewline),
	XtRBoolean, (XtPointer) &defaultTRUE},
{XtNcutToBeginningOfLine, XtCCutToBeginningOfLine, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.cutToBeginningOfLine),
	XtRBoolean, (XtPointer) &defaultTRUE},
{XtNbackground, XtCBackground, XtRPixel, sizeof(Pixel),
	XtOffsetOf(XtermWidgetRec, core.background_pixel),
	XtRString, "XtDefaultBackground"},
{XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
	XtOffsetOf(XtermWidgetRec, screen.foreground),
	XtRString, "XtDefaultForeground"},
{XtNcursorColor, XtCForeground, XtRPixel, sizeof(Pixel),
	XtOffsetOf(XtermWidgetRec, screen.cursorcolor),
	XtRString, "Green"},

{XtNhilight, XtCForeground, XtRPixel, sizeof (Pixel),
        XtOffsetOf(XtermWidgetRec, misc.hilight),
        XtRString, "XtDefaultForeground"},

{XtFcolor0, XtCForeground, XtRPixel, sizeof(Pixel),
        XtOffsetOf(XtermWidgetRec, screen.colors[FCOLOR_0]),
        XtRString, "Black"},
{XtFcolor1, XtCForeground, XtRPixel, sizeof(Pixel),
        XtOffsetOf(XtermWidgetRec, screen.colors[FCOLOR_1]),
        XtRString, "DarkRed"},
{XtFcolor2, XtCForeground, XtRPixel, sizeof(Pixel),
        XtOffsetOf(XtermWidgetRec, screen.colors[FCOLOR_2]),
        XtRString, "DarkGreen"},
{XtFcolor3, XtCForeground, XtRPixel, sizeof(Pixel),
        XtOffsetOf(XtermWidgetRec, screen.colors[FCOLOR_3]),
        XtRString, "Yellow3"},
{XtFcolor4, XtCForeground, XtRPixel, sizeof(Pixel),
        XtOffsetOf(XtermWidgetRec, screen.colors[FCOLOR_4]),
        XtRString, "DarkBlue"},
{XtFcolor5, XtCForeground, XtRPixel, sizeof(Pixel),
        XtOffsetOf(XtermWidgetRec, screen.colors[FCOLOR_5]),
        XtRString, "DarkMagenta"},
{XtFcolor6, XtCForeground, XtRPixel, sizeof(Pixel),
        XtOffsetOf(XtermWidgetRec, screen.colors[FCOLOR_6]),
        XtRString, "DarkCyan"},
{XtFcolor7, XtCForeground, XtRPixel, sizeof(Pixel),
        XtOffsetOf(XtermWidgetRec, screen.colors[FCOLOR_7]),
        XtRString, "Gray"},
{XtFcolorH0, XtCForeground, XtRPixel, sizeof(Pixel),
        XtOffsetOf(XtermWidgetRec, screen.colors[FCOLOR_H0]),
        XtRString, "DarkGray"},
{XtFcolorH1, XtCForeground, XtRPixel, sizeof(Pixel),
        XtOffsetOf(XtermWidgetRec, screen.colors[FCOLOR_H1]),
        XtRString, "Red"},
{XtFcolorH2, XtCForeground, XtRPixel, sizeof(Pixel),
        XtOffsetOf(XtermWidgetRec, screen.colors[FCOLOR_H2]),
        XtRString, "Green"},
{XtFcolorH3, XtCForeground, XtRPixel, sizeof(Pixel),
        XtOffsetOf(XtermWidgetRec, screen.colors[FCOLOR_H3]),
        XtRString, "Yellow"},
{XtFcolorH4, XtCForeground, XtRPixel, sizeof(Pixel),
        XtOffsetOf(XtermWidgetRec, screen.colors[FCOLOR_H4]),
        XtRString, "Blue"},
{XtFcolorH5, XtCForeground, XtRPixel, sizeof(Pixel),
        XtOffsetOf(XtermWidgetRec, screen.colors[FCOLOR_H5]),
        XtRString, "Magenta"},
{XtFcolorH6, XtCForeground, XtRPixel, sizeof(Pixel),
        XtOffsetOf(XtermWidgetRec, screen.colors[FCOLOR_H6]),
        XtRString, "Cyan"},
{XtFcolorH7, XtCForeground, XtRPixel, sizeof(Pixel),
        XtOffsetOf(XtermWidgetRec, screen.colors[FCOLOR_H7]),
        XtRString, "White"},

{XtBcolor0, XtCBackground, XtRPixel, sizeof(Pixel),
        XtOffsetOf(XtermWidgetRec, screen.colors[BCOLOR_0]),
        XtRString, "Black"},
{XtBcolor1, XtCBackground, XtRPixel, sizeof(Pixel),
        XtOffsetOf(XtermWidgetRec, screen.colors[BCOLOR_1]),
        XtRString, "DarkRed"},
{XtBcolor2, XtCBackground, XtRPixel, sizeof(Pixel),
        XtOffsetOf(XtermWidgetRec, screen.colors[BCOLOR_2]),
        XtRString, "DarkGreen"},
{XtBcolor3, XtCBackground, XtRPixel, sizeof(Pixel),
        XtOffsetOf(XtermWidgetRec, screen.colors[BCOLOR_3]),
        XtRString, "Yellow3"},
{XtBcolor4, XtCBackground, XtRPixel, sizeof(Pixel),
        XtOffsetOf(XtermWidgetRec, screen.colors[BCOLOR_4]),
        XtRString, "DarkBlue"},
{XtBcolor5, XtCBackground, XtRPixel, sizeof(Pixel),
        XtOffsetOf(XtermWidgetRec, screen.colors[BCOLOR_5]),
        XtRString, "DarkMagenta"},
{XtBcolor6, XtCBackground, XtRPixel, sizeof(Pixel),
        XtOffsetOf(XtermWidgetRec, screen.colors[BCOLOR_6]),
        XtRString, "DarkCyan"},
{XtBcolor7, XtCBackground, XtRPixel, sizeof(Pixel),
        XtOffsetOf(XtermWidgetRec, screen.colors[BCOLOR_7]),
        XtRString, "Gray"},

{XtNeightBitInput, XtCEightBitInput, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.input_eight_bits), 
	XtRBoolean, (XtPointer) &defaultTRUE},
{XtNeightBitOutput, XtCEightBitOutput, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.output_eight_bits), 
	XtRBoolean, (XtPointer) &defaultTRUE},
{XtNgeometry,XtCGeometry, XtRString, sizeof(char *),
	XtOffsetOf(XtermWidgetRec, misc.geo_metry),
	XtRString, (XtPointer) NULL},
{XtNalwaysHighlight,XtCAlwaysHighlight,XtRBoolean,
        sizeof(Boolean),XtOffsetOf(XtermWidgetRec, screen.always_highlight),
        XtRBoolean, (XtPointer) &defaultFALSE},
{XtNappcursorDefault,XtCAppcursorDefault,XtRBoolean,
        sizeof(Boolean),XtOffsetOf(XtermWidgetRec, misc.appcursorDefault),
        XtRBoolean, (XtPointer) &defaultFALSE},
{XtNappkeypadDefault,XtCAppkeypadDefault,XtRBoolean,
        sizeof(Boolean),XtOffsetOf(XtermWidgetRec, misc.appkeypadDefault),
        XtRBoolean, (XtPointer) &defaultFALSE},
{XtNbellSuppressTime, XtCBellSuppressTime, XtRInt, sizeof(int),
        XtOffsetOf(XtermWidgetRec, screen.bellSuppressTime),
        XtRInt, (XtPointer) &defaultBellSuppressTime},
{XtNtekGeometry,XtCGeometry, XtRString, sizeof(char *),
	XtOffsetOf(XtermWidgetRec, misc.T_geometry),
	XtRString, (XtPointer) NULL},
{XtNinternalBorder,XtCBorderWidth,XtRInt, sizeof(int),
	XtOffsetOf(XtermWidgetRec, screen.border),
	XtRInt, (XtPointer) &defaultIntBorder},
{XtNjumpScroll, XtCJumpScroll, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.jumpscroll),
	XtRBoolean, (XtPointer) &defaultTRUE},
#ifdef ALLOWLOGGING
{XtNlogFile, XtCLogfile, XtRString, sizeof(char *),
	XtOffsetOf(XtermWidgetRec, screen.logfile),
	XtRString, (XtPointer) NULL},
{XtNlogging, XtCLogging, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, misc.log_on),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNlogInhibit, XtCLogInhibit, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, misc.logInhibit),
	XtRBoolean, (XtPointer) &defaultFALSE},
#endif
{XtNloginShell, XtCLoginShell, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, misc.login_shell),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNmarginBell, XtCMarginBell, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.marginbell),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNpointerColor, XtCForeground, XtRPixel, sizeof(Pixel),
	XtOffsetOf(XtermWidgetRec, screen.mousecolor),
	XtRString, "XtDefaultForeground"},
{XtNpointerColorBackground, XtCBackground, XtRPixel, sizeof(Pixel),
	XtOffsetOf(XtermWidgetRec, screen.mousecolorback),
	XtRString, "XtDefaultBackground"},
{XtNpointerShape,XtCCursor, XtRCursor, sizeof(Cursor),
	XtOffsetOf(XtermWidgetRec, screen.pointer_cursor),
	XtRString, (XtPointer) "xterm"},
{XtNmultiClickTime,XtCMultiClickTime, XtRInt, sizeof(int),
	XtOffsetOf(XtermWidgetRec, screen.multiClickTime),
	XtRInt, (XtPointer) &defaultMultiClickTime},
{XtNmultiScroll,XtCMultiScroll, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.multiscroll),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNnMarginBell,XtCColumn, XtRInt, sizeof(int),
	XtOffsetOf(XtermWidgetRec, screen.nmarginbell),
	XtRInt, (XtPointer) &defaultNMarginBell},
{XtNreverseVideo,XtCReverseVideo,XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, misc.re_verse),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNresizeGravity, XtCResizeGravity, XtRGravity, sizeof(XtGravity),
	XtOffsetOf(XtermWidgetRec, misc.resizeGravity),
	XtRImmediate, (XtPointer) SouthWestGravity},
{XtNreverseWrap,XtCReverseWrap, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, misc.reverseWrap),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNautoWrap,XtCAutoWrap, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, misc.autoWrap),
	XtRBoolean, (XtPointer) &defaultTRUE},
{XtNsaveLines, XtCSaveLines, XtRInt, sizeof(int),
	XtOffsetOf(XtermWidgetRec, screen.savelines),
	XtRInt, (XtPointer) &defaultSaveLines},
{XtNscrollBar, XtCScrollBar, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, misc.scrollbar),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNscrollTtyOutput,XtCScrollCond, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.scrollttyoutput),
	XtRBoolean, (XtPointer) &defaultTRUE},
{XtNscrollKey, XtCScrollCond, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.scrollkey),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNscrollLines, XtCScrollLines, XtRInt, sizeof(int),
	XtOffsetOf(XtermWidgetRec, screen.scrolllines),
	XtRInt, (XtPointer) &defaultScrollLines},
{XtNsignalInhibit,XtCSignalInhibit,XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, misc.signalInhibit),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNtekInhibit, XtCTekInhibit, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, misc.tekInhibit),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNtekSmall, XtCTekSmall, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, misc.tekSmall),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNtekStartup, XtCTekStartup, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.TekEmu),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNtiteInhibit, XtCTiteInhibit, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, misc.titeInhibit),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNvisualBell, XtCVisualBell, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.visualbell),
	XtRBoolean, (XtPointer) &defaultFALSE},
{XtNallowSendEvents, XtCAllowSendEvents, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, screen.allowSendEvents),
	XtRBoolean, (XtPointer) &defaultFALSE},
{"font1", "Font1", XtRString, sizeof(String),
	XtOffsetOf(XtermWidgetRec, screen.menu_font_names[fontMenu_font1]),
	XtRString, (XtPointer) NULL},
{"font2", "Font2", XtRString, sizeof(String),
	XtOffsetOf(XtermWidgetRec, screen.menu_font_names[fontMenu_font2]),
	XtRString, (XtPointer) NULL},
{"font3", "Font3", XtRString, sizeof(String),
	XtOffsetOf(XtermWidgetRec, screen.menu_font_names[fontMenu_font3]),
	XtRString, (XtPointer) NULL},
{"font4", "Font4", XtRString, sizeof(String),
	XtOffsetOf(XtermWidgetRec, screen.menu_font_names[fontMenu_font4]),
	XtRString, (XtPointer) NULL},
{"font5", "Font5", XtRString, sizeof(String),
	XtOffsetOf(XtermWidgetRec, screen.menu_font_names[fontMenu_font5]),
	XtRString, (XtPointer) NULL},
{"font6", "Font6", XtRString, sizeof(String),
	XtOffsetOf(XtermWidgetRec, screen.menu_font_names[fontMenu_font6]),
	XtRString, (XtPointer) NULL},
{"dynamicColors", "DynamicColors", XtRBoolean, sizeof(Boolean),
	XtOffsetOf(XtermWidgetRec, misc.dynamicColors),
	XtRBoolean, (XtPointer) &defaultTRUE},

{XtBBS, XtCBoolean, XtRBoolean, sizeof (Boolean),
    XtOffsetOf (XtermWidgetRec, misc.bbs),
    XtRBoolean, (XtPointer)&defaultFALSE},
{XtUseBold, XtCBoolean, XtRBoolean, sizeof (Boolean),
    XtOffsetOf (XtermWidgetRec, misc.usebold),
    XtRBoolean, (XtPointer)&defaultTRUE}
};

static void VTClassInit(void);
static void VTInitialize(Widget wrequest, Widget wnew, ArgList args, Cardinal *num_args);
static void VTRealize(Widget w, XtValueMask *valuemask, XSetWindowAttributes *values);
static void VTExpose(Widget w, XEvent *event, Region region);
static void VTResize(Widget w);
static void VTDestroy(Widget w);
static Boolean VTSetValues(Widget cur, Widget request, Widget new, ArgList args, Cardinal *num_args);

static WidgetClassRec xtermClassRec = {
  {
/* core_class fields */	
    /* superclass	  */	(WidgetClass) &widgetClassRec,
    /* class_name	  */	"VT100",
    /* widget_size	  */	sizeof(XtermWidgetRec),
    /* class_initialize   */    VTClassInit,
    /* class_part_initialize */ NULL,
    /* class_inited       */	FALSE,
    /* initialize	  */	VTInitialize,
    /* initialize_hook    */    NULL,				
    /* realize		  */	VTRealize,
    /* actions		  */	actionsList,
    /* num_actions	  */	XtNumber(actionsList),
    /* resources	  */	resources,
    /* num_resources	  */	XtNumber(resources),
    /* xrm_class	  */	NULLQUARK,
    /* compress_motion	  */	TRUE,
    /* compress_exposure  */	FALSE,
    /* compress_enterleave */   TRUE,
    /* visible_interest	  */	FALSE,
    /* destroy		  */	VTDestroy,
    /* resize		  */	VTResize,
    /* expose		  */	VTExpose,
    /* set_values	  */	VTSetValues,
    /* set_values_hook    */    NULL,
    /* set_values_almost  */    NULL,
    /* get_values_hook    */    NULL,
    /* accept_focus	  */	NULL,
    /* version            */    XtVersion,
    /* callback_offsets   */    NULL,
    /* tm_table           */    defaultTranslations,
    /* query_geometry     */    XtInheritQueryGeometry,
    /* display_accelerator*/    XtInheritDisplayAccelerator,
    /* extension          */    NULL
  }
};

WidgetClass xtermWidgetClass = (WidgetClass)&xtermClassRec;

void ansi_modes (XtermWidget termw, void (*func) (/* ??? */));
void dpmodes (XtermWidget termw, void (*func) (/* ??? */));
void savemodes (XtermWidget termw);
void restoremodes (XtermWidget termw);
void unparseseq (register ANSI *ap, int fd);
void unparseputn (unsigned int n, int fd);
void unparseputc (char c, int fd);
void SwitchBufPtrs (register TScreen *screen);
void ShowCursor (void);
void HideCursor (void);
void VTReset (Boolean full);
void set_vt_box (TScreen *screen);
void set_cursor_gcs (TScreen *screen);


int in_put (void);
extern void Bell (void);
extern void CursorBack (register TScreen *screen, int n);
extern void CarriageReturn (register TScreen *screen);
extern void Index (register TScreen *screen, register int amount);
extern void xevents (void);
extern int TabNext (unsigned int *tabs, int col);
extern void InsertChar (register TScreen *screen, register int n);
extern void CursorUp (register TScreen *screen, int n);
extern void CursorDown (register TScreen *screen, int n);
extern void CursorForward (register TScreen *screen, int n);
extern void CursorSet (register TScreen *screen, register int row, register int col, unsigned int flags);
extern void ClearBelow (register TScreen *screen);
extern void ClearAbove (register TScreen *screen);
extern void ClearScreen (register TScreen *screen);
extern void RefreshCXtermInput (TScreen *screen);
extern void ClearRight (register TScreen *screen);
extern void ClearLeft (register TScreen *screen);
extern void ClearLine (register TScreen *screen);
extern void InsertLine (register TScreen *screen, register int n);
extern void DeleteLine (register TScreen *screen, register int n);
extern void DeleteChar (register TScreen *screen, register int n);
extern void TabClear (unsigned int *tabs, int col);
extern void TabZonk (unsigned int *tabs);
extern void FlushScroll (register TScreen *screen);
extern void ScrnRefresh (register TScreen *screen, int toprow, int leftcol, int nrows, int ncols, Boolean force);
extern void CursorSave (register XtermWidget term, register SavedCursor *sc);
extern void CursorRestore (register XtermWidget term, register SavedCursor *sc);
extern void TabSet (unsigned int *tabs, int col);
extern void RevIndex (register TScreen *screen, register int amount);
extern void do_osc (int (*func) (/* ??? */));
extern void Cleanup (int code);
extern void Panic (char *s, int a);
extern void WindowScroll (register TScreen *screen, int top);
extern void SysError (int i);
extern int AddToRefresh (register TScreen *screen);
extern void ScreenWrite (TScreen *screen, char *str, register unsigned int flags, register unsigned int cur_fg, register unsigned int cur_bg, register int length);
extern int ScreenResize (register TScreen *screen, int width, int height, unsigned long *flags);
extern void ReverseVideo (XtermWidget termw);
extern void TrackText (register int frow, register int fcol, register int trow, register int tcol);
extern void set_vt_visibility (Boolean on);
extern int HandleExposure (register TScreen *screen, register XEvent *event);
int set_character_class (register char *s);
extern void CreateCXtermInput (XtermWidget xw);
extern void ScrollBarOn (XtermWidget xw, int init, int doalloc);
extern void DestroyCXtermInput (XtermWidget xw);
extern void TabReset (unsigned int *tabs);
int LoadNewFont (TScreen *screen, char *nfontname, char *bfontname, int doresize, int fontnum);
extern void Exit (int n);
extern void recolor_cursor (Cursor cursor, long unsigned int fg, long unsigned int bg);
extern void ResizeCXtermInput (TScreen *screen);
extern void RealizeCXtermInput (XtermWidget xw);
extern void ScrollBarOff (register TScreen *screen);
extern void ResetCXtermInput (XtermWidget xw);
extern int SetCharacterClassRange (register int low, register int high, register int value);
extern void VisualBell (void);
extern void DoResizeScreen (register XtermWidget xw);
extern void ResizeScrollBar (register Widget scrollWidget, int x, int y, unsigned int height);
extern void Redraw (void);

static void rollTheWheel (void)
{
    if ((XtAppPending (app_con)&XtIMTimer)) {
        do {
            XtAppProcessEvent (app_con, XtIMTimer);
        } while ((XtAppPending (app_con)&XtIMTimer));
    }
}

static int doinput (void)
{
    if (bcnt > 0) {
        --bcnt;
        rollTheWheel ();
        return ((int)*bptr++);
    }
    return (in_put ());
}

/* This function has been modified by Zhuang Hao S.S.* */
static void VTparse(void)
{
	register TScreen *screen = &term->screen;
	register int *parsestate = groundtable;
	register unsigned int c;
	register unsigned char *cp;
	register int row, col, top, bot, scstype=0;
	extern int TrackMouse(int func, int startrow, int startcol, int firstrow, int lastrow);
#ifdef HANZI
	int	idx;
	Char	*cols, *attrs;
#endif /* HANZI */

	if(setjmp(vtjmpbuf))
		parsestate = groundtable;
	for( ; ; ) {
        /* doinput (): return the first char from the input queue
         *             and move the queue pointer to the next char.
         *             Thus, on return, bptr is pointed to the next char
         *             and *(bptr-1) is the char returned.
         *             bcnt is the number of chars still in queue.
         */

        switch (parsestate[c = doinput()]) {
		 case CASE_PRINT:
            /* printable characters */
            *--bptr = c;
            ++bcnt;
            if (screen->cur_col > 0) {
                cols = screen->buf[(idx = screen->cur_row*4)]+screen->cur_col-1;
                attrs = screen->buf[idx+1]+screen->cur_col-1;
                if (Is1stByteHZ (*attrs)) {
                    *--bptr = c = *cols;
                    --(screen->cur_col);
                    ++bcnt;
                }
            }
            cp = bptr;
            top = bcnt > TEXT_BUF_SIZE ? TEXT_BUF_SIZE : bcnt;
            do {
                if ((c&0x80)) {
                    while (top > 0 && ((*cp)&0x80) && groundtable[*cp] == CASE_PRINT) {
                        ++cp;
                        --bcnt;
                        --top;
                        if (! top || groundtable[*cp] != CASE_PRINT) {
                            break;
	    				}
                        ++cp;
                        --bcnt;
                        --top;
                    }
                    dotext (screen, (unsigned)(term->flags|CHARHANZI),
                      term->cur_foreground,
                      term->cur_background,
                      'B', bptr, cp);   /* ASCII Mode, don't remap any char ! */
                }
                else {
                    while (top > 0 && groundtable[*cp] == CASE_PRINT
                      && ! ((*cp)&0x80)) {
                        ++cp;
                        --top;
                        --bcnt;
                    }
                    if (screen->curss) {
                        dotext (screen, (unsigned)(term->flags&~CHARHANZI),
                          term->cur_foreground,
                          term->cur_background,
                          screen->gsets[screen->curss], bptr, bptr + 1);
                        screen->curss = 0;
                        bptr++;
                    }
                    if (bptr < cp) {
                        dotext (screen, (unsigned)(term->flags&~CHARHANZI),
                          term->cur_foreground,
                          term->cur_background,
                          screen->gsets[screen->curgl], bptr, cp);
                    }
                }
                c = *(bptr = cp);
            } while (groundtable[c] == CASE_PRINT && top > 0);
			bptr = cp;
			break;

		 case CASE_GROUND_STATE:
			/* exit ignore mode */
			parsestate = groundtable;
			break;

		 case CASE_IGNORE_STATE:
			/* Ies: ignore anything else */
			parsestate = igntable;
			break;

		 case CASE_IGNORE_ESC:
			/* Ign: escape */
			parsestate = iestable;
			break;

		 case CASE_IGNORE:
			/* Ignore character */
			break;

		 case CASE_BELL:
			/* bell */
			Bell();
			break;

		 case CASE_BS:
			/* backspace */
			CursorBack(screen, 1);
			break;

		 case CASE_CR:
			/* carriage return */
			CarriageReturn(screen);
			parsestate = groundtable;
            if (screen->ext_flags&BBS) {
                term->flags &=
                  ~(INVERSE|BLINK|UNDERLINE|FG_COLOR|BG_COLOR|BOLD);
            }
			break;

		 case CASE_ESC:
			/* escape */
			parsestate = esctable;
			break;

		 case CASE_VMOT:
			/*
			 * form feed, line feed, vertical tab
			 */
			Index(screen, 1);
			if (term->flags & LINEFEED)
				CarriageReturn(screen);
			if (QLength(screen->display) > 0 ||
			    GetBytesAvailable (ConnectionNumber(screen->display)) > 0)
			  xevents();
			parsestate = groundtable;
            if (screen->ext_flags&BBS) {
                term->flags &=
                  ~(INVERSE|BLINK|UNDERLINE|FG_COLOR|BG_COLOR|BOLD);
            }
			break;

		 case CASE_TAB:
			/* tab */
			screen->cur_col = TabNext(term->tabs, screen->cur_col);
			if (screen->cur_col > screen->max_col)
				screen->cur_col = screen->max_col;
			break;

		 case CASE_SI:
			screen->curgl = 0;
			break;

		 case CASE_SO:
			screen->curgl = 1;
			break;

		 case CASE_SCR_STATE:
			/* enter scr state */
			parsestate = scrtable;
			break;

		 case CASE_SCS0_STATE:
			/* enter scs state 0 */
			scstype = 0;
			parsestate = scstable;
			break;

		 case CASE_SCS1_STATE:
			/* enter scs state 1 */
			scstype = 1;
			parsestate = scstable;
			break;

		 case CASE_SCS2_STATE:
			/* enter scs state 2 */
			scstype = 2;
			parsestate = scstable;
			break;

		 case CASE_SCS3_STATE:
			/* enter scs state 3 */
			scstype = 3;
			parsestate = scstable;
			break;

		 case CASE_ESC_IGNORE:
			/* unknown escape sequence */
			parsestate = eigtable;
			break;

		 case CASE_ESC_DIGIT:
			/* digit in csi or dec mode */
			if((row = param[nparam - 1]) == DEFAULT)
				row = 0;
			param[nparam - 1] = 10 * row + (c - '0');
			break;

		 case CASE_ESC_SEMI:
			/* semicolon in csi or dec mode */
			param[nparam++] = DEFAULT;
			break;

		 case CASE_DEC_STATE:
			/* enter dec mode */
			parsestate = dectable;
			break;

		 case CASE_ICH:
			/* ICH */
			if((row = param[0]) < 1)
				row = 1;
			InsertChar(screen, row);
			parsestate = groundtable;
			break;

		 case CASE_CUU:
			/* CUU */
			if((row = param[0]) < 1)
				row = 1;
			CursorUp(screen, row);
			parsestate = groundtable;
			break;

		 case CASE_CUD:
			/* CUD */
			if((row = param[0]) < 1)
				row = 1;
			CursorDown(screen, row);
			parsestate = groundtable;
			break;

		 case CASE_CUF:
			/* CUF */
			if((row = param[0]) < 1)
				row = 1;
			CursorForward(screen, row);
			parsestate = groundtable;
			break;

		 case CASE_CUB:
			/* CUB */
			if((row = param[0]) < 1)
				row = 1;
			CursorBack(screen, row);
			parsestate = groundtable;
			break;

		 case CASE_CUP:
			/* CUP | HVP */
			if((row = param[0]) < 1)
				row = 1;
			if(nparam < 2 || (col = param[1]) < 1)
				col = 1;
			CursorSet(screen, row-1, col-1, term->flags);
			parsestate = groundtable;
			break;

		 case CASE_HP_BUGGY_LL:
			/* Some HP-UX applications have the bug that they
			   assume ESC F goes to the lower left corner of
			   the screen, regardless of what terminfo says. */
			if (screen->hp_ll_bc)
			    CursorSet(screen, screen->max_row, 0, term->flags);
			parsestate = groundtable;
			break;

		 case CASE_ED:
			/* ED */
			switch (param[0]) {
			 case DEFAULT:
			 case 0:
				ClearBelow(screen);
				break;

			 case 1:
				ClearAbove(screen);
				break;

			 case 2:
				ClearScreen(screen);
				break;
			}
			parsestate = groundtable;
#ifdef HANZI
			RefreshCXtermInput(screen);
#endif /* HANZI */
			break;

		 case CASE_EL:
			/* EL */
			switch (param[0]) {
			 case DEFAULT:
			 case 0:
				ClearRight(screen);
				break;
			 case 1:
				ClearLeft(screen);
				break;
			 case 2:
				ClearLine(screen);
				break;
			}
			parsestate = groundtable;
			break;

		 case CASE_IL:
			/* IL */
			if((row = param[0]) < 1)
				row = 1;
			InsertLine(screen, row);
			parsestate = groundtable;
			break;

		 case CASE_DL:
			/* DL */
			if((row = param[0]) < 1)
				row = 1;
			DeleteLine(screen, row);
			parsestate = groundtable;
			break;

		 case CASE_DCH:
			/* DCH */
			if((row = param[0]) < 1)
				row = 1;
			DeleteChar(screen, row);
			parsestate = groundtable;
			break;

		 case CASE_TRACK_MOUSE:
		 	/* Track mouse as long as in window and between
			   specified rows */
			TrackMouse(param[0], param[2]-1, param[1]-1,
			 param[3]-1, param[4]-2);
			break;

		 case CASE_DECID:
			param[0] = -1;		/* Default ID parameter */
			/* Fall through into ... */
		 case CASE_DA1:
			/* DA1 */
			if (param[0] <= 0) {	/* less than means DEFAULT */
				reply.a_type   = CSI;
				reply.a_pintro = '?';
				reply.a_nparam = 2;
				reply.a_param[0] = 1;		/* VT102 */
				reply.a_param[1] = 2;		/* VT102 */
				reply.a_inters = 0;
				reply.a_final  = 'c';
				unparseseq(&reply, screen->respond);
			}
			parsestate = groundtable;
			break;

		 case CASE_TBC:
			/* TBC */
			if ((row = param[0]) <= 0) /* less than means default */
				TabClear(term->tabs, screen->cur_col);
			else if (row == 3)
				TabZonk(term->tabs);
			parsestate = groundtable;
			break;

		 case CASE_SET:
			/* SET */
			ansi_modes(term, bitset);
			parsestate = groundtable;
			break;

		 case CASE_RST:
			/* RST */
			ansi_modes(term, bitclr);
			parsestate = groundtable;
			break;

		 case CASE_SGR:
			/* SGR */
			for (row=0; row<nparam; ++row) {
				switch (param[row]) {
				 case DEFAULT:
				 case 0:
					term->flags &=
					  ~(INVERSE|BLINK|UNDERLINE|FG_COLOR|
					    BG_COLOR|BOLD);
					break;
				 case 1:
                    term->flags |= BOLD;
                    break;
				 case 5:	/* Blink, really.	*/
					term->flags |= BLINK;
					break;
				 case 2:
					term->flags &= ~BOLD;
					break;
				 case 4:	/* Underscore		*/
					term->flags |= UNDERLINE;
					break;
				 case 7:
					term->flags |= INVERSE;
					break;
				 case 30:
				 case 31:
				 case 32:
				 case 33:
				 case 34:
				 case 35:
				 case 36:
				 case 37:
					term->flags |= FG_COLOR;
					term->cur_foreground = param[row]-30;
                    if ((term->flags&BOLD)) {
                        term->cur_foreground += 8;
                    }
					break;
				 case 40:
				 case 41:
				 case 42:
				 case 43:
				 case 44:
				 case 45:
				 case 46:
				 case 47:
					term->flags |= BG_COLOR;
					term->cur_background = param[row] - 40;
					break;
				}
			}
			parsestate = groundtable;
			break;

		 case CASE_CPR:
			/* CPR */
			if ((row = param[0]) == 5) {
				reply.a_type = CSI;
				reply.a_pintro = 0;
				reply.a_nparam = 1;
				reply.a_param[0] = 0;
				reply.a_inters = 0;
				reply.a_final  = 'n';
				unparseseq(&reply, screen->respond);
			} else if (row == 6) {
				reply.a_type = CSI;
				reply.a_pintro = 0;
				reply.a_nparam = 2;
				reply.a_param[0] = screen->cur_row+1;
				reply.a_param[1] = screen->cur_col+1;
				reply.a_inters = 0;
				reply.a_final  = 'R';
				unparseseq(&reply, screen->respond);
			}
			parsestate = groundtable;
			break;

		 case CASE_HP_MEM_LOCK:
		 case CASE_HP_MEM_UNLOCK:
			if(screen->scroll_amt)
			    FlushScroll(screen);
			if (parsestate[c] == CASE_HP_MEM_LOCK)
			    screen->top_marg = screen->cur_row;
			else
			    screen->top_marg = 0;
			parsestate = groundtable;
			break;

		 case CASE_DECSTBM:
			/* DECSTBM - set scrolling region */
			if((top = param[0]) < 1)
				top = 1;
			if(nparam < 2 || (bot = param[1]) == DEFAULT
			   || bot > screen->max_row + 1
			   || bot == 0)
				bot = screen->max_row+1;
			if (bot > top) {
				if(screen->scroll_amt)
					FlushScroll(screen);
				screen->top_marg = top-1;
				screen->bot_marg = bot-1;
				CursorSet(screen, 0, 0, term->flags);
			}
			parsestate = groundtable;
			break;

		 case CASE_DECREQTPARM:
			/* DECREQTPARM */
			if ((row = param[0]) == DEFAULT)
				row = 0;
			if (row == 0 || row == 1) {
				reply.a_type = CSI;
				reply.a_pintro = 0;
				reply.a_nparam = 7;
				reply.a_param[0] = row + 2;
				reply.a_param[1] = 1;	/* no parity */
				reply.a_param[2] = 1;	/* eight bits */
				reply.a_param[3] = 112;	/* transmit 9600 baud */
				reply.a_param[4] = 112;	/* receive 9600 baud */
				reply.a_param[5] = 1;	/* clock multiplier ? */
				reply.a_param[6] = 0;	/* STP flags ? */
				reply.a_inters = 0;
				reply.a_final  = 'x';
				unparseseq(&reply, screen->respond);
			}
			parsestate = groundtable;
			break;

		 case CASE_DECSET:
			/* DECSET */
			dpmodes(term, bitset);
			parsestate = groundtable;
			if(screen->TekEmu)
				return;
			break;

		 case CASE_DECRST:
			/* DECRST */
			dpmodes(term, bitclr);
			parsestate = groundtable;
			break;

		 case CASE_DECALN:
			/* DECALN */
			if(screen->cursor_state)
				HideCursor();
			for(row = screen->max_row ; row >= 0 ; row--) {
				bzero(screen->buf[4 * row + 1],
				 col = screen->max_col + 1);
				for(cp = (unsigned char *)screen->buf[4 * row] ; col > 0 ; col--)
					*cp++ = (unsigned char) 'E';
			}
			ScrnRefresh(screen, 0, 0, screen->max_row + 1,
			 screen->max_col + 1, False);
			parsestate = groundtable;
			break;

		 case CASE_GSETS:
			screen->gsets[scstype] = c;
			parsestate = groundtable;
			break;

		 case CASE_DECSC:
			/* DECSC */
			CursorSave(term, &screen->sc);
			parsestate = groundtable;
			break;

		 case CASE_DECRC:
			/* DECRC */
			CursorRestore(term, &screen->sc);
			parsestate = groundtable;
			break;

		 case CASE_DECKPAM:
			/* DECKPAM */
			term->keyboard.flags |= KYPD_APL;
			update_appkeypad();
			parsestate = groundtable;
			break;

		 case CASE_DECKPNM:
			/* DECKPNM */
			term->keyboard.flags &= ~KYPD_APL;
			update_appkeypad();
			parsestate = groundtable;
			break;

		 case CASE_IND:
			/* IND */
			Index(screen, 1);
			if (QLength(screen->display) > 0 ||
			    GetBytesAvailable (ConnectionNumber(screen->display)) > 0)
			  xevents();
			parsestate = groundtable;
			break;

		 case CASE_NEL:
			/* NEL */
			Index(screen, 1);
			CarriageReturn(screen);
			
			if (QLength(screen->display) > 0 ||
			    GetBytesAvailable (ConnectionNumber(screen->display)) > 0)
			  xevents();
			parsestate = groundtable;
			break;

		 case CASE_HTS:
			/* HTS */
			TabSet(term->tabs, screen->cur_col);
			parsestate = groundtable;
			break;

		 case CASE_RI:
			/* RI */
			RevIndex(screen, 1);
			parsestate = groundtable;
			break;

		 case CASE_SS2:
			/* SS2 */
			screen->curss = 2;
			parsestate = groundtable;
			break;

		 case CASE_SS3:
			/* SS3 */
			screen->curss = 3;
			parsestate = groundtable;
			break;

		 case CASE_CSI_STATE:
			/* enter csi state */
			nparam = 1;
			param[0] = DEFAULT;
			parsestate = csitable;
			break;

		 case CASE_OSC:
			/* Operating System Command: ESC ] */
			do_osc(finput);
			parsestate = groundtable;
			break;

		 case CASE_RIS:
			/* RIS */
			VTReset(TRUE);
			parsestate = groundtable;
			break;

		 case CASE_LS2:
			/* LS2 */
			screen->curgl = 2;
			parsestate = groundtable;
			break;

		 case CASE_LS3:
			/* LS3 */
			screen->curgl = 3;
			parsestate = groundtable;
			break;

		 case CASE_LS3R:
			/* LS3R */
			screen->curgr = 3;
			parsestate = groundtable;
			break;

		 case CASE_LS2R:
			/* LS2R */
			screen->curgr = 2;
			parsestate = groundtable;
			break;

		 case CASE_LS1R:
			/* LS1R */
			screen->curgr = 1;
			parsestate = groundtable;
			break;

		 case CASE_XTERM_SAVE:
			savemodes(term);
			parsestate = groundtable;
			break;

		 case CASE_XTERM_RESTORE:
			restoremodes(term);
			parsestate = groundtable;
			break;
		}
	}
}

static int finput(void)
{
	return(doinput());
}


static Char *v_buffer;		/* pointer to physical buffer */
static Char *v_bufstr = NULL;	/* beginning of area to write */
static Char *v_bufptr;		/* end of area to write */
static Char *v_bufend;		/* end of physical buffer */

/* These lines are modified by Zhuang Hao S.S.* */
/* #define	ptymask()	(v_bufptr > v_bufstr ? pty_mask : 0) */
#include    <string.h>

/* Write data to the pty as typed by the user, pasted with the mouse,
   or generated by us in response to a query ESC sequence. */

int v_write(int f, Char *data, int len)
{
	int riten;
	int c = len;

	if (v_bufstr == NULL  &&  len > 0) {
	        v_buffer = XtMalloc(len);
		v_bufstr = v_buffer;
		v_bufptr = v_buffer;
		v_bufend = v_buffer + len;
	}
#ifdef DEBUG
	if (debug) {
	    fprintf(stderr, "v_write called with %d bytes (%d left over)",
		    len, v_bufptr - v_bufstr);
	    if (len > 1  &&  len < 10) 
	      fprintf(stderr, " \"%.*s\"", len, (char *)data);
	    fprintf(stderr, "\n");
	}
#endif

    /* This line is modified by Zhuang Hao S.S.* */
	if (f != g__pty_fd)
		return(write(f, (char *)data, len));

	/*
	 * Append to the block we already have.
	 * Always doing this simplifies the code, and
	 * isn't too bad, either.  If this is a short
	 * block, it isn't too expensive, and if this is
	 * a long block, we won't be able to write it all
	 * anyway.
	 */

	if (len > 0) {
	    if (v_bufend < v_bufptr + len) { /* we've run out of room */
		if (v_bufstr != v_buffer) {
		    /* there is unused space, move everything down */
		    /* possibly overlapping memmove here */
#ifdef DEBUG
		    if (debug)
			fprintf(stderr, "moving data down %d\n",
				v_bufstr - v_buffer);
#endif
		    memmove( v_buffer, v_bufstr, v_bufptr - v_bufstr);
		    v_bufptr -= v_bufstr - v_buffer;
		    v_bufstr = v_buffer;
		}
		if (v_bufend < v_bufptr + len) {
		    /* still won't fit: get more space */
		    /* Don't use XtRealloc because an error is not fatal. */
		    int size = v_bufptr - v_buffer; /* save across realloc */
		    v_buffer = (Char *)realloc(v_buffer, size + len);
		    if (v_buffer) {
#ifdef DEBUG
			if (debug)
			    fprintf(stderr, "expanded buffer to %d\n",
				    size + len);
#endif
			v_bufstr = v_buffer;
			v_bufptr = v_buffer + size;
			v_bufend = v_bufptr + len;
		    } else {
			/* no memory: ignore entire write request */
			fprintf(stderr, "%s: cannot allocate buffer space\n",
				xterm_name);
			v_buffer = v_bufstr; /* restore clobbered pointer */
			c = 0;
		    }
		}
	    }
	    if (v_bufend >= v_bufptr + len) {
		/* new stuff will fit */
		memmove( v_bufptr, data, len);
		v_bufptr += len;
	    }
	}

	/*
	 * Write out as much of the buffer as we can.
	 * Be careful not to overflow the pty's input silo.
	 * We are conservative here and only write
	 * a small amount at a time.
	 *
	 * If we can't push all the data into the pty yet, we expect write
	 * to return a non-negative number less than the length requested
	 * (if some data written) or -1 and set errno to EAGAIN,
	 * EWOULDBLOCK, or EINTR (if no data written).
	 *
	 * (Not all systems do this, sigh, so the code is actually
	 * a little more forgiving.)
	 */

#define MAX_PTY_WRITE 128	/* 1/2 POSIX minimum MAX_INPUT */

	if (v_bufptr > v_bufstr) {
	    riten = write(f, v_bufstr, v_bufptr - v_bufstr <= MAX_PTY_WRITE ?
			  	       v_bufptr - v_bufstr : MAX_PTY_WRITE);
	    if (riten < 0) {
#ifdef DEBUG
		if (debug) perror("write");
#endif
		riten = 0;
	    }
#ifdef DEBUG
	    if (debug)
		fprintf(stderr, "write called with %d, wrote %d\n",
			v_bufptr - v_bufstr <= MAX_PTY_WRITE ?
			v_bufptr - v_bufstr : MAX_PTY_WRITE,
			riten);
#endif
	    v_bufstr += riten;
	    if (v_bufstr >= v_bufptr) /* we wrote it all */
		v_bufstr = v_bufptr = v_buffer;
	}

	/*
	 * If we have lots of unused memory allocated, return it
	 */
	if (v_bufend - v_bufptr > 1024) { /* arbitrary hysteresis */
	    /* save pointers across realloc */
	    int start = v_bufstr - v_buffer;
	    int size = v_bufptr - v_buffer;
	    int allocsize = size ? size : 1;
	    
	    v_buffer = (Char *)realloc(v_buffer, allocsize);
	    if (v_buffer) {
		v_bufstr = v_buffer + start;
		v_bufptr = v_buffer + size;
		v_bufend = v_buffer + allocsize;
#ifdef DEBUG
		if (debug)
		    fprintf(stderr, "shrunk buffer to %d\n", allocsize);
#endif
	    } else {
		/* should we print a warning if couldn't return memory? */
		v_buffer = v_bufstr - start; /* restore clobbered pointer */
	    }
	}
	return(c);
}

/* These 2 lines are modified by Zhuang Hao S.S.* */
static fd_set select_mask;
static fd_set write_mask;

static int pty_read_bytes;

int in_put(void)
{
    int qlen;
    register TScreen *screen = &term->screen;
    register int i;
    static struct timeval select_timeout;

    for( ; ; ) {
        rollTheWheel ();
        /* This line is modified by Zhuang Hao S.S.* */
        if (FD_ISSET (g__pty_fd, &select_mask) && eventMode == NORMAL) {
#ifdef ALLOWLOGGING
	    if (screen->logging)
		FlushLog(screen);
#endif
	    bcnt = read(screen->respond, (char *)(bptr = buffer), BUF_SIZE);
	    if (bcnt < 0) {
		if (errno == EIO) {
		    Cleanup (0);
		}
		else if (!E_TEST(errno)) {
		    Panic(
			  "input: read returned unexpected error (%d)\n",
			  errno);
		}
	    } else if (bcnt == 0) {
		Panic("input: read returned zero\n", 0);
	    }
	    else {
		/* read from pty was successful */
		if (!screen->output_eight_bits) {
		    register int bc = bcnt;
		    register Char *b = bptr;

		    for (; bc > 0; bc--, b++) {
			*b &= (Char) 0x7f;
		    }
		}
		if ( screen->scrollWidget && screen->scrollttyoutput &&
		     screen->topline < 0)
		    WindowScroll(screen, 0);  /* Scroll to bottom */
		pty_read_bytes += bcnt;
		/* stop speed reading at some point to look for X stuff */
		/* (4096 is just a random large number.) */
		if (pty_read_bytes > 4096) {
            /* This line is modified by Zhuang Hao S.S.* */
		    FD_CLR (g__pty_fd, &select_mask);
		}
		break;
	    }
	}

	pty_read_bytes = 0;
	/* update the screen */
	if (screen->scroll_amt)
	    FlushScroll(screen);
	if (screen->cursor_set && (screen->cursor_col != screen->cur_col
				   || screen->cursor_row != screen->cur_row)) {
	    if (screen->cursor_state)
		HideCursor();
	    ShowCursor();
	} else if (screen->cursor_set != screen->cursor_state) {
	    if (screen->cursor_set)
		ShowCursor();
	    else
		HideCursor();
	}

	XFlush(screen->display); /* always flush writes before waiting */

	/* Update the masks and, unless X events are already in the queue,
	   wait for I/O to be possible. */
    /* These lines are modified by Zhuang Hao S.S.* */
    do {
        qlen = QLength (screen->display);
        memcpy (&select_mask, &Select_mask, sizeof (Select_mask));
        if (v_bufptr > v_bufstr) {
            memcpy (&write_mask, &pty_mask, sizeof (pty_mask));
        }
        else {
            FD_ZERO (&write_mask);
        }
        select_timeout.tv_sec = 0;
        select_timeout.tv_usec = qlen ? 0 : 200000;
	i = select(max_plus1, &select_mask, &write_mask, 
		(fd_set *)0, &select_timeout);
        rollTheWheel ();
    } while (! i && ! qlen);

	if (i < 0) {
	    if (errno != EINTR) {
		SysError(ERROR_SELECT);
	    }
	    continue;
	} 

	/* if there is room to write more data to the pty, go write more */
    /* These lines are modified by Zhuang Hao S.S.* */
    if (v_bufptr > v_bufstr && FD_ISSET (g__pty_fd, &write_mask)) {
	    v_write(screen->respond, (Char *)0, 0); /* flush buffer */
	}

	/* if there are X events already in our queue, it
	   counts as being readable */
	if (QLength(screen->display) || FD_ISSET (g__X_fd, &select_mask)) {
	    xevents();
	}

    }
    bcnt--;
    return(*bptr++);
}

/*
 * process a string of characters according to the character set indicated
 * by charset.  worry about end of line conditions (wraparound if selected).
 */
static void
dotext(register TScreen *screen, unsigned int flags, unsigned int fg, unsigned int bg, char charset, char *buf, char *ptr)
                    	        
            	      
            	       
        	        
        	     		/* start of characters to process */
        	     		/* end */
{
	register char	*s;
	register int	len;
	register int	n;
	register int	next_col;

	switch (charset) {
	case 'A':	/* United Kingdom set			*/
		for (s=buf; s<ptr; ++s)
			if (*s == '#')
				*s = '\036';	/* UK pound sign*/
		break;

	case 'B':	/* ASCII set				*/
		break;

	case '0':	/* special graphics (line drawing)	*/
		for (s=buf; s<ptr; ++s)
			if (*s>=0x5f && *s<=0x7e)
				*s = *s == 0x5f ? 0x7f : *s - 0x5f;
		break;

	default:	/* any character sets we don't recognize*/
		return;
	}

	len = ptr - buf; 
	ptr = buf;
	while (len > 0) {
		n = screen->max_col - screen->cur_col +1;
		if (n <= 1) {
			if (screen->do_wrap && (flags&WRAPAROUND)) {
			    /* mark that we had to wrap this line */
			    Index(screen, 1);
			    screen->cur_col = 0;
			    screen->do_wrap = 0;
			    n = screen->max_col+1;
			} else
			    n = 1;
		}
		if (len < n)
			n = len;
		next_col = screen->cur_col + n;
		WriteText(screen, ptr, n, flags, fg, bg);
		/*
		 * the call to WriteText updates screen->cur_col.
		 * If screen->cur_col != next_col, we must have
		 * hit the right margin, so set the do_wrap flag.
		 */
		screen->do_wrap = (screen->cur_col < next_col);
		len -= n;
		ptr += n;
	}
}

/*
 * write a string str of length len onto the screen at
 * the current cursor position.  update cursor position.
 */
static void
WriteText(register TScreen *screen, register char *str, register int len, unsigned int flags, unsigned int fg, unsigned int bg)
{
	register int cx, cy;
	register unsigned fgs = flags;
    register Pixel fg_pix, bg_pix;
	GC	currentGC;
    extern XtermWidget term;

    if (! (fgs&FG_COLOR)) {
        fg = (fgs&BOLD) ? BOLD_BIT : 0;
    }
    if (! (screen->ext_flags&USEBOLD) && ! (fgs&FG_COLOR) && (fg&BOLD_BIT)) {
        fg_pix = screen->hilightcolor;
    }
    else {
	    fg_pix = (fgs&FG_COLOR) ? screen->colors[fg&COLORMASK] : screen->foreground;
    }
	bg_pix = (fgs&BG_COLOR) ? screen->colors[(bg&COLORMASK)+N_FGCOLOR] : term->core.background_pixel;

	if(screen->cur_row - screen->topline <= screen->max_row) {
		if(screen->cursor_state)
			HideCursor();
	/*
	 *	make sure that the correct GC is current
	 */
#ifdef HANZI
		if (fgs & CHARHANZI)
			if (fgs & INVERSE) {
				if ((fg&BOLD_BIT))
                        currentGC = screen->hz_reverseboldGC;
				else    currentGC = screen->hz_reverseGC;
				XSetForeground(screen->display, currentGC, bg_pix);
				XSetBackground(screen->display, currentGC, fg_pix);
			} else {  /* not inverse */
				if ((fg&BOLD_BIT))
					currentGC = screen->hz_normalboldGC;
				else    currentGC = screen->hz_normalGC;
				XSetForeground(screen->display, currentGC, fg_pix);
				XSetBackground(screen->display, currentGC, bg_pix);
			}
		else
#endif /* HANZI */
			if (fgs & INVERSE) {
				if ((fg&BOLD_BIT))
					currentGC = screen->reverseboldGC;
				else    currentGC = screen->reverseGC;
				XSetForeground(screen->display, currentGC, bg_pix);
				XSetBackground(screen->display, currentGC, fg_pix);
			} else {  /* not inverse */
				if ((fg&BOLD_BIT))
					currentGC = screen->normalboldGC;
				else    currentGC = screen->normalGC;
				XSetForeground(screen->display, currentGC, fg_pix);
				XSetBackground(screen->display, currentGC, bg_pix);
			}

		if (fgs & INSERT)
			InsertChar(screen, len);
		if (!(AddToRefresh(screen))) {
			if(screen->scroll_amt)
				FlushScroll(screen);
			cx = CursorX(screen, screen->cur_col);
#ifdef HANZI
			cy = CursorY(screen, screen->cur_row)+FontAscent(screen);
			if (fgs & CHARHANZI)
 			    HZ_XDrawImString16(screen->display, TextWindow(screen), currentGC,
					FGCSinScreen(screen, fg),
					cx, cy, (XChar2b *)str, len/2);
			else
			    XDrawImageString(screen->display, TextWindow(screen), currentGC,
					cx, cy, str, len);
#else  /* HANZI */
			cy = CursorY(screen, screen->cur_row)+screen->fnt_norm->ascent;
		 	XDrawImageString(screen->display, TextWindow(screen), currentGC,
					cx, cy, str, len);
#endif /* HANZI */

#ifdef HANZI
			if (fgs & CHARHANZI) {
				if((fg&BOLD_BIT) && screen->hz_enbolden)
					if (currentGC == screen->hz_normalGC || screen->hz_reverseGC)
						HZ_XDrawString16(screen->display, TextWindow(screen),
							currentGC, screen->hz_boldFGCS,
							cx + 1, cy, (XChar2b *)str, len/2);
			} else
#endif /* HANZI */
				if((fg&BOLD_BIT) && screen->enbolden)
					if (currentGC == screen->normalGC || screen->reverseGC)
						XDrawString(screen->display, TextWindow(screen),
					      	currentGC,cx + 1, cy, str, len);
			if(fgs & UNDERLINE) 
				XDrawLine(screen->display, TextWindow(screen), currentGC,
					cx, cy+1,
					cx + len * FontWidth(screen), cy+1);
	/*
	 * the following statements compile data to compute the average 
	 * number of characters written on each call to XText.  The data
	 * may be examined via the use of a "hidden" escape sequence.
	 */
			ctotal += len;
			++ntotal;
		}
	}
	ScreenWrite(screen, str, flags, fg, bg, len);
	CursorForward(screen, len);
}
 
/*
 * process ANSI modes set, reset
 */
void ansi_modes(XtermWidget termw, void (*func) (/* ??? */))
{
	register int	i;

	for (i=0; i<nparam; ++i) {
		switch (param[i]) {
		case 4:			/* IRM				*/
			(*func)(&termw->flags, INSERT);
			break;

		case 20:		/* LNM				*/
			(*func)(&termw->flags, LINEFEED);
			update_autolinefeed();
			break;
		}
	}
}

/*
 * process DEC private modes set, reset
 */
void dpmodes(XtermWidget termw, void (*func) (/* ??? */))
{
	register TScreen	*screen	= &termw->screen;
	register int	i, j;

	for (i=0; i<nparam; ++i) {
		switch (param[i]) {
		case 1:			/* DECCKM			*/
			(*func)(&termw->keyboard.flags, CURSOR_APL);
			update_appcursor();
			break;
		case 2:			/* ANSI/VT52 mode		*/
			if (func == bitset) {
				screen->gsets[0] =
					screen->gsets[1] =
					screen->gsets[2] =
					screen->gsets[3] = 'B';
				screen->curgl = 0;
				screen->curgr = 2;
			}
			break;
		case 3:			/* DECCOLM			*/
			if(screen->c132) {
				ClearScreen(screen);
				CursorSet(screen, 0, 0, termw->flags);
				if((j = func == bitset ? 132 : 80) !=
				 ((termw->flags & IN132COLUMNS) ? 132 : 80) ||
				 j != screen->max_col + 1) {
				        Dimension replyWidth, replyHeight;
					XtGeometryResult status;

					status = XtMakeResizeRequest (
					    (Widget) termw, 
					    (Dimension) FontWidth(screen) * j
					        + 2*screen->border
						+ screen->scrollbar,
					    (Dimension) FontHeight(screen)
						* (screen->max_row + 1)
#ifdef HANZI
						+ screen->hzIwin_height
#endif /* HANZI */
						+ 2 * screen->border,
					    &replyWidth, &replyHeight);

					if (status == XtGeometryYes ||
					    status == XtGeometryDone) {
					    ScreenResize (&termw->screen,
							  replyWidth,
							  replyHeight,
							  &termw->flags);
					}
				}
				(*func)(&termw->flags, IN132COLUMNS);
			}
			break;
		case 4:			/* DECSCLM (slow scroll)	*/
			if (func == bitset) {
				screen->jumpscroll = 0;
				if (screen->scroll_amt)
					FlushScroll(screen);
			} else
				screen->jumpscroll = 1;
			(*func)(&termw->flags, SMOOTHSCROLL);
			update_jumpscroll();
			break;
		case 5:			/* DECSCNM			*/
			j = termw->flags;
			(*func)(&termw->flags, REVERSE_VIDEO);
			if ((termw->flags ^ j) & REVERSE_VIDEO)
				ReverseVideo(termw);
			/* update_reversevideo done in RevVid */
			break;

		case 6:			/* DECOM			*/
			(*func)(&termw->flags, ORIGIN);
			CursorSet(screen, 0, 0, termw->flags);
			break;

		case 7:			/* DECAWM			*/
			(*func)(&termw->flags, WRAPAROUND);
			update_autowrap();
			break;
		case 8:			/* DECARM			*/
			/* ignore autorepeat */
			break;
		case 9:			/* MIT bogus sequence		*/
			if(func == bitset)
				screen->send_mouse_pos = 1;
			else
				screen->send_mouse_pos = 0;
			break;
		case 38:		/* DECTEK			*/
			if(func == bitset && !(screen->inhibit & I_TEK)) {
#ifdef ALLOWLOGGING
				if(screen->logging) {
					FlushLog(screen);
					screen->logstart = Tbuffer;
				}
#endif
				screen->TekEmu = TRUE;
			}
			break;
		case 40:		/* 132 column mode		*/
			screen->c132 = (func == bitset);
			update_allow132();
			break;
		case 41:		/* curses hack			*/
			screen->curses = (func == bitset);
			update_cursesemul();
			break;
		case 44:		/* margin bell			*/
			screen->marginbell = (func == bitset);
			if(!screen->marginbell)
				screen->bellarmed = -1;
			update_marginbell();
			break;
		case 45:		/* reverse wraparound	*/
			(*func)(&termw->flags, REVERSEWRAP);
			update_reversewrap();
			break;
#ifdef ALLOWLOGGING
		case 46:		/* logging		*/
#ifdef ALLOWLOGFILEONOFF
			/*
			 * if this feature is enabled, logging may be 
			 * enabled and disabled via escape sequences.
			 */
			if(func == bitset)
				StartLog(screen);
			else
				CloseLog(screen);
#else
			Bell();
			Bell();
#endif /* ALLOWLOGFILEONOFF */
			break;
#endif
		case 47:		/* alternate buffer */
			if (!termw->misc.titeInhibit) {
			    if(func == bitset)
				ToAlternate(screen);
			    else
				FromAlternate(screen);
			}
			break;
		case 1000:		/* xterm bogus sequence		*/
			if(func == bitset)
				screen->send_mouse_pos = 2;
			else
				screen->send_mouse_pos = 0;
			break;
		case 1001:		/* xterm sequence w/hilite tracking */
			if(func == bitset)
				screen->send_mouse_pos = 3;
			else
				screen->send_mouse_pos = 0;
			break;
		}
	}
}

/*
 * process xterm private modes save
 */
void savemodes(XtermWidget termw)
{
	register TScreen	*screen	= &termw->screen;
	register int i;

	for (i = 0; i < nparam; i++) {
		switch (param[i]) {
		case 1:			/* DECCKM			*/
			screen->save_modes[0] = termw->keyboard.flags &
			 CURSOR_APL;
			break;
		case 3:			/* DECCOLM			*/
			if(screen->c132)
			    screen->save_modes[1] = termw->flags & IN132COLUMNS;
			break;
		case 4:			/* DECSCLM (slow scroll)	*/
			screen->save_modes[2] = termw->flags & SMOOTHSCROLL;
			break;
		case 5:			/* DECSCNM			*/
			screen->save_modes[3] = termw->flags & REVERSE_VIDEO;
			break;
		case 6:			/* DECOM			*/
			screen->save_modes[4] = termw->flags & ORIGIN;
			break;

		case 7:			/* DECAWM			*/
			screen->save_modes[5] = termw->flags & WRAPAROUND;
			break;
		case 8:			/* DECARM			*/
			/* ignore autorepeat */
			break;
		case 9:			/* mouse bogus sequence */
			screen->save_modes[7] = screen->send_mouse_pos;
			break;
		case 40:		/* 132 column mode		*/
			screen->save_modes[8] = screen->c132;
			break;
		case 41:		/* curses hack			*/
			screen->save_modes[9] = screen->curses;
			break;
		case 44:		/* margin bell			*/
			screen->save_modes[12] = screen->marginbell;
			break;
		case 45:		/* reverse wraparound	*/
			screen->save_modes[13] = termw->flags & REVERSEWRAP;
			break;
#ifdef ALLOWLOGGING
		case 46:		/* logging		*/
			screen->save_modes[14] = screen->logging;
			break;
#endif
		case 47:		/* alternate buffer		*/
			screen->save_modes[15] = screen->alternate;
			break;
		case 1000:		/* mouse bogus sequence		*/
		case 1001:
			screen->save_modes[7] = screen->send_mouse_pos;
			break;
		}
	}
}

/*
 * process xterm private modes restore
 */
void restoremodes(XtermWidget termw)
{
	register TScreen	*screen	= &termw->screen;
	register int i, j;

	for (i = 0; i < nparam; i++) {
		switch (param[i]) {
		case 1:			/* DECCKM			*/
			termw->keyboard.flags &= ~CURSOR_APL;
			termw->keyboard.flags |= screen->save_modes[0] &
			 CURSOR_APL;
			update_appcursor();
			break;
		case 3:			/* DECCOLM			*/
			if(screen->c132) {
				ClearScreen(screen);
				CursorSet(screen, 0, 0, termw->flags);
				if((j = (screen->save_modes[1] & IN132COLUMNS)
				 ? 132 : 80) != ((termw->flags & IN132COLUMNS)
				 ? 132 : 80) || j != screen->max_col + 1) {
				        Dimension replyWidth, replyHeight;
					XtGeometryResult status;
					status = XtMakeResizeRequest (
					    (Widget) termw,
					    (Dimension) FontWidth(screen) * j 
						+ 2*screen->border
						+ screen->scrollbar,
					    (Dimension) FontHeight(screen)
						* (screen->max_row + 1)
#ifdef HANZI
						+ screen->hzIwin_height
#endif /* HANZI */
						+ 2*screen->border,
					    &replyWidth, &replyHeight);

					if (status == XtGeometryYes ||
					    status == XtGeometryDone) {
					    ScreenResize (&termw->screen,
							  replyWidth,
							  replyHeight,
							  &termw->flags);
					}
				}
				termw->flags &= ~IN132COLUMNS;
				termw->flags |= screen->save_modes[1] &
				 IN132COLUMNS;
			}
			break;
		case 4:			/* DECSCLM (slow scroll)	*/
			if (screen->save_modes[2] & SMOOTHSCROLL) {
				screen->jumpscroll = 0;
				if (screen->scroll_amt)
					FlushScroll(screen);
			} else
				screen->jumpscroll = 1;
			termw->flags &= ~SMOOTHSCROLL;
			termw->flags |= screen->save_modes[2] & SMOOTHSCROLL;
			update_jumpscroll();
			break;
		case 5:			/* DECSCNM			*/
			if((screen->save_modes[3] ^ termw->flags) & REVERSE_VIDEO) {
				termw->flags &= ~REVERSE_VIDEO;
				termw->flags |= screen->save_modes[3] & REVERSE_VIDEO;
				ReverseVideo(termw);
				/* update_reversevideo done in RevVid */
			}
			break;
		case 6:			/* DECOM			*/
			termw->flags &= ~ORIGIN;
			termw->flags |= screen->save_modes[4] & ORIGIN;
			CursorSet(screen, 0, 0, termw->flags);
			break;

		case 7:			/* DECAWM			*/
			termw->flags &= ~WRAPAROUND;
			termw->flags |= screen->save_modes[5] & WRAPAROUND;
			update_autowrap();
			break;
		case 8:			/* DECARM			*/
			/* ignore autorepeat */
			break;
		case 9:			/* MIT bogus sequence		*/
			screen->send_mouse_pos = screen->save_modes[7];
			break;
		case 40:		/* 132 column mode		*/
			screen->c132 = screen->save_modes[8];
			update_allow132();
			break;
		case 41:		/* curses hack			*/
			screen->curses = screen->save_modes[9];
			update_cursesemul();
			break;
		case 44:		/* margin bell			*/
			if(!(screen->marginbell = screen->save_modes[12]))
				screen->bellarmed = -1;
			update_marginbell();
			break;
		case 45:		/* reverse wraparound	*/
			termw->flags &= ~REVERSEWRAP;
			termw->flags |= screen->save_modes[13] & REVERSEWRAP;
			update_reversewrap();
			break;
#ifdef ALLOWLOGGING
		case 46:		/* logging		*/
#ifdef ALLOWLOGFILEONOFF
			if(screen->save_modes[14])
				StartLog(screen);
			else
				CloseLog(screen);
#endif /* ALLOWLOGFILEONOFF */
			/* update_logging done by StartLog and CloseLog */
			break;
#endif
		case 47:		/* alternate buffer */
			if (!termw->misc.titeInhibit) {
			    if(screen->save_modes[15])
				ToAlternate(screen);
			    else
				FromAlternate(screen);
			    /* update_altscreen done by ToAlt and FromAlt */
			}
			break;
		case 1000:		/* mouse bogus sequence		*/
		case 1001:
			screen->send_mouse_pos = screen->save_modes[7];
			break;
		}
	}
}

/*
 * set a bit in a word given a pointer to the word and a mask.
 */
static void bitset(unsigned int *p, int mask)
{
	*p |= mask;
}

/*
 * clear a bit in a word given a pointer to the word and a mask.
 */
static void bitclr(unsigned int *p, int mask)
{
	*p &= ~mask;
}

void unparseseq(register ANSI *ap, int fd)
{
	register int	c;
	register int	i;
	register int	inters;

	c = ap->a_type;
	if (c>=0x80 && c<=0x9F) {
		unparseputc(ESC, fd);
		c -= 0x40;
	}
	unparseputc(c, fd);
	c = ap->a_type;
	if (c==ESC || c==DCS || c==CSI || c==OSC || c==PM || c==APC) {
		if (ap->a_pintro != 0)
			unparseputc((char) ap->a_pintro, fd);
		for (i=0; i<ap->a_nparam; ++i) {
			if (i != 0)
				unparseputc(';', fd);
			unparseputn((unsigned int) ap->a_param[i], fd);
		}
		inters = ap->a_inters;
		for (i=3; i>=0; --i) {
			c = (inters >> (8*i)) & 0xff;
			if (c != 0)
				unparseputc(c, fd);
		}
		unparseputc((char) ap->a_final, fd);
	}
}

void unparseputn(unsigned int n, int fd)
{
	unsigned int	q;

	q = n/10;
	if (q != 0)
		unparseputn(q, fd);
	unparseputc((char) ('0' + (n%10)), fd);
}

void unparseputc(char c, int fd)
{
	char	buf[2];
	register int i = 1;
	extern XtermWidget term;

	if((buf[0] = c) == '\r' && (term->flags & LINEFEED)) {
		buf[1] = '\n';
		i++;
	}
	v_write(fd, (char *)buf, i);
}

void unparsefputs (register char *s, int fd)
{
    if (s) {
	while (*s) unparseputc (*s++, fd);
    }
}

static void SwitchBufs(register TScreen *screen);

static void
ToAlternate(register TScreen *screen)
{
	extern ScrnBuf Allocate(register int nrow, register int ncol, char **addr);

	if(screen->alternate)
		return;
	if(!screen->altbuf)
		screen->altbuf = Allocate(screen->max_row + 1, screen->max_col
		 + 1, &screen->abuf_address);
	SwitchBufs(screen);
	screen->alternate = TRUE;
	update_altscreen();
}

static void
FromAlternate(register TScreen *screen)
{
	if(!screen->alternate)
		return;
	screen->alternate = FALSE;
	SwitchBufs(screen);
	update_altscreen();
}

static void
SwitchBufs(register TScreen *screen)
{
	register int rows, top;

	if(screen->cursor_state)
		HideCursor();
	rows = screen->max_row + 1;
	SwitchBufPtrs(screen);
	TrackText(0, 0, 0, 0);	/* remove any highlighting */
	if((top = -screen->topline) <= screen->max_row) {
		if(screen->scroll_amt)
			FlushScroll(screen);
		if(top == 0)
			XClearWindow(screen->display, TextWindow(screen));
		else
			XClearArea(
			    screen->display,
			    TextWindow(screen),
			    (int) screen->border + screen->scrollbar,
			    (int) top * FontHeight(screen) + screen->border,
			    (unsigned) Width(screen),
			    (unsigned) (screen->max_row - top + 1)
				* FontHeight(screen),
			    FALSE);
	}
	ScrnRefresh(screen, 0, 0, rows, screen->max_col + 1, False);
#ifdef HANZI
	RefreshCXtermInput(screen);
#endif /* HANZI */
}

/* swap buffer line pointers between alt and regular screens */

void SwitchBufPtrs(register TScreen *screen)
{
    register int rows = screen->max_row + 1;
    char *save [4 * MAX_ROWS];

    memmove( (char *)save, (char *)screen->buf, 4 * sizeof(char *) * rows);
    memmove( (char *)screen->buf, (char *)screen->altbuf, 
	  4 * sizeof(char *) * rows);
    memmove( (char *)screen->altbuf, (char *)save, 4 * sizeof(char *) * rows);
}


static XtIntervalId s_intervalId;

void
blinkHandler (XtPointer userptr, XtIntervalId *id)
{
    TScreen *screen = &term->screen;
    ScrnBlinkRefresh (screen, 0, 0, screen->max_row+1, screen->max_col+1);
    if (screen->cursor_state) {
        ShowCursor ();
    }
    if (screen->blink_state) {
        screen->blink_state = 0;
        s_intervalId = XtAppAddTimeOut
          (app_con, screen->blink_on_ms, blinkHandler, (XtPointer)0);
    }
    else {
        screen->blink_state = -1;
        s_intervalId = XtAppAddTimeOut
          (app_con, screen->blink_off_ms, blinkHandler, (XtPointer)0);
    }
}

void VTRun(void)
{
	register TScreen *screen = &term->screen;
	register int i;

    if ((screen->ext_flags&USEBOLD)) {
        for (i = 1; i < N_COLOR; ++i) {
            screen->colors[i] = screen->colors[i+N_FGCOLOR] = screen->colors[i+N_COLOR];
        }
        screen->colors[N_COLOR] = screen->colors[0];
    }
	if (! screen->Vshow) {
	    set_vt_visibility (TRUE);
	} 
	
	update_vttekmode();
	update_vtshow();
	update_tekshow();
	set_vthide_sensitivity();

	if (screen->allbuf == NULL) VTallocbuf ();

	screen->cursor_state = OFF;
	screen->cursor_set = ON;

    screen->blink_state = 0;
    screen->blink_on_ms = 500;
    screen->blink_off_ms = 500;

	bcnt = 0;
	bptr = buffer;

	while(Tpushb > Tpushback) {
		*bptr++ = *--Tpushb;
		bcnt++;
	}
	bcnt += (i = Tbcnt);
	for( ; i > 0 ; i--)
		*bptr++ = *Tbptr++;
	bptr = buffer;
    s_intervalId = XtAppAddTimeOut
      (app_con, screen->blink_on_ms, blinkHandler, (XtPointer)0);

	if(!setjmp(VTend)){
		VTparse();
	}

    XtRemoveTimeOut (s_intervalId);
	HideCursor();
	screen->cursor_set = OFF;
}

/*ARGSUSED*/
static void VTExpose(Widget w, XEvent *event, Region region)
{
	register TScreen *screen = &term->screen;

#ifdef DEBUG
	if(debug)
		fputs("Expose\n", stderr);
#endif	/* DEBUG */
	if (event->type == Expose)
		HandleExposure (screen, event);
}

static void VTGraphicsOrNoExpose (XEvent *event)
{
	register TScreen *screen = &term->screen;
	if (screen->incopy <= 0) {
		screen->incopy = 1;
		if (screen->scrolls > 0)
			screen->scrolls--;
	}
	if (event->type == GraphicsExpose)
	  if (HandleExposure (screen, event))
		screen->cursor_state = OFF;
	if ((event->type == NoExpose) || ((XGraphicsExposeEvent *)event)->count == 0) {
		if (screen->incopy <= 0 && screen->scrolls > 0)
			screen->scrolls--;
		if (screen->scrolls)
			screen->incopy = -1;
		else
			screen->incopy = 0;
	}
}

/*ARGSUSED*/
static void VTNonMaskableEvent (Widget w, XtPointer closure, XEvent *event, Boolean *cont)
         			/* unused */
                  		/* unused */
              
              			/* unused */
{
    switch (event->type) {
       case GraphicsExpose:
       case NoExpose:
	  VTGraphicsOrNoExpose (event);
	  break;
      }
}




static void VTResize(Widget w)
{
    if (XtIsRealized(w))
      ScreenResize (&term->screen, term->core.width, term->core.height,
		    &term->flags);
}

				
extern Atom wm_delete_window;	/* for ICCCM delete window */

static String xterm_trans =
    "<ClientMessage>WM_PROTOCOLS: DeleteWindow()\n\
     <MappingNotify>: KeyboardMapping()\n";

int VTInit (void)
{
    register TScreen *screen = &term->screen;
    Widget vtparent = term->core.parent;

    XtRealizeWidget (vtparent);
    XtOverrideTranslations(vtparent, XtParseTranslationTable(xterm_trans));
    (void) XSetWMProtocols (XtDisplay(vtparent), XtWindow(vtparent),
			    &wm_delete_window, 1);

    if (screen->allbuf == NULL) VTallocbuf ();
    return (1);
}

static void VTallocbuf (void)
{
    register TScreen *screen = &term->screen;
    int nrows = screen->max_row + 1;
    extern ScrnBuf Allocate(register int nrow, register int ncol, char **addr);

    /* allocate screen buffer now, if necessary. */
    if (screen->scrollWidget)
      nrows += screen->savelines;
    screen->allbuf = Allocate (nrows, screen->max_col + 1,
     &screen->sbuf_address);
    if (screen->scrollWidget)
      screen->buf = &screen->allbuf[4 * screen->savelines];
    else
      screen->buf = screen->allbuf;
    return;
}

static void VTClassInit (void)
{
    XtAddConverter(XtRString, XtRGravity, XmuCvtStringToGravity,
		   (XtConvertArgList) NULL, (Cardinal) 0);
}


/* ARGSUSED */
static void VTInitialize (Widget wrequest, Widget wnew, ArgList args, Cardinal *num_args)
{
   XtermWidget request = (XtermWidget) wrequest;
   XtermWidget new     = (XtermWidget) wnew;
   int i;

   /* Zero out the entire "screen" component of "new" widget,
      then do field-by-field assigment of "screen" fields
      that are named in the resource list. */

   bzero ((char *) &new->screen, sizeof(new->screen));
   new->screen.c132 = request->screen.c132;
   new->screen.curses = request->screen.curses;
   new->screen.hp_ll_bc = request->screen.hp_ll_bc;
   new->screen.foreground = request->screen.foreground;
   new->screen.cursorcolor = request->screen.cursorcolor;
   new->screen.border = request->screen.border;
   new->screen.jumpscroll = request->screen.jumpscroll;
#ifdef ALLOWLOGGING
   new->screen.logfile = request->screen.logfile;
#endif
   new->screen.marginbell = request->screen.marginbell;
   new->screen.mousecolor = request->screen.mousecolor;
   new->screen.mousecolorback = request->screen.mousecolorback;
   new->screen.multiscroll = request->screen.multiscroll;
   new->screen.nmarginbell = request->screen.nmarginbell;
   new->screen.savelines = request->screen.savelines;
   new->screen.scrolllines = request->screen.scrolllines;
   new->screen.scrollttyoutput = request->screen.scrollttyoutput;
   new->screen.scrollkey = request->screen.scrollkey;
   new->screen.visualbell = request->screen.visualbell;
   new->screen.TekEmu = request->screen.TekEmu;
   new->misc.re_verse = request->misc.re_verse;
   new->screen.multiClickTime = request->screen.multiClickTime;
   new->screen.bellSuppressTime = request->screen.bellSuppressTime;
   new->screen.charClass = request->screen.charClass;
   new->screen.cutNewline = request->screen.cutNewline;
   new->screen.cutToBeginningOfLine = request->screen.cutToBeginningOfLine;
   new->screen.always_highlight = request->screen.always_highlight;
   new->screen.pointer_cursor = request->screen.pointer_cursor;
   new->screen.input_eight_bits = request->screen.input_eight_bits;
   new->screen.output_eight_bits = request->screen.output_eight_bits;
   new->screen.allowSendEvents = request->screen.allowSendEvents;
   new->misc.titeInhibit = request->misc.titeInhibit;
   for (i = fontMenu_font1; i <= fontMenu_lastBuiltin; i++) {
       new->screen.menu_font_names[i] = request->screen.menu_font_names[i];
   }
   for (i = 0; i < MAXCOLORS; i++) {
       new->screen.colors[i] = request->screen.colors[i];
   }
   /* set default in realize proc */
   new->screen.menu_font_names[fontMenu_fontdefault] = NULL;
   new->screen.menu_font_names[fontMenu_fontescape] = NULL;
   new->screen.menu_font_names[fontMenu_fontsel] = NULL;
   new->screen.menu_font_number = fontMenu_fontdefault;
#ifdef HANZI
   new->screen.lspacing = request->screen.lspacing;
   new->misc.hz_encoding = request->misc.hz_encoding;
   new->misc.hz_mode = request->misc.hz_mode;
   new->misc.it_dirs = request->misc.it_dirs;
   new->misc.assoc_files = request->misc.assoc_files;
#endif /* HANZI */

    /*
     * The definition of -rv now is that it changes the definition of 
     * XtDefaultForeground and XtDefaultBackground.  So, we no longer
     * need to do anything special.
     */
   new->keyboard.flags = 0;
   new->screen.display = new->core.screen->display;
   new->core.height = new->core.width = 1;
      /* dummy values so that we don't try to Realize the parent shell 
	 with height or width of 0, which is illegal in X.  The real
	 size is computed in the xtermWidget's Realize proc,
	 but the shell's Realize proc is called first, and must see
	 a valid size. */

   /* look for focus related events on the shell, because we need
    * to care about the shell's border being part of our focus.
    */
   XtAddEventHandler(XtParent(new), EnterWindowMask, FALSE,
		(XtEventHandler) HandleEnterWindow, (Opaque)NULL);
   XtAddEventHandler(XtParent(new), LeaveWindowMask, FALSE,
		(XtEventHandler) HandleLeaveWindow, (Opaque)NULL);
   XtAddEventHandler(XtParent(new), FocusChangeMask, FALSE,
	(XtEventHandler) HandleFocusChange, (Opaque)NULL);
   XtAddEventHandler((Widget)new, 0L, TRUE,
		VTNonMaskableEvent, (Opaque)NULL);
   XtAddEventHandler((Widget)new, PropertyChangeMask, FALSE,
		     HandleBellPropertyChange, (Opaque)NULL);
   new->screen.bellInProgress = FALSE;

   set_character_class (new->screen.charClass);

#ifdef HANZI
   /* create Input Area, but don't realize it */
   CreateCXtermInput (new);

#endif /* HANZI */
   /* create it, but don't realize it */
   ScrollBarOn (new, TRUE, FALSE);

   /* make sure that the resize gravity acceptable */
   if ( new->misc.resizeGravity != NorthWestGravity &&
        new->misc.resizeGravity != SouthWestGravity) {
       extern XtAppContext app_con;
       Cardinal nparams = 1;

       XtAppWarningMsg(app_con, "rangeError", "resizeGravity", "XTermError",
		       "unsupported resizeGravity resource value (%d)",
		       (String *) &(new->misc.resizeGravity), &nparams);
       new->misc.resizeGravity = SouthWestGravity;
   }

   return;
}


static void VTDestroy (Widget w)
{
#ifdef HANZI
   DestroyCXtermInput ((XtermWidget)w);
#endif /* HANZI */
    XtFree(((XtermWidget)w)->screen.selection);
}

/*ARGSUSED*/
static void VTRealize (Widget w, XtValueMask *valuemask, XSetWindowAttributes *values)
{
	unsigned int width, height;
	register TScreen *screen = &term->screen;
	int xpos, ypos, pr;
	XSizeHints		sizehints;
	int scrollbar_width;

	TabReset (term->tabs);

	screen->menu_font_names[fontMenu_fontdefault] = term->misc.f_n;
	screen->fnt_norm = screen->fnt_bold = NULL;
#ifdef HANZI
	screen->hz_fnt_norm = screen->hz_fnt_bold = NULL;
	if (!LoadNewFont(screen, term->misc.f_hn, term->misc.f_hb, False, 0)) {
	    if (XmuCompareISOLatin1(term->misc.f_hn, "cclib16st") != 0) {
		fprintf (stderr, 
		     "%s:  unable to open font \"%s\", trying \"cclib16st\"....\n",
		     xterm_name, term->misc.f_hn);
		(void) LoadNewFont (screen, "cclib16st", NULL, False, 0);
/*
		screen->menu_font_names[fontMenu_fontdefault] = "cclib16st";
*/
	    } else
		fprintf (stderr, "%s:  unable to open font \"%s\".\n",
		         xterm_name, term->misc.f_hn);
	}
	if (!LoadNewFont(screen, term->misc.f_n, term->misc.f_b, False, 0)) {
	    if (XmuCompareISOLatin1(term->misc.f_n, "8x16") != 0) {
		fprintf (stderr, 
		     "%s:  unable to open font \"%s\", trying \"8x16\"....\n",
		     xterm_name, term->misc.f_n);
		(void) LoadNewFont (screen, "8x16", NULL, False, 0);
		screen->menu_font_names[fontMenu_fontdefault] = "8x16";
	    }
	    if (!screen->fnt_norm) {	/* it's very messy: no 8x16 font */
		fprintf (stderr, 
		    "%s:  unable to open font \"8x16\", %s\n",
		     xterm_name, "trying the ugly \"fixed\"....");
		(void) LoadNewFont (screen, "fixed", NULL, False, 0);
		screen->menu_font_names[fontMenu_fontdefault] = "fixed";
	    }
	}
#else  /* HANZI */
	if (!LoadNewFont(screen, term->misc.f_n, term->misc.f_b, False, 0)) {
	    if (XmuCompareISOLatin1(term->misc.f_n, "fixed") != 0) {
		fprintf (stderr, 
		     "%s:  unable to open font \"%s\", trying \"fixed\"....\n",
		     xterm_name, term->misc.f_n);
		(void) LoadNewFont (screen, "fixed", NULL, False, 0);
		screen->menu_font_names[fontMenu_fontdefault] = "fixed";
	    }
	}
#endif /* HANZI */

	/* really screwed if we couldn't open default font */
#ifdef HANZI
	if (!screen->hz_fnt_norm) {
	    fprintf (stderr, "%s:  unable to locate a suitable Chinese font\n",
		     xterm_name);
	    Exit (1);
	}
#endif /* HANZI */
	if (!screen->fnt_norm) {
	    fprintf (stderr, "%s:  unable to locate a suitable font\n",
		     xterm_name);
	    Exit (1);
	}

	/* making cursor */
	if (!screen->pointer_cursor) 
	  screen->pointer_cursor = make_colored_cursor(XC_xterm, 
						       screen->mousecolor,
						       screen->mousecolorback);
	else 
	  recolor_cursor (screen->pointer_cursor, 
			  screen->mousecolor, screen->mousecolorback);

	scrollbar_width = (term->misc.scrollbar ?
			   screen->scrollWidget->core.width /* +
			   screen->scrollWidget->core.border_width */ : 0);

	/* set defaults */
	xpos = 1; ypos = 1; width = 80; height = 24;
	pr = XParseGeometry (term->misc.geo_metry, &xpos, &ypos,
			     &width, &height);
	screen->max_col = (width - 1);	/* units in character cells */
	screen->max_row = (height - 1);	/* units in character cells */
	update_font_info (&term->screen, False);

	width = screen->fullVwin.fullwidth;
	height = screen->fullVwin.fullheight;

	if ((pr & XValue) && (XNegative&pr)) 
	  xpos += DisplayWidth(screen->display, DefaultScreen(screen->display))
			- width - (term->core.parent->core.border_width * 2);
	if ((pr & YValue) && (YNegative&pr))
	  ypos += DisplayHeight(screen->display,DefaultScreen(screen->display))
			- height - (term->core.parent->core.border_width * 2);

	/* set up size hints for window manager; min 1 char by 1 char */
	sizehints.base_width = 2 * screen->border + scrollbar_width;
#ifdef HANZI
	sizehints.base_height = 2 * screen->border + screen->hzIwin_height;
#else  /* HANZI */
	sizehints.base_height = 2 * screen->border;
#endif /* HANZI */
	sizehints.width_inc = FontWidth(screen);
	sizehints.height_inc = FontHeight(screen);
	sizehints.min_width = sizehints.base_width + sizehints.width_inc;
	sizehints.min_height = sizehints.base_height + sizehints.height_inc;
	sizehints.flags = (PBaseSize|PMinSize|PResizeInc);
	sizehints.x = xpos;
	sizehints.y = ypos;
	if ((XValue&pr) || (YValue&pr)) {
	    sizehints.flags |= USSize|USPosition;
	    sizehints.flags |= PWinGravity;
	    switch (pr & (XNegative | YNegative)) {
	      case 0:
		sizehints.win_gravity = NorthWestGravity;
		break;
	      case XNegative:
		sizehints.win_gravity = NorthEastGravity;
		break;
	      case YNegative:
		sizehints.win_gravity = SouthWestGravity;
		break;
	      default:
		sizehints.win_gravity = SouthEastGravity;
		break;
	    }
	} else {
	    /* set a default size, but do *not* set position */
	    sizehints.flags |= PSize;
	}
	sizehints.width = width;
	sizehints.height = height;
	if ((WidthValue&pr) || (HeightValue&pr)) 
	  sizehints.flags |= USSize;
	else sizehints.flags |= PSize;

	(void) XtMakeResizeRequest((Widget) term,
				   (Dimension)width, (Dimension)height,
				   &term->core.width, &term->core.height);

	/* XXX This is bogus.  We are parsing geometries too late.  This
	 * is information that the shell widget ought to have before we get
	 * realized, so that it can do the right thing.
	 */
        if (sizehints.flags & USPosition)
	    XMoveWindow (XtDisplay(term), term->core.parent->core.window,
			 sizehints.x, sizehints.y);

	XSetWMNormalHints (XtDisplay(term), term->core.parent->core.window,
			   &sizehints);
	XFlush (XtDisplay(term));	/* get it out to window manager */

	/* use ForgetGravity instead of SouthWestGravity because translating
	   the Expose events for ConfigureNotifys is too hard */
	values->bit_gravity = ((term->misc.resizeGravity == NorthWestGravity)
				? NorthWestGravity
				: ForgetGravity);
	term->screen.fullVwin.window = term->core.window =
	  XCreateWindow(XtDisplay(term), XtWindow(term->core.parent),
		term->core.x, term->core.y,
		term->core.width, term->core.height, term->core.border_width,
		(int) term->core.depth,
		InputOutput, CopyFromParent,	
		*valuemask|CWBitGravity, values);

	set_cursor_gcs (screen);

	/* Reset variables used by ANSI emulation. */

	screen->gsets[0] = 'B';			/* ASCII_G		*/
	screen->gsets[1] = 'B';
	screen->gsets[2] = 'B';			/* DEC supplemental.	*/
	screen->gsets[3] = 'B';
	screen->curgl = 0;			/* G0 => GL.		*/
	screen->curgr = 2;			/* G2 => GR.		*/
	screen->curss = 0;			/* No single shift.	*/

	XDefineCursor(screen->display, VShellWindow, screen->pointer_cursor);

        screen->cur_col = screen->cur_row = 0;
	screen->max_col = Width(screen)/screen->fullVwin.f_width - 1;
	screen->top_marg = 0;
	screen->bot_marg = screen->max_row = Height(screen) /
#ifdef HANZI
				FontHeight(screen) - 1;
#else  /* HANZI */
				screen->fullVwin.f_height - 1;
#endif /* HANZI */

	screen->sc.row = screen->sc.col = screen->sc.flags = 0;

	/* Mark screen buffer as unallocated.  We wait until the run loop so
	   that the child process does not fork and exec with all the dynamic
	   memory it will never use.  If we were to do it here, the
	   swap space for new process would be huge for huge savelines. */
	if (!tekWidget)			/* if not called after fork */
	  screen->buf = screen->allbuf = NULL;

	screen->do_wrap = 0;
	screen->scrolls = screen->incopy = 0;
	set_vt_box (screen);

	screen->savedlines = 0;

#ifdef HANZI
	ResizeCXtermInput (screen);
	RealizeCXtermInput (term);	/* do resize */
#endif /* HANZI */
	if (term->misc.scrollbar) {
		screen->scrollbar = 0;
		ScrollBarOn (term, FALSE, TRUE);
	}
	CursorSave (term, &screen->sc);
	return;
}

static Boolean VTSetValues (Widget cur, Widget request, Widget new, ArgList args, Cardinal *num_args)
{
    XtermWidget curvt = (XtermWidget) cur;
    XtermWidget newvt = (XtermWidget) new; 
    Boolean refresh_needed = FALSE;
    Boolean fonts_redone = FALSE;

    if(curvt->core.background_pixel != newvt->core.background_pixel
       || curvt->screen.foreground != newvt->screen.foreground
       || curvt->screen.menu_font_names[curvt->screen.menu_font_number]
          != newvt->screen.menu_font_names[newvt->screen.menu_font_number]
       || curvt->misc.f_n != newvt->misc.f_n) {
	if(curvt->misc.f_n != newvt->misc.f_n)
	    newvt->screen.menu_font_names[fontMenu_fontdefault] = newvt->misc.f_n;
	if (LoadNewFont(&newvt->screen,
			newvt->screen.menu_font_names[curvt->screen.menu_font_number],
			newvt->screen.menu_font_names[curvt->screen.menu_font_number],
			TRUE, newvt->screen.menu_font_number)) {
	    /* resizing does the redisplay, so don't ask for it here */
	    refresh_needed = TRUE;
	    fonts_redone = TRUE;
	} else
	    if(curvt->misc.f_n != newvt->misc.f_n)
		newvt->screen.menu_font_names[fontMenu_fontdefault] = curvt->misc.f_n;
    }
    if(!fonts_redone
       && curvt->screen.cursorcolor != newvt->screen.cursorcolor) {
	set_cursor_gcs(&newvt->screen);
	refresh_needed = TRUE;
    }
    if(curvt->misc.re_verse != newvt->misc.re_verse) {
	newvt->flags ^= REVERSE_VIDEO;
	ReverseVideo(newvt);
	newvt->misc.re_verse = !newvt->misc.re_verse; /* ReverseVideo toggles */
	refresh_needed = TRUE;
    }
    if(curvt->screen.mousecolor != newvt->screen.mousecolor
       || curvt->screen.mousecolorback != newvt->screen.mousecolorback) {
	recolor_cursor (newvt->screen.pointer_cursor, 
			newvt->screen.mousecolor,
			newvt->screen.mousecolorback);
	refresh_needed = TRUE;
    }
    if (curvt->misc.scrollbar != newvt->misc.scrollbar) {
	if (newvt->misc.scrollbar) {
	    ScrollBarOn (newvt, FALSE, FALSE);
	} else {
	    ScrollBarOff (&newvt->screen);
	}
	update_scrollbar();
    }

    return refresh_needed;
}

/*
 * Shows cursor at new cursor position in screen.
 */
void ShowCursor(void)
{
	register TScreen *screen = &term->screen;
	register int x, y, flags;
	Char c, fg;
#ifdef HANZI
	XChar2b cc;		/* if it is a Hanzi			*/
	int shiftleft = 0;	/* should the cursor be shifted on left	*/
#endif /* HANZI */
	GC	currentGC;
	Boolean	in_selection;

	if (eventMode != NORMAL) return;

	if (screen->cur_row - screen->topline > screen->max_row)
		return;
	c = screen->buf[y = 4 * (screen->cursor_row = screen->cur_row)]
	 [x = screen->cursor_col = screen->cur_col];
	flags = screen->buf[y + 1][x];
#ifdef HANZI
	if (flags & CHARHANZI) {
		if (c == 0) {
			c = ' ';
			flags &= ~CHARHANZI;
		} else if (Is1stByteHZ (flags)) {
			cc.byte1 = c;
			cc.byte2 = screen->buf[y][x+1];
		} else {
			shiftleft = x > 0 ? 1 : 0;
			cc.byte1 = x > 0 ? screen->buf[y][x-1] : 0;
			cc.byte2 = c;
		}
	}
#endif /* HANZI */

    fg = screen->buf[y+2][x-shiftleft];

	if (c == 0)
		c = ' ';

	if (screen->cur_row > screen->endHRow ||
	    (screen->cur_row == screen->endHRow &&
	     screen->cur_col >= screen->endHCol) ||
	    screen->cur_row < screen->startHRow ||
	    (screen->cur_row == screen->startHRow &&
	     screen->cur_col < screen->startHCol))
	    in_selection = False;
	else
	    in_selection = True;

	if(screen->select || screen->always_highlight) {
#ifdef HANZI
	    if (flags & CHARHANZI) {
		if (( (flags & INVERSE) && !in_selection) ||
		    (!(flags & INVERSE) &&  in_selection)){
		    /* text is reverse video */
		    if (screen->hz_cursorGC) {
			currentGC = screen->hz_cursorGC;
		    } else {
			if ((fg&BOLD_BIT)) {
				currentGC = screen->hz_normalboldGC;
			} else {
				currentGC = screen->hz_normalGC;
			}
		    }
		} else { /* normal video */
		    if (screen->hz_reversecursorGC) {
			currentGC = screen->hz_reversecursorGC;
		    } else {
			if ((fg&BOLD_BIT)) {
				currentGC = screen->hz_reverseboldGC;
			} else {
				currentGC = screen->hz_reverseGC;
			}
		    }
		}
	    } else
#endif /* HANZI */
		if (( (flags & INVERSE) && !in_selection) ||
		    (!(flags & INVERSE) &&  in_selection)){
		    /* text is reverse video */
		    if (screen->cursorGC) {
			currentGC = screen->cursorGC;
		    } else {
			if ((fg&BOLD_BIT)) {
				currentGC = screen->normalboldGC;
			} else {
				currentGC = screen->normalGC;
			}
		    }
		} else { /* normal video */
		    if (screen->reversecursorGC) {
			currentGC = screen->reversecursorGC;
		    } else {
			if ((fg&BOLD_BIT)) {
				currentGC = screen->reverseboldGC;
			} else {
				currentGC = screen->reverseGC;
			}
		    }
		}
	} else { /* not selected */
#ifdef HANZI
	    if (flags & CHARHANZI) {
		if (( (flags & INVERSE) && !in_selection) ||
		    (!(flags & INVERSE) &&  in_selection)) {
		    /* text is reverse video */
			currentGC = screen->hz_reverseGC;
		} else { /* normal video */
			currentGC = screen->hz_normalGC;
		}
	    } else
#endif /* HANZI */
		if (( (flags & INVERSE) && !in_selection) ||
		    (!(flags & INVERSE) &&  in_selection)) {
		    /* text is reverse video */
			currentGC = screen->reverseGC;
		} else { /* normal video */
			currentGC = screen->normalGC;
		}
	    
	}

#ifdef HANZI
	x = CursorX (screen, screen->cur_col - shiftleft);
	y = CursorY(screen, screen->cur_row) + FontAscent(screen);
#else  /* HANZI */
	x = CursorX (screen, screen->cur_col);
	y = CursorY(screen, screen->cur_row) + 
	  screen->fnt_norm->ascent;
#endif /* HANZI */
#ifdef HANZI
      if (flags & CHARHANZI) {
	HZ_XDrawImString16(screen->display, TextWindow(screen), currentGC,
		FGCSinScreen(screen, fg), x, y, &cc, 1);

	if((fg&BOLD_BIT) && screen->hz_enbolden) /* no bold font */
		HZ_XDrawString16(screen->display, TextWindow(screen), currentGC,
			screen->hz_boldFGCS, x + 1, y, &cc, 1);
	if(flags & UNDERLINE) 
		XDrawLine(screen->display, TextWindow(screen), currentGC,
			x, y+1, x + 2 * FontWidth(screen), y+1);
	if (!screen->select && !screen->always_highlight) {
		screen->hz_box->x = x;
		screen->hz_box->y = y - screen->hz_fnt_norm->ascent;
		/* Don't use FontAscent(screen) here. See set_vt_box() */

		XDrawLines (screen->display, TextWindow(screen), 
			    screen->hz_cursoroutlineGC ? screen->hz_cursoroutlineGC 
			    			    : currentGC,
			    screen->hz_box, NBOX, CoordModePrevious);
	}
      } else {
#endif /* HANZI */
	XDrawImageString(screen->display, TextWindow(screen), currentGC,
		x, y, (char *) &c, 1);

	if((fg&BOLD_BIT) && screen->enbolden) /* no bold font */
		XDrawString(screen->display, TextWindow(screen), currentGC,
			x + 1, y, (char *) &c, 1);
	if(flags & UNDERLINE) 
		XDrawLine(screen->display, TextWindow(screen), currentGC,
			x, y+1, x + FontWidth(screen), y+1);
	if (!screen->select && !screen->always_highlight) {
		screen->box->x = x;
		screen->box->y = y - screen->fnt_norm->ascent;
		XDrawLines (screen->display, TextWindow(screen), 
			    screen->cursoroutlineGC ? screen->cursoroutlineGC 
			    			    : currentGC,
			    screen->box, NBOX, CoordModePrevious);
	}
#ifdef HANZI
      }	/* if (flags & CHARHANZI) */
#endif /* HANZI */
	screen->cursor_state = ON;
}

/*
 * hide cursor at previous cursor position in screen.
 * This function has been modified by Zhuang Hao S.S.*
 */
void HideCursor(void)
{
	register TScreen *screen = &term->screen;
	GC	currentGC;
	register int x, y, flags;
	char c;
#ifdef HANZI
	XChar2b cc;		/* if it is a Hanzi			*/
	int shiftleft = 0;	/* should the cursor shifted on left	*/
#endif /* HANZI */
	Boolean	in_selection;
    Pixel fg_pix, bg_pix;
    unsigned int bg, fg;

	if(screen->cursor_row - screen->topline > screen->max_row)
		return;
	c = screen->buf[y = 4 * screen->cursor_row][x = screen->cursor_col];
	flags = screen->buf[y + 1][x];
#ifdef HANZI
	if (flags & CHARHANZI) {
		if (c == 0) {
			c = ' ';
			flags &= ~CHARHANZI;
		} else if (Is1stByteHZ (flags)) {
			cc.byte1 = c;
			cc.byte2 = screen->buf[y][x+1];
		} else {
			shiftleft = x > 0 ? 1 : 0;
			cc.byte1 = x > 0 ? screen->buf[y][x-1] : 0;
			cc.byte2 = c;
		}
	}
#endif /* HANZI */

    fg = screen->buf[y+2][x-shiftleft];
    bg = screen->buf[y+3][x-shiftleft];

	if (screen->cursor_row > screen->endHRow ||
	    (screen->cursor_row == screen->endHRow &&
	     screen->cursor_col >= screen->endHCol) ||
	    screen->cursor_row < screen->startHRow ||
	    (screen->cursor_row == screen->startHRow &&
	     screen->cursor_col < screen->startHCol))
	    in_selection = False;
	else
	    in_selection = True;

#ifdef HANZI
      if (flags & CHARHANZI) {
	if (( (flags & INVERSE) && !in_selection) ||
	    (!(flags & INVERSE) &&  in_selection)) {
		if((fg&BOLD_BIT)) {
			currentGC = screen->hz_reverseboldGC;
		} else {
			currentGC = screen->hz_reverseGC;
		}
	} else {
		if((fg&BOLD_BIT)) {
			currentGC = screen->hz_normalboldGC;
		} else {
			currentGC = screen->hz_normalGC;
		}
	}
      } else
#endif /* HANZI */
	if (( (flags & INVERSE) && !in_selection) ||
	    (!(flags & INVERSE) &&  in_selection)) {
		if((fg&BOLD_BIT)) {
			currentGC = screen->reverseboldGC;
		} else {
			currentGC = screen->reverseGC;
		}
	} else {
		if((fg&BOLD_BIT)) {
			currentGC = screen->normalboldGC;
		} else {
			currentGC = screen->normalGC;
		}
	}

    if (! (screen->ext_flags&USEBOLD) && ! (flags&FG_COLOR) && (fg&BOLD_BIT)) {
        fg_pix = screen->hilightcolor;
    }
    else {
	    fg_pix = (flags&FG_COLOR) ? screen->colors[fg&COLORMASK] : screen->foreground;
    }
	bg_pix = (flags&BG_COLOR) ? screen->colors[(bg&COLORMASK)+N_FGCOLOR] : term->core.background_pixel;

	XSetForeground(screen->display, currentGC, fg_pix);
	XSetBackground(screen->display, currentGC, bg_pix);

	if (c == 0)
		c = ' ';
#ifdef HANZI
	x = CursorX (screen, screen->cursor_col - shiftleft);
	y = (((screen->cursor_row - screen->topline) * FontHeight(screen))) +
	 screen->border + FontAscent(screen);
#else  /* HANZI */
	x = CursorX (screen, screen->cursor_col);
	y = (((screen->cursor_row - screen->topline) * FontHeight(screen))) +
	 screen->border;
	y = y+screen->fnt_norm->ascent;
#endif /* HANZI */
#ifdef HANZI
      if (flags & CHARHANZI) {
	HZ_XDrawImString16(screen->display, TextWindow(screen), currentGC,
		FGCSinScreen(screen, fg), x, y, &cc, 1);
	if((fg&BOLD_BIT) && screen->hz_enbolden)
		HZ_XDrawString16(screen->display, TextWindow(screen), currentGC,
			screen->hz_boldFGCS, x + 1, y, &cc, 1);
	if(flags & UNDERLINE) 
		XDrawLine(screen->display, TextWindow(screen), currentGC,
			x, y+1, x + 2 * FontWidth(screen), y+1);
      } else {
#endif /* HANZI */
	XDrawImageString(screen->display, TextWindow(screen), currentGC,
		x, y, &c, 1);
	if((fg&BOLD_BIT) && screen->enbolden)
		XDrawString(screen->display, TextWindow(screen), currentGC,
			x + 1, y, &c, 1);
	if(flags & UNDERLINE) 
		XDrawLine(screen->display, TextWindow(screen), currentGC,
			x, y+1, x + FontWidth(screen), y+1);
#ifdef HANZI
      } /* if (flags & CHARHANZI) */
#endif /* HANZI */
	screen->cursor_state = OFF;
}

void VTReset(Boolean full)
{
	register TScreen *screen = &term->screen;

	/* reset scrolling region */
	screen->top_marg = 0;
	screen->bot_marg = screen->max_row;
	term->flags &= ~ORIGIN;
	if(full) {
		TabReset (term->tabs);
		term->keyboard.flags = 0;
		update_appcursor();
		update_appkeypad();
		screen->gsets[0] = 'B';
		screen->gsets[1] = 'B';
		screen->gsets[2] = 'B';
		screen->gsets[3] = 'B';
		screen->curgl = 0;
		screen->curgr = 2;
		screen->curss = 0;
		FromAlternate(screen);
		ClearScreen(screen);
		screen->cursor_state = OFF;
		if (term->flags & REVERSE_VIDEO)
			ReverseVideo(term);

		term->flags = term->initflags;
		update_reversevideo();
		update_autowrap();
		update_reversewrap();
		update_autolinefeed();
		screen->jumpscroll = !(term->flags & SMOOTHSCROLL);
		update_jumpscroll();
		if(screen->c132 && (term->flags & IN132COLUMNS)) {
		        Dimension junk;
			XtMakeResizeRequest(
			    (Widget) term,
			    (Dimension) 80*FontWidth(screen)
				+ 2 * screen->border + screen->scrollbar,
			    (Dimension) FontHeight(screen)
#ifdef HANZI
				+ screen->hzIwin_height
#endif /* HANZI */
			        * (screen->max_row + 1) + 2 * screen->border,
			    &junk, &junk);
			XSync(screen->display, FALSE);	/* synchronize */
			if(QLength(screen->display) > 0)
				xevents();
		}
		CursorSet(screen, 0, 0, term->flags);
#ifdef HANZI
		ResetCXtermInput(term);
#endif /* HANZI */
	}
	longjmp(vtjmpbuf, 1);	/* force ground state in parser */
}



/*
 * set_character_class - takes a string of the form
 * 
 *                 low[-high]:val[,low[-high]:val[...]]
 * 
 * and sets the indicated ranges to the indicated values.
 */

int set_character_class (register char *s)
{
    register int i;			/* iterator, index into s */
    int len;				/* length of s */
    int acc;				/* accumulator */
    int low, high;			/* bounds of range [0..127] */
    int base;				/* 8, 10, 16 (octal, decimal, hex) */
    int numbers;			/* count of numbers per range */
    int digits;				/* count of digits in a number */
    static char *errfmt = "%s:  %s in range string \"%s\" (position %d)\n";
    extern char *ProgramName;

    if (!s || !s[0]) return -1;

    base = 10;				/* in case we ever add octal, hex */
    low = high = -1;			/* out of range */

    for (i = 0, len = strlen (s), acc = 0, numbers = digits = 0;
	 i < len; i++) {
	char c = s[i];

	if (isspace(c)) {
	    continue;
	} else if (isdigit(c)) {
	    acc = acc * base + (c - '0');
	    digits++;
	    continue;
	} else if (c == '-') {
	    low = acc;
	    acc = 0;
	    if (digits == 0) {
		fprintf (stderr, errfmt, ProgramName, "missing number", s, i);
		return (-1);
	    }
	    digits = 0;
	    numbers++;
	    continue;
	} else if (c == ':') {
	    if (numbers == 0)
	      low = acc;
	    else if (numbers == 1)
	      high = acc;
	    else {
		fprintf (stderr, errfmt, ProgramName, "too many numbers",
			 s, i);
		return (-1);
	    }
	    digits = 0;
	    numbers++;
	    acc = 0;
	    continue;
	} else if (c == ',') {
	    /*
	     * now, process it
	     */

	    if (high < 0) {
		high = low;
		numbers++;
	    }
	    if (numbers != 2) {
		fprintf (stderr, errfmt, ProgramName, "bad value number", 
			 s, i);
	    } else if (SetCharacterClassRange (low, high, acc) != 0) {
		fprintf (stderr, errfmt, ProgramName, "bad range", s, i);
	    }

	    low = high = -1;
	    acc = 0;
	    digits = 0;
	    numbers = 0;
	    continue;
	} else {
	    fprintf (stderr, errfmt, ProgramName, "bad character", s, i);
	    return (-1);
	}				/* end if else if ... else */

    }

    if (low < 0 && high < 0) return (0);

    /*
     * now, process it
     */

    if (high < 0) high = low;
    if (numbers < 1 || numbers > 2) {
	fprintf (stderr, errfmt, ProgramName, "bad value number", s, i);
    } else if (SetCharacterClassRange (low, high, acc) != 0) {
	fprintf (stderr, errfmt, ProgramName, "bad range", s, i);
    }

    return (0);
}

/* ARGSUSED */
static void HandleKeymapChange(Widget w, XEvent *event, String *params, Cardinal *param_count)
{
    static XtTranslations keymap, original;
    static XtResource key_resources[] = {
	{ XtNtranslations, XtCTranslations, XtRTranslationTable,
	      sizeof(XtTranslations), 0, XtRTranslationTable, (XtPointer)NULL}
    };
    char mapName[1000];
    char mapClass[1000];

    if (*param_count != 1) return;

    if (original == NULL) original = w->core.tm.translations;

    if (strcmp(params[0], "None") == 0) {
	XtOverrideTranslations(w, original);
	return;
    }
    (void) sprintf( mapName, "%sKeymap", params[0] );
    (void) strcpy( mapClass, mapName );
    if (islower(mapClass[0])) mapClass[0] = toupper(mapClass[0]);
    XtGetSubresources( w, (XtPointer)&keymap, mapName, mapClass,
		       key_resources, (Cardinal)1, NULL, (Cardinal)0 );
    if (keymap != NULL)
	XtOverrideTranslations(w, keymap);
}


/* ARGSUSED */
static void HandleBell(Widget w, XEvent *event, String *params, Cardinal *param_count)
             
                  		/* unused */
                   		/* [0] = volume */
                          	/* 0 or 1 */
{
    int percent = (*param_count) ? atoi(params[0]) : 0;

    XBell( XtDisplay(w), percent );
}


/* ARGSUSED */
static void HandleVisualBell(Widget w, XEvent *event, String *params, Cardinal *param_count)
             
                  		/* unused */
                   		/* unused */
                          	/* unused */
{
    VisualBell();
}


/* ARGSUSED */
static void HandleIgnore(Widget w, XEvent *event, String *params, Cardinal *param_count)
             
                  		/* unused */
                   		/* unused */
                          	/* unused */
{
    /* do nothing, but check for funny escape sequences */
    (void) SendMousePosition(w, event);
}


/* ARGSUSED */
static void
DoSetSelectedFont(Widget w, XtPointer client_data, Atom *selection, Atom *type, XtPointer value, long unsigned int *length, int *format)
{
    char *val = (char *)value;
    int len;
    if (*type != XA_STRING  ||  *format != 8) {
	Bell();
	return;
    }
    len = strlen(val);
    if (len > 0) {
	if (val[len-1] == '\n') val[len-1] = '\0';
	/* Do some sanity checking to avoid sending a long selection
	   back to the server in an OpenFont that is unlikely to succeed.
	   XLFD allows up to 255 characters and no control characters;
	   we are a little more liberal here. */
	if (len > 1000  ||  strchr(val, '\n'))
	    return;
	if (!LoadNewFont (&term->screen, val, NULL, True, fontMenu_fontsel))
	    Bell();
    }
}

void FindFontSelection (char *atom_name, int justprobe)
{
    static AtomPtr *atoms;
    static int atomCount = 0;
    AtomPtr *pAtom;
    int a;
    Atom target;

    if (!atom_name) atom_name = "PRIMARY";

    for (pAtom = atoms, a = atomCount; a; a--, pAtom++) {
	if (strcmp(atom_name, XmuNameOfAtom(*pAtom)) == 0) break;
    }
    if (!a) {
	atoms = (AtomPtr*) XtRealloc ((char *)atoms,
				      sizeof(AtomPtr)*(atomCount+1));
	*(pAtom = &atoms[atomCount++]) = XmuMakeAtom(atom_name);
    }

    target = XmuInternAtom(XtDisplay(term), *pAtom);
    if (justprobe) {
	term->screen.menu_font_names[fontMenu_fontsel] = 
	  XGetSelectionOwner(XtDisplay(term), target) ? _Font_Selected_ : NULL;
    } else {
	XtGetSelectionValue((Widget)term, target, XA_STRING,
			    DoSetSelectedFont, NULL,
			    XtLastTimestampProcessed(XtDisplay(term)));
    }
    return;
}


/* ARGSUSED */
void HandleSetFont(Widget w, XEvent *event, String *params, Cardinal *param_count)
             
                  		/* unused */
                   		/* unused */
                          	/* unused */
{
    int fontnum;
    char *name1 = NULL, *name2 = NULL;

    if (*param_count == 0) {
	fontnum = fontMenu_fontdefault;
    } else {
	int unsigned maxparams = 1;		/* total number of params allowed */

	switch (params[0][0]) {
	  case 'd': case 'D': case '0':
	    fontnum = fontMenu_fontdefault; break;
	  case '1':
	    fontnum = fontMenu_font1; break;
	  case '2':
	    fontnum = fontMenu_font2; break;
	  case '3':
	    fontnum = fontMenu_font3; break;
	  case '4':
	    fontnum = fontMenu_font4; break;
	  case '5':
	    fontnum = fontMenu_font5; break;
	  case '6':
	    fontnum = fontMenu_font6; break;
	  case 'e': case 'E':
	    fontnum = fontMenu_fontescape; maxparams = 3; break;
	  case 's': case 'S':
	    fontnum = fontMenu_fontsel; maxparams = 2; break;
	  default:
	    Bell();
	    return;
	}
	if (*param_count > maxparams) {	 /* see if extra args given */
	    Bell();
	    return;
	}
	switch (*param_count) {		/* assign 'em */
	  case 3:
	    name2 = params[2];
	    /* fall through */
	  case 2:
	    name1 = params[1];
	    break;
	}
    }

    SetVTFont (fontnum, True, name1, name2);
}


void SetVTFont (int i, int doresize, char *name1, char *name2)
{
    TScreen *screen = &term->screen;

    if (i < 0 || i >= NMENUFONTS) {
	Bell();
	return;
    }
    if (i == fontMenu_fontsel) {	/* go get the selection */
	FindFontSelection (name1, False);  /* name1 = atom, name2 is ignored */
	return;
    }
    if (!name1) name1 = screen->menu_font_names[i];
    if (!LoadNewFont(screen, name1, name2, doresize, i)) {
	Bell();
    }
    return;
}


int LoadNewFont (TScreen *screen, char *nfontname, char *bfontname, int doresize, int fontnum)
{
    XFontStruct *nfs = NULL, *bfs = NULL;
    XGCValues xgcv;
    unsigned long mask;
    GC new_normalGC = NULL, new_normalboldGC = NULL;
    GC new_reverseGC = NULL, new_reverseboldGC = NULL;
    char *tmpname = NULL;

    if (!nfontname) return 0;
#ifdef HANZI
    if (!*nfontname) return 0;
#endif /* HANZI */

    if (fontnum == fontMenu_fontescape &&
	nfontname != screen->menu_font_names[fontnum]) {
	tmpname = (char *) malloc (strlen(nfontname) + 1);
	if (!tmpname) return 0;
	strcpy (tmpname, nfontname);
    }

    if (!(nfs = XLoadQueryFont (screen->display, nfontname))) goto bad;
    if (nfs->ascent + nfs->descent == 0  ||  nfs->max_bounds.width == 0)
	goto bad;		/* can't use a 0-sized font */

    if (! (screen->ext_flags&USEBOLD) || !(bfontname && 
#ifdef HANZI
	  (* bfontname) &&
#endif /* HANZI */
	  (bfs = XLoadQueryFont (screen->display, bfontname))))
      bfs = nfs;
    else
	if (bfs->ascent + bfs->descent == 0  ||  bfs->max_bounds.width == 0)
	    goto bad;		/* can't use a 0-sized font */

    mask = (GCFont | GCForeground | GCBackground | GCGraphicsExposures |
	    GCFunction);

    xgcv.font = nfs->fid;
    xgcv.foreground = screen->foreground;
    xgcv.background = term->core.background_pixel;
    xgcv.graphics_exposures = TRUE;	/* default */
    xgcv.function = GXcopy;

    new_normalGC = XCreateGC(screen->display,
                           DefaultRootWindow(screen->display), mask, &xgcv);
    if (!new_normalGC) goto bad;

    if (nfs == bfs) {			/* there is no bold font */
	new_normalboldGC = new_normalGC;
    } else {
	xgcv.font = bfs->fid;
        new_normalboldGC = XCreateGC(screen->display,
                                   DefaultRootWindow(screen->display),
                                   mask, &xgcv);
	if (!new_normalboldGC) goto bad;
    }

    xgcv.font = nfs->fid;
    xgcv.foreground = term->core.background_pixel;
    xgcv.background = screen->foreground;
    new_reverseGC = XCreateGC(screen->display,
                            DefaultRootWindow(screen->display), mask, &xgcv);
    if (!new_reverseGC) goto bad;

    if (nfs == bfs) {			/* there is no bold font */
	new_reverseboldGC = new_reverseGC;
    } else {
	xgcv.font = bfs->fid;
        new_reverseboldGC = XCreateGC(screen->display,
                                      DefaultRootWindow(screen->display),
                                      mask, &xgcv);
	if (!new_reverseboldGC) goto bad;
    }

#ifdef HANZI
  /*
   * This is an ad hoc way to incorprate the Hanzi fonts.
   * The best way is to write a separated Load function,
   * as well as SetVTFont, etc.		-- ygz
   */
  if (nfs->max_byte1 - nfs->min_byte1 > 0) {
    /* If it has more than one rows of characters, it is a Hanzi font. */
    /* The bfs is not checked.  If a user messes it up, well, too bad! */

    if (screen->hz_normalGC != screen->hz_normalboldGC)
	XtReleaseGC ((Widget) term, screen->hz_normalboldGC);
    XtReleaseGC ((Widget) term, screen->hz_normalGC);
    if (screen->hz_reverseGC != screen->hz_reverseboldGC)
	XtReleaseGC ((Widget) term, screen->hz_reverseboldGC);
    XtReleaseGC ((Widget) term, screen->hz_reverseGC);
    screen->hz_normalGC = new_normalGC;
    screen->hz_normalboldGC = new_normalboldGC;
    screen->hz_reverseGC = new_reverseGC;
    screen->hz_reverseboldGC = new_reverseboldGC;
    screen->hz_fnt_norm = nfs;
    screen->hz_fnt_bold = bfs;
    screen->hz_enbolden = ((nfs == bfs) && (screen->ext_flags&USEBOLD));

    if ((nfs->max_byte1 < 128) && (nfs->max_char_or_byte2 < 128))
	screen->hz_normalFGCS = FGCS_GL;
    else if ((nfs->min_byte1 >= 128) && (nfs->min_char_or_byte2 >= 128))
	screen->hz_normalFGCS = FGCS_GR;
    else
	screen->hz_normalFGCS = FGCS_MIX;

    if ((bfs->max_byte1 < 128) && (bfs->max_char_or_byte2 < 128))
	screen->hz_boldFGCS = FGCS_GL;
    else if ((bfs->min_byte1 >= 128) && (bfs->min_char_or_byte2 >= 128))
	screen->hz_boldFGCS = FGCS_GR;
    else
	screen->hz_boldFGCS = FGCS_MIX;

  } else {
#endif /* HANZI */
    if (screen->normalGC != screen->normalboldGC)
	XtReleaseGC ((Widget) term, screen->normalboldGC);
    XtReleaseGC ((Widget) term, screen->normalGC);
    if (screen->reverseGC != screen->reverseboldGC)
	XtReleaseGC ((Widget) term, screen->reverseboldGC);
    XtReleaseGC ((Widget) term, screen->reverseGC);
    screen->normalGC = new_normalGC;
    screen->normalboldGC = new_normalboldGC;
    screen->reverseGC = new_reverseGC;
    screen->reverseboldGC = new_reverseboldGC;
    screen->fnt_norm = nfs;
    screen->fnt_bold = bfs;
    screen->enbolden = ((nfs == bfs) && (screen->ext_flags&USEBOLD));
#ifdef HANZI
  } /* if (nfs->max_byte1 != nfs->min_byte1) */
#endif /* HANZI */
    set_menu_font (False);
    screen->menu_font_number = fontnum;
    set_menu_font (True);
    if (tmpname) {			/* if setting escape or sel */
	if (screen->menu_font_names[fontnum])
	  free (screen->menu_font_names[fontnum]);
	screen->menu_font_names[fontnum] = tmpname;
	if (fontnum == fontMenu_fontescape) {
	    set_sensitivity (term->screen.fontMenu,
			     fontMenuEntries[fontMenu_fontescape].widget,
			     TRUE);
	}
    }
    set_cursor_gcs (screen);
    update_font_info (screen, doresize);
    return 1;

  bad:
    if (tmpname) free (tmpname);
    if (new_normalGC)
#ifdef HANZI
      XtReleaseGC ((Widget) term, new_normalGC);
#else  /* HANZI */
      XtReleaseGC ((Widget) term, screen->normalGC);
#endif /* HANZI */
    if (new_normalGC && new_normalGC != new_normalboldGC)
      XtReleaseGC ((Widget) term, new_normalboldGC);
    if (new_reverseGC)
      XtReleaseGC ((Widget) term, new_reverseGC);
    if (new_reverseGC && new_reverseGC != new_reverseboldGC)
      XtReleaseGC ((Widget) term, new_reverseboldGC);
    if (nfs) XFreeFont (screen->display, nfs);
    if (bfs && nfs != bfs) XFreeFont (screen->display, bfs);
    return 0;
}


static void
update_font_info (TScreen *screen, int doresize)
{
    int i, j, width, height, scrollbar_width;

#ifdef HANZI
    if (!screen->fnt_norm || !screen->hz_fnt_norm)
	return;

    screen->fullVwin.f_width = Max(screen->fnt_norm->max_bounds.width,
				   screen->hz_fnt_norm->max_bounds.width / 2);
    screen->fullVwin.f_ascent = Max(screen->fnt_norm->ascent,
				    screen->hz_fnt_norm->ascent);
    screen->fullVwin.f_descent = Max(screen->fnt_norm->descent,
				     screen->hz_fnt_norm->descent);
    screen->fullVwin.f_height = (screen->fullVwin.f_ascent +
				 screen->fullVwin.f_descent) +
				 screen->lspacing;	/* plus line spacing */
    scrollbar_width = (term->misc.scrollbar ?
		       screen->scrollWidget->core.width +
		       screen->scrollWidget->core.border_width : 0);
    screen->hzIwin_height = 1 + FontHeight(screen) * screen->hzIwin.rows;
    i = 2 * screen->border + scrollbar_width;
    j = 2 * screen->border + screen->hzIwin_height;
    width = (screen->max_col + 1) * FontWidth(screen) + i;
    height = (screen->max_row + 1) * FontHeight(screen) + j;
#else  /* HANZI */
    screen->fullVwin.f_width = screen->fnt_norm->max_bounds.width;
    screen->fullVwin.f_height = (screen->fnt_norm->ascent +
				 screen->fnt_norm->descent);
    scrollbar_width = (term->misc.scrollbar ? 
		       screen->scrollWidget->core.width +
		       screen->scrollWidget->core.border_width : 0);
    i = 2 * screen->border + scrollbar_width;
    j = 2 * screen->border;
    width = (screen->max_col + 1) * screen->fullVwin.f_width + i;
    height = (screen->max_row + 1) * screen->fullVwin.f_height + j;
#endif /* HANZI */
    screen->fullVwin.fullwidth = width;
    screen->fullVwin.fullheight = height;
    screen->fullVwin.width = width - i;
    screen->fullVwin.height = height - j;

    if (doresize) {
	if (VWindow(screen)) {
	    XClearWindow (screen->display, VWindow(screen));
	}
	DoResizeScreen (term);		/* set to the new natural size */
	if (screen->scrollWidget)
	  ResizeScrollBar (screen->scrollWidget, -1, -1,
#ifdef HANZI
			   FullHeight(screen));
#else  /* HANZI */
			   Height(screen) + screen->border * 2);
#endif /* HANZI */
#ifdef HANZI
	ResizeCXtermInput (screen);
#endif /* HANZI */
	Redraw ();
    }
    set_vt_box (screen);
}

void set_vt_box (TScreen *screen)
{
	XPoint	*vp;

#ifdef HANZI
	/* In case two fonts are not of the same height, try not to leave
	   a mess. So don't use FontHeight(screen). */

	vp = &VTbox[1];
	(vp++)->x = FontWidth(screen) - 1;
	(vp++)->y = screen->fnt_norm->ascent + screen->fnt_norm->descent - 1;
	(vp++)->x = -(FontWidth(screen) - 1);
	vp->y = -(screen->fnt_norm->ascent + screen->fnt_norm->descent - 1);
	screen->box = VTbox;

	vp = &VThz_box[1];
	(vp++)->x = FontWidth(screen) * 2 - 1;
	(vp++)->y = screen->hz_fnt_norm->ascent + screen->hz_fnt_norm->descent - 1;
	(vp++)->x = -(FontWidth(screen) * 2 - 1);
	vp->y = -(screen->hz_fnt_norm->ascent + screen->hz_fnt_norm->descent - 1);
	screen->hz_box = VThz_box;
#else  /* HANZI */
	vp = &VTbox[1];
	(vp++)->x = FontWidth(screen) - 1;
	(vp++)->y = FontHeight(screen) - 1;
	(vp++)->x = -(FontWidth(screen) - 1);
	vp->y = -(FontHeight(screen) - 1);
	screen->box = VTbox;
#endif /* HANZI */
}


void set_cursor_gcs (TScreen *screen)
{
    XGCValues xgcv;
    unsigned long mask;
    unsigned long cc = screen->cursorcolor;
    unsigned long fg = screen->foreground;
    unsigned long bg = term->core.background_pixel;
    GC new_cursorGC = NULL, new_reversecursorGC = NULL;
    GC new_cursoroutlineGC = NULL;

    /*
     * Let's see, there are three things that have "color":
     *
     *     background
     *     text
     *     cursorblock
     *
     * And, there are four situation when drawing a cursor, if we decide
     * that we like have a solid block of cursor color with the letter
     * that it is highlighting shown in the background color to make it
     * stand out:
     *
     *     selected window, normal video - background on cursor
     *     selected window, reverse video - foreground on cursor
     *     unselected window, normal video - foreground on background
     *     unselected window, reverse video - background on foreground
     *
     * Since the last two are really just normalGC and reverseGC, we only
     * need two new GC's.  Under monochrome, we get the same effect as
     * above by setting cursor color to foreground.
     */

#ifdef HANZI
    if (!screen->hz_fnt_norm || !screen->fnt_norm)
	return;

    /* Just duplicate the code from the non-HANZI part to here. */

    xgcv.font = screen->hz_fnt_norm->fid;
    mask = (GCForeground | GCBackground | GCFont);
    if (cc != fg && cc != bg) {
	/* we have a colored cursor */
	xgcv.foreground = fg;
	xgcv.background = cc;
	new_cursorGC = XtGetGC ((Widget) term, mask, &xgcv);

	if (screen->always_highlight) {
	    new_reversecursorGC = (GC) 0;
	    new_cursoroutlineGC = (GC) 0;
	} else {
	    xgcv.foreground = bg;
	    xgcv.background = cc;
	    new_reversecursorGC = XtGetGC ((Widget) term, mask, &xgcv);
	    xgcv.foreground = cc;
	    xgcv.background = bg;
	    new_cursoroutlineGC = XtGetGC ((Widget) term, mask, &xgcv);
		}
    } else {
	new_cursorGC = (GC) 0;
	new_reversecursorGC = (GC) 0;
	new_cursoroutlineGC = (GC) 0;
    }
    if (screen->hz_cursorGC) XtReleaseGC ((Widget)term, screen->hz_cursorGC);
    if (screen->hz_reversecursorGC)
	XtReleaseGC ((Widget)term, screen->hz_reversecursorGC);
    if (screen->hz_cursoroutlineGC)
	XtReleaseGC ((Widget)term, screen->hz_cursoroutlineGC);
    screen->hz_cursorGC = new_cursorGC;
    screen->hz_reversecursorGC = new_reversecursorGC;
    screen->hz_cursoroutlineGC = new_cursoroutlineGC;

#endif /* HANZI */
    xgcv.font = screen->fnt_norm->fid;
    mask = (GCForeground | GCBackground | GCFont);
    if (cc != fg && cc != bg) {
	/* we have a colored cursor */
	xgcv.foreground = fg;
	xgcv.background = cc;
	new_cursorGC = XtGetGC ((Widget) term, mask, &xgcv);

	if (screen->always_highlight) {
	    new_reversecursorGC = (GC) 0;
	    new_cursoroutlineGC = (GC) 0;
	} else {
	    xgcv.foreground = bg;
	    xgcv.background = cc;
	    new_reversecursorGC = XtGetGC ((Widget) term, mask, &xgcv);
	    xgcv.foreground = cc;
	    xgcv.background = bg;
	    new_cursoroutlineGC = XtGetGC ((Widget) term, mask, &xgcv);
		}
    } else {
	new_cursorGC = (GC) 0;
	new_reversecursorGC = (GC) 0;
	new_cursoroutlineGC = (GC) 0;
    }
    if (screen->cursorGC) XtReleaseGC ((Widget)term, screen->cursorGC);
    if (screen->reversecursorGC)
	XtReleaseGC ((Widget)term, screen->reversecursorGC);
    if (screen->cursoroutlineGC)
	XtReleaseGC ((Widget)term, screen->cursoroutlineGC);
    screen->cursorGC = new_cursorGC;
    screen->reversecursorGC = new_reversecursorGC;
    screen->cursoroutlineGC = new_cursoroutlineGC;
}
