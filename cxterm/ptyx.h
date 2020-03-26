/* $Id: ptyx.h,v 1.4.2.2 2004/08/03 17:33:31 htl10 Exp $ */
/***********************************************************************
* Copyright 1994 by Yongguang Zhang.
* Copyright 1990, 1991 by Yongguang Zhang and Pong Man-Chi.
*
* All rights reserved.  Under the same copyright and permission term as
* the original.  Absolutely no warranties of any kinds.
************************************************************************/

/*
 *	$XConsortium: ptyx.h,v 1.63 94/08/02 19:24:44 converse Exp $
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

/* ptyx.h */
/* @(#)ptyx.h	X10/6.6	11/10/86 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <X11/IntrinsicP.h>
#include <X11/Xmu/Misc.h>	/* For Max() and Min(). */
#ifndef CXTERM_COMPATIBLE
#include <X11/Xfuncs.h>
#include <X11/Xosdefs.h>
#else
# if (X11RELEASE <= 4)
#  ifdef __STDC__
#   undef NOSTDHDRS
#  else
#   define NOSTDHDRS
#  endif
#  include "../Xcompat/Xfuncs.h"
#  include "../Xcompat/Xosdefs.h"
# elif (X11RELEASE == 5)
#  include <X11/Xosdefs.h>		/* should placed before Xfuncs.h */
#  include "../Xcompat/Xfuncs.h"	/* has R6 new stuffs */
# endif
#endif

/* Extra Xlib definitions */
#define AllButtonsUp(detail, ignore)  (\
		((ignore) == Button1) ? \
				(((detail)&(Button2Mask|Button3Mask)) == 0) \
				: \
		 (((ignore) == Button2) ? \
		  		(((detail)&(Button1Mask|Button3Mask)) == 0) \
				: \
		  		(((detail)&(Button1Mask|Button2Mask)) == 0)) \
		)

#define MAX_COLS	200
#define MAX_ROWS	128

/*
** System V definitions
*/

#ifdef att
#define ATT
#endif

#ifdef SVR4
#undef  SYSV                   /* predefined on Solaris 2.4 */
#define SYSV                   /* SVR4 is (approx) superset of SVR3 */
#define ATT
#endif

#ifdef SYSV
#ifdef X_NOT_POSIX
#ifndef CRAY
#define	dup2(fd1,fd2)	((fd1 == fd2) ? fd1 : \
				(close(fd2), fcntl(fd1, F_DUPFD, fd2)))
#endif
#endif
#endif /* SYSV */

/*
** Definitions to simplify ifdef's for pty's.
*/
#define USE_PTY_DEVICE 1
#define USE_PTY_SEARCH 1

#if defined(__osf__) || (defined(linux) && defined(__GLIBC__) && (__GLIBC__ >= 2) && (__GLIBC_MINOR__ >= 1))
#undef USE_PTY_DEVICE
#undef USE_PTY_SEARCH
#define USE_PTS_DEVICE 1
#elif defined(VMS)
#undef USE_PTY_DEVICE
#undef USE_PTY_SEARCH
#elif defined(PUCC_PTYD)
#undef USE_PTY_SEARCH
#endif

#if defined(SYSV) && defined(i386) && !defined(SVR4)
#define ATT
#define USE_HANDSHAKE
#define USE_ISPTS_FLAG 1
#endif

#if (defined(ATT) && !defined(__sgi)) || defined(__MVS__) || (defined(SYSV) && defined(i386)) || (defined (__GLIBC__) && ((__GLIBC__ > 2) || (__GLIBC__ == 2) && (__GLIBC_MINOR__ >= 1)))
#define USE_USG_PTYS
#else
#define USE_HANDSHAKE
#endif

/*
** allow for mobility of the pty master/slave directories
*/
#ifndef PTYDEV
#ifdef hpux
#define	PTYDEV		"/dev/ptym/ptyxx"
#elif defined(__MVS__)
#define        PTYDEV          "/dev/ptypxxxx"
#else
#define	PTYDEV		"/dev/ptyxx"
#endif
#endif	/* !PTYDEV */

#ifndef TTYDEV
#ifdef __hpux
#define TTYDEV		"/dev/pty/ttyxx"
#elif defined(__MVS__)
#define TTYDEV         "/dev/ptypxxxx"
#elif defined(USE_PTS_DEVICE)
#define TTYDEV         "/dev/pts/0"
#else
#define	TTYDEV		"/dev/ttyxx"
#endif
#endif	/* !TTYDEV */

#ifndef PTYCHARLEN
#ifdef CRAY
#define PTYCHARLEN 3
#elif defined(__MVS__)
#define PTYCHARLEN 4
#else
#define PTYCHARLEN 2
#endif
#endif

#ifndef PTYCHAR1
#ifdef hpux
#define PTYCHAR1	"zyxwvutsrqp"
#else	/* !hpux */
#define	PTYCHAR1	"pqrstuvwxyzPQRSTUVWXYZ"
#endif	/* !hpux */
#endif	/* !PTYCHAR1 */

#ifndef PTYCHAR2
#ifdef hpux
#define	PTYCHAR2	"fedcba9876543210"
#else	/* !hpux */
#define	PTYCHAR2	"0123456789abcdef"
#endif	/* !hpux */
#endif	/* !PTYCHAR2 */

/* Until the translation manager comes along, I have to do my own translation of
 * mouse events into the proper routines. */

typedef enum {NORMAL, LEFTEXTENSION, RIGHTEXTENSION} EventMode;

/*
 * The origin of a screen is 0, 0.  Therefore, the number of rows
 * on a screen is screen->max_row + 1, and similarly for columns.
 */

typedef unsigned char Char;		/* to support 8 bit chars */
typedef Char **ScrnBuf;

/*
 * ANSI emulation.
 */
#define INQ	0x05
#define	FF	0x0C			/* C0, C1 control names		*/
#define	LS1	0x0E
#define	LS0	0x0F
#define	CAN	0x18
#define	SUB	0x1A
#define	ESC	0x1B
#define US	0x1F
#define	DEL	0x7F
#define HTS     ('H'+0x40)
#define	SS2	0x8E
#define	SS3	0x8F
#define	DCS	0x90
#define	OLDID	0x9A			/* ESC Z			*/
#define	CSI	0x9B
#define	ST	0x9C
#define	OSC	0x9D
#define	PM	0x9E
#define	APC	0x9F
#define	RDEL	0xFF

#define NMENUFONTS 9			/* entries in fontMenu */

#define	NBOX	5			/* Number of Points in box	*/
#define	NPARAM	10			/* Max. parameters		*/

typedef struct {
	unsigned char	a_type;
	unsigned char	a_pintro;
	unsigned char	a_final;
	unsigned char	a_inters;
	char	a_nparam;		/* # of parameters		*/
	char	a_dflt[NPARAM];		/* Default value flags		*/
	short	a_param[NPARAM];	/* Parameters			*/
	char	a_nastyf;		/* Error flag			*/
} ANSI;

typedef struct {
	int		row;
	int		col;
	unsigned long	flags;	/* Vt100 saves graphics rendition. Ugh! */
	char		curgl;
	char		curgr;
	char		gsets[4];
} SavedCursor;

#define TEK_FONT_LARGE 0
#define TEK_FONT_2 1
#define TEK_FONT_3 2
#define TEK_FONT_SMALL 3
#define	TEKNUMFONTS 4

/* Actually there are 5 types of lines, but four are non-solid lines */
#define	TEKNUMLINES	4

typedef struct {
	int	x;
	int	y;
	int	fontsize;
	int	linetype;
} Tmodes;

/***====================================================================***/

#define	TEXT_FG		0
#define	TEXT_BG		1
#define	TEXT_CURSOR	2
#define	MOUSE_FG	3
#define	MOUSE_BG	4
#define	TEK_FG		5
#define	TEK_BG		6
#define	NCOLORS		7

#define	COLOR_DEFINED(s,w)	((s)->which&(1<<(w)))
#define	COLOR_VALUE(s,w)	((s)->colors[w])
#define	SET_COLOR_VALUE(s,w,v)	(((s)->colors[w]=(v)),((s)->which|=(1<<(w))))

#define	COLOR_NAME(s,w)		((s)->names[w])
#define	SET_COLOR_NAME(s,w,v)	(((s)->names[w]=(v)),((s)->which|=(1<<(w))))

#define	UNDEFINE_COLOR(s,w)	((s)->which&=(~((w)<<1)))
#define	OPPOSITE_COLOR(n)	(((n)==TEXT_FG?TEXT_BG:\
				 ((n)==TEXT_BG?TEXT_FG:\
				 ((n)==MOUSE_FG?MOUSE_BG:\
				 ((n)==MOUSE_BG?MOUSE_FG:\
				 ((n)==TEK_FG?TEK_BG:\
				 ((n)==TEXT_BG?TEK_FG:(n))))))))

typedef struct {
	unsigned	which;
	Pixel		colors[NCOLORS];
	char		*names[NCOLORS];
} ScrnColors;

/***====================================================================***/

#define MAXCOLORS 24
#define COLORMASK 0x0F      /* Color Mask, only low 4 bits */
#define	N_FGCOLOR 16
#define N_COLOR   8
#define BOLD_BIT        8   /* bit3 as bold-bit */

#define FCOLOR_0		0
#define FCOLOR_1		1
#define FCOLOR_2		2
#define FCOLOR_3		3
#define FCOLOR_4		4
#define FCOLOR_5		5
#define FCOLOR_6		6
#define FCOLOR_7		7
#define FCOLOR_H0		8
#define FCOLOR_H1		9
#define FCOLOR_H2		10
#define FCOLOR_H3		11
#define FCOLOR_H4		12
#define FCOLOR_H5		13
#define FCOLOR_H6		14
#define FCOLOR_H7		15

#define BCOLOR_0		16
#define BCOLOR_1		17
#define BCOLOR_2		18
#define BCOLOR_3		19
#define BCOLOR_4		20
#define BCOLOR_5		21
#define BCOLOR_6		22
#define BCOLOR_7		23

typedef struct {
	int Twidth;
	int Theight;
} T_fontsize;

typedef struct {
	short *bits;
	int x;
	int y;
	int width;
	int height;
} BitmapBits;

#define	SAVELINES		64      /* default # lines to save      */
#define SCROLLLINES 1			/* default # lines to scroll    */
#ifdef HANZI
# define	FGCS_GL		0	/* font in GL plane */
# define	FGCS_GR		1	/* font in GR plane */
# define	FGCS_MIX	2	/* mixture of GL and GR planes */
#endif /* HANZI */

typedef struct {
/* These parameters apply to both windows */
	Display		*display;	/* X display for screen		*/
	int		respond;	/* socket for responses
					   (position report, etc.)	*/
	long		pid;		/* pid of process on far side   */
	int		uid;		/* user id of actual person	*/
	int		gid;		/* group id of actual person	*/
	GC		normalGC;	/* normal painting		*/
	GC		reverseGC;	/* reverse painting		*/
	GC		normalboldGC;	/* normal painting, bold font	*/
	GC		reverseboldGC;	/* reverse painting, bold font	*/
	GC		cursorGC;	/* normal cursor painting	*/
	GC		reversecursorGC;/* reverse cursor painting	*/
	GC		cursoroutlineGC;/* for painting lines around    */
#ifdef HANZI
	GC		hz_normalGC;	/* normal painting	(Hanzi)	*/
	GC		hz_reverseGC;	/* reverse painting	(Hanzi)	*/
	GC		hz_normalboldGC;/* normal, bold font	(Hanzi)	*/
	GC		hz_reverseboldGC;/* reverse, bold font	(Hanzi)	*/
	GC		hz_cursorGC;	/* normal cursor 	(Hanzi)	*/
	GC		hz_reversecursorGC;/* reverse cursor 	(Hanzi)	*/
	GC		hz_cursoroutlineGC;/* lines around	(Hanzi)	*/
	int		hz_normalFGCS;	/* normal painting	(GR/GL) */
	int		hz_boldFGCS;	/* bold font painting	(GR/GL) */
#endif /* HANZI */
    /* These are variables added by Zhuang Hao S.S.* */
    int             blink_state;
    unsigned long   blink_on_ms;
    unsigned long   blink_off_ms;
    unsigned long   ext_flags;

    Pixel       hilightcolor;   /* hilighted foreground color */
    Pixel       foreground;	/* foreground color		*/
    Pixel       cursorcolor;	/* Cursor color			*/
    Pixel       mousecolor;	/* Mouse color			*/
    Pixel       mousecolorback;	/* Mouse color background	*/
    Pixel       colors[MAXCOLORS]; /* ANSI color emulation      */
	int		border;		/* inner border			*/
	Cursor		arrow;		/* arrow cursor			*/
	unsigned short	send_mouse_pos;	/* user wants mouse transition  */
					/* and position information	*/
	int		select;		/* xterm selected		*/
	Boolean		visualbell;	/* visual bell mode		*/
	Boolean		allowSendEvents;/* SendEvent mode		*/
	Boolean		grabbedKbd;	/* keyboard is grabbed		*/
#ifdef ALLOWLOGGING
	int		logging;	/* logging mode			*/
	int		logfd;		/* file descriptor of log	*/
	char		*logfile;	/* log file name		*/
	unsigned char	*logstart;	/* current start of log buffer	*/
#endif
	int		inhibit;	/* flags for inhibiting changes	*/

/* VT window parameters */
	struct {
		Window	window;		/* X window id			*/
		int	width;		/* width of columns		*/
		int	height;		/* height of rows		*/
		int	fullwidth;	/* full width of window		*/
		int	fullheight;	/* full height of window	*/
		int	f_width;	/* width of fonts in pixels	*/
		int	f_height;	/* height of fonts in pixels	*/
#ifdef HANZI
		int	f_ascent;	/* ascent of fonts in pixels	*/
		int	f_descent;	/* descent of fonts in pixels	*/
#endif /* HANZI */
	} fullVwin;
	Cursor pointer_cursor;		/* pointer cursor in window	*/

	/* Terminal fonts must be of the same size and of fixed width */
	XFontStruct	*fnt_norm;	/* normal font of terminal	*/
	XFontStruct	*fnt_bold;	/* bold font of terminal	*/
	int		enbolden;	/* overstrike for bold font	*/
	XPoint		*box;		/* draw unselected cursor	*/
#ifdef HANZI
	XFontStruct	*hz_fnt_norm;	/* normal HZ font of terminal	*/
	XFontStruct	*hz_fnt_bold;	/* bold HZ font of terminal	*/
	int		hz_enbolden;	/* overstrike for bold HZ font	*/
	XPoint		*hz_box;	/* draw unselected HZ cursor	*/
#endif /* HANZI */

	int		cursor_state;	/* ON or OFF			*/
	int		cursor_set;	/* requested state		*/
	int		cursor_col;	/* previous cursor column	*/
	int		cursor_row;	/* previous cursor row		*/
	int		cur_col;	/* current cursor column	*/
	int		cur_row;	/* current cursor row		*/
	int		max_col;	/* rightmost column		*/
	int		max_row;	/* bottom row			*/
	int		top_marg;	/* top line of scrolling region */
	int		bot_marg;	/* bottom line of  "	    "	*/
	Widget		scrollWidget;	/* pointer to scrollbar struct	*/
	int		scrollbar;	/* if > 0, width of scrollbar, and
						scrollbar is showing	*/
	int		topline;	/* line number of top, <= 0	*/
	int		savedlines;     /* number of lines that've been saved */
	int		savelines;	/* number of lines off top to save */
	int		scrolllines;	/* number of lines to button scroll */
	Boolean		scrollttyoutput; /* scroll to bottom on tty output */
	Boolean		scrollkey;	/* scroll to bottom on key	*/
	
	ScrnBuf		buf;		/* ptr to visible screen buf (main) */
	ScrnBuf		allbuf;		/* screen buffer (may include
					   lines scrolled off top)	*/
	char		*sbuf_address;	/* main screen memory address   */
	ScrnBuf		altbuf;		/* alternate screen buffer	*/
	char		*abuf_address;	/* alternate screen memory address */
	Boolean		alternate;	/* true if using alternate buf	*/
	unsigned short	do_wrap;	/* true if cursor in last column
					    and character just output    */
	int		incopy;		/* 0 idle; 1 XCopyArea issued;
					    -1 first GraphicsExpose seen,
					    but last not seen		*/
	int		copy_src_x;	/* params from last XCopyArea ... */
	int		copy_src_y;
	unsigned int	copy_width;
	unsigned int	copy_height;
	int		copy_dest_x;
	int		copy_dest_y;
	Boolean		c132;		/* allow change to 132 columns	*/
	Boolean		curses;		/* kludge line wrap for more	*/
	Boolean		hp_ll_bc;	/* kludge HP-style ll for xdb	*/
	Boolean		marginbell;	/* true if margin bell on	*/
	int		nmarginbell;	/* columns from right margin	*/
	int		bellarmed;	/* cursor below bell margin	*/
	Boolean 	multiscroll;	/* true if multi-scroll		*/
	int		scrolls;	/* outstanding scroll count,
					    used only with multiscroll	*/
	SavedCursor	sc;		/* data for restore cursor	*/
	int		save_modes[19];	/* save dec private modes	*/

	/* Improved VT100 emulation stuff.				*/
	char		gsets[4];	/* G0 through G3.		*/
	int		curgl;		/* Current GL setting.		*/
	int		curgr;		/* Current GR setting.		*/
	int		curss;		/* Current single shift.	*/
	int		scroll_amt;	/* amount to scroll		*/
	int		refresh_amt;	/* amount to refresh		*/
	Boolean		jumpscroll;	/* whether we should jumpscroll */
	Boolean         always_highlight; /* whether to highlight cursor */

#ifdef HANZI
	int		lspacing;	/* space in pixel between lines	*/
	/* Hanzi input area stuff					*/
	struct {
		int	rows;		/* number of rows in input area	*/
		int	x, y;		/* upperleft of the input area	*/
		int	width, height;	/* width, height of input area  */
		int	rule_x, rule_y;	/* left end of the rule line	*/
		int	rule_length;	/* length of the rule line	*/
	} hzIwin;
	int		hzIwin_height;	/* height of Hanzi input area	*/

#endif /* HANZI */
/* Tektronix window parameters */
	GC		TnormalGC;	/* normal painting		*/
	GC		TcursorGC;	/* normal cursor painting	*/
	Pixel		Tforeground;	/* foreground color		*/
	Pixel		Tbackground;	/* Background color		*/
	Pixel		Tcursorcolor;	/* Cursor color			*/
	int		Tcolor;		/* colors used			*/
	Boolean		Vshow;		/* VT window showing		*/
	Boolean		Tshow;		/* Tek window showing		*/
	Boolean		waitrefresh;	/* postpone refresh		*/
	struct {
		Window	window;		/* X window id			*/
		int	width;		/* width of columns		*/
		int	height;		/* height of rows		*/
		int	fullwidth;	/* full width of window		*/
		int	fullheight;	/* full height of window	*/
		double	tekscale;	/* scale factor Tek -> vs100	*/
	} fullTwin;
	int		xorplane;	/* z plane for inverts		*/
	GC		linepat[TEKNUMLINES]; /* line patterns		*/
	Boolean		TekEmu;		/* true if Tektronix emulation	*/
	int		cur_X;		/* current x			*/
	int		cur_Y;		/* current y			*/
	Tmodes		cur;		/* current tek modes		*/
	Tmodes		page;		/* starting tek modes on page	*/
	int		margin;		/* 0 -> margin 1, 1 -> margin 2	*/
	int		pen;		/* current Tektronix pen 0=up, 1=dn */
	char		*TekGIN;	/* nonzero if Tektronix GIN mode*/
	int		gin_terminator; /* Tek strap option */

	int		multiClickTime;	 /* time between multiclick selects */
	int		bellSuppressTime; /* msecs after Bell before another allowed */
	Boolean		bellInProgress; /* still ringing/flashing prev bell? */
	char		*charClass;	/* for overriding word selection */
	Boolean		cutNewline;	/* whether or not line cut has \n */
	Boolean		cutToBeginningOfLine;  /* line cuts to BOL? */
	char		*selection;	/* the current selection */
	int		selection_size; /* size of allocated buffer */
	int		selection_length; /* number of significant bytes */
	Time		selection_time;	/* latest event timestamp */
	int		startHRow, startHCol, /* highlighted text */
			endHRow, endHCol,
			startHCoord, endHCoord;
	Atom*		selection_atoms; /* which selections we own */
	Cardinal	sel_atoms_size;	/*  how many atoms allocated */
	Cardinal	selection_count; /* how many atoms in use */
	Boolean		input_eight_bits;/* use 8th bit instead of ESC prefix */
	Boolean		output_eight_bits; /* honor all bits or strip */
	Pixmap		menu_item_bitmap;	/* mask for checking items */
	Widget		mainMenu, vtMenu, tekMenu, fontMenu;
	char*		menu_font_names[NMENUFONTS];
	int		menu_font_number;
#ifdef HANZI
	Widget		configPopup;	/* popup panel for HANZI input */
#endif /* HANZI */
} TScreen;

typedef struct _TekPart {
    XFontStruct *Tfont[TEKNUMFONTS];
    int		tobaseline[TEKNUMFONTS]; /* top to baseline for each font */
    char	*initial_font;		/* large, 2, 3, small */
    char	*gin_terminator_str;	/* ginTerminator resource */
} TekPart;



/* meaning of bits in screen.select flag */
#define	INWINDOW	01	/* the mouse is in one of the windows */
#define	FOCUS		02	/* one of the windows is the focus window */

#define MULTICLICKTIME 250	/* milliseconds */

typedef struct
{
	unsigned	flags;
} TKeyboard;

typedef struct _Misc {
    char *geo_metry;
    char *T_geometry;
    char *f_n;
    char *f_b;
#ifdef HANZI
    char *f_hn;
    char *f_hb;
    char *hz_mode;
    char *hz_encoding;		/* Hanzi encoding identifier */
    char *it_dirs;		/* directories for input tables	*/
    char *assoc_files;		/* Hanzi association-list files */
#endif /* HANZI */
#ifdef ALLOWLOGGING
    Boolean log_on;
#endif
    Boolean login_shell;
    Boolean re_verse;
    int resizeGravity;
    Boolean reverseWrap;
    Boolean autoWrap;
    Boolean logInhibit;
    Boolean signalInhibit;
    Boolean tekInhibit;
    Boolean scrollbar;
    Boolean titeInhibit;
    Boolean tekSmall;	/* start tek window in small size */
    Boolean dynamicColors;
    Boolean appcursorDefault;
    Boolean appkeypadDefault;
    Boolean bbs;        /* if is in ANSI-BBS mode */
    Boolean usebold;    /* if use bold fonts instead of hilight fonts */
    Pixel   hilight;
} Misc;

typedef struct {int foo;} XtermClassPart, TekClassPart;

typedef struct _XtermClassRec {
    CoreClassPart  core_class;
    XtermClassPart xterm_class;
} XtermClassRec;

typedef struct _TekClassRec {
    CoreClassPart core_class;
    TekClassPart tek_class;
} TekClassRec;

/* define masks for flags */
#define CAPS_LOCK	0x01
#define KYPD_APL	0x02
#define CURSOR_APL	0x04


#define N_MARGINBELL	10
#define MAX_TABS	320
#define TAB_ARRAY_SIZE	10	/* number of ints to provide MAX_TABS bits */

typedef unsigned Tabs [TAB_ARRAY_SIZE];

typedef struct _XtermWidgetRec {
    CorePart	core;
    TKeyboard	keyboard;	/* terminal keyboard		*/
    TScreen	screen;		/* terminal screen		*/
    unsigned long	flags;		/* mode flags			*/
    unsigned    cur_foreground;	/* current foreground color	*/
    unsigned    cur_background;	/* current background color	*/
    unsigned	initflags;	/* initial mode flags		*/
    Tabs	tabs;		/* tabstops of the terminal	*/
    Misc	misc;		/* miscellaneous parameters	*/
} XtermWidgetRec, *XtermWidget;

typedef struct _TekWidgetRec {
    CorePart core;
    TekPart tek;
} TekWidgetRec, *TekWidget;

#define BUF_SIZE 4096

/*
 * terminal flags
 * There are actually two namespaces mixed together here.
 * One is the set of flags that can go in screen->buf attributes
 * and which must fit in a char.
 * The other is the global setting stored in
 * term->flags and screen->save_modes.  This need only fit in an unsigned.
 */

#ifdef HANZI
#define	ATTRIBUTES	0xFF	/* mask: user-visible attributes */
#else  /* HANZI */
#define	ATTRIBUTES	0x07	/* mask: user-visible attributes */
#endif /* HANZI */
/* global flags and character flags (visible character attributes) */
#define INVERSE		0x01	/* invert the characters to be output */
#define UNDERLINE	0x02	/* true if underlining */
#define BLINK		0x04
/* character flags (internal attributes) */
#define CHARDRAWN	0x08	 /* a character has been drawn here on the
				   screen.  Used to distinguish blanks from
				   empty parts of the screen when selecting */

#define FG_COLOR        0x10     /* true if background set */
#define BG_COLOR        0x20     /* true if foreground set */

#ifdef HANZI
#define FIRSTBYTE	0x80	/* is the first byte in a Hanzi (internal) */
#define CHARHANZI	0x40	/* is part of a Hanzi   (global & visible) */
#endif /* HANZI */
/* This attribute is NOT written into buf[3], only in term->flags */
#define BOLD   0x100   /* The color is hilighted */

/* global flags */
#define WRAPAROUND	0x400	/* true if auto wraparound mode */
#define	REVERSEWRAP	0x800	/* true if reverse wraparound mode */
#define REVERSE_VIDEO	0x1000	/* true if screen white on black */
#define LINEFEED	0x2000	/* true if in auto linefeed mode */
#define ORIGIN		0x4000	/* true if in origin mode */
#define INSERT		0x8000	/* true if in insert mode */
#define SMOOTHSCROLL	0x10000	/* true if in smooth scroll mode */
#define IN132COLUMNS	0x20000	/* true if in 132 column mode */


/* Bit masks for newly added ext_flags, by Zhuang Hao S.S.* */
#define BBS         1       /* ANSI-BBS Mode */
#define USEBOLD     2       /* Use bold fonts */

#define VWindow(screen)		(screen->fullVwin.window)
#define VShellWindow		term->core.parent->core.window
#define TextWindow(screen)      (screen->fullVwin.window)
#define TWindow(screen)		(screen->fullTwin.window)
#define TShellWindow		tekWidget->core.parent->core.window
#define Width(screen)		(screen->fullVwin.width)
#define Height(screen)		(screen->fullVwin.height)
#define FullWidth(screen)	(screen->fullVwin.fullwidth)
#define FullHeight(screen)	(screen->fullVwin.fullheight)
#define FontWidth(screen)	(screen->fullVwin.f_width)
#define FontHeight(screen)	(screen->fullVwin.f_height)
#ifdef HANZI
#define FontAscent(screen)	(screen->fullVwin.f_ascent)
#define FontDescent(screen)	(screen->fullVwin.f_descent)
#endif /* HANZI */
#define TWidth(screen)		(screen->fullTwin.width)
#define THeight(screen)		(screen->fullTwin.height)
#define TFullWidth(screen)	(screen->fullTwin.fullwidth)
#define TFullHeight(screen)	(screen->fullTwin.fullheight)
#define TekScale(screen)	(screen->fullTwin.tekscale)

#define CursorX(screen,col) ((col) * FontWidth(screen) + screen->border \
			+ screen->scrollbar)
#define CursorY(screen,row) ((((row) - screen->topline) * FontHeight(screen)) \
			+ screen->border)

#define	TWINDOWEVENTS	(KeyPressMask | ExposureMask | ButtonPressMask |\
			 ButtonReleaseMask | StructureNotifyMask |\
			 EnterWindowMask | LeaveWindowMask | FocusChangeMask)

#define	WINDOWEVENTS	(TWINDOWEVENTS | PointerMotionMask)

#ifdef HANZI
# define Is1stByteHZ(f)	(((f) & CHARHANZI) &&  ((f) & FIRSTBYTE))
# define Is2ndByteHZ(f)	(((f) & CHARHANZI) && !((f) & FIRSTBYTE))
# define ROWSINPUTAREA	2	/* number of rows in hanzi input areas */

# define FGCSinScreen(s,f)	\
		(((f) & BOLD_BIT) ? (s)->hz_boldFGCS : (s)->hz_normalFGCS)
#endif /* HANZI */

#define TEK_LINK_BLOCK_SIZE 1024

typedef struct Tek_Link
{
	struct Tek_Link	*next;	/* pointer to next TekLink in list
				   NULL <=> this is last TekLink */
	short fontsize;		/* character size, 0-3 */
	short count;		/* number of chars in data */
	char *ptr;		/* current pointer into data */
	char data [TEK_LINK_BLOCK_SIZE];
} TekLink;

/* flags for cursors */
#define	OFF		0
#define	ON		1
#define	CLEAR		0
#define	TOGGLE		1

/* flags for inhibit */
#ifdef ALLOWLOGGING
#define	I_LOG		0x01
#endif
#define	I_SIGNAL	0x02
#define	I_TEK		0x04

extern Cursor make_colored_cursor(int cursorindex, long unsigned int fg, long unsigned int bg);
extern int GetBytesAvailable(int fd);
extern void first_map_occurred(void);
extern int kill_process_group(int pid, int sig);
