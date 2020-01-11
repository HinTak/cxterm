/*
 *	$Id: HZpopup.c,v 1.3 2003/05/06 07:40:19 htl10 Exp $
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

/* HZpopup.c		Popup panels for input configuration or help
 *
 */

#include "HZinput.h"		/* X headers included here. */

/* all these widgets too */
#include <stdio.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Cardinals.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/List.h>
#include <X11/Xaw/Viewport.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>

#define SCOPE static
#include "widgets/xp.vh"

#define POPUP_SHELL_NAME	"configPopup"
#define MAX_IM_IN_LIST	(1024)	/* 1024 .cit files total, should be enough */

static String im_list[MAX_IM_IN_LIST];

static void MakeIMList(CXtermInputModule *cxin, char *hzinputdir, String *list, int *pnum), FillParam(CXtermInputModule *cxin);
static void AlignWidgets(void);

static void PlacePopup(Widget w, XtPointer closure, XtPointer call_data), PopDown(Widget w, XtPointer closure, XtPointer call_data);
static void SetInputDir(Widget w, XtPointer closure, XtPointer call_data), ChooseIM(Widget w, XtPointer closure, XtPointer call_data), SwitchIM(Widget w, XtPointer closure, XtPointer call_data), SetParam(Widget w, XtPointer closure, XtPointer call_data);
static void ToggleSelectItem(Widget w, XtPointer closure, XtPointer call_data);

static int LabelWidth(Widget widget, char *widget_name);
static void TextSize(Widget widget, char *widget_name, int *h, int *w), ListSize(Widget widget, char *widget_name, int *h, int *w); 



extern void SetHZinDir (XtermWidget xw, String dir);
extern void SetHZinMethod (XtermWidget xw, String name, int doredraw);
extern int sprintf (char *, const char *, ...);
extern void SetHZinParam (char *str);

void HZCreatePopup(CXtermInputModule *cxin)
{
  Widget popup;
  Widget parent = (Widget)(cxin->xterm);

    if (cxin->xterm->screen.configPopup)	/* already created */
	return;

    /* create the popup */

    popup = XtCreatePopupShell( POPUP_SHELL_NAME, transientShellWidgetClass,
				parent, NULL, ZERO );
    XtAddCallback( popup, XtNpopupCallback,   PlacePopup, (XtPointer) 0 );

#include "widgets/xp.ch"

    XtAddCallback( cmd_id,   XtNcallback, SetInputDir, (XtPointer) cxin );
    XtAddCallback( cmd_im,   XtNcallback, SwitchIM,    (XtPointer) cxin );
    XtAddCallback( cmd_ip,   XtNcallback, SetParam,    (XtPointer) cxin );
    XtAddCallback( lst,      XtNcallback, ChooseIM,    (XtPointer) 0 );
    XtAddCallback( cmd_exit, XtNcallback, PopDown,     (XtPointer) popup );

    AlignWidgets();

    cxin->xterm->screen.configPopup = popup;
}

void HZPopupConfig(CXtermInputModule *cxin)
{
  Widget popup = cxin->xterm->screen.configPopup;
  int num = 0;
  extern Atom wm_delete_window;		/* from charproc.c */

    /* set current hzinputdir (and the list of input methods) */
    XtVaSetValues( txt_id,   XtNstring, cxin->xterm->misc.it_dirs,   NULL );
    XawListUnhighlight( lst );
    MakeIMList( cxin,  cxin->xterm->misc.it_dirs, im_list, &num );
    XtVaSetValues( lst,   XtNlist, im_list,   XtNnumberStrings, num,   NULL );

    /* set current input method */
    XtVaSetValues( txt_im,   XtNstring, cxin->chzim->name,           NULL );

    /* set the current input parameters */
    FillParam( cxin );

    XtPopup( popup, XtGrabNone );
    XSetWMProtocols( XtDisplay(popup), XtWindow(popup), &wm_delete_window, 1 );
}

/*
 * Callbacks
 */

/* ARGSUSED */
static void
PlacePopup(Widget w, XtPointer closure, XtPointer call_data)
{
  Position x = 0, y = 0;
  Dimension width, height;
  unsigned int win_width, win_height;
  Widget top_widget;
  Window root;
  int useless;

    XtRealizeWidget(w);
    XtVaGetValues( w,   XtNwidth, &width,   XtNheight, &height,   NULL );

    top_widget = w;
    while (XtParent(top_widget))
	top_widget = XtParent(top_widget);

    XGetGeometry( XtDisplay(top_widget), XtWindow(top_widget), &root,
		  &useless, &useless, &win_width, &win_height,
		  (unsigned int *)&useless, (unsigned int *)&useless );
    XtTranslateCoords( top_widget,
		(Position)((win_width - width)/2),
		(Position)((win_height - height)/2),
		&x, &y );

    XtVaSetValues( w,
		XtNx, x,		XtNy, y,
		XtNheight, height,	XtNwidth, width,
		XtNbaseHeight, height,	XtNbaseWidth, width,
		XtNminHeight, height,	XtNminWidth, width,
		XtNmaxHeight, height,	XtNmaxWidth, width,
		NULL );
}

/* ARGSUSED */
static void
PopDown(Widget w, XtPointer closure, XtPointer call_data)
{
    XtPopdown( (Widget)closure );
}

/* ARGSUSED */
static void
SetInputDir(Widget w, XtPointer closure, XtPointer call_data)
{
  CXtermInputModule *cxin = (CXtermInputModule *)closure;
  String value, buffer;
  int num = 0;

    XtVaGetValues( txt_id, XtNstring, &value, NULL );
    buffer = XtNewString(value);
    SetHZinDir ( cxin->xterm, buffer );
    XtFree(buffer);

    XawListUnhighlight(lst);
    MakeIMList( cxin, value, im_list, &num );
    XtVaSetValues( lst,   XtNlist, im_list,   XtNnumberStrings, num,   NULL );
}

/* ARGSUSED */
static void
ChooseIM(Widget w, XtPointer closure, XtPointer call_data)
{
    XtVaSetValues( txt_im,
		   XtNstring, ((XawListReturnStruct *)call_data)->string,
		   NULL );
}

/* ARGSUSED */
static void
SwitchIM(Widget w, XtPointer closure, XtPointer call_data)
{
  CXtermInputModule *cxin = (CXtermInputModule *)closure;
  String value, buffer;

    XtVaGetValues( txt_im, XtNstring, &value, NULL );
    buffer = XtNewString(value);
    SetHZinMethod (cxin->xterm, buffer, True);

    /* the switch might not be successful */
    if (strcmp(buffer, cxin->chzim->name) != 0) {
	/* not successful -- restore the old name */
	XBell(XtDisplay(txt_im), 0);
	XtVaSetValues( txt_im,   XtNstring, cxin->chzim->name,   NULL );
    } else {
	/* parameters may have changed with the input method */
	FillParam (cxin);
    }
    XtFree(buffer);
}

/* ARGSUSED */
static void
SetParam(Widget w, XtPointer closure, XtPointer call_data)
{
  String label;
  char param_buf[1024];

    XtVaGetValues( cmd_sl, XtNlabel, &label, NULL );
    sprintf(param_buf, "auto-select=%s", label);
    SetHZinParam (param_buf);

    XtVaGetValues( cmd_sg, XtNlabel, &label, NULL );
    sprintf(param_buf, "auto-segment=%s", label);
    SetHZinParam (param_buf);

    XtVaGetValues( cmd_as, XtNlabel, &label, NULL );
    sprintf(param_buf, "association=%s", label);
    SetHZinParam (param_buf);

    /* some changes may be disallowed */
    FillParam ( (CXtermInputModule *)closure );
}

/* ARGSUSED */
static void
ToggleSelectItem(Widget w, XtPointer closure, XtPointer call_data)
{
  String label;

    XtVaGetValues( w, 		      XtNlabel, &label,   NULL );
    XtVaSetValues( (Widget)closure,   XtNlabel,  label,   NULL );
}


/*
 * align widgets and give a better look
 */
static void
AlignWidgets(void)
{
  int maxw, w;
  int h1, h2, h3;

    /* align all the labels by making them the same width */

    maxw = 0;
    w = LabelWidth(lb_id,  lb_id__name );  if (w > maxw) maxw = w;
    w = LabelWidth(lb_im,  lb_im__name );  if (w > maxw) maxw = w;
    w = LabelWidth(lb_im2, lb_im2__name);  if (w > maxw) maxw = w;
    w = LabelWidth(lb_ip,  lb_ip__name );  if (w > maxw) maxw = w;
    
    XtVaSetValues( lb_id,    XtNwidth, maxw,   NULL);
    XtVaSetValues( lb_im,    XtNwidth, maxw,   NULL);
    XtVaSetValues( lb_im2,   XtNwidth, maxw,   NULL);
    XtVaSetValues( lb_ip,    XtNwidth, maxw,   NULL);

    /* align all the text input widgets by making them the same width */

    maxw = 0;
    TextSize(txt_id, txt_id__name, &h1, &w);  if (w > maxw) maxw = w;
    TextSize(txt_im, txt_im__name, &h2, &w);  if (w > maxw) maxw = w;
    ListSize(lst,    lst__name,    &h3, &w);  if (w > maxw) maxw = w;

    XtVaSetValues( txt_id,   XtNwidth, maxw,   XtNheight, h1,   NULL );
    XtVaSetValues( txt_im,   XtNwidth, maxw,   XtNheight, h2,   NULL );
    XtVaSetValues( vp_im,    XtNwidth, maxw,   XtNheight, h3,   NULL );

    XtVaSetValues( lst,   XtNdefaultColumns, 1,   XtNlongest, maxw,   NULL );

    /* align all the parameter names */

    maxw = 0;
    w = LabelWidth(lb_sl, lb_sl__name);  if (w > maxw) maxw = w;
    w = LabelWidth(lb_sg, lb_sg__name);  if (w > maxw) maxw = w;
    w = LabelWidth(lb_as, lb_as__name);  if (w > maxw) maxw = w;

    XtVaSetValues( lb_sl,   XtNwidth, maxw,   NULL);
    XtVaSetValues( lb_sg,   XtNwidth, maxw,   NULL);
    XtVaSetValues( lb_as,   XtNwidth, maxw,   NULL);
}

static void
MakeIMList(CXtermInputModule *cxin, char *hzinputdir, String *list, int *pnum)
{
  static char listbuffer[4096];		/* buffer for the list */
  char *ptr;
  int i;

    *pnum = 0;
    ptr = listbuffer;

    /* in-core input methods */
    for (i = 0; i < cxin->numHZim; i++) {
	if (cxin->imtbl[i].type == im_type_Builtin) {
	    strcpy( ptr, cxin->imtbl[i].name );
	    list[(*pnum)++] = ptr;
	    ptr += strlen(ptr) + 1;
	}
    }

    /* start the search */
    if (! HZfindsuffix(CIT_SUFFIX, hzinputdir, ptr))
	return;		/* nothing found. */
    list[(*pnum)++] = ptr;
    ptr += strlen(ptr) + 1;

    while ( HZfindsuffix(NULL, NULL, ptr) ) {
	list[(*pnum)++] = ptr;
	ptr += strlen(ptr) + 1;
    }
}

static void
FillParam(CXtermInputModule *cxin)
{
  String label;

    switch (cxin->chzim->m.auto_select) {
      case HZ_AUTOSELECT_ALWAYS:
	label = "Always";
	break;
      case HZ_AUTOSELECT_WHENNOMATCH:
	label = "WhenNoMatch";
	break;
      default:
	label = "Never";
	break;
    }
    XtVaSetValues( cmd_sl,   XtNlabel, label,   NULL );

    if (cxin->chzim->m.do_auto_segment) {
	label = "Yes";
    } else {
	label = "No";
    }
    XtVaSetValues( cmd_sg,   XtNlabel, label,   NULL );

    if (cxin->chzim->m.do_ps_assoc) {
	label = "Yes";
    } else {
	label = "No";
    }
    XtVaSetValues( cmd_as,   XtNlabel, label,   NULL );
}

static int default_row = 5;		/* default: 5 rows in the list */
static int default_col = 20;		/* default: 20 columns width */
static int default_thickness = 14;

static int
LabelWidth(Widget widget, char *widget_name)
{
  String label;
  Dimension margin;
  XFontStruct *fs;
  XCharStruct chars;
  int useless;

    XtVaGetValues( widget,
		   XtNlabel, &label,
		   XtNfont, &fs,
		   XtNinternalWidth, &margin,
		   NULL );
    XTextExtents( fs, label, strlen(label), &useless, &useless, &useless,
		  &chars );

    return (chars.width + margin * 2);
}

static void
TextSize(Widget widget, char *widget_name, int *h, int *w)
{
  XFontStruct *fs;
  Position top_margin, bottom_margin, left_margin, right_margin, spacing;
  Dimension scroll_bar;
  int col;
  static XtResource scroll_resource[] = {
    { XtNthickness, XtCThickness, XtRDimension, sizeof(int),
	0, XtRDimension, (XtPointer) &default_thickness }
  };
  static XtResource text_resource[] = {
    { "numCols", "NumCols", XtRInt, sizeof(int),
	0, XtRInt, (XtPointer) &default_col }
  };

    XtVaGetValues( widget,
		   XtNfont, &fs,
		   XtNtopMargin, &top_margin,
		   XtNbottomMargin, &bottom_margin,
		   XtNleftMargin, &left_margin,
		   XtNrightMargin, &right_margin,
		   NULL );

    XtGetSubresources( widget, (XtPointer)&scroll_bar,
			"hScrollbar", "Scrollbar",	/* yes, I cheat here */
			scroll_resource, XtNumber(scroll_resource),
			NULL, ZERO );
    if (scroll_bar == 0)
	scroll_bar = default_thickness;
    XtGetSubresources( XtParent(widget), (XtPointer)&col,
			XtName(widget), "AsciiText",
			text_resource, XtNumber(text_resource), NULL, ZERO );

    *h = top_margin + (fs->ascent + fs->descent) + bottom_margin + scroll_bar;
    *w = left_margin + (fs->max_bounds.width) * col + right_margin;

/* Don't know why, sometimes fs->ascent, fs->descent, fs->max_bounds give out
 * unreasonable value, the following is a simple-minded fixed.
 *	--- Changsen Xu <xucs007@yahoo.com>, Dec/5/2001
 */
    *h = 40;
    *w = 200;

}

static void
ListSize(Widget widget, char *widget_name, int *h, int *w)
{
  XFontStruct *fs;
  Dimension vertical_margin, horizontal_margin, spacing;
  struct _resource {
    int row;
    int col;
  } res;
  static XtResource list_resource[] = {
    { "numRows", "NumRows", XtRInt, sizeof(int),
	XtOffsetOf(struct _resource, row), XtRInt, (XtPointer) &default_row },
    { "numCols", "NumCols", XtRInt, sizeof(int),
	XtOffsetOf(struct _resource, col), XtRInt, (XtPointer) &default_col }
  };

    XtVaGetValues( widget, 
		   XtNfont, &fs,
		   XtNinternalHeight, &vertical_margin,
		   XtNinternalWidth, &horizontal_margin,
		   XtNrowSpacing, &spacing,
		   NULL );

    XtGetSubresources( XtParent(widget), (XtPointer)&res,
			XtName(widget), "AsciiText",
			list_resource, XtNumber(list_resource), NULL, ZERO );

    *h = (fs->ascent + fs->descent + spacing) * res.row + vertical_margin * 2;
    *w = (fs->max_bounds.width) * res.col + horizontal_margin * 2;
}

