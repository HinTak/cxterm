/*
 *	$Id: HZbuiltn.c,v 1.2 2003/05/06 07:40:19 htl10 Exp $
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

/*
 * This file implements various built-in input filters,
 * 
 * The Function Template for input filters is:
 *
 *	int hzXXXfilter (cxin, ch, strbuf)
 *	    CXtermInputModule *cxin;
 *	    char ch;
 *	    char strbuf[];
 */


#include "HZinput.h"
#include <ctype.h>

#define hex2dec(ch)			\
	(isdigit(ch) ? (ch)-'0' :	\
		(((ch)>='a') && ((ch)<='f')) ? (ch)-'a'+10 : (ch)-'A'+10)

/*ARGSUSED*/
static int hzASCIIfilter (CXtermInputModule *cxin, char ch, char *strbuf)
{
    strbuf[0] = ch;
    return(1);
}

/* IC: Internal Code */
static int hzICfilter (CXtermInputModule *cxin, char ch, char *strbuf)
{
    HZInputArea *ia = &cxin->hzia;

    /*
     * Should call HZiaSet(), but, just want some extra performance ...
     */
    if (isxdigit (ch)) {
	if (cxin->hzinbufCount == 3) {
	    strbuf[0] = hex2dec(cxin->hzinbuf[0]) * 16
			+ hex2dec(cxin->hzinbuf[1]);
	    strbuf[1] = hex2dec(cxin->hzinbuf[2]) * 16 + hex2dec(ch);
	    if (! (strbuf[0] & 0x80)) {		/* first byte not set */
		strbuf[0] |= 0x80;		/* turn on both bytes */
		strbuf[1] |= 0x80;
	    }
	    cxin->hzinbufCount = 0;

	    memmove(ia->buffer.text[0] + ia->lenPrompt, "   ", 3);
	    bzero(ia->buffer.dpy_attr[0] + ia->lenPrompt, 3);
	    ia->lenInbuf = 0;
	    HZiaFlush (ia, cxin->screen);

	    return (2);	/* 2 bytes converted in strbuf[] */
	} else {
	    cxin->hzinbuf[ cxin->hzinbufCount++ ] = ch;

	    ia->buffer.text[0][ia->lenPrompt + ia->lenInbuf ] = ch;
	    ia->buffer.dpy_attr[0][ia->lenPrompt + ia->lenInbuf ] = CHARDRAWN;
	    ia->lenInbuf++;
	    HZiaFlush (ia, cxin->screen);

	    return (0);
	}
    }

    switch (ch) {
	case '\010':	/* \b */
	case '\177':	/* DEL */
	    if (cxin->hzinbufCount == 0)
		return (1);	/* nothing to delete, pass the key out */
	    cxin->hzinbufCount-- ;
	    ia->lenInbuf-- ;
	    ia->buffer.text[0][ia->lenPrompt + ia->lenInbuf ] = ' ';
	    ia->buffer.dpy_attr[0][ia->lenPrompt + ia->lenInbuf ] = 0;
	    HZiaFlush (ia, cxin->screen);
	    return (0);

	case '\015':	/* \r */
	case '\025':	/* ^U */
	    if (cxin->hzinbufCount == 0)
		return (1);	/* nothing to kill, pass the key out */
	    cxin->hzinbufCount = 0;
	    ia->lenInbuf = 0;
	    memmove(ia->buffer.text[0] + ia->lenPrompt, "   ", 3);
	    bzero(ia->buffer.dpy_attr[0] + ia->lenPrompt, 3);
	    HZiaFlush (ia, cxin->screen);
	    return (0);

	default:
	    return (1);
    }
}

/* GB: QW */
static int hzQWfilter (CXtermInputModule *cxin, char ch, char *strbuf)
{
    HZInputArea *ia = &cxin->hzia;

    /*
     * Should call HZiaSet(), but, just want some extra performance ...
     */
    if (isdigit (ch)) {
	if (cxin->hzinbufCount == 3) {
	    strbuf[0] = (cxin->hzinbuf[0] - '0') * 10
			+ (cxin->hzinbuf[1] - '0') + 0xa0;
	    strbuf[1] = (cxin->hzinbuf[2] - '0') * 10 + (ch - '0') + 0xa0;
	    cxin->hzinbufCount = 0;

	    ia->lenInbuf = 0;
	    memmove(ia->buffer.text[0] + ia->lenPrompt, "   ", 3);
	    bzero(ia->buffer.dpy_attr[0] + ia->lenPrompt, 3);
	    HZiaFlush (ia, cxin->screen);

	    return (2);	/* 2 bytes converted in strbuf[] */
	} else {
	    cxin->hzinbuf[ cxin->hzinbufCount++ ] = ch;

	    ia->buffer.text[0][ia->lenPrompt + ia->lenInbuf ] = ch;
	    ia->buffer.dpy_attr[0][ia->lenPrompt + ia->lenInbuf ] = CHARDRAWN;
	    ia->lenInbuf++;
	    HZiaFlush (ia, cxin->screen);

	    return (0);
	}
    }

    switch (ch) {
	case '\010':	/* \b */
	case '\177':	/* DEL */
	    if (cxin->hzinbufCount == 0)
		return (1);	/* nothing to delete, pass the key out */
	    cxin->hzinbufCount-- ;
	    ia->lenInbuf-- ;
	    ia->buffer.text[0][ia->lenPrompt + ia->lenInbuf ] = ' ';
	    ia->buffer.dpy_attr[0][ia->lenPrompt + ia->lenInbuf ] = 0;
	    HZiaFlush (ia, cxin->screen);
	    return (0);

	case '\015':	/* \r */
	case '\025':	/* ^U */
	    if (cxin->hzinbufCount == 0)
		return (1);	/* nothing to kill, pass the key out */
	    cxin->hzinbufCount = 0;
	    ia->lenInbuf = 0;
	    memmove(ia->buffer.text[0] + ia->lenPrompt, "   ", 3);
	    bzero(ia->buffer.dpy_attr[0] + ia->lenPrompt, 3);
	    HZiaFlush (ia, cxin->screen);
	    return (0);

	default:
	    return (1);
    }
}

/*****  parameters for builtin input methods  *****/

struct pdHZinput {
    char *name;				/* name of input method */
    char *prompt;			/* prompt of the input method */
    int (*filter)(FP_hzFilter);		/* HZ filter routine */
};

/* GB predefined */
static struct pdHZinput GBpredefine[] = {
  {
  /* 0: The first one must be ASCII -- ISO 8859-1 */
	"ASCII",
	"\323\242\316\304\312\344\310\353 (ASCII input)",
	    /* yin wen shu ru (English Input) */
	(int (*)(FP_hzFilter))hzASCIIfilter,
  },
  {
  /* 1: IC -- Internal Code, encoding independent */
	"IC",
	"\272\272\327\326\312\344\310\353\241\313\304\332\302\353\241\313 ",
	    /* han zi shu ru (Chinese Input) :: nei ma (internal code) :: */
	(int (*)(FP_hzFilter))hzICfilter,
  },
  {
  /* 2: QW -- Position, for GB coding */
	"QW",
	"\272\272\327\326\312\344\310\353\241\313\307\370\316\273\241\313 ",
	    /* han zi shu ru (Chinese Input) :: qu wei (position) :: */
	(int (*)(FP_hzFilter))hzQWfilter,
  },
  {
  /* last: null-pad */
	NULL, NULL, (int (*)(FP_hzFilter))hzTableFilter,
  },
};

/* BIG5 predefined */
static struct pdHZinput BIG5predefine[] = {
  {
  /* 0: The first one must be ASCII -- ISO 8859-1 */
	"ASCII",
	"\255\136\244\345\277\351\244\112 (ASCII input)",
	    /* yin wen shu ru (English Input) */
	(int (*)(FP_hzFilter))hzASCIIfilter,
  },
  {
  /* 1: IC -- Internal Code, encoding independent */
	"IC",
	"\272\176\246\162\277\351\244\112\241\107\244\272\275\130\241\107 ",
	    /* han zi shu ru (Chinese Input) : nei ma (internal code) : */
	(int (*)(FP_hzFilter))hzICfilter,
  },
  {
  /* last: null-pad */
	NULL, NULL, (int (*)(FP_hzFilter))hzTableFilter,
  },
};

/* JIS predefined */
static struct pdHZinput JISpredefine[] = {
  {
  /* 0: The first one must be ASCII -- ISO 8859-1 */
	"ASCII",
	"\261\321\270\354 (ASCII input)",
	    /* English (ASCII input) */
	(int (*)(FP_hzFilter))hzASCIIfilter,
  },
  {
  /* 1: IC -- Internal Code, encoding independent */
	"IC",
	"\306\374\313\334\270\354\241\332hex code\241\333 ",
	    /* Nihongo [hex code] */
	(int (*)(FP_hzFilter))hzICfilter,
  },
  {
  /* last: null-pad */
	NULL, NULL, (int (*)(FP_hzFilter))hzTableFilter,
  },
};

/* KS predefined */
static struct pdHZinput KSpredefine[] = {
  {
  /* 0: The first one must be ASCII -- ISO 8859-1 */
	"ASCII",
	"\241\274 ASCII input \241\275",
	    /* [ English Input ] */
	(int (*)(FP_hzFilter))hzASCIIfilter,
  },
  {
  /* 1: IC -- Internal Code, encoding independent */
	"IC",
	"\307\321\261\333\300\324\267\302\241\274hex code\241\275 ",
	    /* Hangul Input [ hex code ] */
	(int (*)(FP_hzFilter))hzICfilter,
  },
  {
  /* last: null-pad */
	NULL, NULL, (int (*)(FP_hzFilter))hzTableFilter,
  },
};

static keyPrompt builtin_keyprompt[128];
static unsigned short builtin_keytype[128];

/*
 * HZLoadBuiltin -- load the built-in HZ input tables
 */
void HZLoadBuiltin(CXtermInputModule *cxin)
{
    struct pdHZinput *p;
    int i;

    switch (cxin->encode) {
      case GB_ENCODE:
	p = GBpredefine;
	break;
      case BIG5_ENCODE:
	p = BIG5predefine;
	break;
      case JIS_ENCODE:
	p = JISpredefine;
	break;
      case KS_ENCODE:
	p = KSpredefine;
	break;
      default:
	p = GBpredefine;	/* just use the default case! */
    }

    for (i = 0; i < 128; i++) {
	builtin_keyprompt[i].prompt[0] = (unsigned char)i;
	builtin_keyprompt[i].ptlen = 1;
	builtin_keytype[i] = HZ_KEY_INPUT_MASK;		/* useless anyway */
    }

    while (p->name) {
	cxin->imtbl[ cxin->numHZim ].type = im_type_Builtin;
	cxin->imtbl[ cxin->numHZim ].name = p->name;
	cxin->imtbl[ cxin->numHZim ].hzif = p->filter;

	cxin->imtbl[ cxin->numHZim ].f.prompt = (Char *)(p->prompt);
	cxin->imtbl[ cxin->numHZim ].f.lenPrompt = strlen(p->prompt);
	cxin->imtbl[ cxin->numHZim ].f.keyprompt = builtin_keyprompt;
	cxin->imtbl[ cxin->numHZim ].f.choicelb = NULL;
	cxin->imtbl[ cxin->numHZim ].f.maxchoice = 0;

	cxin->imtbl[ cxin->numHZim ].k.keytype = builtin_keytype;
	cxin->imtbl[ cxin->numHZim ].k.def_assockey = '\0';

	cxin->imtbl[ cxin->numHZim ].m.auto_select = HZ_AUTOSELECT_ALWAYS;
	cxin->imtbl[ cxin->numHZim ].m.do_auto_segment = False;
	cxin->imtbl[ cxin->numHZim ].m.do_ps_assoc = False;

	cxin->numHZim++;
	p++;
    }
}
