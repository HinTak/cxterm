/*
 *	$Id: HZtrie.c,v 1.2 2003/05/06 07:40:19 htl10 Exp $
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
 * Trie matching routines
 */

#include "HZinput.h"

#ifdef __CYGWIN32__
#define FOPEN_R		"rb"
#else
#define FOPEN_R		"r"
#endif

/*ARGSUSED*/
HZInputContext *Trie_AddInput (HZInputContext *ic, char key)
                       
#if NeedWidePrototypes
             
#else
             
#endif
{
  HZInputContext *new_ic;

    switch (ic->im->k.keytype[(int)key]) {

      case HZ_KEY_WILDCARD:
      case HZ_KEY_WILDCHAR:
	new_ic = NewIC (ic_type_Trie_wc, ic->im, ic->al,
			ic->keysq, ic->matched, ic->pending);
	if (!new_ic)
	    return(NULL);
	new_ic->tCurTNptr = ic->tCurTNptr;
	new_ic->tCurHZptr = ic->tCurHZptr;
	new_ic->tCurHZend = ic->tCurHZend;
	new_ic->tHZtotal = ic->tHZtotal;
	new_ic->tHZcount = ic->tHZcount;
	new_ic->tWcCntx->wildcard[0] = key;
	new_ic->tWcCntx->wildcard[1] = '\0';
	new_ic->tWcCntx->wclen = 1;
	new_ic->tWcCntx->topHZend = ic->tCurHZend;
	new_ic->pending++;
	return (new_ic);

      case HZ_KEY_ASSOCIATION:
	return (Assoc_AddInput(ic, key));	/* transition to association */

      default:
	new_ic = DupIC(ic);
	if (!new_ic)
	    return(NULL);
	new_ic->pending++;
	return(new_ic);
    }
}

int Trie_Match (HZInputContext *ic)
{
  HZinputTable *hztbl = ic->im->im_hztbl;
  char *kptr, tmpChar[100]="";
  int stop;

    if (ic->matched == 0) {	/* at the top */
	ic->tCurTNptr = hztbl->trieList;
	ic->tCurHZend = hztbl->hzList + hztbl->sizeHZList;
	/* the tCurHZend now points to the end of the whole HZ list */
	if (ic->pending == 0) {
	    ic->tCurHZptr = &(hztbl->hzList[ic->tCurTNptr->tn_hzidx]);
	    ic->tHZtotal = ic->tCurTNptr->tn_numHZchoice;
	    ic->tHZcount = 0;
	    ic->derivability = IC_WHOLESET;	/* at top ... */
	    return (IC_OK);
	}
    }
    if (ic->pending == 0)
	return (IC_OK);

    if (IC_isTrieWc(ic))
	stop = ic->tWcCntx->wclen;	/* stop till reaching the wildcards */
    else
	stop = 0;			/* stop till all is matched */

    kptr = ic->keysq + ic->matched;
    while (ic->pending > stop) {
	unsigned char i;
	trieNode *tnptr;
	Boolean match = False;

	tnptr = &(hztbl->trieList[ic->tCurTNptr->tn_nextKeys]);
	i = 0;
	while (i < ic->tCurTNptr->tn_numNextKeys) {
	    if ((unsigned char)(*kptr) == (unsigned char)(tnptr->tn_key)) {
		match = True;
		break;
	    }
	    i++;  tnptr++;
	}
	if (! match) {	/* fail to match a valid keystroke sequence */
	    ic->derivability = IC_EMPTYSET;
	    return (IC_FAIL);
	}

	/* now we got it */
	if ((i + 1) < ic->tCurTNptr->tn_numNextKeys) {
	    /* the end of this segment is at the begining of next node */
	    ic->tCurHZend = &(hztbl->hzList[ (tnptr+1)->tn_hzidx ]);
	}
	/* else  ic->tCurHZend  unchanged */

	ic->tCurTNptr = tnptr;
	ic->pending--;  ic->matched++;
	kptr++;
    }

    ic->tCurHZptr = &(hztbl->hzList[ic->tCurTNptr->tn_hzidx]);

    ic->tHZtotal = ic->tCurTNptr->tn_numHZchoice;
    ic->tHZcount = 0;
    ic->derivability = ic->tHZtotal;
    return (IC_OK);
}


/* This function is added by Changsen Xu, it's to
 * check (multi)Input-Keys for one given CJK character.
 * adapted from utils/cit2tit.c
 */
void Trie_Match_HZ (int top, unsigned int trieIdx, HZinputTable *hztbl, char *hzkeys, char *hzchar)
{
    register trieNode *tnptr = &( hztbl->trieList[trieIdx] );
    register int i;
    #define MAX_KEYS 50
    static unsigned char keystack[MAX_KEYS];
    int num;		/* number of choices under the key sequence in stack */

    keystack[top] = tnptr->tn_key;

    if (tnptr->tn_numNextKeys) {	/* there can be more input keys */
	int end_hzidx = hztbl->trieList[tnptr->tn_nextKeys].tn_hzidx;
	num = 0;
	for ( i = tnptr->tn_hzidx; i < end_hzidx; i++, num++ )
	    if (hztbl->hzList[i].byte1 == HZ_PHRASE_TAG)
		i += hztbl->hzList[i].byte2;
    } else
	num = tnptr->tn_numHZchoice;

    if ((num > 0) || (tnptr->tn_numNextKeys == 0)) {
	/* there are HZs exclusively under the keys, or no more input keys */
	XChar2b *phz = &hztbl->hzList[tnptr->tn_hzidx];
	for (i = 0; i < num; i++) {
	    if (phz->byte1 != HZ_PHRASE_TAG) {
		/* hanzi */
		if( (0xff & phz->byte1) == (0xff&hzchar[0]) &&
		    (0xff & phz->byte2) == (0xff&hzchar[1]) )  {

		    keystack[top+1]='\0';
		    if ( (int) strlen(hzkeys)>=top && 
			!strcmp( hzkeys+strlen(hzkeys)-top, keystack+1) ) 
			continue; /* overlap with previous key, skip */
		    else {  /* remember this key */
			if( strlen(hzkeys) + strlen(keystack) >= MAX_KEYS-5)
			/* too long, HZ in many phrases) */
			{   if ( strcmp(hzkeys+strlen(hzkeys)-4, " ...") )
				strcat(hzkeys, " ...");
			    return;
			}
			if (hzkeys[0] != '\0') strcat (hzkeys, "; ");
			strcat (hzkeys, keystack+1);
		    }
		}
		phz ++;
	    } else { /* phrase, or multi characters for one key */
                register int j, hzlen = (phz++)->byte2;
                for (j = 0; j < hzlen; j++) {
		    if( (0xff & phz->byte1) == (0xff&hzchar[0]) &&
			(0xff & phz->byte2) == (0xff&hzchar[1]) )  {
			keystack[top+1]='\0';
			if ( (int) strlen(hzkeys)>=top && 
			    !strcmp( hzkeys+strlen(hzkeys)-top, keystack+1) ) 
			    continue; /* overlap with previous key, skip */
			else {  /* remember this key */
			    if( strlen(hzkeys) + strlen(keystack) >= MAX_KEYS-5)
			    /* too long, HZ in many phrases) */
			    {	if ( strcmp(hzkeys+strlen(hzkeys)-4, " ...") )
				    strcat(hzkeys, " ...");
				return;
			    }

			    if (hzkeys[0] != '\0') strcat (hzkeys, "; ");
			    strcat (hzkeys, keystack+1);
			}
		    }
		    phz++;
                }
	    }
	}
    }

    for (i = 0; i < tnptr->tn_numNextKeys; i++)
	Trie_Match_HZ (top + 1, tnptr->tn_nextKeys + i, hztbl, hzkeys, hzchar);
}



int Trie_Restart (HZInputContext *ic)
{
    if (ic->tCurTNptr) {
	ic->tCurHZptr = &(ic->im->im_hztbl->hzList[ic->tCurTNptr->tn_hzidx]);
	ic->tHZtotal = ic->tCurTNptr->tn_numHZchoice;
    }
    ic->tHZcount = 0;
    return (ic->derivability);
}

short int Trie_GetNext (HZInputContext *ic, XChar2b **phzc)
{
    if (ic->tHZcount < ic->tHZtotal) {
	short int nhz;

	ic->tHZcount++ ;
	if (ic->tCurHZptr->byte1 == HZ_PHRASE_TAG) {	/* phrase! */
	    *phzc = ic->tCurHZptr + 1;
	    nhz = ic->tCurHZptr->byte2;
	    ic->tCurHZptr += 1 + nhz;
	    return (nhz);
	} else {
	    *phzc = ic->tCurHZptr++ ;
	    return (1);
	}
    } else
	return (-1);
}

Boolean Trie_Derivable (HZInputContext *ic, XChar2b *hzc)
{
  XChar2b *ptr;
  unsigned short int i;

    if (ic->im->im_hzidx)
	return (HZIdxInRange(ic->im->im_hzidx, hzc,
			     ic->tCurHZptr - ic->im->im_hztbl->hzList,
			     ic->tCurHZend - ic->im->im_hztbl->hzList));

    /* in case for future parameterized checking */
    ptr = ic->tCurHZptr;
    for (i = ic->tHZcount; i < ic->tHZtotal; i++) {
	if (ptr->byte1 == HZ_PHRASE_TAG) {
	    ptr += 1 + ptr->byte2;
	    continue;
	}
	if (ptr->byte1 == hzc->byte1 && ptr->byte2 == hzc->byte2)
	    return (TRUE);
	ptr++;
    }
    return (FALSE);
}

void Trie_Init (HZInputContext *ic)
{
    ic->derivability = IC_WHOLESET;	/* at top ... */
    ic->tCurTNptr = NULL;
    ic->tCurHZptr = NULL;
    ic->tHZtotal = 0;
    ic->tWcCntx = NULL;
}

/****************************************************************/
/*	Routines to load Trie from external file (CIT)		*/
/****************************************************************/

#include <stdio.h>
#include <errno.h>
#include <sys/param.h>
#ifndef MAXPATHLEN
#define MAXPATHLEN	1024		/* Added by Zhuang Hao S.S.* for SCO */
#endif

/*
 * LoadCIT -- load the input table from .cit file
 */
int LoadCIT (char *name, char *inputdir, char *encname, HZinputTable *hztbl)
{
  char basename[80];
  char filename[MAXPATHLEN];
  char magic[2];
  FILE *file;

    strcpy (basename, name);
    strcat (basename, CIT_SUFFIX);
    if (! HZfindfile (basename, inputdir, filename)) {
	HZprintfMsg("Unable to locate the input table for %s", name);
	return(-1);
    }

    file = fopen (filename, FOPEN_R);
    if (! file) {
	HZprintfMsg("Unable to open the input table file \"%s\", errno=%d",
		    filename, errno);
	return(-1);
    }

    HZprintfMsg("Loading input table ...");

    (void) fread (magic, 2, 1, file);
    if (strncmp (magic, MAGIC_CIT, 2) != 0) {
	HZprintfMsg("Not a legal %s file for %s", CIT_SUFFIX, name);
	return(-1);
    }
    if (fread((char *)hztbl, sizeof(HZinputTable), 1, file) == 0) {
	HZprintfMsg("Error in loading input table for %s", name);
	return(-1);
    }

    if (hztbl->encode != HZencode(encname)) {
	HZprintfMsg("%s is not a \"%s\" encoding input method", name, encname);
	return(-1);
    }
    if (hztbl->version != CIT_VERSION) {
	HZprintfMsg("Version mismatched for %s, rerun tit2cit", name);
	return(-1);
    }
    hztbl->trieList = (trieNode *) calloc (hztbl->sizeTrieList,
					   sizeof(trieNode));
    hztbl->hzList = (XChar2b *) calloc (hztbl->sizeHZList, sizeof(XChar2b));
    if ((! hztbl->hzList) || (! hztbl->trieList)) {
	    HZprintfMsg("No memory to load input table for %s", name);
	    if (hztbl->trieList)  free ((char *)(hztbl->trieList));
	    if (hztbl->hzList)  free ((char *)(hztbl->hzList));
	    return(-1);
    }
    if ((fread ((char *)(hztbl->trieList), sizeof(trieNode),
		(int)hztbl->sizeTrieList, file) != hztbl->sizeTrieList) ||
	(fread ((char *)hztbl->hzList, sizeof(XChar2b),
		(int)(hztbl->sizeHZList), file) != hztbl->sizeHZList))
    {
	    HZprintfMsg("Error in loading input table for %s", name);
	    free ((char *)(hztbl->trieList));
	    free ((char *)(hztbl->hzList));
	    return(-1);
    }
    fclose (file);
    return(0);
}

/*
 * UnloadCIT -- unload the input table, free the memory
 */
void UnloadCIT (HZinputTable *hztbl)
{
    if (! hztbl)
	return;
    if (hztbl->trieList)
	free ((char *)(hztbl->trieList));
    if (hztbl->hzList)
	free ((char *)(hztbl->hzList));
    free ((char *)(hztbl));
}


#define	OFLST2HZIDX(o)		(-(o)-2)
#define	HZIDX2OFLST(i)		(-((i)+2))

HZimHZListIndex *MakeHZIdx(HZinputTable *hztbl)
{
  HZimHZListIndex *hzidx;
  XChar2b *pHZ, *pHZbegin, *pHZend;
  int oflistcnt = 0;
  int i;

    HZprintfMsg(" Building index ...");
    hzidx = (HZimHZListIndex *)malloc(sizeof(HZimHZListIndex));
    if (! hzidx) {
	HZprintfMsg("No memory to build index table.");
	return (NULL);
    }
    hzidx->oflistlen = hztbl->sizeHZList;	/* just a guess */
    hzidx->oflist = (int *)calloc(hzidx->oflistlen, sizeof(int));
    if (! hzidx->oflist) {
	HZprintfMsg("No memory to build index table.");
	free((char *)hzidx);
	return (NULL);
    }
    for (i = 0; i < MAX_HZ_NUM; i++)
	hzidx->idxlist[i] = -1;

    pHZbegin = hztbl->hzList;
    pHZend = pHZbegin + hztbl->sizeHZList;
    for (pHZ = pHZbegin; pHZ < pHZend; pHZ++) {
	int n;

	if (pHZ->byte1 == HZ_PHRASE_TAG) {	/* ignore phrases */
	    pHZ+= pHZ->byte2;
	    continue;
	}
	n = HZSUBSCRIPT(pHZ);
	if (hzidx->idxlist[n] == -1) {	/* empty slot */
	    hzidx->idxlist[n] = pHZ - pHZbegin;
	    continue;
	}
	if (hzidx->idxlist[n] >= 0) {	/* already have one slot */
	    register XChar2b *p;
	    int first_idx = hzidx->idxlist[n];

	    hzidx->idxlist[n] = OFLST2HZIDX(oflistcnt);
	    hzidx->oflist[ oflistcnt++ ] = first_idx;
	    for (p = pHZ; p < pHZend; p++) {

		if (p->byte1 == HZ_PHRASE_TAG) {	/* ignore phrases */
		    p += p->byte2;
		    continue;
		}
		if (p->byte1 != pHZ->byte1 || p->byte2 != pHZ->byte2)
		    continue;

		if (oflistcnt >= hzidx->oflistlen) {
		    hzidx->oflistlen *= 2;
		    hzidx->oflist = (int *)realloc(hzidx->oflist,
					hzidx->oflistlen * sizeof(int));
		    if (! hzidx->oflist) {
			HZprintfMsg("No memory to build index table.");
			free((char *)hzidx);
			return (NULL);
		    }
		}
		hzidx->oflist[ oflistcnt++ ] = p - pHZbegin;
	    }
	    hzidx->oflist[ oflistcnt++ ] = -1;	/* end of list mark */
	}
	/* (hzidx->idxlist[n] < -1)	-- already been taken care of */
    }
    hzidx->oflist = (int *)realloc(hzidx->oflist, oflistcnt * sizeof(int));
    hzidx->oflistlen = oflistcnt;

    return(hzidx);
}

int HZIdxInRange(HZimHZListIndex *hzidx, XChar2b *hzc, int idx1, int idx2)
{
  int nidx = hzidx->idxlist[ HZSUBSCRIPT(hzc) ];
  register int *pidx;

    if (nidx == -1)
	return (FALSE);			/* un-derivable */
    if (nidx >= 0)
	return ((idx1 <= nidx) && (nidx < idx2));
    pidx = &(hzidx->oflist[ HZIDX2OFLST(nidx) ]);
    while (*pidx != -1) {
	if ((idx1 <= *pidx) && (*pidx < idx2))
	    return (TRUE);
	pidx++;
    }
    return (FALSE);
}

void FreeHZIdx(HZimHZListIndex *hzidx)
{
    if (! hzidx)
	return;
    if (hzidx->oflist)
	free((char *)hzidx->oflist);
    free((char *)hzidx);
}
