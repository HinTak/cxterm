/*
 *	$Id: HZtrieWc.c,v 1.2 2003/05/06 07:40:19 htl10 Exp $
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
 * Trie matching routines with wildcards
 */

#include "HZinput.h"

static void Trie_wcStartAtTop(HZInputContext *ic);
static int Trie_wcContSearch (HZInputContext *ic);
static int Trie_wcNextTrieNode(HZwildcardContext *pwc);
static int WildcardMatch (char *string, char *pattern, short unsigned int *keytype);


/*ARGSUSED*/
HZInputContext *Trie_wc_AddInput (HZInputContext *ic, char key)
                       
#if NeedWidePrototypes
             
#else
             
#endif
{
    switch (ic->im->k.keytype[(int)key]) {
      case HZ_KEY_ASSOCIATION:
	return (Assoc_AddInput(ic, key));	/* transition to association */

      default:
	ic = DupIC(ic);
	if (!ic)
	    return (NULL);
	ic->tWcCntx->wildcard[ ic->tWcCntx->wclen++ ] = key;
	ic->tWcCntx->wildcard[ ic->tWcCntx->wclen   ] = '\0';
	ic->pending++;
	return (ic);
    }
}

int Trie_wc_Match (HZInputContext *ic)
{
    /* to match the non-wildcard prefix first */
    if (Trie_Match(ic) == IC_FAIL)
	return (IC_FAIL);

    if (ic->pending == 0) 
	return (IC_OK);

    if ((ic->matched == 0) && (ic->tWcCntx->wclen == 1) &&
			(ic->tWcCntx->wildcard[0] == HZ_KEY_WILDCARD))
	ic->derivability = IC_WHOLESET;
    else
	ic->derivability = IC_UNCERTAIN;

    Trie_wcStartAtTop(ic);		/* set the start point */
    if (Trie_wcContSearch(ic)) {
	ic->matched += ic->pending;
	ic->pending =0;
	return (IC_OK);
    } else
	return (IC_FAIL);
}

int Trie_wc_Restart (HZInputContext *ic)
{
    /* restart the non-wildcard prefix */
    if (Trie_Restart(ic) == IC_EMPTYSET)
	return (IC_EMPTYSET);

    Trie_wcStartAtTop(ic);		/* set the start point */
    if (Trie_wcContSearch(ic) == IC_FAIL)
	return (IC_EMPTYSET);
    else
	return (ic->derivability);
}

short int Trie_wc_GetNext (HZInputContext *ic, XChar2b **phzc)
{
  short int nhz;

    while (1) {
	nhz = Trie_GetNext(ic, phzc);	/* get choices from the expansion */
	if (nhz >= 0)
	    return nhz;

	/* no more choice for this expansion, search for next one */
	if (Trie_wcContSearch(ic) == IC_FAIL)
	    return (-1);
    }
    /*NOTREACHED*/
}

Boolean Trie_wc_Derivable (HZInputContext *ic, XChar2b *hzc)
{
    if (Trie_wc_Restart(ic) == IC_WHOLESET)
	return (TRUE);

    /* will be way slow! */
    while (1) {
	if (Trie_Derivable (ic, hzc))
	    return (TRUE);

	/* no more choice for this expansion, search for next one */
	if (Trie_wcContSearch(ic) == IC_FAIL)
	    return (FALSE);
    }
    /*NOTREACHED*/
}

void Trie_wc_Init (HZInputContext *ic)
{
    ic->derivability = IC_WHOLESET;
    ic->tWcCntx = XtNew(HZwildcardContext);
    ic->tWcCntx->wildcard[0] = '\0';
}

void Trie_wc_CleanUp(HZInputContext *ic)
{
    if (ic->tWcCntx)
	XtFree((char *)(ic->tWcCntx));
}

void Trie_wc_Copy (HZInputContext *new_ic, HZInputContext *ic)
{
    new_ic->tWcCntx = XtNew(HZwildcardContext);
    memmove(new_ic->tWcCntx, ic->tWcCntx, sizeof(HZwildcardContext));
}

/*********************** Traversal in Trie ***********************/

/*
 * Search in the trie for the given suffix in the input sequent
 * that contains wildcard(s).  The "top" node matches the longest
 * prefix in the input sequent that contains no wildcard.
 * The traversal is a Deep-First-Search starting from "top".
 * But it visits the node before visiting any child under the node.
 *
 * WildcardMatch() returns 3 values.  WILD_MATCH is for an exact match.
 * WILD_PREFIX means the current traversal path (from "top" to the current
 * node) matches a prefix of the wildcard pattern, which might suggest a
 * possible match if the traversal goes deeper.  WILD_UNMATCH is a total
 * mismatch, in which case the traversal should stop go any deeper.
 */

#define WILD_MATCH	0		/* exact match */
#define WILD_PREFIX	1		/* no match, but maybe if go deeper */
#define WILD_UNMATCH	2		/* complete mismatch */

/*
 * Set the search start point at the top of the expansion point
 */
static void Trie_wcStartAtTop(HZInputContext *ic)
{
  HZwildcardContext *pwc = ic->tWcCntx;

    pwc->depth = 0;
    pwc->tNstack[0] = ic->tCurTNptr;		/* the lowest fix point */
    pwc->tNnumSb[0] = 0;
    bzero ((char *)(pwc->repcode), sizeof(pwc->repcode));
}

/*
 * Search the Trie from the last leftover (or top) expansion point
 */
static int Trie_wcContSearch(HZInputContext *ic)
{
  HZwildcardContext *pwc = ic->tWcCntx;
  HZinputTable *hztbl = ic->im->im_hztbl;

    if (! pwc->tNstack[0])
	return (IC_FAIL);	/* the traversal has been exhausted */

    while (1) {
	trieNode *tnptr;

	switch (WildcardMatch(pwc->repcode, pwc->wildcard, ic->im->k.keytype)) {

	  case WILD_MATCH:
	    /* Good.  The matching trieNode is at pwc->tNstack[pwc->depth] */

	    tnptr = pwc->tNstack[pwc->depth];
	    ic->tCurHZptr = &(hztbl->hzList[ tnptr->tn_hzidx ]);
	    ic->tHZtotal = tnptr->tn_numHZchoice;
	    ic->tHZcount = 0;

	    /* move the search resume point to next node */
	    if ( Trie_wcNextTrieNode(pwc) == IC_OK )
		ic->tCurHZend =
			&(hztbl->hzList[ pwc->tNstack[pwc->depth]->tn_hzidx ]);
	    else
		ic->tCurHZend = pwc->topHZend;

	    return(IC_OK);

	  case WILD_PREFIX:
	    /* go down */

	    tnptr = pwc->tNstack[pwc->depth];
	    if (tnptr->tn_numNextKeys > 0) {
		trieNode *new_tnptr = &(hztbl->trieList[ tnptr->tn_nextKeys ]);

		pwc->depth++ ;
		pwc->tNnumSb[ pwc->depth ] = tnptr->tn_numNextKeys - 1;
		pwc->tNstack[ pwc->depth ] = new_tnptr;

		pwc->repcode[ pwc->depth - 1 ] = new_tnptr->tn_key;
		break;
	    }

	    /* No more additional key, hence no match for this node. */
	    /* Don't go down, move forward */

	    if ( Trie_wcNextTrieNode(pwc) == IC_FAIL )
		return (IC_FAIL);
	    break;

	  case WILD_UNMATCH :

	    /* go to the next sibling */
	    if ( Trie_wcNextTrieNode(pwc) == IC_FAIL )
		return (IC_FAIL);
	    break;
	}
    }
}

/*
 * Move to next sibling, if no more sibling, move up to parent's sibling
 */
static int Trie_wcNextTrieNode(HZwildcardContext *pwc)
{
    while (pwc->tNnumSb[ pwc->depth ] == 0) {
	/* no more sibling, go up */
	if (pwc->depth == 0) {
	    /* now at the topmost; we've tried everything! */
	    pwc->tNstack[ 0 ] = NULL;
	    return(IC_FAIL);
	} else {
	    pwc->depth--;
	    pwc->repcode[pwc->depth] = '\0';
	}
    }
    pwc->tNnumSb[ pwc->depth ]-- ;
    pwc->tNstack[ pwc->depth ]++ ; 
    pwc->repcode[ pwc->depth-1 ] = pwc->tNstack[ pwc->depth ]->tn_key;
    return(IC_OK);
}

/********************** WILDCARD STRING MATCH ************************/

/* use global variable just save some time in the recursive match */
static unsigned short *wildkeytype;	/* for checking wildcards */
static int match(char *string, char *pattern);

/*
 * match string with pattern, under the given keytype definition
 */
static int WildcardMatch(char *string, char *pattern, short unsigned int *keytype)
{
    wildkeytype = keytype;
    return ( match( string, pattern ) );
}


/*
 * recursively match string (with no wildcard) against pattern (with wildcard)
 */
static int match(char *string, char *pattern)
{
#define car(s)		(*(s))
#define cdr(s)		((s)+1)
#define empty(s)	(!(*(s)))

#define iswildcard(c)	(wildkeytype[c] == HZ_KEY_WILDCARD)
#define iswildchar(c)	(wildkeytype[c] == HZ_KEY_WILDCHAR)

    if (empty(pattern))
	return (empty(string) ? WILD_MATCH : WILD_UNMATCH);
    else
    if (iswildcard((int) car(pattern))) {
	int x = match(string, cdr(pattern));
	if (x == WILD_UNMATCH)
	    return (match(cdr(string), pattern));
	else
	    return x;
    } else
	if (empty(string))
	    return WILD_PREFIX;
        else if (iswildchar((int) car(pattern)) || car(pattern) == car(string))
	    return match(cdr(string), cdr(pattern));
	else
	    return WILD_UNMATCH;
}
