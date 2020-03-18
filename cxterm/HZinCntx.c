/*
 *	$Id: HZinCntx.c,v 1.2 2003/05/06 07:40:19 htl10 Exp $
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

/* HZinCntx.c		The Input Context module
 * 
 * Input context is a data structure for incremental input conversion.
 */

#include "HZinput.h"

HZInputContext *HZinputSearch (CXtermInputModule *cxin, char key, HZInputContext *ic)
                            
#if NeedWidePrototypes
            			/* the current input key-stroke */
#else
             			/* the current input key-stroke */
#endif
                       		/* the previous input context */
{
  HZInputContext mold;

    /* First check if it is not a continuation on a previous input.
     * If no, start a new input context (always start from ic_type_Trie).
     * If yes, duplicate the previous input as the starting point.
     */
    if ((!ic) || IC_isNoType(ic) || IC_isAssocPs(ic)) {
	ic = &mold;
	ic->type = ic_type_Trie;
	ic->im = cxin->chzim;
	ic->al = cxin->hzal;
	ic->keysq = cxin->hzinbuf;
	ic->matched = 0;
	ic->pending = 0;
	Trie_Init(ic);
    }

    return ( HZaddInputIC(ic, key) );
}

/*
 * Search for associations
 */
HZInputContext *HZassocSearch (CXtermInputModule *cxin, short int nhz)
                            
#if NeedWidePrototypes
            			/* number of hanzi in the last input unit */ 
#else
                  		/* number of hanzi in the last input unit */ 
#endif
{
  HZInputContext *ic;

    if (! cxin->hzal)
	return (NULL);
    if (nhz >= cxin->hzal->maxhz)
	return (NULL);		/* will match nothing */

    ic = NewIC (ic_type_Assoc_ps, NULL, cxin->hzal, NULL, 0, 0);
    ic->aLatestInput = cxin->latestInput;
    ic->aLastInNum = nhz;
    ic->aPtrList = ic->al->phrases;
    ic->derivability = IC_UNCERTAIN;
    return (ic);
}

/****************** definition table for all input contexts *****************/


/*
 * Here is the transition matrix for AddInput:
 *
 *		  Normal	Wildcards	Association-key
 *	      +-------------------------------------------------+
 * Trie       | Trie		Trie_wc		Assoc		|
 * Trie_wc    | Trie_wc		Trie_wc		Assoc		|
 * Assoc      | Assoc		Assoc		Assoc		|
 *	      +-------------------------------------------------+
 */

static struct _HZicOpTable {
#if NeedNestedPrototypes
    void (*init)( FP_ic_Init );
    void (*cleanup)( FP_ic_CleanUp );
    void (*copy)( FP_ic_Copy );
    HZInputContext *(*addInput)( FP_ic_AddInput );
    int (*match)( FP_ic_Match );
    int (*restart)( FP_ic_Restart );
    short int (*getNext)( FP_ic_GetNext );
    Boolean (*derivable)( FP_ic_Derivable );
#else
    void (*init)( );
    void (*cleanup)( );
    void (*copy)( );
    HZInputContext *(*addInput)( );
    int (*match)( );
    int (*restart)( );
    short int (*getNext)( );
    Boolean (*derivable)( );
#endif
} HZicOpTable[] = {

/* ic_type_NoType */
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },

/* ic_type_Builtin */
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },

/* ic_type_Trie */
    {	Trie_Init,		NULL,			NULL,
	Trie_AddInput,		Trie_Match,		Trie_Restart,
	Trie_GetNext,		Trie_Derivable				},

/* ic_type_Trie_wc */
    {	Trie_wc_Init,		Trie_wc_CleanUp,	Trie_wc_Copy,
	Trie_wc_AddInput,	Trie_wc_Match,		Trie_wc_Restart,
	Trie_wc_GetNext,	Trie_wc_Derivable			},

/* ic_type_Assoc */
    {	Assoc_Init,		Assoc_CleanUp,		Assoc_Copy,
	Assoc_AddInput,		Assoc_Match,		Assoc_Restart,	
	Assoc_GetNext,		NULL,					},

/* ic_type_Assoc_ps */
    {	NULL,			NULL,			NULL,
	NULL,			NULL,			Assoc_ps_Restart,
	Assoc_ps_GetNext,	NULL					},

/* ic_type_Unknown */
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};

/*
 * Build a input context tree for saving the input key, from the previous ic.
 */
HZInputContext *HZaddInputIC (HZInputContext *ic, char key)
                       
#if NeedWidePrototypes
             			/* the input key */
#else
             			/* the input key */
#endif
{
    if (HZicOpTable[ic->type].addInput)
	return ( (*HZicOpTable[ic->type].addInput)(ic, key) );
    else
	return (NULL);
}

/*
 * According to the new input key, check if it match at least one HZ choices.
 */
int HZmatchIC (HZInputContext *ic)
                       	/* the input context */
{
    if (! HZicOpTable[ic->type].match)
	return (IC_FAIL);
    return ((*HZicOpTable[ic->type].match)(ic));
}

/*
 * Reset the input context, so that in the next call of HZgetNextIC(),
 * the search will restart from beginning and return the first candidate.
 */
int HZrestartIC (HZInputContext *ic)
                       	/* the input context */
{
    if (ic == NULL)
	return (IC_EMPTYSET);
    if (ic->pending)
	if (HZmatchIC(ic) == IC_FAIL)
	    return (IC_EMPTYSET);
    if (HZicOpTable[ic->type].restart)
	return ( (*HZicOpTable[ic->type].restart)(ic) );
    else
	return (IC_EMPTYSET);
}

/*
 * Try to find next candidate.  Return the number of HZ in the candidate.
 */
short int HZgetNextIC (HZInputContext *ic, XChar2b **phzc)
                       
                   		/* return address of the HZ (phrase) choice */
{
    if (ic == NULL || ic->derivability == IC_EMPTYSET ||
	(! HZicOpTable[ic->type].getNext))
	return (-1);		/* no candidate */
    return( (*HZicOpTable[ic->type].getNext)(ic, phzc) );
}

/*
 * Check if the given hanzi can be derivable from the given input context.
 */
Boolean HZderivableIC (HZInputContext *ic, XChar2b *hzc)
                       
                 		/* the HZ choice */
{
    if ((!ic) || (ic->derivability == IC_EMPTYSET))
	return (FALSE);
    if (ic->derivability == IC_WHOLESET)
	return (TRUE);

    if (HZicOpTable[ic->type].derivable)
	return ( (*HZicOpTable[ic->type].derivable)(ic, hzc) );
    else
	return (FALSE);
}


static HZInputContext *AllocIC (void);
static void DeallocIC (HZInputContext *ic);


/* allocate an input context and set the initial values */
HZInputContext *NewIC (HZicType type, HZInputMethod *im, HZAssocList *al, char *keysqbuf, int matched, int pending)
{
  HZInputContext *ic = AllocIC ();

    if (!ic)
	return (NULL);
    ic->type = type;
    ic->im = im;
    ic->al = al;
    ic->keysq = keysqbuf;
    ic->matched = matched;
    ic->pending = pending;
    if (HZicOpTable[type].init)
	(*HZicOpTable[type].init)(ic);
    return (ic);
}

/* recursively free the input context tree */
void FreeIC (HZInputContext *ic)
{
    if (HZicOpTable[ic->type].cleanup)
	(*HZicOpTable[ic->type].cleanup)(ic);
    ic->type = ic_type_NoType;
    DeallocIC (ic);
}

/* deep copy of an input context tree */
HZInputContext *DupIC (HZInputContext *ic)
{
  HZInputContext *new_ic;

    if (! ic)
	return (NULL);
    new_ic = AllocIC ();
    memmove (new_ic, ic, sizeof(HZInputContext));
    if (HZicOpTable[ic->type].copy)
	(*HZicOpTable[ic->type].copy)(new_ic, ic);
    return (new_ic);
}

/********************* IC management subroutines *********************/

#include <stdio.h>

/* we have a pool of input contexts ready */

#define	MAX_IC_POOL	256

static HZInputContext ic_pool [MAX_IC_POOL];
static short int ic_free_list [MAX_IC_POOL];
static short int ic_free_hdr;
#ifdef DEBUG_IC_ALLOC
static short int ic_allocated;	/* for statistics */
#endif

void InitICpool (void)
{
  int i;

    for (i = 0; i < MAX_IC_POOL-1; i++)
	ic_free_list[i] = i + 1;
    ic_free_list[MAX_IC_POOL-1] = -1;
    ic_free_hdr = 0;
#ifdef DEBUG_IC_ALLOC
    ic_allocated = 0;
#endif
}

static HZInputContext *AllocIC (void)
{
  HZInputContext *ic;

    if (ic_free_hdr == -1) {	/* running out of IC */
	fprintf (stderr, "Panic: running out of input context\n");
	/* should do a garbage collection here */
	exit (1);
    }
    ic = &(ic_pool[ic_free_hdr]);
#ifdef DEBUG_IC_ALLOC
    printf("IC allocate %3d, (total = %3d)\n", ic_free_hdr, ic_allocated+1);
#endif
    ic_free_hdr = ic_free_list[ic_free_hdr];
#ifdef DEBUG_IC_ALLOC
    ic_allocated ++ ;
#endif
    return (ic);
}

static void DeallocIC (HZInputContext *ic)
{
  short int idx = ic - ic_pool;		/* the array subscript for this ic */

    ic_free_list[idx] = ic_free_hdr;
    ic_free_hdr = idx;
#ifdef DEBUG_IC_ALLOC
    ic_allocated -- ;
    printf("IC deallocate %3d, (total = %3d)\n", idx, ic_allocated);
#endif
}
