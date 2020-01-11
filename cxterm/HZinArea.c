/*
 *	$Id: HZinArea.c,v 1.2 2003/05/06 07:40:19 htl10 Exp $
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

/* HZinArea.c		Input Area management module
 *
 * The file maintains the input area (bottom N lines) of the cxterm window.
 * The number of lines in the input area is ROWSINPUTAREA defined in ptyx.h. 
 * The current implementation of the input area uses 2 lines:
 *
 *		--------------------------------------- (the rule line)
 * line 1:	<prompt><input_buffer>         <status>
 * line 2:	<selection_list or message>
 *
 * The input area is divided into 4 parts:
 *	prompt (of the current input method),
 *	inbuf buffer (for the current key stroke sequence),
 *	status (for input processing state report), and
 * 	selection list (for the current on-screen HZ candidate list,
 *		sometimes overloaded to display error messages).
 *
 * The file contains routines to update the data structures and
 * to redraw the actual xterm window (by calling X) if necessary.
 * By calling these routines, cxterm input conversion module do not
 * need to worry how and when the actual text is drawed (just like
 * the curse library for input area).
 *
 * Each of the four parts of input area has its own refresh type.
 * It is possible that some parts have been changed and others not.
 * Due to the nature of incremental input conversion, it is very often
 * that only some portion of the text are updated.  We don't need
 * to completely redraw every characters in the input area.
 */

#include <ctype.h>
#include "HZinput.h"		/* X headers included here. */


extern int HZ_XDrawImString16(Display *display, Drawable d, GC gc, int fgcs, int x, int y, XChar2b *string, int length);	/* screen.c */

static void HZiaDrawRuleLine(TScreen *screen);
static void HZiaDrawText(TScreen *screen, int row, int col, Char *str, int nbytes, Char attr);
static void HZiaClearText(TScreen *screen, int row1, int col1, int row2, int col2);
static void HZiaFillStippe(HZInputArea *ia, TScreen *screen);

#define	IAPUTHANZI(ia,r,c,ch1,ch2,d,click)				\
	{ int _c;							\
	  if ((_c = (c)) + 1 < ia->cols) {				\
	    (ia)->buffer.text[(r)][_c] = (ch1);				\
	    (ia)->buffer.dpy_attr[(r)][_c] = 				\
			CHARDRAWN | CHARHANZI | FIRSTBYTE | (d);	\
	    (ia)->buffer.click_attr[(r)][ _c++ ] = (click);		\
	    (ia)->buffer.text[(r)][_c] = (ch2);				\
	    (ia)->buffer.dpy_attr[(r)][_c] = CHARDRAWN | CHARHANZI |(d);\
	    (ia)->buffer.click_attr[(r)][_c] = (click);			\
	  } else {							\
	    (ia)->buffer.text[(r)][_c] = ' ';				\
	    (ia)->buffer.dpy_attr[(r)][_c] = 0;				\
	    (ia)->buffer.click_attr[(r)][_c] = IA_Attr_Noop;		\
	  }								\
	}

#define	IAPUTCHAR(ia,r,c,ch,d,click)					\
	{ int _c;							\
	    (ia)->buffer.text[(r)][_c=(c)] = (ch);			\
	    (ia)->buffer.dpy_attr[(r)][_c] = CHARDRAWN | (d);		\
	    (ia)->buffer.click_attr[(r)][_c] = (click);			\
	}

#define	IACLEAR(a,r,f,t)						\
	{ int _i;							\
	  for (_i = f; _i < t; _i++)  a.text[r][_i] = ' ';		\
	  for (_i = f; _i < t; _i++)  a.dpy_attr[r][_i] = 0x0;		\
	  for (_i = f; _i < t; _i++)  a.click_attr[r][_i] = IA_Attr_Noop;\
	}

#define	IASHIFT(a,r,old,new,l)	{ 					\
	memmove(a.text[r] + new, a.text[r] + old, l);			\
	memmove(a.dpy_attr[r] + new, a.dpy_attr[r] + old, l);		\
	memmove(a.click_attr[r] + new, a.click_attr[r] + old, l);	\
    }

/*
 * Initialize the Input Area.
 */

extern void Bell (void);

void HZiaInit(HZInputArea *ia, TScreen *screen)
{
  int i;

    ia->cols = screen->max_col + 1;
    ia->rows = screen->hzIwin.rows;

    for (i = 0; i < ia->rows; i++) {
	ia->buffer.text[i] = (Char *)malloc(ia->cols);
	ia->buffer.dpy_attr[i] = (Char *)malloc(ia->cols);
	ia->buffer.click_attr[i] = (Char *)malloc(ia->cols);
	ia->screen.text[i] = (Char *)malloc(ia->cols);
	ia->screen.dpy_attr[i] = (Char *)malloc(ia->cols);
	ia->screen.click_attr[i] = (Char *)malloc(ia->cols);

	IACLEAR(ia->buffer, i, 0, ia->cols);
	IACLEAR(ia->screen, i, 0, ia->cols);
    }

    ia->lenPrompt = 0;
    ia->lenInbuf = 0;
    ia->lenStatus = 0;
    ia->lenSelection = 0;

    ia->sensitive = True;
}

/*
 * Clear up the data structures of the input area
 */
/*ARGSUSED*/
void HZiaCleanUp (HZInputArea *ia)
{
  int i;

    for (i = 0; i < ia->rows; i++) {
	free((char *)ia->buffer.text[i]);
	free((char *)ia->buffer.dpy_attr[i]);
	free((char *)ia->buffer.click_attr[i]);
	free((char *)ia->screen.text[i]);
	free((char *)ia->screen.dpy_attr[i]);
	free((char *)ia->screen.click_attr[i]);
    }
}

/*
 * Set the frame of the input area (prompt, key labels, etc)
 */
void HZiaSetFrame (HZInputArea *ia, HZInputMethod *hzim)
{
  int col, len = hzim->f.lenPrompt;
  Char *ptr;
    ia->maxchoice = hzim->f.maxchoice;
    ia->choicelb = hzim->f.choicelb;
    ia->keyprompt = hzim->f.keyprompt;

    if (hzim->f.lenPrompt >= ia->cols) {
	len = ia->cols;
    } else if (ia->lenInbuf) {
	int shift = ia->lenInbuf;

	if (hzim->f.lenPrompt + shift > ia->cols)
	    shift = ia->cols - hzim->f.lenPrompt;

	/* shift the old input buffer */
	memmove( &ia->buffer.text[0][hzim->f.lenPrompt],
		 &ia->buffer.text[0][ia->lenPrompt], shift );
	memmove( &ia->buffer.dpy_attr[0][hzim->f.lenPrompt],
		 &ia->buffer.dpy_attr[0][ia->lenPrompt], shift );
	memmove( &ia->buffer.click_attr[0][hzim->f.lenPrompt],
		 &ia->buffer.click_attr[0][ia->lenPrompt], shift );

	/* clear up the extra spaces left by moving the input buffer */
	IACLEAR(ia->buffer, 0, hzim->f.lenPrompt+shift, ia->lenPrompt+shift);
    } else 
	/* wipe out the trailing part of the old prompt */
	IACLEAR(ia->buffer, 0, hzim->f.lenPrompt, ia->lenPrompt);

    /* put down the new prompt */
    ptr = hzim->f.prompt;
    for (col = 0; col < len; col++, ptr++) {
	if ((*ptr) & 0x80)
	    IAPUTHANZI(ia, 0, col++, *ptr++, *ptr, 0, IA_Attr_Menu)
	else
	    IAPUTCHAR(ia, 0, col, *ptr, 0, IA_Attr_Menu)
    }
    ia->lenPrompt = len;
}

/*
 * Update the data structure (mostly called by HZ input conversion module)
 */
void HZiaSet (HZInputArea *ia, HZChoiceList *cl, char *inbuf, int inbufCount, int cursor, char *status)
                    	/* the input area */
                     	/* the new choice list; NULL if unchanged */
                	/* the new input buffer; NULL if unchanged */
                   	/* the length of the new input buffer */
               		/* the cursor position in the input buffer */
                 	/* the status report string; NULL if unchanged */
{
    if (inbuf)
	HZiaSetInBuf(ia, inbuf, inbufCount, cursor);
    if (status)
	HZiaSetStatus(ia, status);
    if (cl)
	HZiaSetChoiceList(ia, cl);
}

void HZiaSetInBuf (HZInputArea *ia, char *inbuf, int inbufCount, int cursor)
                    	/* the input area */
                	/* the new input buffer */
                   	/* the length of the new input buffer */
               		/* the cursor position in the input buffer */
{
  register int i;
  register int col = ia->lenPrompt;

    for (i = 0; (i < inbufCount) && (col < ia->cols); i++) {
	Char attr = 0;

	if (i == cursor)
	    attr |= INVERSE;

	if ((ia->keyprompt) && !(inbuf[i] & 0x80)) {
	    register unsigned short j = 0;
	    register Char *p = ia->keyprompt[(int) inbuf[i]].prompt;

	    while ((j < ia->keyprompt[(int) inbuf[i]].ptlen) && (col < ia->cols)) {
		if ((*p) & 0x80) {
		    IAPUTHANZI(ia, 0, col++, *p++, *p++, attr,
				IA_Attr_InputCursorMask|i)
		    j++;
		} else 
		    IAPUTCHAR(ia, 0, col, *p++, attr,
				IA_Attr_InputCursorMask|i)
		col++;
		j++;
	    }
	} else
	    IAPUTCHAR(ia, 0, col++, inbuf[i], attr, IA_Attr_InputCursorMask|i)
    }
    IACLEAR(ia->buffer, 0, col, ia->lenPrompt + ia->lenInbuf);
    ia->lenInbuf = col - ia->lenPrompt;
}

void HZiaSetStatus (HZInputArea *ia, register char *status)
                    		/* the input area */
                          	/* the status report string */
{
  int len = strlen(status);
  register int col = ia->cols - len;

    if (col < 0) {
	status += -col;
	len -= -col;
	col = 0;
    }
    IACLEAR(ia->buffer, 0, ia->cols - ia->lenPrompt, col);
    while (*status) {
	if ((*status) & 0x80)
	    IAPUTHANZI(ia, 0, col++, *status++, *status++, 0, IA_Attr_Noop)
	else
	    IAPUTCHAR(ia, 0, col, *status++, 0, IA_Attr_Noop)
	col++;
    }
    ia->lenStatus = len;
}

void HZiaSetChoiceList (HZInputArea *ia, HZChoiceList *cl)
                    	/* the input area */
                     	/* the new choice list; NULL if unchanged */
{
  register int col = 0;

    if ((cl->selNum > 0) && (ia->choicelb)) {
	/* the choice list has been updated */
	register int i;
	register struct hz_choice *pch = &(cl->choices[cl->selPos]);

	/* build the "< 1.XXXX 2.XX .... >" thing from the choice list */

	if (cl->selPos > 0)
	    IAPUTCHAR(ia, 1, col++, '<', 0, IA_Attr_MoveLeft)
	else
	    IAPUTCHAR(ia, 1, col++, ' ', 0, IA_Attr_Noop)
	for (i = 0; i < cl->selNum; i++, pch++) {
	    register int j;

	    IAPUTCHAR(ia, 1, col++, ' ', 0, IA_Attr_Noop)
	    IAPUTCHAR(ia, 1, col++, ia->choicelb[i], 0, IA_Attr_PickThis)
	    IAPUTCHAR(ia, 1, col++, '.', 0, IA_Attr_PickThis)
	    for (j = 0; j < pch->nhz; j++, col++, col++)
		IAPUTHANZI(ia, 1, col, pch->hzc[j].byte1, pch->hzc[j].byte2,
			   0, IA_Attr_PickMe)
	}
	IAPUTCHAR(ia, 1, col++, ' ', 0, IA_Attr_Noop)
	if ((cl->selPos + cl->selNum >= cl->numChoices) && cl->exhaust)
	    IAPUTCHAR(ia, 1, col++, ' ', 0, IA_Attr_Noop)
	else
	    IAPUTCHAR(ia, 1, col++, '>', 0, IA_Attr_MoveRight)
    }
    IACLEAR(ia->buffer, 1, col, ia->lenSelection);
    ia->lenSelection = col;
}

/*
 * Resize the input area (caused by the resizing of xterm)
 */
void HZiaResize(HZInputArea *ia, TScreen *screen, CXtermInputModule *cxin)
{
#define	REALLOC(x,sz)	x = (Char *)realloc((char *)x, sz)

  int new_cols = screen->max_col + 1;
  int i;
  int status_oldx, status_newx;

    if (new_cols == ia->cols)
	return;

    status_oldx = ia->cols - ia->lenStatus;
    status_newx = new_cols - ia->lenStatus;

    /* if the screen shrinks, shift the status inside the screen area */
    if ((new_cols < ia->cols) && (ia->lenStatus < new_cols)) {
	IASHIFT(ia->buffer, 0, status_oldx, status_newx, ia->lenStatus);
	IASHIFT(ia->screen, 0, status_oldx, status_newx, ia->lenStatus);
    }

    for (i = 0; i < ia->rows; i++) {
	REALLOC( ia->buffer.text[i], new_cols );
	REALLOC( ia->buffer.dpy_attr[i], new_cols );
	REALLOC( ia->buffer.click_attr[i], new_cols );
	REALLOC( ia->screen.text[i], new_cols );
	REALLOC( ia->screen.dpy_attr[i], new_cols );
	REALLOC( ia->screen.click_attr[i], new_cols );

	if (new_cols > ia->cols) {
	    /* clear the new enlarged part */
	    IACLEAR( ia->buffer, i, ia->cols, new_cols);
	    IACLEAR( ia->screen, i, ia->cols, new_cols);
	}
    }

    /* if the screen is widened, shift the status portion to the rightmost */
    if (new_cols > ia->cols) {
	IASHIFT(ia->buffer, 0, status_oldx, status_newx, ia->lenStatus);
	IACLEAR(ia->buffer, 0, status_oldx, status_newx);
	IASHIFT(ia->screen, 0, status_oldx, status_newx, ia->lenStatus);
	IACLEAR(ia->screen, 0, status_oldx, status_newx);
    }

    ia->cols = new_cols;

    /* the selection window now may hold more or less choices, re-make it */
    (void) HZclMakeSelection (&cxin->hzcl, ia, cxin->cur_ic);

    /* choice list may have been updated */
    HZiaSetChoiceList (ia, &cxin->hzcl);

    /* don't redraw, leave it to the Expose event */

#undef REALLOC
}

/*
 * Really draw (selectively) on the screen according to the data structures.
 */
void HZiaFlush(HZInputArea *ia, TScreen *screen)
{
#define DPY_ATTR_MASK	(INVERSE | UNDERLINE | BLINK | CHARDRAWN | CHARHANZI)

  int i;

    for (i = 0; i < ia->rows; i++) {
	Char *buf = ia->buffer.text[i];
	Char *scr = ia->screen.text[i];
	Char *abuf = ia->buffer.dpy_attr[i];
	Char *ascr = ia->screen.dpy_attr[i];
	int j = 0;

	while (j < ia->cols) {
	    Char attr;
	    int from;

	    /* skip the unmodified segment */
	    while ((j < ia->cols) && (buf[j] == scr[j]) &&
		   ((abuf[j] & DPY_ATTR_MASK) == (ascr[j] & DPY_ATTR_MASK)))
		j++;
	    if (j == ia->cols)
		break;		/* no more change this line */ 

	    if ((abuf[j] & CHARHANZI) && !(abuf[j] & FIRSTBYTE))
		from = --j;	/* hanzi, but not at 1st byte */
	    else
		from = j;

	    /* find a modified segment */
	    attr = abuf[ from ] & DPY_ATTR_MASK;
	    if (attr & CHARHANZI) {
		j++; j++;
		while ((j < ia->cols) && ((abuf[j] & DPY_ATTR_MASK) == attr) &&
		       ((buf[j] != scr[j]) || (buf[j+1] != scr[j+1]) ||
			((ascr[j] & DPY_ATTR_MASK) != attr)))
		{
		    j++; j++;
		}
	    } else {
		j++; 
		while ((j < ia->cols) && ((abuf[j] & DPY_ATTR_MASK) == attr) &&
		       ((buf[j] != scr[j]) || 
			((ascr[j] & DPY_ATTR_MASK) != attr)))
		    j++;
	    }

	    /* from "from" to "j" */
	    if (attr)
		/* draw "buf", from "from" to "j", using attribute "attr" */
		HZiaDrawText (screen, i, from, &(buf[from]), j - from, attr);
	    else
		/* empty attribute, it is actually a clear */
		HZiaClearText (screen, i, from, i, j - 1);

	    /* install the changes to screen */ 
	    memmove(scr + from, buf + from, j - from);
	    memmove(ascr + from, abuf + from, j - from);
	    memmove(&ia->screen.click_attr[i][from],
		    &ia->buffer.click_attr[i][from], j - from);
	}
    }
    if (! ia->sensitive)
	HZiaFillStippe(ia, screen);

#undef DPY_ATTR_MASK
}

/*
 * Do a complete redraw on everything in the input area
 */
void HZiaRedraw(HZInputArea *ia, TScreen *screen)
{
    int i;
    extern XtermWidget term;

    XSetForeground(screen->display, screen->hz_normalGC, screen->foreground);
    XSetBackground(screen->display, screen->hz_normalGC, term->core.background_pixel);
    XSetForeground(screen->display, screen->normalGC, screen->foreground);
    XSetBackground(screen->display, screen->normalGC, term->core.background_pixel);

    HZiaClearText (screen, 0, 0, ia->rows, ia->cols - 1);
    HZiaDrawRuleLine (screen);

    for (i = 0; i < ia->rows; i++) {
	IACLEAR(ia->screen, i, 0, ia->cols);
    }
    HZiaFlush(ia, screen);
}

/*
 * Temporally display one line of message on the 2nd row of the input area
 */ 
void HZiaShowMesg (HZInputArea *ia, TScreen *screen, char *mesgstr)
{
    register int col = 0;
    extern XtermWidget term;

    XSetForeground(screen->display, screen->hz_normalGC, screen->foreground);
    XSetBackground(screen->display, screen->hz_normalGC, term->core.background_pixel);
    XSetForeground(screen->display, screen->normalGC, screen->foreground);
    XSetBackground(screen->display, screen->normalGC, term->core.background_pixel);


    while ((*mesgstr) && (col < ia->cols)) {
	if ((*mesgstr) & 0x80)
	    IAPUTHANZI(ia, 1, col++, *mesgstr++, *mesgstr++, 0, IA_Attr_Noop)
	else
	    IAPUTCHAR(ia, 1, col, *mesgstr++, 0, IA_Attr_Noop)
	col++;
    }
    IACLEAR(ia->buffer, 1, col, ia->lenSelection);
    ia->lenSelection = col;

    HZiaFlush (ia, screen);
    XFlush(screen->display);	/* always flush the message display */
}

/*
 * Invoke actions for a click event in the Input Area
 */ 
void HZiaClickAction (HZInputArea *ia, CXtermInputModule *cxin, int x, int y)
{
    TScreen *tscreen = cxin->screen;
    HZChoiceList *cl = &cxin->hzcl;
    int row, col;		/* row, col in the Input Area */
    int from, to;		/* start/end pos for the click on choice */
    Boolean was_exhaust;

    row = (y - tscreen->hzIwin.y) / FontHeight(tscreen);
    col = (x - tscreen->hzIwin.x) / FontWidth(tscreen);

    /* inside the input area? */
    if ((col < 0) || (col >= ia->cols) || (row < 0) || (row >= ia->rows))
	return;

    switch (ia->screen.click_attr[row][col]) {

      case IA_Attr_PickMe:
	from = col;
	while ((from > 0) &&
	       (ia->screen.click_attr[row][from-1] == IA_Attr_PickMe))
	    from--;

	to = col + 1;
	while ((to < ia->cols) &&
	       (ia->screen.click_attr[row][to] == IA_Attr_PickMe))
	    to++;

	XtermWritePty((char *)&(ia->screen.text[row][from]), to - from);
	return;

      case IA_Attr_PickThis:	/* the choice after this label */
	from = col + 1;
	while ((from < ia->cols) &&
		(ia->screen.click_attr[row][from] != IA_Attr_PickMe))
	    from++;
	if (from == ia->cols)	/* reach the end */
	    return;

	to = from + 1;
	while ((to < ia->cols) &&
	       (ia->screen.click_attr[row][to] == IA_Attr_PickMe))
	    to++;

	XtermWritePty((char *)&(ia->screen.text[row][from]), to - from);
	return;

      case IA_Attr_MoveRight:
	was_exhaust = cl->exhaust;
	if (HZclMoveForward (cl, ia, cxin->cur_ic) == 0) {
	    Bell ();
	    if (was_exhaust)
		return;
	    /* if not exhausted before, redisplay the current choice list */
	}
	HZiaSetChoiceList (ia, cl);
	HZiaFlush (ia, cxin->screen);
	return;
	/* break; */

      case IA_Attr_MoveLeft:
	if (HZclMoveBackward (cl, ia, cxin->cur_ic) == 0) {
	    Bell ();
	    return;
	}
	HZiaSetChoiceList (ia, cl);
	HZiaFlush (ia, cxin->screen);
	return;
	/* break; */

      default:
	if (ia->screen.click_attr[row][col] & IA_Attr_InputCursorMask) {
	    cxin->cursor =
		ia->screen.click_attr[row][col] ^ IA_Attr_InputCursorMask;
	    HZiaSetInBuf (ia, cxin->hzinbuf, cxin->hzinbufCount, cxin->cursor);
	    HZiaFlush (ia, tscreen);
	}
	break;
    }
}


/*
 * HZiaDrawRuleLine -- draw the line that leading the input area
 */
static void HZiaDrawRuleLine (TScreen *screen)
{
    XDrawLine (screen->display, TextWindow(screen),
		screen->hz_normalGC,
		screen->hzIwin.rule_x, screen->hzIwin.rule_y,
		screen->hzIwin.rule_x + screen->hzIwin.rule_length,
		screen->hzIwin.rule_y); 
}

/*
 * HZiaDrawText -- draw text at (row, col) of input area 
 */
static void HZiaDrawText (TScreen *screen, int row, int col, Char *str, int nbytes, Char attr)
{
  int x = screen->hzIwin.x + FontWidth(screen) * col;
  int y = screen->hzIwin.y + FontHeight(screen) * row + FontAscent(screen);
  GC gc;

    if (attr & CHARHANZI) {
	gc = (attr & INVERSE) ? screen->hz_reverseGC : screen->hz_normalGC;
	HZ_XDrawImString16 (screen->display, TextWindow(screen), gc,
			screen->hz_normalFGCS, x, y, (XChar2b *)str, nbytes/2);
    } else {
    char buf[128];
    int i, len;
    len = nbytes < 128 ? nbytes : 128;
    for (i = 0; i < len; ++i) {
        buf[i] = isprint (str[i]) ? str[i] : '-';
    }
	gc = (attr & INVERSE) ? screen->reverseGC : screen->normalGC;
	XDrawImageString (screen->display, TextWindow(screen), gc,
			x, y, buf, len);
    }
    if (attr & UNDERLINE)
	XDrawLine(screen->display, TextWindow(screen), gc, x, y+1,
			x + nbytes * FontWidth(screen), y+1);
}

/*
 * HZiaClearText -- clear from (row1, col1) to (row2, col2) of input area
 */
static void HZiaClearText (TScreen *screen, int row1, int col1, int row2, int col2)
{
    if ((row1 > row2) || (col1 > col2))
	return;

    XClearArea (screen->display, TextWindow(screen),
	screen->hzIwin.x + FontWidth (screen) * col1,
	screen->hzIwin.y + FontHeight(screen) * row1,
	FontWidth (screen) * (col2 - col1 + 1),
	FontHeight(screen) * (row2 - row1 + 1),
	FALSE);
}

/*
 * Make the whole input area look insensitive
 */
static void HZiaFillStippe (HZInputArea *ia, TScreen *screen)
{
    static GC gc = 0;
    extern XtermWidget term;

    if (! gc) {		/* first time call, create a new one */
	long fg = WhitePixel(screen->display, DefaultScreen(screen->display));
	long bg = BlackPixel(screen->display, DefaultScreen(screen->display));
	unsigned long mask;
	XGCValues values;
	static char bitmap[] = { 0x02, 0x01 };

	values.function = GXcopy;
	values.stipple = XCreatePixmapFromBitmapData (screen->display,
				TextWindow(screen), bitmap, 2, 2, fg, bg, 1);
	values.fill_style = FillStippled;
	values.fill_rule = EvenOddRule;

	mask = GCFunction | GCStipple | GCFillStyle | GCFillRule;
	gc = XCreateGC (screen->display, TextWindow(screen), mask, &values);
	if (! gc)
	    return;
    }

    XCopyGC(screen->display, screen->reverseGC, GCForeground|GCBackground, gc);
    XSetForeground(screen->display, gc, term->core.background_pixel);
    XSetBackground(screen->display, gc, term->core.background_pixel);

    XFillRectangle (screen->display, TextWindow(screen), gc,
	screen->hzIwin.x, screen->hzIwin.y,
	FontWidth (screen) * ia->cols, FontHeight(screen) * ia->rows);
}
