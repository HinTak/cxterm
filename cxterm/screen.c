/* $Id: screen.c,v 1.2.2.1 2004/08/14 02:43:03 htl10 Exp $ */
/***********************************************************************
* Copyright 1994 by Yongguang Zhang.
* Copyright 1990, 1991 by Yongguang Zhang and Pong Man-Chi.
*
* All rights reserved.  Under the same copyright and permission term as
* the original.  Absolutely no warranties of any kinds.
************************************************************************/

/*
 *	$XConsortium: screen.c,v 1.33 94/04/02 17:34:36 gildea Exp $
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

/* screen.c */

#include "ptyx.h"
#include "error.h"
#include "data.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#ifdef SVR4
#include <termios.h>
#else
#include <sys/ioctl.h>
#endif

#ifdef att
#include <sys/termio.h>
#include <sys/stream.h>			/* get typedef used in ptem.h */
#include <sys/ptem.h>
#endif

#ifdef HANZI
extern int HZ_XDrawImString16(Display *display, Drawable d, GC gc, int fgcs, int x, int y, XChar2b *string, int length);
extern int HZ_XDrawString16(Display *display, Drawable d, GC gc, int fgcs, int x, int y, XChar2b *string, int length);
#endif /* HANZI */


extern void SysError (int i);
extern void HideCursor (void);
extern void SwitchBufPtrs (register TScreen *screen);
extern void ScrollSelection (register TScreen *screen, register int amount);
extern void ResizeScrollBar (register Widget scrollWidget, int x, int y, unsigned int height);
extern void ResizeSelection (TScreen *screen, int rows, int cols);
extern void ResizeCXtermInput (TScreen *screen);

ScrnBuf Allocate (register int nrow, register int ncol, Char **addr)
/*
   allocates memory for a 2-dimensional array of chars and returns a pointer
   thereto
   each line is formed from four char arrays.  The first one is the actual
   character array, the second one is the attributes, the third is the
   foreground color, and the fourth is the background color.
 */
                        
            
{
	register ScrnBuf base;
	register Char *tmp;
	register int i;

	if ((base = (ScrnBuf) calloc ((unsigned)(nrow *= 4), sizeof (char *))) == 0)
		SysError (ERROR_SCALLOC);

	if ((tmp = (Char *)calloc ((unsigned) (nrow * ncol), sizeof(char))) == 0)
		SysError (ERROR_SCALLOC2);

	*addr = tmp;
	for (i = 0; i < nrow; i++, tmp += ncol)
		base[i] = tmp;

	return (base);
}

/*
 *  This is called when the screen is resized.
 *  Returns the number of lines the text was moved down (neg for up).
 *  (Return value only necessary with SouthWestGravity.)
 */
static
int Reallocate(ScrnBuf *sbuf, Char **sbufaddr, int nrow, int ncol, int oldrow, int oldcol)
{
	register ScrnBuf base;
	register Char *tmp;
	register int i, minrows, mincols;
	Char *oldbuf;
	int move_down = 0, move_up = 0;
	
	if (sbuf == NULL || *sbuf == NULL)
		return 0;

	oldrow *= 4;
	oldbuf = *sbufaddr;

	/*
	 * Special case if oldcol == ncol - straight forward realloc and
	 * update of the additional lines in sbuf
	 */

	/* this is a good idea, but doesn't seem to be implemented.  -gildea */

	/* 
	 * realloc sbuf, the pointers to all the lines.
	 * If the screen shrinks, remove lines off the top of the buffer
	 * if resizeGravity resource says to do so.
	 */
	nrow *= 4;
	if (nrow < oldrow  &&  term->misc.resizeGravity == SouthWestGravity) {
	    /* Remove lines off the top of the buffer if necessary. */
	    move_up = oldrow-nrow 
		        - 4*(term->screen.max_row - term->screen.cur_row);
	    if (move_up < 0)
		move_up = 0;
	    /* Overlapping memmove here! */
	    memmove( *sbuf, *sbuf+move_up, 
		  (oldrow-move_up)*sizeof((*sbuf)[0]) );
	}
	*sbuf = (ScrnBuf) realloc((char *) (*sbuf),
				  (unsigned) (nrow * sizeof(char *)));
	if (*sbuf == 0)
	    SysError(ERROR_RESIZE);
	base = *sbuf;

	/* 
	 *  create the new buffer space and copy old buffer contents there
	 *  line by line.
	 */
	if ((tmp = (Char *)calloc((unsigned) (nrow * ncol), sizeof(char))) == 0)
		SysError(ERROR_SREALLOC);
	*sbufaddr = tmp;
	minrows = (oldrow < nrow) ? oldrow : nrow;
	mincols = (oldcol < ncol) ? oldcol : ncol;
	if (nrow > oldrow  &&  term->misc.resizeGravity == SouthWestGravity) {
	    /* move data down to bottom of expanded screen */
	    move_down = Min(nrow-oldrow, 4*term->screen.savedlines);
	    tmp += ncol*move_down;
	}
	for (i = 0; i < minrows; i++, tmp += ncol) {
		memmove( tmp, base[i], mincols);
	}
	/*
	 * update the pointers in sbuf
	 */
	for (i = 0, tmp = *sbufaddr; i < nrow; i++, tmp += ncol)
	    base[i] = tmp;

        /* Now free the old buffer */
	free(oldbuf);

	return move_down ? move_down/4 : -move_up/4; /* convert to rows */
}

void ScreenWrite (TScreen *screen, char *str, register unsigned int flags, register unsigned int cur_fg, register unsigned int cur_bg, register int length)
/*
   Writes str into buf at row row and column col.  Characters are set to match
   flags.
 */
                
          
                        
                                 
                    		/* length of string */
{
	int len;
	register Char *attrs;
	register Char *fgs, *bgs;
	register int avail  = screen->max_col - screen->cur_col + 1;
	register Char *col;

	if (length > avail)
	    length = avail;
	if (length <= 0)
		return;

	col = screen->buf[avail = 4 * screen->cur_row] + screen->cur_col;
	attrs = screen->buf[avail + 1] + screen->cur_col;
	fgs = screen->buf[avail + 2] + screen->cur_col;
	bgs = screen->buf[avail + 3] + screen->cur_col;
	flags &= ATTRIBUTES;
	flags |= CHARDRAWN;
	memmove( col, str, len = length);
#ifdef HANZI
	if (flags & CHARHANZI) {
		while (length > 1) {
			*attrs++ = flags|FIRSTBYTE;		/* 1st byte of HZ */
			*fgs++ = cur_fg;
			*bgs++ = cur_bg;

			*attrs++ = flags&~FIRSTBYTE;	/* 2nd byte of HZ */
			*fgs++ = cur_fg;
			*bgs++ = cur_bg;

            length -= 2;
		}
        if (length > 0) {
			*attrs++ = flags|FIRSTBYTE;		/* 1st byte of HZ */
			*fgs++ = cur_fg;
			*bgs++ = cur_bg;
        }
    }
	else
#endif /* HANZI */
	while(length-- > 0) {
		*attrs++ = flags&~FIRSTBYTE;
		*fgs++ = cur_fg;
		*bgs++ = cur_bg;
	}
#ifdef HANZI
    if (! (flags&CHARHANZI) && len < avail
     && (*attrs&(CHARHANZI|FIRSTBYTE)) == CHARHANZI){
        col[len] = ' ';
        *attrs &= ~CHARHANZI;
    }
#endif
}

void ScrnInsertLine (register ScrnBuf sb, int last, register int where, register int n, register int size)
/*
   Inserts n blank lines at sb + where, treating last as a bottom margin.
   Size is the size of each entry in sb.
   Requires: 0 <= where < where + n <= last
   	     n <= MAX_ROWS
 */
                    
         
                            
{
	register int i;
	char *save [4 * MAX_ROWS];


	/* save n lines at bottom */
	memmove( (char *) save, (char *) &sb [4 * (last -= n - 1)], 
		4 * sizeof (char *) * n);
	
	/* clear contents of old rows */
	for (i = 4 * n - 1; i >= 0; i--)
		bzero ((char *) save [i], size);

	/*
	 * WARNING, overlapping copy operation.  Move down lines (pointers).
	 *
	 *   +----|---------|--------+
	 *
	 * is copied in the array to:
	 *
	 *   +--------|---------|----+
	 */
	memmove( (char *) &sb [4 * (where + n)], (char *) &sb [4 * where], 
		4 * sizeof (char *) * (last - where));

	/* reuse storage for new lines at where */
	memmove( (char *) &sb[4 * where], (char *)save, 4 * sizeof(char *) * n);
}


void ScrnDeleteLine (register ScrnBuf sb, register int last, int where, register int n, register int size)
/*
   Deletes n lines at sb + where, treating last as a bottom margin.
   Size is the size of each entry in sb.
   Requires 0 <= where < where + n < = last
   	    n <= MAX_ROWS
 */
                    
                           
          
{
	register int i;
	char *save [4 * MAX_ROWS];

	/* save n lines at where */
	memmove( (char *)save, (char *) &sb[4 * where], 4 * sizeof(char *) * n);

	/* clear contents of old rows */
	for (i = 4 * n - 1 ; i >= 0 ; i--)
		bzero ((char *) save [i], size);

	/* move up lines */
	memmove( (char *) &sb[4 * where], (char *) &sb[4 * (where + n)], 
		4 * sizeof (char *) * ((last -= n - 1) - where));

	/* reuse storage for new bottom lines */
	memmove( (char *) &sb[4 * last], (char *)save, 
		4 * sizeof(char *) * n);
}


void ScrnInsertChar (ScrnBuf sb, int row, register int col, register int n, int size)
    /*
      Inserts n blanks in sb at row, col.  Size is the size of each row.
      */
               
                  
                        
{
	register int i, j;
	register Char *ptr = sb [4 * row];
	register Char *attrs = sb [4 * row + 1];
    register Char *fg = sb[4*row+2];
    register Char *bg = sb[4*row+3];

	for (i = size - 1; i >= col + n; i--) {
		ptr[i] = ptr[j = i - n];
		attrs[i] = attrs[j];
        fg[i] = fg[j];
        bg[i] = bg[j];
	}

	for (i = col; i < col+n; ptr[i] = ' ', attrs[i] = CHARDRAWN, ++i);
}


void ScrnDeleteChar (ScrnBuf sb, register int row, register int col, register int n, register int size)
    /*
      Deletes n characters in sb at row, col. Size is the size of each row.
      */
               
                           
                        
{
	register Char *ptr = sb[4 * row];
	register Char *attrs = sb[4 * row + 1];
	register int nbytes = (size - n - col);

	memmove( ptr + col, ptr + col + n, nbytes);
	memmove( attrs + col, attrs + col + n, nbytes);
	bzero (ptr + size - n, n);
	bzero (attrs + size - n, n);
}


void ScrnRefresh (register TScreen *screen, int toprow, int leftcol, int nrows, int ncols, Boolean force)
/*
   Repaints the area enclosed by the parameters.
   Requires: (toprow, leftcol), (toprow + nrows, leftcol + ncols) are
   	     coordinates of characters in screen;
	     nrows and ncols positive.
 */
                         
                                  
              			/* ... leading/trailing spaces */
{
	int y = toprow * FontHeight(screen) + screen->border +
#ifdef HANZI
		FontAscent(screen);
#else  /* HANZI */
		screen->fnt_norm->ascent;
#endif /* HANZI */
	register int row;
	register int topline = screen->topline;
	int maxrow = toprow + nrows - 1;
	int scrollamt = screen->scroll_amt;
	int max = screen->max_row;

	if(screen->cursor_col >= leftcol && screen->cursor_col <=
	 (leftcol + ncols - 1) && screen->cursor_row >= toprow + topline &&
	 screen->cursor_row <= maxrow + topline)
		screen->cursor_state = OFF;
	for (row = toprow; row <= maxrow; y += FontHeight(screen), row++) {
	   register Char *chars;
	   register Char *attrs;
	   register Char *fgs, *bgs;
	   register int col = leftcol;
	   int maxcol = leftcol + ncols - 1;
	   int lastind;
	   int flags;
	   int fg, bg;
	   int x, n;
	   GC gc;
	   Boolean hilite;
	   Pixel fg_pix, bg_pix;
	   extern XtermWidget term;

	   if (row < screen->top_marg || row > screen->bot_marg)
		lastind = row;
	   else
		lastind = row - scrollamt;

	   if (lastind < 0 || lastind > max)
	   	continue;

	   chars = screen->buf [4 * (lastind + topline)];
	   attrs = screen->buf [4 * (lastind + topline) + 1];
	   fgs = screen->buf [4 * (lastind + topline) + 2];
	   bgs = screen->buf [4 * (lastind + topline) + 3];

#ifdef HANZI
#if 0
	   if (Is2ndByteHZ (attrs[col]))
		col--;		/* draw the whole HANZI */
	   if (Is1stByteHZ (attrs[maxcol]))
		maxcol++;	/* draw the whole HANZI */
#endif
#endif /* HANZI */
	   if (row < screen->startHRow || row > screen->endHRow ||
	       (row == screen->startHRow && maxcol < screen->startHCol) ||
	       (row == screen->endHRow && col >= screen->endHCol))
	       {
	       /* row does not intersect selection; don't hilite */
	       if (!force) {
		   while (col <= maxcol && !(attrs[col]&CHARDRAWN)) col++;
		   while (col <= maxcol && !(attrs[maxcol]&CHARDRAWN)) maxcol--;
	       }
	       hilite = False;
	   }
	   else {
	       /* row intersects selection; split into pieces of single type */
	       if (row == screen->startHRow && col < screen->startHCol) {
		   ScrnRefresh(screen, row, col, 1, screen->startHCol - col,
			       force);
		   col = screen->startHCol;
	       }
	       if (row == screen->endHRow && maxcol >= screen->endHCol) {
		   ScrnRefresh(screen, row, screen->endHCol, 1,
			       maxcol - screen->endHCol + 1, force);
		   maxcol = screen->endHCol - 1;
	       }
	       /* remaining piece should be hilited */
	       hilite = True;
	   }

	   if (col > maxcol) continue;

	   flags = attrs[col];
       if ((flags&(CHARHANZI|FIRSTBYTE)) == CHARHANZI && col > 0) {
           flags = attrs[--col];
       }
           fg = fgs[col];
           bg = bgs[col];

           if (! (screen->ext_flags&USEBOLD) && ! (flags&FG_COLOR) && (fg&BOLD_BIT)) {
               fg_pix = screen->hilightcolor;
           }
           else {
               fg_pix = (flags & FG_COLOR) ? screen->colors[fg&COLORMASK]
                                           : screen->foreground;
           }
           bg_pix = (flags & BG_COLOR) ? screen->colors[(bg&COLORMASK)+N_FGCOLOR]
                                       : term->core.background_pixel;

#ifdef HANZI
	   if (flags & CHARHANZI) {
	     if ( (!hilite && (flags & INVERSE) != 0) ||
		  (hilite && (flags & INVERSE) == 0) ) {
                 if ((fg&BOLD_BIT)) gc = screen->hz_reverseboldGC;
	         else gc = screen->hz_reverseGC;
                 XSetForeground(screen->display, gc, bg_pix);
                 XSetBackground(screen->display, gc, fg_pix);
	     } else {
	         if ((fg&BOLD_BIT)) gc = screen->hz_normalboldGC;
	         else gc = screen->hz_normalGC;
                 XSetForeground(screen->display, gc, fg_pix);
                 XSetBackground(screen->display, gc, bg_pix);
         }
       }
	   else 
#endif /* HANZI */
       {
	   if ( (!hilite && (flags & INVERSE) != 0) ||
	        (hilite && (flags & INVERSE) == 0) ) {
	       if ((fg&BOLD_BIT)) gc = screen->reverseboldGC;
	       else gc = screen->reverseGC;
               XSetForeground(screen->display, gc, bg_pix);
               XSetBackground(screen->display, gc, fg_pix);
	   } else {
	       if ((fg&BOLD_BIT)) gc = screen->normalboldGC;
	       else gc = screen->normalGC;
               XSetForeground(screen->display, gc, fg_pix);
               XSetBackground(screen->display, gc, bg_pix);
       }
       }

	   x = CursorX(screen, col);
	   lastind = col;

	   for (; col <= maxcol; col++) {
                if (attrs[col] != flags || fgs[col] != fg
                 || ((flags&BG_COLOR) && ((bgs[col]^bg)&COLORMASK))) { 
          if (col > lastind) {
#ifdef HANZI
		   if ((flags&CHARHANZI)) {
			HZ_XDrawImString16(screen->display, TextWindow(screen),
				gc, FGCSinScreen(screen, fg),
				x, y, (XChar2b *) &chars[lastind],
				(n = col - lastind) /2 );
		   } else
#endif /* HANZI */
                   XDrawImageString(screen->display, TextWindow(screen),
                                    gc, x, y, (char *) &chars[lastind],
                                    n = col - lastind);

#ifdef HANZI
		   if (flags & CHARHANZI) {
		       if((fg&BOLD_BIT) && screen->hz_enbolden)
			    HZ_XDrawString16(screen->display,
				TextWindow(screen), gc, screen->hz_boldFGCS,
				x + 1, y, (XChar2b *) &chars[lastind], n/2 );
		   } else
#endif /* HANZI */
		   if((fg&BOLD_BIT) && screen->enbolden)
		 	XDrawString(screen->display, TextWindow(screen), 
			 gc, x + 1, y, (char *) &chars[lastind], n);
		   if(flags & UNDERLINE) 
			XDrawLine(screen->display, TextWindow(screen), 
			 gc, x, y+1, x+n*FontWidth(screen), y+1);
          }

		   x += (col - lastind) * FontWidth(screen);

		   lastind = col;

           flags = attrs[col];
		   fg = fgs[col];
		   bg = bgs[col];

           if (! (screen->ext_flags&USEBOLD) && ! (flags&FG_COLOR) && (fg&BOLD_BIT)) {
               fg_pix = screen->hilightcolor;
           }
           else {
		       fg_pix = (flags & FG_COLOR) ? screen->colors[fg&COLORMASK]
						                   : screen->foreground;
           }
		   bg_pix = (flags & BG_COLOR) ? screen->colors[(bg&COLORMASK)+N_FGCOLOR]
						: term->core.background_pixel;

#ifdef HANZI
		   if (flags & CHARHANZI) {
		     if ((!hilite && (flags & INVERSE) != 0) ||
			 (hilite && (flags & INVERSE) == 0) ) {
			if ((fg&BOLD_BIT)) gc = screen->hz_reverseboldGC;
			else gc = screen->hz_reverseGC;
			XSetForeground(screen->display, gc, bg_pix);
			XSetBackground(screen->display, gc, fg_pix);
		   } else {
			if ((fg&BOLD_BIT)) gc = screen->hz_normalboldGC;
			else gc = screen->hz_normalGC;
			  XSetForeground(screen->display, gc, fg_pix);
			  XSetBackground(screen->display, gc, bg_pix);
		    }
		   } else 
#endif /* HANZI */
           {
	   	   if ((!hilite && (flags & INVERSE) != 0) ||
		       (hilite && (flags & INVERSE) == 0) ) {
	       		if ((fg&BOLD_BIT)) gc = screen->reverseboldGC;
	       		else gc = screen->reverseGC;
			XSetForeground(screen->display, gc, bg_pix);
			XSetBackground(screen->display, gc, fg_pix);
		    } else {
			if ((fg&BOLD_BIT)) gc = screen->normalboldGC;
			else gc = screen->normalGC;
			XSetForeground(screen->display, gc, fg_pix);
			XSetBackground(screen->display, gc, bg_pix);
		    }
            }
		}

        if (! (flags&CHARDRAWN)) {
            chars[col] = ' ';
#ifdef HANZI
            flags &= ~(CHARHANZI|FIRSTBYTE);
            attrs[col] = flags;
#endif
        }
#ifdef HANZI
		if (Is1stByteHZ(flags)) {
			if(chars[col] == 0 || chars[col+1] == 0) {
				chars[col] = chars[col+1] = ' ';
                attrs[col] &= ~(CHARHANZI|FIRSTBYTE);
                attrs[col+1] = attrs[col];
                col -= 2;
            }
            ++col;
		}
        else
#endif /* HANZI */
		if(chars[col] == 0)
			chars[col] = ' ';
	   }


#ifdef HANZI
	   if (flags & CHARHANZI) 
		if ( (!hilite && (flags & INVERSE) != 0) ||
		     (hilite && (flags & INVERSE) == 0) ) {
		    if ((fg&BOLD_BIT)) gc = screen->hz_reverseboldGC;
		    else gc = screen->hz_reverseGC;
                    XSetForeground(screen->display, gc, bg_pix);
                    XSetBackground(screen->display, gc, fg_pix);
		} else {
		    if ((fg&BOLD_BIT)) gc = screen->hz_normalboldGC;
		    else gc = screen->hz_normalGC;
                    XSetForeground(screen->display, gc, fg_pix);
                    XSetBackground(screen->display, gc, bg_pix);
                }
	   else 
#endif /* HANZI */
	   if ( (!hilite && (flags & INVERSE) != 0) ||
	        (hilite && (flags & INVERSE) == 0) ) {
	       if ((fg&BOLD_BIT)) gc = screen->reverseboldGC;
	       else gc = screen->reverseGC;
               XSetForeground(screen->display, gc, bg_pix);
               XSetBackground(screen->display, gc, fg_pix);
           } else {
	       if ((fg&BOLD_BIT)) gc = screen->normalboldGC;
	       else gc = screen->normalGC;
               XSetForeground(screen->display, gc, fg_pix);
               XSetBackground(screen->display, gc, bg_pix);
           }
#ifdef HANZI
	   if (flags & CHARHANZI) {
	       HZ_XDrawImString16(screen->display, TextWindow(screen), gc,
		    FGCSinScreen(screen, fg),
		    x, y, (XChar2b *) &chars[lastind], (n = col - lastind)/2 );
	   } else
#endif /* HANZI */
	   XDrawImageString(screen->display, TextWindow(screen), gc, 
	         x, y, (char *) &chars[lastind], n = col - lastind);
#ifdef HANZI
	   if (flags & CHARHANZI) {
	       if((fg&BOLD_BIT) && screen->hz_enbolden)
		    HZ_XDrawString16(screen->display, TextWindow(screen), gc,
		    screen->hz_boldFGCS,
		    x + 1, y, (XChar2b *) &chars[lastind], n/2);
	   } else
#endif /* HANZI */
	   if((fg&BOLD_BIT) && screen->enbolden)
		XDrawString(screen->display, TextWindow(screen), gc,
		x + 1, y, (char *) &chars[lastind], n);
	   if(flags & UNDERLINE) 
		XDrawLine(screen->display, TextWindow(screen), gc, 
		 x, y+1, x + n * FontWidth(screen), y+1);
	}
}

void
ScrnBlinkRefresh (register TScreen *screen, int toprow, int leftcol, int nrows, int ncols)
{
    int y = toprow*FontHeight (screen)+screen->border+
#ifdef HANZI
        FontAscent (screen);
#else  /* HANZI */
        screen->fnt_norm->ascent;
#endif /* HANZI */
    register int row;
    register int topline = screen->topline;
    int maxrow = toprow+nrows-1;
    int max = screen->max_row;
    int scrollamt = screen->scroll_amt;

    for (row = toprow; row <= maxrow; y += FontHeight (screen), ++row) {
        register Char *chars;
        register Char *attrs;
        register Char *fgs, *bgs;
        register int col = leftcol;
        int maxcol = leftcol+ncols-1;
        int lastind;
        int flags;
        int fg, bg;
        int x, n;
        GC gc;
        Boolean hilite;
        Pixel fg_pix, bg_pix;
        extern XtermWidget term;

        lastind = (row < screen->top_marg || row > screen->bot_marg) ? row : row-scrollamt;
        if (lastind < 0 || lastind > max)
            continue;

        chars = screen->buf[4*(lastind+topline)];
        attrs = screen->buf[4*(lastind+topline)+1];
        fgs = screen->buf[4*(lastind+topline)+2];
        bgs = screen->buf[4*(lastind+topline)+3];

        if (row < screen->startHRow || row > screen->endHRow
         || (row == screen->startHRow && maxcol < screen->startHCol)
         || (row == screen->endHRow && col >= screen->endHCol)) {
            /* row does not intersect selection; don't hilite */
            hilite = False;
        }
        else {
            /* row intersects selection; split into pieces of single type */
            if (row == screen->startHRow && col < screen->startHCol) {
                ScrnBlinkRefresh (screen, row, col, 1, screen->startHCol-col);
                col = screen->startHCol;
            }
            if (row == screen->endHRow && maxcol >= screen->endHCol) {
                ScrnBlinkRefresh (screen, row, screen->endHCol, 1, maxcol-screen->endHCol+1);
                maxcol = screen->endHCol-1;
            }
            /* remaining piece should be hilited */
            hilite = True;
        }
  	    while (col <= maxcol && (attrs[col]&(BLINK|CHARDRAWN)) != (BLINK|CHARDRAWN)) col++;
   	    while (col <= maxcol && (attrs[maxcol]&(BLINK|CHARDRAWN)) != (BLINK|CHARDRAWN)) maxcol--;
        if (col > maxcol) continue;

        flags = attrs[col];
        fg = fgs[col];
        bg = bgs[col];
        bg_pix = (flags&BG_COLOR) ? screen->colors[(bg&COLORMASK)+N_FGCOLOR]
                                  : term->core.background_pixel;
        if (screen->blink_state) {
            if (! (screen->ext_flags&USEBOLD) && ! (flags&FG_COLOR) && (fg&BOLD_BIT)) {
                fg_pix = screen->hilightcolor;
            }
            else {
                fg_pix = (flags&FG_COLOR) ? screen->colors[fg&COLORMASK]
                                          : screen->foreground;
            }
        }
        else {
            fg_pix = bg_pix;
        }

#ifdef HANZI
        if (flags & CHARHANZI) {
            if ((! hilite && (flags & INVERSE) != 0) || (hilite && (flags & INVERSE) == 0)) {
                gc = ((fg&BOLD_BIT)) ?
                     screen->hz_reverseboldGC : screen->hz_reverseGC;
                XSetForeground (screen->display, gc, bg_pix);
                XSetBackground (screen->display, gc, fg_pix);
            }
            else {
                gc = ((fg&BOLD_BIT)) ?
                     screen->hz_normalboldGC : screen->hz_normalGC;
                XSetForeground (screen->display, gc, fg_pix);
                XSetBackground (screen->display, gc, bg_pix);
            }
        }
        else
#endif /* HANZI */
        {
            if ((!hilite && (flags & INVERSE) != 0) || (hilite && (flags & INVERSE) == 0)) {
                gc = ((fg&BOLD_BIT)) ?
                     screen->reverseboldGC : screen->reverseGC;
                XSetForeground (screen->display, gc, bg_pix);
                XSetBackground (screen->display, gc, fg_pix);
            }
            else {
                gc = ((fg&BOLD_BIT)) ?
                     screen->normalboldGC : screen->normalGC;
                XSetForeground (screen->display, gc, fg_pix);
                XSetBackground (screen->display, gc, bg_pix);
            }
        }

        x = CursorX(screen, col);
        lastind = col;

        for (; col <= maxcol; ++col) {
            if (attrs[col] != flags || fgs[col] != fg
             || (flags & BG_COLOR && ((bgs[col]^bg)&COLORMASK))) {
                if (col > lastind) {
#ifdef HANZI
                    if (flags&CHARHANZI) {
                        HZ_XDrawImString16 (screen->display, TextWindow (screen), gc,
                          FGCSinScreen(screen, fg), x, y, (XChar2b *)&chars[lastind],
                          (col-lastind)/2);
                    }
                    else
#endif /* HANZI */
                        XDrawImageString (screen->display, TextWindow (screen), gc,
                          x, y, (char *)&chars[lastind], col-lastind);
                    n = col-lastind;
#ifdef HANZI
                    if (flags&CHARHANZI) {
                        if ((fg&BOLD_BIT) && screen->hz_enbolden)
                            HZ_XDrawString16 (screen->display, TextWindow (screen), gc,
                              screen->hz_boldFGCS, x+1, y, (XChar2b *)&chars[lastind], n/2);
                    }
                    else
#endif /* HANZI */
                        if ((fg&BOLD_BIT) && screen->enbolden)
                            XDrawString (screen->display, TextWindow (screen), gc, x+1, y,
                              (char *) &chars[lastind], n);
                    if (flags&UNDERLINE)
                        XDrawLine (screen->display, TextWindow (screen), gc, x, y+1,
                          x+n*FontWidth(screen), y+1);
                }

                for (; col <= maxcol && (attrs[col]&(BLINK|CHARDRAWN)) != (BLINK|CHARDRAWN); ++col);
                if (col > maxcol) {
                    lastind = col;
                    break;
                }
                x += (col-lastind)*FontWidth (screen);
                lastind = col;
                flags = attrs[col];
                fg = fgs[col];
                bg = bgs[col];
                bg_pix = (flags&BG_COLOR) ? screen->colors[(bg&COLORMASK)+N_FGCOLOR]
                                          : term->core.background_pixel;
                if (screen->blink_state) {
                    if (! (screen->ext_flags&USEBOLD) && ! (flags&FG_COLOR) && (fg&BOLD_BIT)) {
                        fg_pix = screen->hilightcolor;
                    }
                    else {
                        fg_pix = (flags&FG_COLOR) ? screen->colors[fg&COLORMASK]
                                                  : screen->foreground;
                    }
                }
                else {
                    fg_pix = bg_pix;
                }
#ifdef HANZI
                if (flags & CHARHANZI) {
                    if ((! hilite && (flags&INVERSE) != 0) || (hilite && (flags&INVERSE) == 0)) {
                        gc = ((fg&BOLD_BIT)) ?
                             screen->hz_reverseboldGC : screen->hz_reverseGC;
                        XSetForeground (screen->display, gc, bg_pix);
                        XSetBackground (screen->display, gc, fg_pix);
                    }
                    else {
                        gc = ((fg&BOLD_BIT)) ?
                             screen->hz_normalboldGC : screen->hz_normalGC;
                        XSetForeground (screen->display, gc, fg_pix);
                        XSetBackground (screen->display, gc, bg_pix);
                    }
                }
                else
#endif /* HANZI */
                {
                    if ((!hilite && (flags&INVERSE) != 0) || (hilite && (flags&INVERSE) == 0)) {
                        gc = ((fg&BOLD_BIT)) ?
                             screen->reverseboldGC : screen->reverseGC;
                        XSetForeground (screen->display, gc, bg_pix);
                        XSetBackground (screen->display, gc, fg_pix);
                    }
                    else {
                        gc = ((fg&BOLD_BIT)) ?
                             screen->normalboldGC : screen->normalGC;
                        XSetForeground (screen->display, gc, fg_pix);
                        XSetBackground (screen->display, gc, bg_pix);
                    }
                }
            }
#ifdef HANZI
            if (Is1stByteHZ(flags)) {
                if(chars[col] == 0) {
                    chars[col] = chars[col+1] = ' ';
                    attrs[col] &= ~(CHARHANZI|FIRSTBYTE);
                    attrs[col+1] = attrs[col];
                    col -= 2;
                }
                ++col;
            }
            else
#endif /* HANZI */
            if(chars[col] == 0) {
                chars[col] = ' ';
            }
        }
        if (col > lastind) {
#ifdef HANZI
            if (flags&CHARHANZI) {
                HZ_XDrawImString16 (screen->display, TextWindow (screen), gc,
                  FGCSinScreen(screen, fg), x, y, (XChar2b *)&chars[lastind],
                  (col-lastind)/2);
            }
            else
#endif /* HANZI */
                XDrawImageString (screen->display, TextWindow (screen), gc,
                  x, y, (char *)&chars[lastind], col-lastind);
            n = col-lastind;
#ifdef HANZI
            if (flags&CHARHANZI) {
                if ((fg&BOLD_BIT) && screen->hz_enbolden)
                    HZ_XDrawString16 (screen->display, TextWindow (screen), gc,
                      screen->hz_boldFGCS, x+1, y, (XChar2b *)&chars[lastind], n/2);
            }
            else
#endif /* HANZI */
                if ((fg&BOLD_BIT) && screen->enbolden)
                    XDrawString (screen->display, TextWindow (screen), gc, x+1, y,
                      (char *) &chars[lastind], n);
            if (flags&UNDERLINE)
                XDrawLine (screen->display, TextWindow (screen), gc, x, y+1,
                  x+n*FontWidth(screen), y+1);
        }
	}
}

void ClearBufRows (register TScreen *screen, register int first, register int last)
/*
   Sets the rows first though last of the buffer of screen to spaces.
   Requires first <= last; first, last are rows of screen->buf.
 */
                         
                         
{
	first *= 4;
	last = 4 * last + 3;
	while (first <= last)
		bzero (screen->buf [first++], (screen->max_col + 1));
}

/*
  Resizes screen:
  1. If new window would have fractional characters, sets window size so as to
  discard fractional characters and returns -1.
  Minimum screen size is 1 X 1.
  Note that this causes another ExposeWindow event.
  2. Enlarges screen->buf if necessary.  New space is appended to the bottom
  and to the right
  3. Reduces  screen->buf if necessary.  Old space is removed from the bottom
  and from the right
  4. Cursor is positioned as closely to its former position as possible
  5. Sets screen->max_row and screen->max_col to reflect new size
  6. Maintains the inner border (and clears the border on the screen).
  7. Clears origin mode and sets scrolling region to be entire screen.
  8. Returns 0
  */
int ScreenResize (register TScreen *screen, int width, int height, unsigned long *flags)
{
	int rows, cols;
	int border = 2 * screen->border;
#ifdef HANZI
	int h_hzIwin = screen->hzIwin_height;
#endif /* HANZI */
	int move_down_by;
#if defined(sun) && !defined(SVR4)
#ifdef TIOCSSIZE
	struct ttysize ts;
#endif	/* TIOCSSIZE */
#else	/* sun */
#ifdef TIOCSWINSZ
	struct winsize ws;
#endif	/* TIOCSWINSZ */
#endif	/* sun */
	Window tw = TextWindow (screen);

	/* clear the right and bottom internal border because of NorthWest
	   gravity might have left junk on the right and bottom edges */
	XClearArea (screen->display, tw,
		    width - screen->border, 0,                /* right edge */
		    screen->border, height,           /* from top to bottom */
		    False);
	XClearArea (screen->display, tw, 
		    0, height - screen->border,	                  /* bottom */
		    width, screen->border,         /* all across the bottom */
		    False);
#ifdef HANZI
	/* clear the old HZ input area, for the same reason */
	XClearArea (screen->display, tw,
		    screen->border, height - screen->border - h_hzIwin,
		    width - 2 * screen->border, h_hzIwin,
		    False);
#endif /* HANZI */

	/* round so that it is unlikely the screen will change size on  */
	/* small mouse movements.					*/
#ifdef HANZI
	rows = (height + FontHeight(screen) / 2 - border - h_hzIwin) /
		 FontHeight(screen);
#else  /* HANZI */
	rows = (height + FontHeight(screen) / 2 - border) /
	 FontHeight(screen);
#endif /* HANZI */
	cols = (width + FontWidth(screen) / 2 - border - screen->scrollbar) /
	 FontWidth(screen);
	if (rows < 1) rows = 1;
	if (cols < 1) cols = 1;

	/* update buffers if the screen has changed size */
	if (screen->max_row != rows - 1 || screen->max_col != cols - 1) {
		register int savelines = screen->scrollWidget ?
		 screen->savelines : 0;
		int delta_rows = rows - (screen->max_row + 1);
		
		if(screen->cursor_state)
			HideCursor();
		if ( screen->alternate
		     && term->misc.resizeGravity == SouthWestGravity )
		    /* swap buffer pointers back to make all this hair work */
		    SwitchBufPtrs(screen);
		if (screen->altbuf) 
		    (void) Reallocate(&screen->altbuf, (Char **)&screen->abuf_address,
			 rows, cols, screen->max_row + 1, screen->max_col + 1);
		move_down_by = Reallocate(&screen->allbuf,
					  (Char **)&screen->sbuf_address,
					  rows + savelines, cols,
					  screen->max_row + 1 + savelines,
					  screen->max_col + 1);
		screen->buf = &screen->allbuf[4 * savelines];

		screen->max_row += delta_rows;
		screen->max_col = cols - 1;

		if (term->misc.resizeGravity == SouthWestGravity) {
		    screen->savedlines -= move_down_by;
		    if (screen->savedlines < 0)
			screen->savedlines = 0;
		    if (screen->savedlines > screen->savelines)
			screen->savedlines = screen->savelines;
		    if (screen->topline < -screen->savedlines)
			screen->topline = -screen->savedlines;
		    screen->cur_row += move_down_by;
		    screen->cursor_row += move_down_by;
		    ScrollSelection(screen, move_down_by);

		    if (screen->alternate)
			SwitchBufPtrs(screen); /* put the pointers back */
		}
	
		/* adjust scrolling region */
		screen->top_marg = 0;
		screen->bot_marg = screen->max_row;
		*flags &= ~ORIGIN;

		if (screen->cur_row > screen->max_row)
			screen->cur_row = screen->max_row;
		if (screen->cur_col > screen->max_col)
			screen->cur_col = screen->max_col;
	
#ifdef HANZI
		screen->fullVwin.height = height - border - h_hzIwin;
#else  /* HANZI */
		screen->fullVwin.height = height - border;
#endif /* HANZI */
		screen->fullVwin.width = width - border - screen->scrollbar;

	} else if(FullHeight(screen) == height && FullWidth(screen) == width)
	 	return(0);	/* nothing has changed at all */

	if(screen->scrollWidget)
		ResizeScrollBar(screen->scrollWidget, -1, -1, height);
	
	screen->fullVwin.fullheight = height;
	screen->fullVwin.fullwidth = width;
	ResizeSelection (screen, rows, cols);
#ifdef HANZI
	ResizeCXtermInput (screen);
#endif /* HANZI */
#if defined(sun) && !defined(SVR4)
#ifdef TIOCSSIZE
	/* Set tty's idea of window size */
	ts.ts_lines = rows;
	ts.ts_cols = cols;
	ioctl (screen->respond, TIOCSSIZE, &ts);
#ifdef SIGWINCH
	if(screen->pid > 1) {
		int	pgrp;
		
		if (ioctl (screen->respond, TIOCGPGRP, &pgrp) != -1)
			kill_process_group(pgrp, SIGWINCH);
	}
#endif	/* SIGWINCH */
#endif	/* TIOCSSIZE */
#else	/* sun */
#ifdef TIOCSWINSZ
	/* Set tty's idea of window size */
	ws.ws_row = rows;
	ws.ws_col = cols;
	ws.ws_xpixel = width;
	ws.ws_ypixel = height;
	ioctl (screen->respond, TIOCSWINSZ, (char *)&ws);
#ifdef notdef	/* change to SIGWINCH if this doesn't work for you */
	if(screen->pid > 1) {
		int	pgrp;
		
		if (ioctl (screen->respond, TIOCGPGRP, &pgrp) != -1)
		    kill_process_group(pgrp, SIGWINCH);
	}
#endif	/* SIGWINCH */
#endif	/* TIOCSWINSZ */
#endif	/* sun */
	return (0);
}

/*
 * Sets the attributes from the row, col, to row, col + length according to
 * mask and value. The bits in the attribute byte specified by the mask are
 * set to the corresponding bits in the value byte. If length would carry us
 * over the end of the line, it stops at the end of the line.
 */
void
ScrnSetAttributes(TScreen *screen, int row, int col, unsigned int mask, unsigned int value, register int length)
                
             
                     
                    		/* length of string */
{
	register Char *attrs;
	register int avail  = screen->max_col - col + 1;

	if (length > avail)
	    length = avail;
	if (length <= 0)
		return;
	attrs = screen->buf[4 * row + 1] + col;
	value &= mask;	/* make sure we only change the bits allowed by mask*/
	while(length-- > 0) {
		*attrs &= ~mask;	/* clear the bits */
		*attrs |= value;	/* copy in the new values */
		attrs++;
	}
}

/*
 * Gets the attributes from the row, col, to row, col + length into the
 * supplied array, which is assumed to be big enough.  If length would carry us
 * over the end of the line, it stops at the end of the line. Returns
 * the number of bytes of attributes (<= length)
 */
int
ScrnGetAttributes(TScreen *screen, int row, int col, Char *str, register int length)
                
             
          
                    		/* length of string */
{
	register Char *attrs;
	register int avail  = screen->max_col - col + 1;
	int ret;

	if (length > avail)
	    length = avail;
	if (length <= 0)
		return 0;
	ret = length;
	attrs = screen->buf[4 * row + 1] + col;
	while(length-- > 0) {
		*str++ = *attrs++;
	}
	return ret;
}

Bool
non_blank_line(ScrnBuf sb, register int row, register int col, register int len)
{
	register int	i;
	register Char *ptr = sb [4 * row];

	for (i = col; i < len; i++)	{
		if (ptr[i])
			return True;
	}
	return False;
}
#ifdef HANZI

/*
 * HZ_XDrawImString16(), HZ_XDrawString16()
 *
 * Check the GL/GR font graphical character set, and do some adjustment
 * before calling XDrawImageString16() or XDrawString16().
 */
static XChar2b hzstrbuf[512];	/* 512 hanzi, big enough! */

int HZ_XDrawImString16(Display *display, Drawable d, GC gc, int fgcs, int x, int y, XChar2b *string, int length)
{
  int i;

    if (! length) return (0);
    if (fgcs != FGCS_GL)	/* for GR or mixed font */
	return XDrawImageString16(display, d, gc, x, y, string, length);

    /* GL font */
    for (i = 0; i < length; i++) {
	hzstrbuf[i].byte1 = string[i].byte1 & 0x7f ;
	hzstrbuf[i].byte2 = string[i].byte2 & 0x7f ;
    }
    return XDrawImageString16(display, d, gc, x, y, hzstrbuf, length);
}

int HZ_XDrawString16(Display *display, Drawable d, GC gc, int fgcs, int x, int y, XChar2b *string, int length)
{
  int i;

    if (! length) return (0);
    if (fgcs != FGCS_GL)	/* for GR or mixed font */
	return XDrawString16(display, d, gc, x, y, string, length);

    /* GL font */
    for (i = 0; i < length; i++) {
	hzstrbuf[i].byte1 = string[i].byte1 & 0x7f ;
	hzstrbuf[i].byte2 = string[i].byte2 & 0x7f ;
    }
    return XDrawString16(display, d, gc, x, y, hzstrbuf, length);
}
#endif /* HANZI */
