/* $Id: data.c,v 1.1.1.1 2001/11/13 18:12:09 xucs Exp $ */
/***********************************************************************
* Copyright 1994 by Yongguang Zhang.
* Copyright 1990, 1991 by Yongguang Zhang and Pong Man-Chi.
*
* All rights reserved.  Under the same copyright and permission term as
* the original.  Absolutely no warranties of any kinds.
************************************************************************/

/*
 *	$XConsortium: data.c,v 1.11 93/02/25 17:21:27 gildea Exp $
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

#include "ptyx.h"		/* gets Xt stuff, too */
#include "data.h"
#include <setjmp.h>

XPoint T_boxlarge[NBOX] = {
	{0, 0},
	{8, 0},
	{0, 14},
	{-8, 0},
	{0, -14},
};
XPoint T_box2[NBOX] = {
	{0, 0},
	{7, 0},
	{0, 12},
	{-7, 0},
	{0, -12},
};
XPoint T_box3[NBOX] = {
	{0, 0},
	{5, 0},
	{0, 12},
	{-5, 0},
	{0, -12},
};
XPoint T_boxsmall[NBOX] = {
	{0, 0},
	{5, 0},
	{0, 9},
	{-5, 0},
	{0, -9},
};
jmp_buf Tekend;
int Tbcnt = 0;
Char *Tbuffer;
Char *Tbptr;
TekLink *TekRefresh;
Char *Tpushb;
Char *Tpushback;
int Ttoggled = 0;
int bcnt = 0;
Char __ss_buffer[BUF_SIZE+2];
Char *buffer = __ss_buffer+2;
Char *bptr = __ss_buffer+2;
jmp_buf VTend;
XPoint VTbox[NBOX] = {
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
};
#ifdef HANZI
XPoint VThz_box[NBOX] = {
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
};
#endif /* HANZI */

#ifdef DEBUG
int debug = 0; 		/* true causes error messages to be displayed */
#endif	/* DEBUG */
XtermWidget term;		/* master data structure for client */
char *xterm_name;	/* argv[0] */
int am_slave = 0;	/* set to 1 if running as a slave process */
int max_plus1;

/* These lines are modified by Zhuang Hao S.S.* */
fd_set pty_mask;
fd_set Select_mask;
fd_set X_mask;
int g__pty_fd, g__X_fd;

char *ptydev;
char *ttydev;
#ifdef ALLOWLOGGING
# ifdef HANZI
char log_def_name[] = "CXtermLog.XXXXX";
# else  /* HANZI */
char log_def_name[] = "XtermLog.XXXXX";
# endif /* HANZI */
#endif
int T_lastx = -1;
int T_lasty = -1;

int waitingForTrackInfo = 0;
EventMode eventMode = NORMAL;

GC visualBellGC;

int VTgcFontMask = GCFont;
int TEKgcFontMask = GCFont;

TekWidget tekWidget;