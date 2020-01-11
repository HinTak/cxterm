/*
 *	$Id: HZtable.h,v 1.1.1.1.2.1 2013/08/25 00:49:33 htl10 Exp $
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

/* HZtable.h */

/*
 * Data Structure of the External Chinese Input Table
 *
 * An external input table (CIT) contains:
 *	1) interface definition (keys definition, prompts, ...)
 *	2) input lookup data structure
 *
 * Trie Table:
 *	list of Trie nodes.
 *
 * HZ Table:
 *	2 bytes character sets.
 *
 */

#ifndef	_HZTABLE_H_
#define	_HZTABLE_H_

/*
 * The only imported definition is XChar2b, from <X11/Xlib.h>.
 * To advoid including the tremendous amount of X11 include files,
 * this definition is simply copied here.
 */
#if !defined(_XLIB_H_) && !defined(_X11_XLIB_H_)
typedef struct {		/* normal 16 bit characters are two bytes */
    unsigned char byte1;
    unsigned char byte2;
} XChar2b;
#endif

#define	MAX_PROMPT	24	/* maximum prompt in HZ input erea */
#define MAX_KEYPROMPT	4	/* maximum bytes to display each input key */
#define MAX_CHOICE	16	/* maximum number of choices on one screen */

typedef	struct _keyPrompt {
    unsigned char	prompt[MAX_KEYPROMPT];	/* prompt string */
    unsigned short	ptlen;			/* len of the prompt */
} keyPrompt;

/* Trie Node.  (so organized to optimize access speed) */
typedef	struct _trieNode {
    unsigned char	tn_key;		/* this input key */
    unsigned char	tn_numNextKeys;	/* number of possible next input key */
    unsigned short int	tn_numHZchoice;	/* number of HZ choices */
    unsigned int	tn_nextKeys;	/* table for further input key */
    unsigned int	tn_hzidx;	/* index to HZ list */
} trieNode;

/****************** The external input table ******************/

typedef struct _HZinputTable {
    unsigned char	version;	/* version num of this .cit format */
    char		encode;		/* encoding: GB/BIG5/... */
    unsigned char	defAssocKey;	/* default association key */

    unsigned int	sizeTrieList;	/* size of the whole TRIE list */
    unsigned int	sizeHZList;	/* size of the whole HZ list */
    trieNode		*trieList;	/* start address of the Trie list */
    XChar2b		*hzList;	/* start address of the HZ list */

    unsigned char	autoSelect;	/* auto selection if single choice? */
    unsigned short	keytype[128];	/* valid type of the key */
    keyPrompt		keyprompt[128];	/* display this for the key */

    char		choicelb[MAX_CHOICE];	/* HZ choice label */
    int			maxchoice;	/* maximum number of HZ choice */
    unsigned char	prompt[MAX_PROMPT];
    int			lenPrompt;		/* len of the Prompt */

} HZinputTable;

/*
 * valid type or type mask of a key
 * mask can be OR -- possible for a key to be both input key and selection key
 *
 *	........
 *	   1#### <--- SELECTION_MASK
 *	  1      <--- INPUT_MASK
 *
 *	  11#### <--- both INPUT and SELECTION
 *	  100000 <--- INPUT only
 *
 *	  1  1 0 <--- WILDCARD ('*')
 *	  1  1 1 <--- WILDCHAR ('?')
 *	  1 1    <--- ASSOCIATION key
 *
 *	01   000 <--- BACKSPACE
 *	01   001 <--- KILLALL
 *	01   010 <--- CURSOR BEGIN
 *	01   011 <--- CURSOR END
 *	01   100 <--- CURSOR FORW
 *	01   101 <--- CURSOR BACK
 *	01   110 <--- CURSOR ERASE
 *	10     0 <--- RIGHT
 *	10     1 <--- LEFT
 *	11     0 <--- REPEAT
 *	........
 *
 * HZ_KEY_SELECTION_NUM must be (MAX_CHOICE - 1)
 */
#define	HZ_KEY_INVALID		0	/* not used to input HZ */

#define	HZ_KEY_SELECTION_MASK	0x10	/* used to select a choice */
#define	HZ_KEY_SELECTION_NUM	0x0f	/* num to select a choice */

#define	HZ_KEY_INPUT_MASK	0x20	/* used to convert to HZ */
#define	HZ_KEY_WILDCARD		0x24	/* wildcard (*) for global matching */
#define	HZ_KEY_WILDCHAR		0x25	/* wildchar (?) for global matching */
#define	HZ_KEY_ASSOCIATION	0x28	/* key for continuous input */

/* pre-edit keys */
#define HZ_KEY_EDIT_BACKSPACE	0x40	/* backspace (delete previous char) */
#define HZ_KEY_EDIT_KILL	0x41	/* clear input buffer (delete all) */
#define HZ_KEY_EDIT_BEGIN	0x42	/* move cursor to start of line */
#define HZ_KEY_EDIT_END		0x43	/* move cursor to end of line */
#define HZ_KEY_EDIT_FORW	0x44	/* move cursor forward one char */
#define HZ_KEY_EDIT_BACK	0x45	/* move cursor backward one char */
#define HZ_KEY_EDIT_ERASE	0x46	/* delete current char */

/* choice-list movement */
#define HZ_KEY_RIGHT		0x80	/* select key: more at right */
#define HZ_KEY_LEFT		0x81	/* select key: more at left */

#define HZ_KEY_REPEAT		0xc0	/* repeat key: repeat previous input */

/*
 * auto selection
 */
#define	HZ_AUTOSELECT_NEVER		0
#define	HZ_AUTOSELECT_ALWAYS		1
#define	HZ_AUTOSELECT_WHENNOMATCH	2

/*
 * HZ_PHRASE_TAG -- if the first byte of a HZ (2-bytes) is this value,
 *	the second byte is an integer n and means that the following n HZ
 *	are a multi-HZ phrase.
 */
#define	HZ_PHRASE_TAG	'\001'	/* if a HZ's first byte is this => a phrase */

/*
 * The magic number and the version number of a .cit file.
 */
#define	MAGIC_CIT	"HZ"
#define	CIT_VERSION	2
#define	CIT_SUFFIX	".cit"
#define	TIT_SUFFIX	".tit"

/*
 * Encodings that are understood by cxterm.
 */
#define	GB_ENCODE	0
#define	BIG5_ENCODE	1
#define	JIS_ENCODE	2
#define	KS_ENCODE	3
#define	UNKNOWN_ENCODE	(-1)

#endif /* _HZTABLE_H_ */
