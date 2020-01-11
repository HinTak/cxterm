/*
 *	$Id: HZassoc.c,v 1.2 2003/05/06 07:40:19 htl10 Exp $
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
 * Association searching routines
 */

#include "HZinput.h"

#ifdef __CYGWIN32__
#define FOPEN_R		"rb"
#else
#define FOPEN_R		"r"
#endif

static void MakeSortList(HZassocContext *ac, HZassocContext *pac);
static Boolean AllMatches(HZInputContext **subICs, XChar2b *hzc, char *sortlist, int sortlistlen);


HZInputContext *Assoc_AddInput (HZInputContext *ic, char key)
                       
#if NeedWidePrototypes
             
#else
             
#endif
{
  HZInputContext *new_ic;

    if (! IC_isAssoc(ic)) {	/* converted from some other type */

	if (! ic->al)		/* no associate input! */
	    return(NULL);

	new_ic = NewIC (ic_type_Assoc, ic->im, ic->al,
			ic->keysq, 0, ic->matched + ic->pending);
	if (!new_ic)
	    return(NULL);

	if (ic->im->k.keytype[(int)key] == HZ_KEY_ASSOCIATION) {
	    new_ic->aAsCntx->numSeg = 2;
	    new_ic->aAsCntx->subICs[0] = DupIC(ic);
	    new_ic->aAsCntx->subICs[1] = NewIC(ic_type_Trie, ic->im, ic->al,
			ic->keysq + ic->matched + ic->pending + 1, 0, 0);
	} else {
	    new_ic->aAsCntx->numSeg = 1;
	    new_ic->aAsCntx->subICs[0] = HZaddInputIC(
			ic->aAsCntx->subICs[ ic->aAsCntx->numSeg - 1 ], key);
	}
	new_ic->aAsCntx->prev_ic = NULL;

    } else {

	new_ic = NewIC (ic_type_Assoc, ic->im, ic->al,
			ic->keysq, ic->matched, ic->pending);
	if (! new_ic)
	    return(NULL);
	new_ic->aAsCntx->prev_ic = ic;

	if (ic->im->k.keytype[(int)key] == HZ_KEY_ASSOCIATION) {
	    int i;

	    if (ic->aAsCntx->numSeg >= MAX_ASSOC)
		return (NULL);		/* too many segments */

	    for (i = 0; i < ic->aAsCntx->numSeg; i++)
		new_ic->aAsCntx->subICs[i] = DupIC(ic->aAsCntx->subICs[i]);

	    new_ic->aAsCntx->subICs[i] = NewIC (ic_type_Trie, ic->im, ic->al,
			ic->keysq + ic->matched + ic->pending + 1, 0, 0);
	    new_ic->aAsCntx->numSeg = ic->aAsCntx->numSeg + 1;
	} else {
	    HZInputContext *nic;
	    int i;

	    nic = HZaddInputIC( ic->aAsCntx->subICs[ ic->aAsCntx->numSeg - 1 ],
				key );
	    if (! nic)
		return (NULL);		/* error */

	    for (i = 0; i < ic->aAsCntx->numSeg - 1; i++)
		new_ic->aAsCntx->subICs[i] = DupIC(ic->aAsCntx->subICs[i]);

	    new_ic->aAsCntx->subICs[i] = nic;
	    new_ic->aAsCntx->numSeg = ic->aAsCntx->numSeg;
	}
    }
    new_ic->pending++;
    return(new_ic);
}

int Assoc_Match (HZInputContext *ic)
{
  int i;
  XChar2b *hzc;

    if (ic->pending == 0)
	return (IC_OK);

    ic->aPtrThisCh = NULL;

    /* start from the choice list from previous context */
    /* if the previous context is not matched, try previous of previous */
    ic->aPtrIC = ic->aAsCntx->prev_ic;
    while (ic->aPtrIC && (ic->aPtrIC->pending > 0))
	ic->aPtrIC = ic->aPtrIC->aAsCntx->prev_ic;

    if (ic->aPtrIC) {
	ic->aPtrPrevCh = ic->aPtrIC->aAsCntx->ChList.choices;
	ic->aPtrPhrases = ic->aPtrIC->aPtrPhrases;
	ic->derivability = ic->aPtrIC->derivability;
    } else {
	ic->aPtrPrevCh = NULL;
	ic->aPtrPhrases = ic->al->phrases;
	ic->derivability = IC_UNCERTAIN;
    }

    if (ic->pending == 0) {		/* nothing need to be done */
	ic->aAsCntx->sortlistlen = 0;
	return (IC_OK);
    }

    /* match and restart sub context */
    ic->derivability = IC_UNCERTAIN;
    for (i = 0; i < ic->aAsCntx->numSeg; i++) {
	if (HZmatchIC(ic->aAsCntx->subICs[i]) != IC_OK)
	    return (IC_FAIL);
	if (HZrestartIC(ic->aAsCntx->subICs[i]) == IC_EMPTYSET) {
	    /* some context won't generate any hz at all */
	    ic->derivability = IC_EMPTYSET;
	}
    }

    MakeSortList(ic->aAsCntx, ic->aPtrIC ? ic->aPtrIC->aAsCntx : NULL);

    ic->matched += ic->pending;
    ic->pending = 0;

    /* association input is different, "match" means at least one candidate */
    if (Assoc_GetNext (ic, &hzc) < 0)
	return (IC_FAIL);
    Assoc_Restart (ic);

    return (IC_OK);
}

int Assoc_Restart (HZInputContext *ic)
{
  int i;

    ic->aPtrThisCh = ic->aAsCntx->ChList.choices;

    /* restart all the sub contexts */
    for (i = 0; i < ic->aAsCntx->numSeg; i++)
	(void) HZrestartIC(ic->aAsCntx->subICs[i]);

    return (ic->derivability);
}

short int Assoc_GetNext (HZInputContext *ic, XChar2b **phzc)
{
  HZassocContext *ac = ic->aAsCntx;
  Char *ptr;

    /* first, the current choice list */
    if (ic->aPtrThisCh) {
	if (ic->aPtrThisCh < ac->ChList.choices + ac->ChList.numChoices) {
	    short int nhz = ic->aPtrThisCh->nhz;

	    *phzc = ic->aPtrThisCh->hzc;
	    ic->aPtrThisCh ++;
	    return (nhz);
	}
	ic->aPtrThisCh = NULL;
    }

    /* second: recheck previous choice lists */
    while (ic->aPtrIC) {
	HZassocContext *prev_ac = ic->aPtrIC->aAsCntx;
	struct hz_choice *pch_end = prev_ac->ChList.choices +
				    prev_ac->ChList.numChoices;
	struct hz_choice *pch;

	for (pch = ic->aPtrPrevCh; pch < pch_end; pch++) {
	    if (ac->numSeg <= pch->nhz &&
		AllMatches(ac->subICs, pch->hzc, ac->sortlist, ac->sortlistlen))
	    {
		/* found one, save the pointers and return */
		ic->aPtrPrevCh = pch+1;
		*phzc = pch->hzc;
		(void) HZclAddChoice(&(ic->aAsCntx->ChList), *phzc, pch->nhz);
		return (pch->nhz);
	    }
	}

	/* Not in the latest choice-list.  Install the leftover point
	 * from previous search and start from there
	 */
	ic->aPtrPrevCh = ic->aPtrIC->aPtrPrevCh;
	ic->aPtrPhrases = ic->aPtrIC->aPtrPhrases;
	ic->aPtrIC = ic->aPtrIC->aPtrIC;
	MakeSortList(ic->aAsCntx, ic->aPtrIC ? ic->aPtrIC->aAsCntx : NULL);
    }

    /* now check the associate list */
    ptr = ic->aPtrPhrases;
    while (*ptr) {
	int len = strlen((char *)ptr);
	short int nhz = len/2;

	if (ac->numSeg <= nhz && AllMatches(ac->subICs, (XChar2b *)ptr,
					    ac->sortlist, ac->sortlistlen))
	{
	    /* got one.  save and return */
	    ic->aPtrPhrases = ptr + len + 1;
	    *phzc = (XChar2b *)ptr;
	    (void) HZclAddChoice(&(ic->aAsCntx->ChList), *phzc, nhz);
	    return (nhz);
	}
	ptr += nhz * 2 + 1;
    }

    /* exhausted */
    ic->aPtrPhrases = ptr;
    return (-1);
}

void Assoc_Init (HZInputContext *ic)
{
    ic->aAsCntx = XtNew(HZassocContext);
    ic->aAsCntx->numSeg = 0;
    ic->aAsCntx->prev_ic = NULL;
    HZclInit (&(ic->aAsCntx->ChList));
    ic->aAsCntx->sortlistlen = 0;
}

/* deep copy */
void Assoc_Copy (HZInputContext *nic, HZInputContext *ic)
{
  int i;

    nic->aAsCntx = XtNew(HZassocContext);
    memmove ((char *)nic->aAsCntx, ic->aAsCntx, sizeof(HZassocContext));
    for (i = 0; i < ic->aAsCntx->numSeg; i++)
	nic->aAsCntx->subICs[i] = DupIC(ic->aAsCntx->subICs[i]);
    /* don't copy the choice list */
    HZclInit (&(nic->aAsCntx->ChList));
}

void Assoc_CleanUp(HZInputContext *ic)
{
  int i;

    for (i = 0; i < ic->aAsCntx->numSeg; i++)
	FreeIC (ic->aAsCntx->subICs[i]);
    HZclCleanUp (&(ic->aAsCntx->ChList));
    XtFree((char *)(ic->aAsCntx));
}

static void MakeSortList(HZassocContext *ac, HZassocContext *pac)
                       		/* this association context */
                        	/* previous association context */
{
  register int i, j;
  int val[MAX_ASSOC];

    /* first, only include those segments we need to check */
    ac->sortlistlen = 0;
    for (i = 0; i < ac->numSeg; i++) {

	if (ac->subICs[i]->derivability == IC_WHOLESET)
	    continue;		/* this one always good -- no need to check */

	/* need to check only if ... */
	if ( (!pac)			/* no previous context */
	    || (i >= pac->numSeg)	/* "new" segment */
					/* and unmatched segment ... */
	    || (pac->subICs[i]->matched != ac->subICs[i]->matched)
	    || strncmp (pac->subICs[i]->keysq, ac->subICs[i]->keysq,
			pac->subICs[i]->matched) )
	{
	    val[ ac->sortlistlen ] = ac->subICs[i]->derivability;
	    ac->sortlist[ ac->sortlistlen++ ] = i;
	}
    }

    /* sort it */
    for (i = 0; i < ac->sortlistlen; i++) {
	for (j = i+1; j < ac->sortlistlen; j++) {
	    if ((val[j] > 0) && (val[j] < val[i])) {
		register int tmpv;

		/* swap i, j */
		tmpv = ac->sortlist[i];
		ac->sortlist[i] = ac->sortlist[j];
		ac->sortlist[j] = tmpv;
		tmpv = val[i];  val[i] = val[j];  val[j] = tmpv;
	    }
	}
    }
}

static Boolean AllMatches(HZInputContext **subICs, XChar2b *hzc, char *sortlist, int sortlistlen)
                            
                 
                   	/* the subset to check */
                    
{
  int i;

    /* always check the easiest one first */
    for (i = 0; i < sortlistlen; i++) {
	int idx = sortlist[i];

	if (! HZderivableIC( subICs[idx], &(hzc[idx]) ))
	    return (FALSE);		/* this phrase is no good */
    }
    return (TRUE);
}


/******************* Post-selection Association **********************/

int Assoc_ps_Restart (HZInputContext *ic)
{
    ic->aPtrList = ic->al->phrases;
    return (ic->derivability);
}

short int Assoc_ps_GetNext (HZInputContext *ic, XChar2b **phzc)
{
  /* only consider latest 2 hanzi */
  /* optimized for common case */
  Char hz1b1 = ic->aLatestInput[MAX_SAVEINPUT-1].byte1;
  Char hz1b2 = ic->aLatestInput[MAX_SAVEINPUT-1].byte2;
  Char hz2b1 = ic->aLatestInput[MAX_SAVEINPUT-2].byte1;
  Char hz2b2 = ic->aLatestInput[MAX_SAVEINPUT-2].byte2;
  Char *ptr = ic->aPtrList;

    while (*ptr) {
	int len = strlen((char *)ptr);

	/* match latest input hanzi ? */
	if ((ic->aLastInNum == 1) && (ptr[0] == hz1b1) && (ptr[1] == hz1b2)) {
	    ic->aPtrList = ptr + len + 1;
	    *phzc = (XChar2b *)(ptr + 2);
	    return(len/2 - 1);
	}

	/* match latest two input hanzi (for 3-hz or longer phrase) ? */
	if ((len >= 6) && (ptr[0] == hz2b1) && (ptr[1] == hz2b2) &&
	    		  (ptr[2] == hz1b1) && (ptr[3] == hz1b2))
	{
	    ic->aPtrList = ptr + len + 1;
	    *phzc = (XChar2b *)(ptr + 4);
	    return(len/2 - 2);
	}

	ptr += len + 1;
    }
    ic->aPtrList = ptr;
    return (-1);
}

/****************************************************************/
/*	Routine to load association list from external file	*/
/****************************************************************/

#include <stdio.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/stat.h>

/* the maximum length of an association list? */
#define	MAX_ASSOC_LEN	65536		/* 64K bytes */
#define	MAX_LINE_LEN	256
#ifndef MAXPATHLEN
#define MAXPATHLEN	1024		/* Added by Zhuang Hao S.S.* for SCO */
#endif

HZAssocList *LoadAssocList (char *name, char *inputdir, char *term_encname)
{
  char filename[MAXPATHLEN];
  Char strbuf[MAX_LINE_LEN];
  Boolean has_encode = False;
  Boolean has_prompt = False;
  FILE *file;
  int len_alloc = MAX_ASSOC_LEN;
  struct stat sbuf;
  HZAssocList *hzal;
  Char *phptr;			/* append point in the phrase list */

    if (name == NULL)
	return (NULL);

    if (! HZfindfile (name, inputdir, filename)) {
	HZprintfMsg("Unable to locate the association list %s", name);
	return (NULL);
    }
    file = fopen (filename, FOPEN_R);
    if (! file) {
	HZprintfMsg("Unable to open association file \"%s\", errno=%d",
		    filename, errno);
	return (NULL);
    }

    if (stat(filename, &sbuf) == 0)
	len_alloc = sbuf.st_size;		/* better than guess! */
    hzal = XtNew (HZAssocList);
    hzal->phrases = (Char *) malloc (len_alloc);
    if (! hzal->phrases) {
	HZprintfMsg("No memory to load association list, errno=%d", errno);
	XtFree ((char *)hzal);
	fclose (file);
	return (NULL);
    }

    while (fgets ((char *)strbuf, MAX_LINE_LEN, file)) {

	if (strbuf[0] & 0x80)	/* reach the hanzi text */
	    break;

	if (strncmp ((char *)strbuf, "ENCODE:", 7) == 0) {
	    char encname[MAX_LINE_LEN];

	    HZgetprompt((char *)strbuf, encname);
	    if (HZencode (encname) != HZencode(term_encname)) {
		HZprintfMsg("%s is not in \"%s\" encoding", name, term_encname);
		XtFree ((char *)hzal);
		fclose (file);
		return (NULL);
	    }
	    has_encode = True;
	} else if (strncmp ((char *)strbuf, "PROMPT:", 7) == 0) {
	    HZgetprompt((char *)strbuf, (char *)(hzal->prompt));
	    has_prompt = True;
	}
    }
    if (! has_encode) {
	HZprintfMsg("keyword ENCODE: missing in file \"%s\"", filename);
	XtFree ((char *)hzal);
	fclose (file);
	return (NULL);
    }
    if (! has_prompt) {
	HZprintfMsg("keyword PROMPT: missing in file \"%s\"", filename);
	XtFree ((char *)hzal);
	fclose (file);
	return (NULL);
    }

    hzal->maxhz = 0;
    hzal->numPhrase = 0;
    phptr = hzal->phrases;
    do {	/* we's got one line before, so don't use while loop */
	if (strbuf[0] & 0x80) {		/* one phrase a line */
	    int nhz = 0;
	    register Char *sptr = strbuf;

	    while (*sptr & 0x80) {
		*phptr++ = *sptr++;
		*phptr++ = *sptr++;
		nhz++;
            }
	    if (nhz < 2) {
		/* This is no good -- a phrase must have two or more hanzi */
		phptr -= nhz + nhz;
		continue;
	    }
	    *phptr++ = '\0';
	    if (nhz > hzal->maxhz)
		hzal->maxhz = nhz;
	    hzal->numPhrase++ ;
	}
    } while (fgets ((char *)strbuf, 80, file));
    *phptr++ = '\0';
    fclose (file);

    /* release some extra space */
    hzal->phrases = (Char *) realloc (hzal->phrases, phptr - hzal->phrases);
    return (hzal);
}

void UnloadAssocList(HZAssocList *al)
{
    if (al) {
	if (al->phrases)
	    free((char *) al->phrases);
	free((char *)al);
    }
}
