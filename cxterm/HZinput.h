/*
 *	$Id: HZinput.h,v 1.2 2003/05/06 07:40:19 htl10 Exp $
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

#ifndef _HZINPUT_H_
#define _HZINPUT_H_

/*
 * types imported from xterm (ptyx.h):
 *	typedef	unsigned char Char;
 *	typedef	struct { ... } TScreen;
 *	typedef	struct { ... } Misc;
 *	typedef	struct { ... } *XtermWidget;
 *
 * types imported from X window:
 *	typedef ... Boolean;
 */

#include "ptyx.h"
#include "HZtable.h"
#include <X11/Xos.h>

#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#else
extern char *malloc(), *calloc(), *realloc();
extern char *getenv();
#endif


#if NeedFunctionPrototypes
# if NeedWidePrototypes
#  define FP_hzFilter	struct _CXtermInputModule * /* cxin */,	\
			int /* ch */,  char /* strbuf */ []
# else
#  define FP_hzFilter	struct _CXtermInputModule * /* cxin */,	\
			char /* ch */,  char /* strbuf */ []
# endif
#else
# define FP_hzFilter
#endif
#if NeedNestedPrototypes
  typedef struct _CXtermInputModule * _Forward_definition_but_useless;
# define FPn_hzFilter	FP_hzFilter
#else
# define FPn_hzFilter
#endif


#define MAX_HZIM	24	/* maxinum number of input methods allowed */
#define MAX_INBUF	32	/* maximum length of input conversion buffer */
#define MAX_ASSOC	8	/* maximum number of segment in association */
#define MAX_SAVEINPUT	4	/* maximum number of last input character */
#define	MAX_HZ_NUM	(1<<15)	/* maximum number of hanzi character */

/******************** INPUT METHOD ********************/

typedef enum {
    im_type_NoType,	/* no type */
    im_type_Builtin,	/* Builtin */
    im_type_Simple,	/* Simple input method (e.g. trie) */
    im_type_Unknown
} HZimType;

typedef struct {
    int idxlist[MAX_HZ_NUM];	/* hanzi index list */
    int *oflist;		/* overflow list */
    int oflistlen;		/* overflow list length */
} HZimHZListIndex;

#define HZSUBSCRIPT(phz)	((((phz)->byte1 & 0x007f) << 8) + (phz)->byte2)


typedef struct _HZInputMethod {

    HZimType type;
    char *name;			/* name of the input method */
    int (*hzif)(FPn_hzFilter);	/* input filter (builtin or generic) */

    /* frame */
    struct {
	Char *prompt;
	int lenPrompt;

	keyPrompt *keyprompt;		/* display this for the key */

	char *choicelb;			/* HZ choice label */
	int maxchoice;			/* maximum number of HZ choice */
    } f;

    /* input key definitions */
    struct {
	unsigned short *keytype;	/* type of user keystroke */
	char def_assockey;		/* a default association key */
    } k;

    /* input conversion mode */
    struct {
	unsigned char auto_select;	/* auto-selection mode */
	Boolean do_auto_segment;	/* allow auto-segmentation? */
	Boolean do_ps_assoc;		/* allow post-selection association? */
    } m;

    union {
	struct {
	    HZinputTable *im_InputTable;
	    HZimHZListIndex *im_HZlistIndex;
	} im_Simple;
    } u;

} HZInputMethod;

#define	im_hztbl	u.im_Simple.im_InputTable
#define	im_hzidx	u.im_Simple.im_HZlistIndex
#define	im_keytype	hztbl->keytype

/******************** CHOICE LIST ********************/

/*
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

struct hz_choice {
    XChar2b *hzc;		/* pointer to the HZ choice */
    short int nhz;		/* number of HZ in the choice (if a phrase) */
};

typedef struct _HZChoiceList {

    int numChoices;		/* the number of choices in the choice list */
    Boolean exhaust;		/* the list is exhausted, no more choice */
    struct hz_choice *choices;	/* the list of choices (hz or hz phrases) */
    int num_alloc;		/* the maximum number of records in the list */

    int selPos;			/* position of on-screen selection window */
    int selNum;			/* the number of on-screen selections */

} HZChoiceList;

#define	HZclEmpty(cl)	((cl)->numChoices == 0)
#define	HZclUniq(cl)	((cl)->exhaust && ((cl)->numChoices == 1))

/******************** ASSOCIATION LIST ********************/

typedef	struct _HZAssocList {
    int numPhrase;		/* number of phrases */
    int maxhz;			/* maximum number of hanzi in a phrases */
    Char *phrases;		/* the long string of hanzi phrases */
    Char prompt[MAX_PROMPT+1];	/* to declare the association input */
} HZAssocList;

/******************** INPUT CONTEXT ********************/

/*
 * An input context is a data structure used in matching the keystroke against
 * a certain input method.
 */

typedef enum {
    ic_type_NoType,	/* no type */
    ic_type_Builtin,	/* Builtin */
    ic_type_Trie,	/* TRIE */
    ic_type_Trie_wc,	/* TRIE with wildcard */
    ic_type_Assoc,	/* Association Input */
    ic_type_Assoc_ps,	/* Association after selection */
    ic_type_Unknown
} HZicType;

/* wildcard sub-context */
typedef  struct _HZwildcardContext {
    int wclen;				/* length of the wildcard suffix */
    char wildcard[MAX_INBUF+1];		/* inbuf suffix with wildcard */
    char repcode[MAX_INBUF+1];		/* the repcode suffix */

    int depth;			/* the current depth in the DFS traversal */
    trieNode *tNstack[MAX_INBUF+1];	/* the trieNode stack */
    short int tNnumSb[MAX_INBUF+1];	/* number of sibling trieNode */

    XChar2b *topHZend;			/* the end pointer for HZ list */
} HZwildcardContext;

/* association sub-context */
typedef struct _HZassocContext {

    int numSeg;				/* number of segment */
    struct _HZInputContext *subICs[MAX_ASSOC];
    struct _HZInputContext *prev_ic;	/* the previous one incrementally */
    HZChoiceList ChList;		/* a cache for incremental search */

    /* This is a sorted list of subICs number in the order of derivability.
     * It is used in searching for the next candidate in Assoc context.
     * When we check if a phrase in the association list against the input,
     * we always start from those with smaller positive derivability.
     * I hope this little optimization could improve response time.
     */
    char sortlist[MAX_ASSOC];	
    int sortlistlen;

} HZassocContext;

typedef struct _HZInputContext {

    HZicType type;

    HZInputMethod *im;
    HZAssocList *al;

    char *keysq;	/* the input key-stroke sequence buffer */
    int matched;	/* how many of the keysq have been matched */
    int pending;	/* how many have to be search later */

    union {

/* Trie */

	struct {
	    trieNode *ic_TrieNodePtr;	/* trieNode of the matched inbuf */
	    XChar2b *ic_hzListEnd;	/* HZ choices ended here */
	    XChar2b *ic_hzListPtr;	/* HZ choices to be selected */

		/* total number of choices, and counter for the choice list */
	    unsigned short int ic_HZchoiceTotal;
	    unsigned short int ic_HZchoiceCounter;

		/* for trie context with wildcards */
	    HZwildcardContext* ic_WcContext;
	} ic_Trie;
#	define	tCurTNptr	u.ic_Trie.ic_TrieNodePtr
#	define	tCurHZend	u.ic_Trie.ic_hzListEnd
#	define	tCurHZptr	u.ic_Trie.ic_hzListPtr
#	define	tHZtotal	u.ic_Trie.ic_HZchoiceTotal
#	define	tHZcount	u.ic_Trie.ic_HZchoiceCounter
#	define	tWcCntx		u.ic_Trie.ic_WcContext

/* Association (inter-word association) */

	struct {
	    HZassocContext *ic_AssocContext;
	    struct hz_choice *ic_ptrThisChoices;
	    struct _HZInputContext *ic_ptrIC;
	    struct hz_choice *ic_ptrPrevChoices;
	    Char *ic_ptrPhrases;
	} ic_Assoc;
#	define	aAsCntx		u.ic_Assoc.ic_AssocContext
#	define	aPtrThisCh	u.ic_Assoc.ic_ptrThisChoices
#	define	aPtrIC		u.ic_Assoc.ic_ptrIC
#	define	aPtrPrevCh	u.ic_Assoc.ic_ptrPrevChoices
#	define	aPtrPhrases	u.ic_Assoc.ic_ptrPhrases

/* Association (post-selection) */

	struct {
	    XChar2b *ic_latestInput;
	    Char *ic_ptrList;
	    short int ic_lastInputNum;
	} ic_Assoc_ps;
#	define	aLatestInput	u.ic_Assoc_ps.ic_latestInput
#	define	aPtrList	u.ic_Assoc_ps.ic_ptrList
#	define	aLastInNum	u.ic_Assoc_ps.ic_lastInputNum

    } u;

    int derivability;	/* a hint on how many candidates it might generate */

} HZInputContext;

#define IC_FAIL	0
#define IC_OK	1

#define IC_EMPTYSET	(0)
#define IC_WHOLESET	(-1)
#define IC_UNCERTAIN	(-2)

#define IC_isAssoc(ic)		((ic)->type == ic_type_Assoc)
#define IC_isAssocPs(ic)	((ic)->type == ic_type_Assoc_ps)
#define IC_isTrie(ic)		((ic)->type == ic_type_Trie)
#define IC_isTrieWc(ic)		((ic)->type == ic_type_Trie_wc)
#define IC_isNoType(ic)		((ic)->type == ic_type_NoType)
#define IC_isBultin(ic)		((ic)->type == ic_type_Bultin)

/******************** INPUT AREA ********************/

/*
 * The input area has 4 portions: prompt, input buffer, status,
 * and selection list, in the following layout:
 *
 * 		------------------------------------------
 * line 1:	<prompt><input_buffer>            <status>
 * line 2:	<selection_list or message>
 *
 * The prompt, status, and selection list are usually redrawed,
 * but the input buffer can be partially updated.
 */

#define	MAX_IA_ROWS	ROWSINPUTAREA		/* from "ptyx.h" */

#define	MAX_IA_WIDTH	256	/* the input area can be at most this wide */
#define MAX_DPYINBUF    ( MAX_IA_WIDTH - MAX_PROMPT )
#define	MAX_STATUS	10	/* the input area can be at most this wide */

typedef struct _HZInputArea {

    int cols, rows;			/* number of cols and rows */

    struct mini_screen {
	Char *text[MAX_IA_ROWS];	/* array for text */
	Char *dpy_attr[MAX_IA_ROWS];	/* attributes for drawing text */
	Char *click_attr[MAX_IA_ROWS];	/* attributes for mouse click */
    } buffer, screen;

    int lenPrompt;			/* length of the prompt portion */
    int lenInbuf;			/* length of the inbuf portion*/
    int lenStatus;			/* length of the status portion*/
    int lenSelection;			/* length of the selection portion */

    /* for making choice list */
    int maxchoice;		/* the maximum number of selections allowed */
    char *choicelb;		/* the labels for the selections */
    keyPrompt *keyprompt;	/* the echo labels for the keystroke */

    Boolean sensitive;		/* used with cxtermInput.temp_disable */

} HZInputArea;

/* width of an n-HZ (phrase) selection on screen (3 is the len of " l.") */
#define	SelWidthOnScr(n)	(n)*2+3

/* the width "w" of the selection string for display is too long? */
#define	OverflowSelStr(w,ia,cl)	(((cl)->selNum >= 1) && ((w) > (ia)->cols))

#define HZiaSetSensitive(ia,s)	{(ia)->sensitive = (s);}

/* attributes of the display selection string */
#define IA_Attr_Noop			' '
#define IA_Attr_PickMe			'S'
#define IA_Attr_PickThis		's'
#define IA_Attr_MoveLeft		'<'
#define IA_Attr_MoveRight		'>'
#define IA_Attr_Menu			'm'
#define IA_Attr_Bell			'!'
#define IA_Attr_InputCursorMask		0x80


/******************** INPUT MODULE ********************/

typedef struct _CXtermInputModule {

    XtermWidget xterm;
    TScreen *screen;
    Boolean realized;		/* is the cxterm window realized? */

    int encode;			/* hanzi encoding */

    int numHZim;		/* number of HZ input methods loaded */
    HZInputMethod imtbl[MAX_HZIM];	/* input method definition */

    int mode;			/* current HZ mode (HZ input method number) */
    HZInputMethod *chzim;	/* current HZ input method pointer */
    int (*chzif)(FPn_hzFilter);	/* current HZ input filter function pointer */

    HZChoiceList  hzcl;		/* the choice-list */
    HZInputArea  hzia;		/* input area data structure */
				/* (including the on-screen selection list) */

    HZAssocList *hzal;		/* the association phrase list */

    char hzinbuf[MAX_INBUF];	/* input buffer for keystrokes */
    int hzinbufCount;		/* lenght of keystroks buffer */
    int cursor;			/* for line editing */

    Boolean temp_disable;	/* is the cxterm input temporarily disable */

    /* global input conversion mode */
    struct {
	unsigned char auto_select;	/* auto-selection mode */
	Boolean do_auto_segment;	/* allow auto-segmentation? */
	Boolean do_ps_assoc;		/* allow post-selection association? */
    } global;

    /* cache to facilitate incremental input search */
    HZInputContext *cache_ics[MAX_INBUF];
    HZInputContext *cur_ic;	/* the current input context */

    /* history */
    struct {
	int	inbuflen;
	char	inbuf[MAX_INBUF];
	HZInputContext *ics[MAX_INBUF];
    } history;

    XChar2b latestInput[MAX_SAVEINPUT];	/* latest input characters */
} CXtermInputModule;

#define HZinLineEditMode(cxin)	((cxin)->cursor != -1)

/******************** EXTERNAL FUNCTIONS ********************/

#include "HZfunpro.h"		/* function prototypes */

/******************** VARIABLES ********************/

extern CXtermInputModule cxtermInput;
extern void Trie_Match_HZ (int top, unsigned int trieIdx, HZinputTable *hztbl, char *hzkeys, char *hzchar);
extern void showLatestHZKeysInNewMethod(CXtermInputModule *cxin);

#endif /* _HZINPUT_H_ */
