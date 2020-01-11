/*
 *	$Id: HZchList.c,v 1.2 2003/05/06 07:40:19 htl10 Exp $
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

/* HZchList.c		The Choice List management module
 *
 * A choice list is a list of pointers to HZ (phrase) candidates.
 * A continuous segment of the list is call the on-screen selection window.
 * It is a sliding window of the choice list, which is displayed in the
 * input area.  A user can move it forward (to the right) or backward
 * (to the left) using some keys.  The size of the selection window
 * depends on the width of the input area and the length of each choice.
 * When the selection window reaches the right end of the choice list,
 * cxterm tries to expend the choice list by getting new choices from
 * the current input context.  A choice list is exhausted if there
 * will be no more choice in the current input context.
 */

#include "HZinput.h"		/* X headers included here. */

/*
 * Initialize the Choice List  
 */
void HZclInit (HZChoiceList *cl)
{
    cl->num_alloc = 0;
    cl->choices = (struct hz_choice *)NULL;	/* allocated upon request */
    HZclReset (cl);
}

/*
 * Reset the Choice List.  Make it empty but don't dealloc the space.
 */
void HZclReset (HZChoiceList *cl)
{
    cl->numChoices = 0;
    cl->exhaust = False;
    cl->selPos = 0;
    cl->selNum = 0;
}

/*
 * Free the space occupied the Choice List
 */
void HZclCleanUp (HZChoiceList *cl)
{
    if (cl->choices)
	free ((char *)(cl->choices));
}

/*
 * Pick the n-th selection in the on-screen selection window of the
 * choice list.  Return the number of HZ in the selected candidate.
 */
short int HZclSelect (HZChoiceList *cl, int n, XChar2b **phzc)
                      		/* the choice list */
           			/* the n-th selection (n >= 0) */
                    		/* the return address of the HZ choice */
{
    if (cl->selNum == 0)		/* nothing to select */
	return (0);
    if (n >= cl->selNum)		/* no such selection */
	return (-1);
    *phzc = cl->choices[ cl->selPos + n ].hzc;
    return (cl->choices[ cl->selPos + n ].nhz);
}

/*
 * Make the selection window in the choice list, given the start position.
 * Try to get new choices from the input context and expand the list
 * when the selection window reach the right end of the list.
 *
 * cl->selPos   must be already set to the new start position.
 */
int HZclMakeSelection (HZChoiceList *cl, HZInputArea *ia, HZInputContext *ic)
{
    int len;		/* length of the selection window in input area */

    len = 3;		/* 3: '<' ' ' before the list, and '>' after it */
    cl->selNum = 0;

    /* First, see if there are undisplay choices in the choice list */
    while (cl->selPos + cl->selNum < cl->numChoices) {
	len += SelWidthOnScr( cl->choices[ cl->selPos + cl->selNum ].nhz );
	if (OverflowSelStr(len,ia,cl))		/* selection window too wide */
	    return (cl->selNum);

	cl->selNum ++;
	if (cl->selNum >= ia->maxchoice)	/* too many choices */
	    return (cl->selNum);
    }
    if (cl->exhaust)
	return (cl->selNum);	/* no more choice possible */

    /* Then, check the input context for possible new choices */
    while (1) {
	XChar2b *hzc;
	short int nhz;

	nhz = HZgetNextIC (ic, &hzc);
	if (nhz < 0) {
	    cl->exhaust = True;
	    break;
	}

	if (HZclAddChoice(cl, hzc, nhz) < 0)
	    return (cl->selNum);
	if (cl->numChoices == ic->derivability)
	    cl->exhaust = True;

	len += SelWidthOnScr(nhz);
	if (OverflowSelStr(len,ia,cl))		/* window too wide */
	    break;

	cl->selNum ++;
	if (cl->selNum >= ia->maxchoice)	/* too many choices */
	    break;
    }
    return (cl->selNum);
}

/*
 * Move the selection window forward in the choice list.
 */
int HZclMoveForward (HZChoiceList *cl, HZInputArea *ia, HZInputContext *ic)
{
    int save_selPos = cl->selPos;
    int save_selNum = cl->selNum;

    cl->selPos += cl->selNum;
    if (HZclMakeSelection (cl, ia, ic) == 0) {
	/* no choice to the right, restore the old position of the window */
	cl->selPos = save_selPos;
	cl->selNum = save_selNum;
	return (0);
    } else
	return (cl->selNum);
}

/*
 * Move the selection window backward in the choice list.
 */
/*ARGSUSED*/
int HZclMoveBackward (HZChoiceList *cl, HZInputArea *ia, HZInputContext *ic)
{
    int len = 0;

    if (cl->selPos == 0)	/* no choice to the left */
	return (0);

    cl->selNum = 0;
    while (1) {
	len += SelWidthOnScr (cl->choices[ cl->selPos - 1 ].nhz);
	if (OverflowSelStr(len,ia,cl))		/* too wide */
	    break;

	cl->selNum ++ ;
	cl->selPos -- ;

	if ((cl->selNum >= ia->maxchoice) || (cl->selPos <= 0))
		break;
    }
    return (cl->selNum);
}

/* add a HZ (phrase) candidate into the choice list */
int HZclAddChoice (HZChoiceList *cl, XChar2b *hzc, short int nhz)
                      
                  
#if NeedWidePrototypes
             
#else
                   
#endif
{
    if (cl->choices == NULL) {
	/* start from MAX_CHOICE, the maximum number of choices on screen */
	cl->num_alloc = MAX_CHOICE;
	cl->choices = (struct hz_choice *) calloc (MAX_CHOICE,
				sizeof (struct hz_choice));
	if (! cl->choices) {
	    HZprintfMsg ("No memory to expand the choice list");
	    return (-1);
	}
    } else if (cl->numChoices >= cl->num_alloc) {
	cl->num_alloc += cl->num_alloc;		/* double it */
	cl->choices = (struct hz_choice *) realloc ((char *)(cl->choices),
			(unsigned) (cl->num_alloc * sizeof(struct hz_choice)));
	if (! cl->choices) {
	    HZprintfMsg ("No memory to expand the choice list");
	    return (-1);
	}
    }

    cl->choices[ cl->numChoices   ].hzc = hzc;
    cl->choices[ cl->numChoices++ ].nhz = nhz;
    return (0);
}
