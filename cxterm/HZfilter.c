/*
 *	$Id: HZfilter.c,v 1.2 2003/05/06 07:40:19 htl10 Exp $
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
 * This file implements the generic input table filter.
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
#include <stdio.h>

/*****************************************************************************
 * hzTableFilter -- filter the input character using (external) input method.
 *****************************************************************************/

static int  InputMatchAtEnd(CXtermInputModule *cxin);
static int  InputMatchFromPos(CXtermInputModule *cxin, int pos);
static int  AutoSegmentation(CXtermInputModule *cxin, char ch);
static int  DoSelectionKey(short unsigned int ktype, CXtermInputModule *cxin, char *strbuf);
static void FetchSelection(CXtermInputModule *cxin);
static void UpdateInputArea(CXtermInputModule *cxin);
static int  UpdateIAwAutoSelect(CXtermInputModule *cxin, char *strbuf);
static void DoPostSelection(CXtermInputModule *cxin, XChar2b *hzc, short int nhz);



extern void Bell (void);
extern int sprintf (char *, const char *, ...);

int hzTableFilter (CXtermInputModule *cxin, char ch, char *strbuf)
                            
#if NeedWidePrototypes
           			/* the input keystroke */
#else
            			/* the input keystroke */
#endif
                  		/* the generated character(s) goes here */
{
#define CleanAssocDisplay()	\
	if (cxin->cur_ic && IC_isAssocPs(cxin->cur_ic))	\
		HZimReset (cxin, True);

  unsigned short ktype;
  HZInputArea *ia = &cxin->hzia;
  HZChoiceList *cl = &cxin->hzcl;
  char *inbuf = cxin->hzinbuf;
  char msgbuf[16];
  Boolean was_exhaust;

    ktype = (ch & 0x80) ? HZ_KEY_INVALID : cxin->chzim->k.keytype[(int)ch];

    if (ktype == HZ_KEY_INVALID) {
	/* not a valid key for this input, pass out */
	CleanAssocDisplay();
	strbuf[0] = ch;
	return (1);
    }

    /* Input Key?  (that includes all conjoin, wildcard, wildchar keys) */
    if (ktype & HZ_KEY_INPUT_MASK) {

	if (cxin->hzinbufCount >= MAX_INBUF) {
	    /* the input keystroke buffer is full */
	    /* could only used as a selection key */

	    if (ktype & HZ_KEY_SELECTION_MASK)
		return DoSelectionKey(ktype, cxin, strbuf);

	    /* not a selection key */

	    /* just beep and maintain the current choice-list */
	    HZiaSetStatus(ia, "too long");
	    HZiaFlush (ia, cxin->screen);
	    Bell ();
	    return (0);
	}

	if (! HZinLineEditMode(cxin)) {		/* normal case, appending */

	    /* append the key and match it */
	    inbuf[ cxin->hzinbufCount ] = ch;
	    if (InputMatchAtEnd (cxin)) {
		/* the input key is valid */
		/* fill up the selection list and do auto-selection */
		return UpdateIAwAutoSelect (cxin, strbuf);
	    }
	    /* input key not match */

	    /* maybe it is a selection key */
	    if (ktype & HZ_KEY_SELECTION_MASK) {
		/* now the selection key */
		return DoSelectionKey(ktype, cxin, strbuf);
	    }
	    /* nor a selection key */

	    /* try auto-segmentation */
	    if (AutoSegmentation (cxin, ch)) {
		/* successful */
		/* fill up the selection list and do auto-selection */
		return UpdateIAwAutoSelect (cxin, strbuf);
	    }
	    /* failed auto-segmentation */

	    /* pick up the previous choice if it meets the condition: */
	    /* previous choice is unique, auto-selection is "when-nomatch" */
	    if (HZclUniq(cl) &&
		(cxin->chzim->m.auto_select == HZ_AUTOSELECT_WHENNOMATCH))
	    {
		XChar2b *hzc;
		short int nhz;

		nhz = HZclSelect (cl, 0, &hzc);

		DoPostSelection(cxin, (XChar2b *)NULL, 0);	
		/* NULL -- no post selection association! */

		/* start this key in a new input buffer */
		inbuf[ cxin->hzinbufCount ] = ch;
		if (InputMatchAtEnd (cxin)) {
		    /* No further "auto-select" in continuous input */
		    UpdateInputArea (cxin);
		} else {
		    /* this key is no good as new input keystroke sequence */
		    (void) sprintf (msgbuf, "%c nomatch", ch);
		    HZiaSetStatus (ia, msgbuf);
		    HZiaFlush (ia, cxin->screen);
		    Bell ();
		}

		/* pass out previous choice */
		strncpy (strbuf, (char *)hzc, nhz * 2);
		return (nhz * 2);
	    }
	    /* no continuous input either */

	    /* just beep and maintain the current choice-list */
	    (void) sprintf (msgbuf, "%c nomatch", ch);
	    HZiaSetStatus (ia, msgbuf);
	    HZiaFlush (ia, cxin->screen);
	    Bell ();
	    return 0;

	} else {	/* HZinLineEditMode(cxin) */

	    /* insert key at cursor position */
	    memmove(inbuf + cxin->cursor + 1, inbuf + cxin->cursor,
		    cxin->hzinbufCount - cxin->cursor);
	    inbuf[ cxin->cursor++ ] = ch;
	    cxin->hzinbufCount ++;

	    /* re-match input from the insertion point (cursor - 1) */
	    if (InputMatchFromPos (cxin, cxin->cursor-1)) {
		/* no auto-selection in line editing mode */
		UpdateInputArea (cxin);
		return (0);
	    }
	    /* no match */

	    /* maybe a selection key */
	    if (ktype & HZ_KEY_SELECTION_MASK) {
		/* take out the inserted key */
		memmove(inbuf + cxin->cursor - 1, inbuf + cxin->cursor,
			cxin->hzinbufCount - cxin->cursor + 1);
		cxin->cursor --;
		cxin->hzinbufCount --;

		return DoSelectionKey(ktype, cxin, strbuf);
	    }
	    /* nor a selection key */

	    /* just clean the choice-list and announce the unmatched key */
	    HZclReset (&cxin->hzcl);
	    HZiaSetInBuf (ia, cxin->hzinbuf, cxin->hzinbufCount, cxin->cursor);
	    HZprintfMsg (" empty selection list (bad inserted key %c)", ch);
	    return 0;

	} /* HZinLineEditMode(cxin) ? */
    }

    /* not an input key, maybe a selection key */
    if (ktype & HZ_KEY_SELECTION_MASK)
	return DoSelectionKey(ktype, cxin, strbuf);

    /* some common action first */
    switch (ktype) {

      case HZ_KEY_EDIT_BACKSPACE:
      case HZ_KEY_EDIT_ERASE:
      case HZ_KEY_EDIT_KILL:
      case HZ_KEY_EDIT_BEGIN:
      case HZ_KEY_EDIT_END:
      case HZ_KEY_EDIT_FORW:
      case HZ_KEY_EDIT_BACK:

	if (cxin->hzinbufCount == 0) {
	    /* edit key won't be effective if the input buffer is empty */
	    CleanAssocDisplay();
	    return (1);			/* pass it out */
	}
	break;

      default:
	break;
    }

    switch (ktype) {

      case HZ_KEY_EDIT_BACKSPACE:

	if (! HZinLineEditMode(cxin)) {	/* at the end */

	    /* remove the last key */
	    cxin->hzinbufCount-- ;
	    if (cxin->hzinbufCount == 0) {	/* empty buffer */
		HZimReset (cxin, True);
		return (0);
	    }
	    cxin->cur_ic = cxin->cache_ics[cxin->hzinbufCount - 1];

	    /* we need to restart the this context (but no need to re-match) */
	    (void) HZrestartIC (cxin->cur_ic);
	    UpdateInputArea (cxin);
	    return (0);

	} else {			/* in line editing mode */

	    if (cxin->cursor == 0) {	/* already at the beginning */
		Bell ();
		return (0);
	    }

	    /* remove the key previous to the cursor */
	    memmove(inbuf + cxin->cursor - 1, inbuf + cxin->cursor,
		    cxin->hzinbufCount - cxin->cursor + 1);
	    cxin->hzinbufCount --;
	    cxin->cursor --;

	    if (InputMatchFromPos (cxin, cxin->cursor)) {
		/* no auto-selection in line editing mode */
		UpdateInputArea (cxin);
		return (0);
	    }
	    /* no match */

	    /* just clean the choice-list and announce nomatch */
	    HZclReset (cl);
	    HZiaSetInBuf (ia, cxin->hzinbuf, cxin->hzinbufCount, cxin->cursor);
	    HZprintfMsg (" empty selection list (bad key deletion)");
	    return 0;

	}
	/* break; */

      case HZ_KEY_EDIT_ERASE:

	if (HZinLineEditMode(cxin)) {	/* only active in line editing mode */
	   int pos = cxin->cursor;

	    /* remove the key at the cursor */
	    memmove(inbuf + cxin->cursor, inbuf + cxin->cursor + 1,
		    cxin->hzinbufCount - cxin->cursor);
	    cxin->hzinbufCount --;
	    if (cxin->hzinbufCount == cxin->cursor)
		cxin->cursor = -1;

	    if (InputMatchFromPos (cxin, pos)) {
		/* no auto-selection in line editing mode */
		UpdateInputArea (cxin);
		return (0);
	    }
	    /* no match */

	    /* just clean the choice-list and announce nomatch */
	    HZclReset (cl);
	    HZiaSetInBuf (ia, cxin->hzinbuf, cxin->hzinbufCount, cxin->cursor);
	    HZprintfMsg (" empty selection list (bad key deletion)");
	    return (0);

	}

	Bell ();
	return (0);

      case HZ_KEY_EDIT_KILL:

	HZimReset (cxin, True);
	return (0);

      case HZ_KEY_EDIT_BEGIN:

	if (cxin->cursor != 0) {
	    cxin->cursor = 0;
	    HZiaSetInBuf(ia, inbuf, cxin->hzinbufCount, cxin->cursor);
	    HZiaFlush (ia, cxin->screen);
	}
	return (0);

      case HZ_KEY_EDIT_END:

	if (HZinLineEditMode(cxin)) {	/* when not at the end */
	    cxin->cursor = -1;
	    HZiaSetInBuf(ia, inbuf, cxin->hzinbufCount, cxin->cursor);
	    HZiaFlush (ia, cxin->screen);
	}
	return (0);

      case HZ_KEY_EDIT_FORW:

	if (HZinLineEditMode(cxin)) {	/* when not at the end */
	    cxin->cursor++;
	    if (cxin->cursor >= cxin->hzinbufCount)
		cxin->cursor = -1;
	    HZiaSetInBuf(ia, inbuf, cxin->hzinbufCount, cxin->cursor);
	    HZiaFlush (ia, cxin->screen);
	}
	return (0);

      case HZ_KEY_EDIT_BACK:

	if (! HZinLineEditMode(cxin))	/* when at the end */
	    cxin->cursor = cxin->hzinbufCount - 1;
	else if (cxin->cursor > 0)
	    cxin->cursor--;
	else
	    return (0);
	HZiaSetInBuf (ia, inbuf, cxin->hzinbufCount, cxin->cursor);
	HZiaFlush (ia, cxin->screen);
	return (0);


      case HZ_KEY_RIGHT:

	was_exhaust = cl->exhaust;
	if (HZclEmpty (cl))
	    return (1);		/* nothing to move, pass the key out */
	if (HZclMoveForward (cl, ia, cxin->cur_ic) == 0) {
	    Bell ();		/* no more choice to the right */
	    if (was_exhaust)
		return (0);
	    /* if not exhausted before, redisplay the current choice list */
	}
	HZiaSetChoiceList (ia, cl);
	HZiaFlush (ia, cxin->screen);
	return (0);

      case HZ_KEY_LEFT:

	if (HZclEmpty (cl))
	    return (1);		/* nothing to move, pass the key out */
	if (HZclMoveBackward (cl, ia, cxin->cur_ic) == 0) {
	    Bell ();
	    return (0);		/* no more choice to the left */
	}
	HZiaSetChoiceList (ia, cl);
	HZiaFlush (ia, cxin->screen);
	return (0);


      case HZ_KEY_REPEAT:

	if (cxin->history.inbuflen == 0) {
	    /* nothing to repeat, pass the key out */
	    CleanAssocDisplay();
	    return (1);
	}

	HZrestoreHistory (cxin);
	if (! cxin->cur_ic)
	    return (1);		/* nothing to repeat, pass the key out */

	/* we have shown choice before, so we need to restart the context */
	(void) HZrestartIC (cxin->cur_ic);
	UpdateInputArea (cxin);
	return (0);


      default:	/* unknown key, just pass out */
	return (1);	
    }

#undef CleanAssocDisplay
} /* hzTableFilter() */


/*
 * Match input key that is just appended to the end of the input buffer
 */
static int InputMatchAtEnd(CXtermInputModule *cxin)
{
  HZInputContext *ic;

    ic = HZinputSearch (cxin, cxin->hzinbuf[cxin->hzinbufCount], cxin->cur_ic);
    if (! ic)
	return (0);

    if (HZmatchIC (ic) != IC_OK) {
	FreeIC (ic);	/* get ride of this IC */
	return (0);
    }
    if (cxin->cache_ics[cxin->hzinbufCount])
	FreeIC (cxin->cache_ics[cxin->hzinbufCount]);
    cxin->cache_ics[ cxin->hzinbufCount++ ] = ic;
    cxin->cur_ic = ic;
    return (1);
}

/*
 * Match the rest of the input buffer starting from given position
 */
static int InputMatchFromPos(CXtermInputModule *cxin, int pos)
                            
            			/* position to start match */
{
  HZInputContext *ic;

    if (pos == 0)
	ic = NULL;
    else
	ic = cxin->cache_ics[ pos - 1 ];

    while (pos < cxin->hzinbufCount) {
	ic = HZinputSearch (cxin, cxin->hzinbuf[pos], ic);
	if (cxin->cache_ics[pos])
	    FreeIC (cxin->cache_ics[pos]);
	cxin->cache_ics[ pos++ ] = ic;
    }
    cxin->cur_ic = ic;
    if (HZmatchIC(ic) == IC_OK)
	return 1;
    else
	return 0;
}

/*
 * Try adding an artificial associate key before the user input keystroke
 */
static int AutoSegmentation(CXtermInputModule *cxin, char ch)
{
  HZInputContext *ic = cxin->cur_ic;

    /* No auto segmentation if ... */
    if ( (!ic) ||
	/* ... we are not in any of these three type of ic ... */
	((ic->type != ic_type_Trie) && (ic->type != ic_type_Trie_wc)
		&& (ic->type != ic_type_Assoc)) ||
	/* ... the input method doesn't allow so ... */
	((! ic->im) || (! ic->im->m.do_auto_segment)) ||
	/* ... we are not at the end of the key buffer. */
	(cxin->cursor != -1) )
	    return (0);

    /* insert an association key. */
    cxin->hzinbuf[ cxin->hzinbufCount ] = ic->im->k.def_assockey;
    ic = HZinputSearch (cxin, ic->im->k.def_assockey, ic);
    if (! ic)
	return (0);
    if (HZmatchIC (ic) != IC_OK) {
	FreeIC (ic);
	return (0);
    }
    if (cxin->cache_ics[cxin->hzinbufCount])
	FreeIC (cxin->cache_ics[cxin->hzinbufCount]);
    cxin->cache_ics[ cxin->hzinbufCount++ ] = ic;

    /* followed by the user keystroke */
    cxin->hzinbuf[ cxin->hzinbufCount ] = ch;
    ic = HZinputSearch (cxin, ch, ic);
    if (! ic) {
	cxin->hzinbufCount--;	/* offset the bonus association key */
	return (0);	
    }
    if (HZmatchIC (ic) != IC_OK) {
	cxin->hzinbufCount--;	/* offset the bonus association key */
	FreeIC (ic);
	return (0);
    }
    if (cxin->cache_ics[cxin->hzinbufCount])
	FreeIC (cxin->cache_ics[cxin->hzinbufCount]);
    cxin->cache_ics[ cxin->hzinbufCount++ ] = ic;

    cxin->cur_ic = ic;
    return (1);
}

/*
 * Do the choice list selection action
 */
static int DoSelectionKey(short unsigned int ktype, CXtermInputModule *cxin, char *strbuf)
                         
                            
                  		/* the generated character(s) goes here */
{
  int sel;
  XChar2b *hzc;
  short int nhz;

    sel = (ktype & HZ_KEY_SELECTION_NUM);
    nhz = HZclSelect (&cxin->hzcl, sel, &hzc);
    if (nhz == 0)		/* nothing to select, pass the key out */
	return (1);
    if (nhz < 0) {		/* invalid selection */
	Bell();
	return (0);
    }

    /* after a choice has been made */
    DoPostSelection(cxin, hzc, nhz);

    strncpy (strbuf, (char *)hzc, nhz*2);	/* pass it out */
    return (nhz * 2);
}

/*
 * fill up the on-screen selection list
 */
static void FetchSelection(CXtermInputModule *cxin)
{
    HZclReset (&cxin->hzcl);		/* clear the choice list first */

    /* get the first N candidates on the choice list */
    (void) HZclMakeSelection (&cxin->hzcl, &cxin->hzia, cxin->cur_ic);
}

/*
 * We got a match, fill the choice list and update the input area
 */
static void UpdateInputArea (CXtermInputModule *cxin)
{
  HZInputArea *ia = &cxin->hzia;
  HZChoiceList *cl = &cxin->hzcl;

    FetchSelection(cxin);

    HZiaSetInBuf(ia, cxin->hzinbuf, cxin->hzinbufCount, cxin->cursor);
    if (! HZclEmpty(cl))
	HZiaSetChoiceList (ia, cl);
    else
	HZprintfMsg (" empty selection list.");
    HZiaSetStatus (ia, "");
    HZiaFlush(ia, cxin->screen);
}

/*
 * Fill the choice list, update the input area, and allow auto-selection
 */
static int UpdateIAwAutoSelect (CXtermInputModule *cxin, char *strbuf)
{
  HZInputArea *ia = &cxin->hzia;
  HZChoiceList *cl = &cxin->hzcl;

    FetchSelection (cxin);

    /* If there is only one candidate and if "auto-select" is yes */
    if (HZclUniq (cl) &&
	(cxin->chzim->m.auto_select == HZ_AUTOSELECT_ALWAYS))
    {
	XChar2b *hzc;
	short int nhz;

	nhz = HZclSelect (cl, 0, &hzc);
	strncpy (strbuf, (char *)hzc, nhz*2);

	/* after a chooice has been made */
	DoPostSelection (cxin, hzc, nhz);

	return (nhz * 2);
    } 

    HZiaSetInBuf (ia, cxin->hzinbuf, cxin->hzinbufCount, cxin->cursor);
    if (! HZclEmpty (cl))
	HZiaSetChoiceList (ia, cl);
    else
	HZprintfMsg (" empty selection list.");
    HZiaSetStatus (ia, "");
    HZiaFlush (ia, cxin->screen);
    return (0);
}

/*
 * After a choice is made:  save in history, do association, clear the input
 */
static void DoPostSelection(CXtermInputModule *cxin, XChar2b *hzc, short int nhz)
{
    HZsaveHistory (cxin);

    /* save this input */
    if (nhz >= MAX_SAVEINPUT) {
	memmove( cxin->latestInput, hzc + nhz - MAX_SAVEINPUT,
		 MAX_SAVEINPUT * sizeof(XChar2b) );
    } else {
	memmove( cxin->latestInput, cxin->latestInput + nhz, 
		 (MAX_SAVEINPUT - nhz) * sizeof(XChar2b) );
	memmove( cxin->latestInput + MAX_SAVEINPUT - nhz, hzc,
		 nhz * sizeof(XChar2b) );
    }

    /* For check of key in new inputMethod */

    if (cxin->chzim->m.do_ps_assoc && hzc) {	/* we want association input */
	HZInputContext *ic;

	ic = HZassocSearch(cxin, nhz <= MAX_SAVEINPUT ? nhz : MAX_SAVEINPUT);
	if (ic) {
	    HZInputArea *ia = &cxin->hzia;

	    /* only clear up the counter, no need to do full HZimReset  */
	    cxin->hzinbufCount = 0;
	    cxin->cursor = -1;

	    if (cxin->cache_ics[0])
		FreeIC (cxin->cache_ics[0]);
	    cxin->cache_ics[0] = ic;
	    cxin->cur_ic = ic;

	    /* no auto selection in association */
	    (void) FetchSelection (cxin);
	    HZiaSetInBuf (ia, "", 0, -1);
	    HZiaSetChoiceList (ia, &cxin->hzcl);
	    HZiaSetStatus (ia, (char *)cxin->hzal->prompt);
	    HZiaFlush (ia, cxin->screen);

	    /* The associate list is in display, don't HZimReset! */
	    return;
	}
    }

    HZimReset (cxin, True);
}

