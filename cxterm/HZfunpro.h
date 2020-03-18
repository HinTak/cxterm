/*
 *	$Id: HZfunpro.h,v 1.2 2003/05/06 07:40:19 htl10 Exp $
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

/* all function prototypes */

/* HZInput.c */

extern  void XtermWritePty(char *d, int len);
extern  void HZprintfMsg();

/*
 * the definition of "NeedFunctionPrototypes" comes from <X11/Xfuncproto.h>
 */
#if NeedFunctionPrototypes

# if NeedWidePrototypes
#define FP_ic_AddInput	HZInputContext* /* ic */, int /* key */
# else
#define FP_ic_AddInput	HZInputContext* /* ic */, char /* key */
# endif

#define FP_ic_Match	HZInputContext* /* ic */
#define FP_ic_Restart	HZInputContext* /* ic */
#define FP_ic_GetNext	HZInputContext* /* ic */, XChar2b** /* phzc */
#define FP_ic_Derivable	HZInputContext* /* ic */, XChar2b* /* hzc */
#define FP_ic_Init	HZInputContext* /* ic */
#define FP_ic_Copy	HZInputContext* /* new_ic */, HZInputContext* /* ic */
#define FP_ic_CleanUp	HZInputContext* /* ic */

#else

#define FP_ic_AddInput
#define FP_ic_Match
#define FP_ic_Restart
#define FP_ic_GetNext
#define FP_ic_Derivable
#define FP_ic_Init
#define FP_ic_Copy
#define FP_ic_CleanUp

#endif

/* HZinCntx.c */

extern HZInputContext *HZinputSearch(
#if NeedFunctionPrototypes
	CXtermInputModule *	/* cxin */,
# if NeedWidePrototypes
	int			/* key */,
# else
	char			/* key */,
# endif
	HZInputContext *	/* ic */
#endif
);
extern HZInputContext *HZassocSearch(
#if NeedFunctionPrototypes
	CXtermInputModule *	/* cxin */,
# if NeedWidePrototypes
	int			/* nhz */
# else
	short int		/* nhz */
# endif
#endif
);

extern HZInputContext *HZaddInputIC( FP_ic_AddInput );
extern int HZmatchIC( FP_ic_Match );
extern int HZrestartIC( FP_ic_Restart );
extern short int HZgetNextIC( FP_ic_GetNext );
extern Boolean HZderivableIC( FP_ic_Derivable );

extern void InitICpool(void);
extern HZInputContext *NewIC(
#if NeedFunctionPrototypes
	HZicType		/* type */,
	HZInputMethod *		/* im */,
	HZAssocList *		/* al */,
	char *			/* keysqbuf */,
	int			/* matched */,
	int			/* pending */
#endif
);
extern HZInputContext *DupIC(
#if NeedFunctionPrototypes
	HZInputContext *	/* ic */
#endif
);
extern void FreeIC(
#if NeedFunctionPrototypes
	HZInputContext *	/* ic */
#endif
);


/* HZtrie.c */

extern  HZInputContext *Trie_AddInput(FP_ic_AddInput);
extern  int Trie_Match(FP_ic_Match);
extern  int Trie_Restart(FP_ic_Restart);
extern  short int Trie_GetNext(FP_ic_GetNext);
extern  Boolean Trie_Derivable(FP_ic_Derivable);
extern  void Trie_Init(FP_ic_Init);

extern int LoadCIT(
#if NeedFunctionPrototypes
	char *			/* name */,
	char *			/* inputdir */,
	char *			/* encname */,
	HZinputTable *		/* hztbl */
#endif
);
extern void UnloadCIT(
#if NeedFunctionPrototypes
	HZinputTable *		/* hztbl */
#endif
);
extern HZimHZListIndex *MakeHZIdx(
#if NeedFunctionPrototypes
	HZinputTable *		/* hztbl */
#endif
);
extern int HZIdxInRange(
#if NeedFunctionPrototypes
	HZimHZListIndex *	/* hzidx */,
	XChar2b *		/* hzc */,
	int			/* idx1 */,
	int			/* idx2 */
#endif
);
extern void FreeHZIdx(
#if NeedFunctionPrototypes
	HZimHZListIndex *	/* hzidx */
#endif
);

/* HZtrieWc.c */

extern  HZInputContext *Trie_wc_AddInput(FP_ic_AddInput);
extern  int Trie_wc_Match(FP_ic_Match);
extern  int Trie_wc_Restart(FP_ic_Restart);
extern  short int Trie_wc_GetNext(FP_ic_GetNext);
extern  Boolean Trie_wc_Derivable(FP_ic_Derivable);
extern  void Trie_wc_Init(FP_ic_Init);
extern  void Trie_wc_Copy(FP_ic_Copy);
extern  void Trie_wc_CleanUp(FP_ic_CleanUp);

/* HZassoc.c */

extern  HZInputContext *Assoc_AddInput(FP_ic_AddInput);
extern  int Assoc_Match(FP_ic_Match);
extern  int Assoc_Restart(FP_ic_Restart);
extern  short int Assoc_GetNext(FP_ic_GetNext);
extern  void Assoc_Init(FP_ic_Init);
extern  void Assoc_CleanUp(FP_ic_CleanUp);
extern  void Assoc_Copy(FP_ic_Copy);

extern  int Assoc_ps_Restart(FP_ic_Restart);
extern  short int Assoc_ps_GetNext(FP_ic_GetNext);

extern HZAssocList *LoadAssocList(
#if NeedFunctionPrototypes
	char *			/* name */,
	char *			/* inputdir */,
	char *			/* term_encname */
#endif
);
extern void UnloadAssocList(
#if NeedFunctionPrototypes
	HZAssocList *		/* al */
#endif
);

/* HZfilter.c */

extern	int hzTableFilter( FP_hzFilter );

/* HZbuiltn.c */

extern  void HZLoadBuiltin(
#if NeedFunctionPrototypes
	CXtermInputModule *	/* cxin */
#endif
);

/* HZinMthd.c */

extern  void HZimInit(
#if NeedFunctionPrototypes
	CXtermInputModule *	/* cxin */,
	XtermWidget		/* xterm */,
	TScreen *		/* screen */
#endif
);
extern  void HZimReset(
#if NeedFunctionPrototypes
	CXtermInputModule *	/* cxin */,
# if NeedWidePrototypes
	unsigned int		/* dorefresh */
# else
	Boolean			/* dorefresh */
# endif
#endif
);
extern  void HZimCleanUp(
#if NeedFunctionPrototypes
	CXtermInputModule *	/* cxin */
#endif
);
extern	void HZsaveHistory(
#if NeedFunctionPrototypes
	CXtermInputModule *	/* cxin */
#endif
);
extern	void HZrestoreHistory(
#if NeedFunctionPrototypes
	CXtermInputModule *	/* cxin */
#endif
);
extern  void HZswitchModeByNum(
#if NeedFunctionPrototypes
	CXtermInputModule *	/* cxin */,
	int			/* num */
#endif
);
extern   int HZLoadInputMethod(
#if NeedFunctionPrototypes
	CXtermInputModule *	/* cxin */,
	char *			/* name */,
	char *			/* inputdir */,
	char *			/* encode */,
	HZInputMethod *		/* hzim */
#endif
);
extern  void HZimSetParam(
#if NeedFunctionPrototypes
	CXtermInputModule *	/* cxin */,
	char *			/* str */
#endif
);


/* HZchList.c */

extern	void HZclInit(
#if NeedFunctionPrototypes
	HZChoiceList *		/* cl */
#endif
);
extern	void HZclReset(
#if NeedFunctionPrototypes
	HZChoiceList *		/* cl */
#endif
);
extern	void HZclCleanUp(
#if NeedFunctionPrototypes
	HZChoiceList *		/* cl */
#endif
);
extern short int HZclSelect(
#if NeedFunctionPrototypes
	HZChoiceList *		/* cl */,
	int			/* n */,
	XChar2b **		/* phzc */
#endif
);
extern	 int HZclMakeSelection(
#if NeedFunctionPrototypes
	HZChoiceList *		/* cl */,
	HZInputArea *		/* ia */,
	HZInputContext *	/* ic */
#endif
);
extern	 int HZclMoveForward(
#if NeedFunctionPrototypes
	HZChoiceList *		/* cl */,
	HZInputArea *		/* ia */,
	HZInputContext *	/* ic */
#endif
);
extern	 int HZclMoveBackward(
#if NeedFunctionPrototypes
	HZChoiceList *		/* cl */,
	HZInputArea *		/* ia */,
	HZInputContext *	/* ic */
#endif
);
extern   int HZclAddChoice(
#if NeedFunctionPrototypes
	HZChoiceList *		/* cl */,
	XChar2b *		/* hzc */,
# if NeedWidePrototypes
	int			/* nhz */
# else
	short int		/* nhz */
# endif
#endif
);

/* HZinArea.c */

extern  void HZiaInit(
#if NeedFunctionPrototypes
	HZInputArea *		/* ia */,
	TScreen *		/* screen */
#endif
);
extern  void HZiaCleanUp(
#if NeedFunctionPrototypes
	HZInputArea *		/* ia */
#endif
);
extern  void HZiaSetFrame(
#if NeedFunctionPrototypes
	HZInputArea *		/* ia */,
	HZInputMethod *		/* hzim */
#endif
);
extern  void HZiaSet(
#if NeedFunctionPrototypes
	HZInputArea *		/* ia */,
	HZChoiceList *		/* cl */,
	char *			/* inbuf */,
	int			/* inbufCount */,
	int			/* cursor */,
	char *			/* status */
#endif
);
extern  void HZiaSetInBuf(
#if NeedFunctionPrototypes
	HZInputArea *		/* ia */,
	char *			/* inbuf */,
	int			/* inbufCount */,
	int			/* cursor */
#endif
);
extern  void HZiaSetStatus(
#if NeedFunctionPrototypes
	HZInputArea *		/* ia */,
	char *			/* status */
#endif
);
extern  void HZiaSetChoiceList(
#if NeedFunctionPrototypes
	HZInputArea *		/* ia */,
	HZChoiceList *		/* cl */
#endif
);
extern  void HZiaResize(
#if NeedFunctionPrototypes
	HZInputArea *		/* ia */,
	TScreen *		/* screen */,
	CXtermInputModule *	/* cxin */
#endif
);
extern  void HZiaFlush(
#if NeedFunctionPrototypes
	HZInputArea *		/* ia */,
	TScreen *		/* screen */
#endif
);
extern  void HZiaRedraw(
#if NeedFunctionPrototypes
	HZInputArea *		/* ia */,
	TScreen *		/* screen */
#endif
);
extern  void HZiaShowMesg(
#if NeedFunctionPrototypes
	HZInputArea *		/* ia */,
	TScreen *		/* screen */,
	char *			/* mesgstr */
#endif
);
extern  void HZiaClickAction(
#if NeedFunctionPrototypes
	HZInputArea *		/* ia */,
	CXtermInputModule *	/* cxin */,
	int			/* x */,
	int			/* y */
#endif
);

/* HZpopup.c */

extern void HZPopupConfig( 
#if NeedFunctionPrototypes
	CXtermInputModule *	/* cxin */
#endif
);

/* HZutil.c */

extern int HZencode(
#if NeedFunctionPrototypes
	char *			/* name */
#endif
);
extern char *HZencodeName(
#if NeedFunctionPrototypes
	int			/* encode */
#endif
);
extern int HZfindfile(
#if NeedFunctionPrototypes
	char *			/* name */,
	char *			/* inputdir */,
	char *			/* filename */
#endif
);
extern int HZfindsuffix(
#if NeedFunctionPrototypes
	char *			/* new_suffix */,
	char *			/* new_inputdir */,
	char *			/* basename */
#endif
);
extern int HZgetprompt(
#if NeedFunctionPrototypes
	char *			/* buffer */,
	char *			/* strbuf */
#endif
);
