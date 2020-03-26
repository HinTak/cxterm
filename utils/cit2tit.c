/*
 *	$Id: cit2tit.c,v 1.3 2003/05/06 07:40:20 htl10 Exp $
 */

/***********************************************************************
Copyright 1992,1994 by Yongguang Zhang.  All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of the authors not
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

THE AUTHOR(S) DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
THE AUTHOR(S) BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

***********************************************************************/


/* cit2tit.c */

#include <X11/Xos.h>	/* OS dependent stuff */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __CYGWIN32__
#define FOPEN_R		"rb"
#else
#define FOPEN_R		"r"
#endif

extern char *HZencodeName(int);	/* from HZutil.c */

#include "HZtable.h"

trieNode *trieList;		/* all Tries are saved here */
XChar2b *hzList;		/* all HZ codes will be put here */

HZinputTable hzInputTable;

char *progname, *ifname;

#define MAXCLEN 10240		/* 10K comment, large enough? */
static char comment[MAXCLEN];
static unsigned int clen;

static void ReadInput (FILE *ifile);
static void Output (FILE *ofile);
static void LineOut (int top, unsigned int trieIdx, FILE *ofile);
static void KeyOut (short unsigned int type, char *label, FILE *ofile);
static int Putc (unsigned char ch, FILE *ofile);

void main(int argc, char **argv)
{
    FILE *ifile, *ofile;

    progname = argv[0];
    if (argc == 1) {
	ifname = "(stdin)";
	ifile = stdin;
	ofile = stdout;
    } else {
	ifname = argv[1];
	ifile = fopen (argv[1], FOPEN_R);
	if (! ifile) {
	    perror (argv[1]);
	    exit (1);
	}
	ofile = stdout;
    }

    ReadInput (ifile);
    if (ifile != stdin)
	fclose (ifile);

    Output (ofile);
    exit (0);
}

/*
 * ReadInput -- read .cit format input file
 */
static void ReadInput (FILE *ifile)
{
    char magic[2];

    (void) fread (magic, 2, 1, ifile);		/* read the magic number */
    if (strncmp (magic, MAGIC_CIT, 2) != 0) {
	fprintf (stderr, "Input is not in .cit format\n");
	exit (1);
    }
    if (fread (&hzInputTable, sizeof(HZinputTable), 1, ifile) == 0) {
	fprintf (stderr, "Incorrect .cit input\n");
	exit (1);
    }
    trieList = (trieNode *) calloc (hzInputTable.sizeTrieList,
				    sizeof(trieNode));
    hzList = (XChar2b *) calloc (hzInputTable.sizeHZList, sizeof(XChar2b));
    if ((! trieList) || (! hzList)) {
	perror (progname);
	exit (1);
    }
    if ((fread (trieList, sizeof(trieNode), hzInputTable.sizeTrieList, ifile)
	    != hzInputTable.sizeTrieList) ||
	(fread (hzList, sizeof(XChar2b), hzInputTable.sizeHZList, ifile)
	    != hzInputTable.sizeHZList))
    {
	fprintf (stderr, "Incorrect .cit input\n");
	exit (1);
    }
    if ((fread ((char *)(&clen), sizeof (unsigned int), 1, ifile) != 1) ||
	(fread (comment, 1, clen, ifile) != clen))
    {
	clen = 0;
    }
    if (clen > MAXCLEN)
	clen = MAXCLEN;
}

#define	MAXLW	50

static unsigned char keystack[80];

static void Output (FILE *ofile)
{
    int i;
    int pcomment;

    /* header */

    fprintf (ofile, "# HANZI input table for cxterm\n");
    fprintf (ofile, "# Generated from %s by %s\n", ifname, progname);
    fprintf (ofile, "# To be used by cxterm, convert me to %s format first\n",
		CIT_SUFFIX);
    fprintf (ofile, "# %s version %d\n", CIT_SUFFIX, hzInputTable.version);

    fprintf (ofile, "ENCODE:\t%s\n", HZencodeName(hzInputTable.encode));

    fprintf (ofile, "PROMPT:\t");
    for (i = 0; i < hzInputTable.lenPrompt; i++) {
	(void) Putc (hzInputTable.prompt[i], ofile);
    }
    fprintf (ofile, "\n");

    /* parameters */

    switch (hzInputTable.autoSelect) {
      case HZ_AUTOSELECT_ALWAYS:
	fprintf (ofile, "AUTOSELECT:\tALWAYS\n");
	break;
      case HZ_AUTOSELECT_WHENNOMATCH:
	fprintf (ofile, "AUTOSELECT:\tWHENNOMATCH\n");
	break;
      default:
	fprintf (ofile, "AUTOSELECT:\tNEVER\n");
	break;
    }

    /* comments */

    fprintf (ofile, "#\n");
    pcomment = 1;
    for (i = 0; i < clen; i++) {
	if (pcomment)
	    fprintf (ofile, "COMMENT");
	(void) putc (comment[i], ofile);
	pcomment = (comment[i] == '\n');
    }

    /* key definitions */

    fprintf (ofile, "# input key definitions");	/* don't add \n */

    KeyOut (HZ_KEY_INPUT_MASK, "VALIDINPUTKEY:", ofile);

    KeyOut (HZ_KEY_WILDCARD, "WILDCARDKEY:", ofile);
    KeyOut (HZ_KEY_WILDCHAR, "WILDCHARKEY:", ofile);
    KeyOut (HZ_KEY_ASSOCIATION, "ASSOCIATIONKEY:", ofile);

    fprintf (ofile, "\n# choice list keys");

    KeyOut (HZ_KEY_SELECTION_MASK, "SELECTKEY:", ofile);
    KeyOut (HZ_KEY_RIGHT, "MOVERIGHT:", ofile);
    KeyOut (HZ_KEY_LEFT, "MOVELEFT:", ofile);

    fprintf (ofile, "\n# pre-editing keys");

    KeyOut (HZ_KEY_EDIT_BACKSPACE, "BACKSPACE:", ofile);
    KeyOut (HZ_KEY_EDIT_KILL, "DELETEALL:", ofile);
    KeyOut (HZ_KEY_EDIT_BEGIN, "CURSOR-BEGIN:", ofile);
    KeyOut (HZ_KEY_EDIT_END, "CURSOR-END:", ofile);
    KeyOut (HZ_KEY_EDIT_FORW, "CURSOR-FORW:", ofile);
    KeyOut (HZ_KEY_EDIT_BACK, "CURSOR-BACK:", ofile);
    KeyOut (HZ_KEY_EDIT_ERASE, "CURSOR-ERASE:", ofile);
    KeyOut (HZ_KEY_REPEAT, "REPEATKEY:", ofile);

    /* key prompt */

    for (i = 0; i < 128; i++) {
	int j;

	if (! (hzInputTable.keytype[i] & HZ_KEY_INPUT_MASK))
	    continue;
	if ((hzInputTable.keyprompt[i].ptlen == 1) &&
	    (hzInputTable.keyprompt[i].prompt[0] == i))
		continue;

	fprintf (ofile, "\nKEYPROMPT(");
	(void) Putc (i, ofile);
	fprintf (ofile, "):\t");
	for (j = 0; j < hzInputTable.keyprompt[i].ptlen; j++) {
	    (void) Putc (hzInputTable.keyprompt[i].prompt[j], ofile);
	}
    }
    fprintf (ofile, "\n");

    /* dictionaries */

    fprintf (ofile, "# the following line must not be removed\n");
    fprintf (ofile, "BEGINDICTIONARY\n");
    fprintf (ofile, "#\n");

    LineOut (0, 0, ofile);
}

/* output line by line while traveling the Trie */
static void LineOut (int top, unsigned int trieIdx, FILE *ofile)
{
    register trieNode *tnptr = &trieList[trieIdx];
    register int i;
    int num;		/* number of choices under the key sequence in stack */

    keystack[top] = tnptr->tn_key;

    if (tnptr->tn_numNextKeys) {	/* there can be more input keys */
	int end_hzidx = trieList[tnptr->tn_nextKeys].tn_hzidx;
	num = 0;
	for ( i = tnptr->tn_hzidx; i < end_hzidx; i++, num++ )
	    if (hzList[i].byte1 == HZ_PHRASE_TAG)
		i += hzList[i].byte2;
    } else
	num = tnptr->tn_numHZchoice;

    if ((num > 0) || (tnptr->tn_numNextKeys == 0)) {
	/* there are HZs exclusively under the keys, or no more input keys */

	int cnt = 0;
	XChar2b *phz = &hzList[tnptr->tn_hzidx];

	for (i = 1; i <= top; i++) {
	    (void) Putc (keystack[i], ofile);
	}
	(void) putc ('\t', ofile);
	for (i = 0; i < num; i++) {
	    if (cnt >= MAXLW) {
		register int j;

		(void) putc ('\n', ofile);
		for (j = 1; j <= top; j++) {
		    (void) Putc (keystack[j], ofile);
		}
		(void) putc ('\t', ofile);
		cnt = 0;
	    }
	    if (phz->byte1 != HZ_PHRASE_TAG) {
		/* hanzi */
		(void) putc (phz->byte1, ofile);
		(void) putc ((phz++)->byte2, ofile);
		cnt += 2;
	    } else {
		/* phrase */
                register int j, hzlen = (phz++)->byte2;

		(void) putc ('(', ofile);
                for (j = 0; j < hzlen; j++) {
		    (void) putc (phz->byte1, ofile);
		    (void) putc ((phz++)->byte2, ofile);
                }
		(void) putc (')', ofile);
		cnt += hzlen * 2 + 2;
	    }
	}
	(void) putc ('\n', ofile);
    }

    for (i = 0; i < tnptr->tn_numNextKeys; i++)
	LineOut (top + 1, tnptr->tn_nextKeys + i, ofile);
}

static void KeyOut (short unsigned int type, char *label, FILE *ofile)
{
  int cnt = 0, total = 0;
  int i, n;

    switch (type) {
      case HZ_KEY_INPUT_MASK:

	fprintf (ofile, "\n%s\t", label);
	for (i = 0; i < 128; i++) {
	    if ((hzInputTable.keytype[i] & HZ_KEY_INPUT_MASK) &&
		((hzInputTable.keytype[i] == HZ_KEY_INPUT_MASK) ||
		 (hzInputTable.keytype[i] & HZ_KEY_SELECTION_MASK)))
	    {
		if (cnt >= MAXLW) {
		    fprintf (ofile, "\n%s\t", label);
		    cnt = 0;
		}
		cnt += Putc (i, ofile);
	    }
	}
	return ;

      case HZ_KEY_SELECTION_MASK:

	for (n = 0; n < hzInputTable.maxchoice; n++) {
	    fprintf (ofile, "\n%s\t", label);
	    (void) Putc (hzInputTable.choicelb[n], ofile);
	    for (i = 0; i < 128; i++) {
		if ((hzInputTable.keytype[i] & HZ_KEY_SELECTION_MASK) &&
		    ((hzInputTable.keytype[i] & HZ_KEY_SELECTION_NUM) == n) &&
		    (i != (unsigned char)(hzInputTable.choicelb[n])))
			(void) Putc (i, ofile);
	    }
	}
	return ;

      case HZ_KEY_ASSOCIATION:

	fprintf (ofile, "\n%s\t", label);
	if (hzInputTable.defAssocKey) {
	    cnt += Putc (hzInputTable.defAssocKey, ofile);
	}
	for (i = 0; i < 128; i++) {
	    if ((hzInputTable.keytype[i] == type) &&
		(i != hzInputTable.defAssocKey))
	    {
		if (cnt >= MAXLW) {
		    fprintf (ofile, "\n%s\t", label);
		    cnt = 0;
		}
		cnt += Putc (i, ofile);
	    }
	}
	return ;
 
      default:

	fprintf (ofile, "\n%s\t", label);
	for (i = 0; i < 128; i++) {
	    if (hzInputTable.keytype[i] == type) {
		if (cnt >= MAXLW) {
		    fprintf (ofile, "\n%s\t", label);
		    cnt = 0;
		}
		cnt += Putc (i, ofile);
		total ++;
	    }
	}
	if (! total)
	    fprintf (ofile, "\\000\t\t# empty field");
    }
}
	    
static int Putc (unsigned char ch, FILE *ofile)
{
    if ((ch & 0x80) || (isprint (ch) && (! isspace (ch)))) {
	/* special cases */
	switch (ch) {
	    case '#'  :
	    case '\\' :
		fprintf (ofile, "\\%03o", ch);
		break;
	    default   :
		(void) putc (ch, ofile);
	}
	return (1);
    } else {
	fprintf (ofile, "\\%03o", ch);
	return (4);
    }
}
