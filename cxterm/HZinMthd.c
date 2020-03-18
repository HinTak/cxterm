/*
 *	$Id: HZinMthd.c,v 1.3 2003/05/06 07:40:19 htl10 Exp $
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
 * This file defines some Input Module management routines
 */

#include "HZinput.h"

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <sys/param.h>
#include "data.h"

/* Global variable for toggle-key pressing */
int     g_WaitingToggle = 0;


static void HZinitHistory(CXtermInputModule *cxin);
static void HZcleanHistory(CXtermInputModule *cxin);
static int HZLoadSimpleInputMethod(CXtermInputModule *cxin, char *name, char *inputdir, char *encode, HZInputMethod *hzim);
static void HZimParamCheck(CXtermInputModule *cxin, HZInputMethod *hzim);

/*
 * Initialize the input module
 */
void HZimInit (CXtermInputModule *cxin, XtermWidget xterm, TScreen *screen)
{
    cxin->xterm = xterm;
    cxin->encode = HZencode (xterm->misc.hz_encoding);

    cxin->numHZim = 0;
    cxin->hzinbufCount = 0;
    cxin->cursor = -1;

    HZclInit (&cxin->hzcl);

    cxin->mode = 0;
    cxin->screen = screen;

    InitICpool ();
    HZinitHistory (cxin);

    HZLoadBuiltin (cxin);
    cxin->hzal = NULL;				/* will load later */

    cxin->temp_disable = False;
    cxin->global.do_auto_segment = False;	/* not have AssocList yet */
    cxin->global.do_ps_assoc = False;		/* not have AssocList yet */
}

/*
 * Reset the input module, ready for next keystroke sequence
 */
void HZimReset (CXtermInputModule *cxin, Boolean dorefresh)
                            
#if NeedWidePrototypes
                           
#else
                      
#endif
{
    cxin->hzinbufCount = 0;
    cxin->cursor = -1;
    cxin->cur_ic = NULL;

    HZclReset (&cxin->hzcl);


    HZiaSet (&cxin->hzia, &cxin->hzcl, "", 0, -1, "");	/* reset */

    if (dorefresh)
	HZiaFlush (&cxin->hzia, cxin->screen);
}


/* The following function is added by Changsen Xu, so as to see various
 * Input-Keys of the latest one CJK character.
 */
void showLatestHZKeysInNewMethod (CXtermInputModule *cxin)
{
    char tmpHzKeys[100]="", tmpHz[3]="";

    if ( cxin->latestInput[MAX_SAVEINPUT-1].byte1 != '\0' &&
	 cxin->chzim->im_hztbl != NULL ) {
	tmpHz[0] = cxin->latestInput[MAX_SAVEINPUT-1].byte1;
	tmpHz[1] = cxin->latestInput[MAX_SAVEINPUT-1].byte2;
	tmpHz[2] = '\0';

	Trie_Match_HZ (0, 0, cxin->chzim->im_hztbl, tmpHzKeys, tmpHz);

	/* display in InputArea */
	HZclAddChoice(&cxin->hzcl,&cxin->latestInput[MAX_SAVEINPUT-1], 1);
	cxin->hzcl.numChoices = 1;
	cxin->hzcl.selPos = 0;
	cxin->hzcl.selNum = 1;
	cxin->hzcl.exhaust = True;
	HZiaSetChoiceList(&cxin->hzia, &cxin->hzcl);
	HZiaSetInBuf (&cxin->hzia, tmpHzKeys, strlen(tmpHzKeys), -1);	
    }
}



/*
 * Clear up the input module, discard all loaded external input methods.
 */
void HZimCleanUp (CXtermInputModule *cxin)
{
  int i;

    HZclCleanUp (&cxin->hzcl);

    if (cxin->hzal) {
	UnloadAssocList (cxin->hzal);
	cxin->hzal = NULL;
	cxin->global.do_auto_segment = False;
	cxin->global.do_ps_assoc = False;
    }

    for (i = 0; i < cxin->numHZim; i++) {
	HZInputMethod *hzim = &(cxin->imtbl[i]);

	switch (hzim->type) {
	  case im_type_Simple:
	    UnloadCIT (hzim->im_hztbl);
	    hzim->im_hztbl = NULL;
	    FreeHZIdx (hzim->im_hzidx);
	    hzim->im_hzidx = NULL;
	    XtFree(hzim->name);
	    hzim->name = NULL;
	    cxin->numHZim --;
	    break;
	  default:
	    break;
	}
	hzim->type = im_type_NoType;
    }

    HZcleanHistory (cxin);
}


/* Initialize the conversion history (currently history==1) */
static void HZinitHistory (CXtermInputModule *cxin)
{
  int i;

    cxin->cur_ic = NULL;
    for (i = 0; i < MAX_INBUF; i++)
	cxin->cache_ics[i] = NULL;

    cxin->history.inbuflen = 0;
    for (i = 0; i < MAX_INBUF; i++)
	cxin->history.ics[i] = NULL;
}

/* Clean the input conversion history buffer */
static void HZcleanHistory (CXtermInputModule *cxin)
{
  int i;

    for (i = 0; i < MAX_INBUF; i++)
	if (cxin->history.ics[i]) {
	    FreeIC (cxin->history.ics[i]);
	    cxin->history.ics[i] = NULL;
	}
    for (i = 0; i < MAX_INBUF; i++)
	if (cxin->cache_ics[i]) {
	    FreeIC (cxin->cache_ics[i]);
	    cxin->cache_ics[i] = NULL;
	}
    cxin->cur_ic = NULL;
}

/*
 * Save the current input conversion into the history
 */
void HZsaveHistory (CXtermInputModule *cxin)
{
  int i;

    for (i = 0; i < cxin->hzinbufCount; i++) {
	cxin->history.inbuf[i] = cxin->hzinbuf[i];
	if (cxin->history.ics[i])
	    FreeIC (cxin->history.ics[i]);
	cxin->history.ics[i] = cxin->cache_ics[i];
	cxin->cache_ics[i] = NULL;
    }
    cxin->history.inbuflen = cxin->hzinbufCount;
}

/*
 * Retrieve the previous input conversion from the history
 */
void HZrestoreHistory (CXtermInputModule *cxin)
{
  int i;

    for (i = 0; i < cxin->history.inbuflen; i++) {
	cxin->hzinbuf[i] = cxin->history.inbuf[i];
	if (cxin->cache_ics[i])
	    FreeIC (cxin->cache_ics[i]);
	cxin->cache_ics[i] = cxin->history.ics[i];
	cxin->history.ics[i] = NULL;
    }
    cxin->hzinbufCount = cxin->history.inbuflen;
    cxin->history.inbuflen = 0;
    if (cxin->hzinbufCount > 0)
	cxin->cur_ic = cxin->cache_ics[ cxin->hzinbufCount - 1 ];
    cxin->cursor = -1;
}


/*
 * Switch Hanzi input module
 */
void HZswitchModeByNum (CXtermInputModule *cxin, int num)
{   
    static long oldTime=-1;
    struct timeval curTimeStruct;
    long thisTime;
    Boolean doShowKeys = False;

    if (num >= cxin->numHZim)
	return;

    /* Some UNIX need a second timeZone argument, some not */
    gettimeofday(&curTimeStruct, (struct timezone*)0);

    /* get time in milliseconds */
    thisTime = 1000*curTimeStruct.tv_sec + curTimeStruct.tv_usec/1000;

    /* forget about real switch of different methods */
    if (cxin->mode != num || oldTime == -1) 
	oldTime = thisTime;
    /* only remember time of double same-method "switch" -- no switch at all */
    else {
	if (thisTime - oldTime < 500) { /* 500 is half a second */
	    doShowKeys = True;
	    oldTime = -1; /* Don't remember this time */
	} else 
	    oldTime = thisTime;
    }

    /* switch cxterm HZ state to mode i */
    cxin->mode = num;
    cxin->chzim = &(cxin->imtbl[num]);
    cxin->chzif = cxin->imtbl[num].hzif;
    HZimParamCheck (cxin, cxin->chzim);
    HZiaSetFrame(&(cxin->hzia), cxin->chzim);
    HZimReset (cxin, False);		/* don't refresh the screen now */

    if (doShowKeys)
	showLatestHZKeysInNewMethod(cxin);
}

int HZLoadInputMethod (CXtermInputModule *cxin, char *name, char *inputdir, char *encode, HZInputMethod *hzim)
{
    return (HZLoadSimpleInputMethod (cxin, name, inputdir, encode, hzim));
}

/*
 * HZLoadSimpleInputMethod -- load simple input method
 */
static int HZLoadSimpleInputMethod (CXtermInputModule *cxin, char *name, char *inputdir, char *encode, HZInputMethod *hzim)
{
    hzim->type = im_type_Simple;

    hzim->im_hztbl = (HZinputTable *) malloc (sizeof(HZinputTable));
    if (! hzim->im_hztbl) {
	HZprintfMsg ("No memory to load input method for %s", name);
	return (-1);
    }
    if (LoadCIT (name, inputdir, encode, hzim->im_hztbl) < 0) {
	(void) free ((char *)(hzim->im_hztbl));
	return (-1);
    }
    hzim->hzif = hzTableFilter;

    hzim->name = XtNewString(name);

    hzim->f.prompt = hzim->im_hztbl->prompt;
    hzim->f.lenPrompt = hzim->im_hztbl->lenPrompt;
    hzim->f.keyprompt = hzim->im_hztbl->keyprompt;
    hzim->f.choicelb = hzim->im_hztbl->choicelb;
    hzim->f.maxchoice = hzim->im_hztbl->maxchoice;

    hzim->k.keytype = hzim->im_hztbl->keytype;
    hzim->k.def_assockey = hzim->im_hztbl->defAssocKey;

    hzim->m.auto_select = hzim->im_hztbl->autoSelect;
    hzim->m.do_auto_segment = True;	/* will be correct below */
    hzim->m.do_ps_assoc = True;		/* will be correct below */

    HZimParamCheck (cxin, hzim);

    /* Build HZIdx, only if we do have association list and association key */
    if (cxin->hzal && hzim->k.def_assockey)
	hzim->im_hzidx = MakeHZIdx(hzim->im_hztbl);
    else 
	hzim->im_hzidx = NULL;

    return (0);
}


/***************************** conversion parameters ***********************/

static void set_auto_select(CXtermInputModule *cxin, char *strval)
{
    switch (tolower(strval[0])) {
      case 'n':
      case 'f':
	cxin->chzim->m.auto_select = HZ_AUTOSELECT_NEVER;
	break;
      case 'a':
      case 'y':
      case 't':
	cxin->chzim->m.auto_select = HZ_AUTOSELECT_ALWAYS;
        break;
      case 'w':
	cxin->chzim->m.auto_select = HZ_AUTOSELECT_WHENNOMATCH;
        break;
      default:
	HZprintfMsg ("unknown autoselect value: %s", strval);
    }
}

static void set_do_auto_segment(CXtermInputModule *cxin, char *strval)
{
    switch (tolower(strval[0])) {
      case 'n':
      case 'f':
	cxin->chzim->m.do_auto_segment = False;
	break;
      case 'y':
      case 't':
	cxin->chzim->m.do_auto_segment = True;
        break;
      default:
	HZprintfMsg ("unknown composition value: %s", strval);
    }
}

static void set_do_ps_assoc(CXtermInputModule *cxin, char *strval)
{
    switch (tolower(strval[0])) {
      case 'n':
      case 'f':
	cxin->chzim->m.do_ps_assoc = False;
	break;
      case 'y':
      case 't':
	cxin->chzim->m.do_ps_assoc = True;
        break;
      default:
	HZprintfMsg ("unknown association value: %s", strval);
    }
}

static void set_do_input_conv(CXtermInputModule *cxin, char *strval)
{
  Boolean enable_mode;

    if (tolower (strval[0]) == 'w' && tolower (strval[1]) == 'a') {     /* WAit-toggle */
        g_WaitingToggle = -1;
        return;
    }
    switch (tolower(strval[1])) {
      case 'n':	/* eNable, oN */
      case 'r':	/* tRue*/
	enable_mode = True;
	break;
      case 'a':	/* fAlse */
      case 'f':	/* oFf */
      case 'i':	/* dIsable */
	enable_mode = False;
	break;
      case 'o':	/* tOggle */
        if (! g_WaitingToggle) {
            return;
        }
	if (cxin->temp_disable)
	    enable_mode = True;
	else
	    enable_mode = False;
	break;
      default:
	HZprintfMsg ("unknown input-enable value: %s", strval);
	return;
    }
    if (enable_mode && cxin->temp_disable) {
	cxin->temp_disable = False;
	HZiaSetSensitive (&cxin->hzia, True);
	HZiaRedraw (&cxin->hzia, cxin->screen);
    } else if (!enable_mode && !cxin->temp_disable && (cxin->mode != 0)) {
	/* not to disable ASCII mode */
	cxin->temp_disable = True;
	HZiaSetSensitive (&cxin->hzia, False);
	HZiaRedraw (&cxin->hzia, cxin->screen);
    }
}

static struct param_action {
    char *prefix;
    void (*fun)(
#if NeedFunctionPrototypes
#if NeedNestedPrototypes
	CXtermInputModule *	/* cxin */,
	char *			/* strval */
#endif
#endif
    );
} ParamAct [] = {
    { "auto-select=", set_auto_select },
    { "auto-selection=", set_auto_select },
    { "auto-segment=", set_do_auto_segment },
    { "auto-segmentation=", set_do_auto_segment },
    { "assoc=", set_do_ps_assoc },
    { "association=", set_do_ps_assoc },
    { "input-conv=", set_do_input_conv },
    { "input-conversion=", set_do_input_conv },
    { NULL, NULL },
};

void HZimSetParam (CXtermInputModule *cxin, char *str)
{
  struct param_action *p = ParamAct;

    while (p->prefix) {
	int len = strlen(p->prefix);
	if (strncmp(str, p->prefix, len) == 0) {
	    (*p->fun)(cxin, str + len);
	    HZimParamCheck (cxin, cxin->chzim);
	    return;
	}
	p++;
    }
    HZprintfMsg ("unknown setting: %s", str);
}

static void HZimParamCheck (CXtermInputModule *cxin, HZInputMethod *hzim)
{
    if ((! cxin->global.do_auto_segment) || (! hzim->k.def_assockey))
	hzim->m.do_auto_segment = False;
    if ((! cxin->global.do_ps_assoc) || (! hzim->k.def_assockey))
	hzim->m.do_ps_assoc = False;
}
