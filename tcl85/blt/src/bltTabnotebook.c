
/*
 * bltTabnotebook.c --
 *
 * This module implements a tab notebook widget for the BLT toolkit.
 *
 *	Copyright 1998-2004 George A Howlett.
 *
 *	Permission is hereby granted, free of charge, to any person obtaining
 *	a copy of this software and associated documentation files (the
 *	"Software"), to deal in the Software without restriction, including
 *	without limitation the rights to use, copy, modify, merge, publish,
 *	distribute, sublicense, and/or sell copies of the Software, and to
 *	permit persons to whom the Software is furnished to do so, subject to
 *	the following conditions:
 *
 *	The above copyright notice and this permission notice shall be
 *	included in all copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *	NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *	LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *	OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *	WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "bltInt.h"

#ifndef NO_TABNOTEBOOK
#include "bltOp.h"
#include "bltBind.h"
#include "bltChain.h"
#include "bltFont.h"
#include "bltText.h"
#include "bltHash.h"
#include "bltTile.h"

#define INVALID_FAIL	0
#define INVALID_OK	1

#define FCLAMP(x)	((((x) < 0.0) ? 0.0 : ((x) > 1.0) ? 1.0 : (x)))

#define GAP			3
#define SELECT_PADX		4
#define SELECT_PADY		2
#define OUTER_PAD		2
#define LABEL_PAD		1
#define LABEL_PADX		2
#define LABEL_PADY		2
#define IMAGE_PAD		1
#define CORNER_OFFSET		3

#define TAB_SCROLL_OFFSET	10

#define SLANT_NONE		0
#define SLANT_LEFT		1
#define SLANT_RIGHT		2
#define SLANT_BOTH		(SLANT_LEFT | SLANT_RIGHT)

#define END			(-1)
#define ODD(x)			((x) | 0x01)

#define TAB_VERTICAL		(SIDE_LEFT | SIDE_RIGHT)
#define TAB_HORIZONTAL		(SIDE_TOP | SIDE_BOTTOM)

#define TABWIDTH(s, t)		\
  ((s)->side & TAB_VERTICAL) ? (t)->height : (t)->width)
#define TABHEIGHT(s, t)		\
  ((s)->side & TAB_VERTICAL) ? (t)->height : (t)->width)

#define VPORTWIDTH(s)		 \
  (((s)->side & TAB_HORIZONTAL) ? (Tk_Width((s)->tkwin) - 2 * (s)->inset) : \
   (Tk_Height((s)->tkwin) - 2 * (s)->inset))

#define VPORTHEIGHT(s)		 \
  (((s)->side & TAB_VERTICAL) ? (Tk_Width((s)->tkwin) - 2 * (s)->inset) : \
   (Tk_Height((s)->tkwin) - 2 * (s)->inset))

#define GETATTR(t,attr)		\
   (((t)->attr != NULL) ? (t)->attr : (t)->nbPtr->defTabStyle.attr)

/*
 *---------------------------------------------------------------------------
 *
 *  Internal widget flags:
 *
 *	TNB_LAYOUT		The layout of the widget needs to be
 *				recomputed.
 *
 *	TNB_REDRAW		A redraw request is pending for the widget.
 *
 *	TNB_SCROLL		A scroll request is pending.
 *
 *	TNB_FOCUS		The widget is receiving keyboard events.
 *				Draw the focus highlight border around the
 *				widget.
 *
 *	TNB_MULTIPLE_TIER	Notebook is using multiple tiers.
 *
 *	TNB_STATIC		Notebook does not scroll.
 *
 *---------------------------------------------------------------------------
 */
#define TNB_LAYOUT		(1<<0)
#define TNB_REDRAW		(1<<1)
#define TNB_SCROLL		(1<<2)
#define TNB_FOCUS		(1<<4)

#define TNB_STATIC		(1<<8)
#define TNB_MULTIPLE_TIER	(1<<9)

#define PERFORATION_ACTIVE	(1<<10)

#define TAB_LABEL		(ClientData)0
#define TAB_PERFORATION		(ClientData)1

#define DEF_TNB_ACTIVE_BACKGROUND	RGB_GREY90
#define DEF_TNB_ACTIVE_BG_MONO		STD_ACTIVE_BG_MONO
#define DEF_TNB_ACTIVE_FOREGROUND	STD_ACTIVE_FOREGROUND
#define DEF_TNB_ACTIVE_FG_MONO		STD_ACTIVE_FG_MONO
#define DEF_TNB_BG_MONO			STD_NORMAL_BG_MONO
#define DEF_TNB_BACKGROUND		STD_NORMAL_BACKGROUND
#define DEF_TNB_BORDERWIDTH		"1"
#define DEF_TNB_COMMAND			(char *)NULL
#define DEF_TNB_CURSOR			(char *)NULL
#define DEF_TNB_DASHES			"1"
#define DEF_TNB_FOREGROUND		STD_NORMAL_FOREGROUND
#define DEF_TNB_FG_MONO			STD_NORMAL_FG_MONO
#define DEF_TNB_FONT			STD_FONT
#define DEF_TNB_GAP			"3"
#define DEF_TNB_HEIGHT			"0"
#define DEF_TNB_HIGHLIGHT_BACKGROUND	STD_NORMAL_BACKGROUND
#define DEF_TNB_HIGHLIGHT_BG_MONO	STD_NORMAL_BG_MONO
#define DEF_TNB_HIGHLIGHT_COLOR		RGB_BLACK
#define DEF_TNB_HIGHLIGHT_WIDTH		"2"
#define DEF_TNB_NORMAL_BACKGROUND	STD_NORMAL_BACKGROUND
#define DEF_TNB_NORMAL_FG_MONO		STD_ACTIVE_FG_MONO
#define DEF_TNB_OUTER_PAD		"3"
#define DEF_TNB_RELIEF			"sunken"
#define DEF_TNB_ANGLE			"0.0"
#define DEF_TNB_SCROLL_INCREMENT	"0"
#define DEF_TNB_SELECT_BACKGROUND	STD_NORMAL_BACKGROUND
#define DEF_TNB_SELECT_BG_MONO		STD_SELECT_BG_MONO
#define DEF_TNB_SELECT_BORDERWIDTH 	"1"
#define DEF_TNB_SELECT_CMD		(char *)NULL
#define DEF_TNB_SELECT_FONT		STD_SELECT_FONT
#define DEF_TNB_SELECT_FOREGROUND	STD_SELECT_FOREGROUND
#define DEF_TNB_SELECT_FG_MONO		STD_SELECT_FG_MONO
#define DEF_TNB_SELECT_MODE		"multiple"
#define DEF_TNB_SELECT_RELIEF		"raised"
#define DEF_TNB_SELECT_PAD		"5"
#define DEF_TNB_SHADOW_COLOR		RGB_BLACK
#define DEF_TNB_SIDE			"top"
#define DEF_TNB_SLANT			"none"
#define DEF_TNB_TAB_BACKGROUND		RGB_GREY82
#define DEF_TNB_TAB_BG_MONO		STD_SELECT_BG_MONO
#define DEF_TNB_TAB_RELIEF		"raised"
#define DEF_TNB_TAKE_FOCUS		"1"
#define DEF_TNB_TEXT_COLOR		STD_NORMAL_FOREGROUND
#define DEF_TNB_TEXT_MONO		STD_NORMAL_FG_MONO
#define DEF_TNB_TEXT_SIDE		"left"
#define DEF_TNB_TIERS			"1"
#define DEF_TNB_TILE			(char *)NULL
#define DEF_TNB_WIDTH			"0"
#define DEF_TNB_SAME_WIDTH		"yes"
#define DEF_TNB_TEAROFF			"yes"
#define DEF_TNB_PAGE_WIDTH		"0"
#define DEF_TNB_PAGE_HEIGHT		"0"

#define DEF_TAB_ACTIVE_BG		(char *)NULL
#define DEF_TAB_ACTIVE_FG		(char *)NULL
#define DEF_TAB_ANCHOR			"center"
#define DEF_TAB_BG			(char *)NULL
#define DEF_TAB_COMMAND			(char *)NULL
#define DEF_TAB_DATA			(char *)NULL
#define DEF_TAB_FG			(char *)NULL
#define DEF_TAB_FILL			"none"
#define DEF_TAB_FONT			(char *)NULL
#define DEF_TAB_HEIGHT			"0"
#define DEF_TAB_IMAGE			(char *)NULL
#define DEF_TAB_IPAD			"0"
#define DEF_TAB_PAD			"3"
#define DEF_TAB_PERF_COMMAND		(char *)NULL
#define DEF_TAB_SELECT_BG		(char *)NULL
#define DEF_TAB_SELECT_BORDERWIDTH 	"1"
#define DEF_TAB_SELECT_CMD		(char *)NULL
#define DEF_TAB_SELECT_FG		(char *)NULL
#define DEF_TAB_SELECT_FONT		(char *)NULL
#define DEF_TAB_STATE			"normal"
#define DEF_TAB_STIPPLE			"BLT"
#define DEF_TAB_BIND_TAGS		"all"
#define DEF_TAB_TEXT			(char *)NULL
#define DEF_TAB_VISUAL			(char *)NULL
#define DEF_TAB_WIDTH			"0"
#define DEF_TAB_WINDOW			(char *)NULL

typedef struct NotebookStruct Notebook;

static Tk_GeomRequestProc EmbeddedWidgetGeometryProc;
static Tk_GeomLostSlaveProc EmbeddedWidgetCustodyProc;
static Tk_GeomMgr tabMgrInfo =
{
    (char *)"notebook",		/* Name of geometry manager used by winfo */
    EmbeddedWidgetGeometryProc,	/* Procedure to for new geometry requests */
    EmbeddedWidgetCustodyProc,	/* Procedure when window is taken away */
};

static Blt_OptionParseProc ObjToImage;
static Blt_OptionPrintProc ImageToObj;
static Blt_OptionFreeProc FreeImage;
static Blt_OptionParseProc ObjToChild;
static Blt_OptionPrintProc ChildToObj;
static Blt_OptionParseProc ObjToSlant;
static Blt_OptionPrintProc SlantToObj;

/*
 * Contains a pointer to the widget that's currently being configured.  This
 * is used in the custom configuration parse routine for images.
 */
static Blt_CustomOption imageOption =
{
    ObjToImage, ImageToObj, FreeImage, (ClientData)0,
};

static Blt_CustomOption childOption =
{
    ObjToChild, ChildToObj, NULL, (ClientData)0,
};

static Blt_CustomOption slantOption =
{
    ObjToSlant, SlantToObj, NULL, (ClientData)0,
};

/*
 * TabImage --
 *
 *	When multiple instances of an image are displayed in the same widget,
 *	this can be inefficient in terms of both memory and time.  We only
 *	need one instance of each image, regardless of number of times we use
 *	it.  And searching/deleting instances can be very slow as the list
 *	gets large.
 *
 *	The workaround, employed below, is to maintain a hash table of images
 *	that maintains a reference count for each image.
 */

typedef struct {
    int refCount;		/* Reference counter for this image. */
    Tk_Image tkImage;		/* The Tk image being cached. */
    int width, height;		/* Dimensions of the cached image. */
    Blt_HashEntry *hashPtr;	/* Hash table pointer to the image. */

} TabImage;

#define ImageHeight(tabImage)	((tabImage)->height)
#define ImageWidth(tabImage)	((tabImage)->width)
#define ImageBits(tabImage)	((tabImage)->tkImage)

#define TAB_VISIBLE	(1<<0)
#define TAB_REDRAW	(1<<2)

typedef struct {
    char *name;			/* Identifier for tab entry */
    int state;			/* State of the tab: Disabled, active, or
				 * normal. */
    unsigned int flags;

    int tier;			/* Index of tier [1..numTiers] containing this
				 * tab. */

    int worldX, worldY;		/* Position of tab in world coordinates. */
    int worldWidth, worldHeight;/* Dimensions of the tab, corrected for
				 * orientation (-side).  It includes the
				 * border, padding, label, etc. */
    int screenX, screenY;
    short int screenWidth;
    short int screenHeight;	/*  */

    Notebook *nbPtr;		/* Notebook that includes this tab. Needed for
				 * callbacks can pass only a tab pointer.  */
    Blt_Uid tags;

    /*
     * Tab label:
     */
    Blt_Uid text;		/* String displayed as the tab's label. */
    TabImage *tabImagePtr;	/* Image displayed as the label. */

    short int textWidth, textHeight;
    short int labelWidth, labelHeight;
    Blt_Pad iPadX, iPadY;	/* Internal padding around the text */

    Blt_Font font;

    /*
     * Normal:
     */
    XColor *textColor;		/* Text color */
    Tk_3DBorder border;		/* Background color and border for tab.*/

    /*
     * Selected: Tab is currently selected.
     */
    XColor *selColor;		/* Selected text color */
    Tk_3DBorder selBorder;	/* 3D border of selected folder. */
    Blt_Font selFont;

    /*
     * Active: Mouse passes over the tab.
     */
    Tk_3DBorder activeBorder;	/* Active background color. */
    XColor *activeFgColor;	/* Active text color */

    Pixmap stipple;		/* Stipple for outline of embedded
				 * window when torn off. */
    /*
     * Embedded widget information:
     */
    Tk_Window tkwin;		/* Widget to be mapped when the tab is
				 * selected.  If NULL, don't make space for
				 * the page. */

    int reqWidth, reqHeight;	/* If non-zero, overrides the requested
				 * dimensions of the embedded widget. */

    Tk_Window container;	/* The window containing the embedded widget.
				 * Does not necessarily have to be the
				 * parent. */

    Tk_Anchor anchor;		/* Anchor: indicates how the embedded widget
				 * is positioned within the extra space on the
				 * page. */

    Blt_Pad padX, padY;		/* Padding around embedded widget */

    int fill;			/* Indicates how the window should fill the
				 * page. */

    /*
     * Auxillary information:
     */
    Blt_Uid command;		/* Command (malloc-ed) invoked when the tab is
				 * selected */
    Blt_Uid data;		/* This value isn't used in C code.  It may be
				 * used by clients in Tcl bindings to
				 * associate extra data (other than the label
				 * or name) with the tab. */

    Blt_ChainLink link;		/* Pointer to where the tab resides in the
				 * list of tabs. */
    Blt_Uid perfCommand;	/* Command (malloc-ed) invoked when the tab is
				 * selected */
    GC textGC;
    GC backGC;

    Blt_Tile tile;

} Tab;

static Blt_ConfigSpec tabConfigSpecs[] =
{
    {BLT_CONFIG_BORDER, "-activebackground", "activeBackground",
	"ActiveBackground", DEF_TAB_ACTIVE_BG, 
	Blt_Offset(Tab, activeBorder), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_COLOR, "-activeforeground", "activeForeground",
	"ActiveForeground", DEF_TAB_ACTIVE_FG, 
	Blt_Offset(Tab, activeFgColor), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_ANCHOR, "-anchor", "anchor", "Anchor",
	DEF_TAB_ANCHOR, Blt_Offset(Tab, anchor), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_BORDER, "-background", "background", "Background",
	DEF_TAB_BG, Blt_Offset(Tab, border), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_SYNONYM, "-bg", "background", (char *)NULL, (char *)NULL, 0, 0},
    {BLT_CONFIG_UID, "-bindtags", "bindTags", "BindTags",
	DEF_TAB_BIND_TAGS, Blt_Offset(Tab, tags), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_UID, "-command", "command", "Command",
	DEF_TAB_COMMAND, Blt_Offset(Tab, command), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_UID, "-data", "data", "data",
	DEF_TAB_DATA, Blt_Offset(Tab, data), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_SYNONYM, "-fg", "foreground", (char *)NULL, (char *)NULL, 0, 0},
    {BLT_CONFIG_FILL, "-fill", "fill", "Fill",
	DEF_TAB_FILL, Blt_Offset(Tab, fill), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_COLOR, "-foreground", "foreground", "Foreground",
	DEF_TAB_FG, Blt_Offset(Tab, textColor), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_FONT, "-font", "font", "Font",
	DEF_TAB_FONT, Blt_Offset(Tab, font), 0},
    {BLT_CONFIG_CUSTOM, "-image", "image", "image", DEF_TAB_IMAGE, 
	Blt_Offset(Tab, tabImagePtr), BLT_CONFIG_NULL_OK, &imageOption},
    {BLT_CONFIG_PAD, "-ipadx", "iPadX", "PadX", DEF_TAB_IPAD, 
	Blt_Offset(Tab, iPadX), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_PAD, "-ipady", "iPadY", "PadY", DEF_TAB_IPAD, 
	Blt_Offset(Tab, iPadY), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_PAD, "-padx", "padX", "PadX", 	DEF_TAB_PAD, 
	Blt_Offset(Tab, padX), 0},
    {BLT_CONFIG_PAD, "-pady", "padY", "PadY", DEF_TAB_PAD, 
	Blt_Offset(Tab, padY), 0},
    {BLT_CONFIG_UID, "-perforationcommand", "perforationcommand", 
	"PerforationCommand", DEF_TAB_PERF_COMMAND, 
	Blt_Offset(Tab, perfCommand), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_BORDER, "-selectbackground", "selectBackground", "Background",
	DEF_TAB_SELECT_BG, Blt_Offset(Tab, selBorder), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_FONT, "-selectfont", "selectFont", "Font",
	DEF_TAB_SELECT_FONT, Blt_Offset(Tab, selFont), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_COLOR, "-selectforeground", "selectForeground", "Foreground",
	DEF_TAB_SELECT_FG, Blt_Offset(Tab, selColor), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_STATE, "-state", "state", "State",
	DEF_TAB_STATE, Blt_Offset(Tab, state), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_BITMAP, "-stipple", "stipple", "Stipple",
	DEF_TAB_STIPPLE, Blt_Offset(Tab, stipple), 0},
    {BLT_CONFIG_TILE, "-tile", "tile", "Tile",
	(char *)NULL, Blt_Offset(Tab, tile), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_UID, "-text", "Text", "Text", 
	DEF_TAB_TEXT, Blt_Offset(Tab, text), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_CUSTOM, "-window", "window", "Window", DEF_TAB_WINDOW, 
	Blt_Offset(Tab, tkwin), BLT_CONFIG_NULL_OK, &childOption},
    {BLT_CONFIG_PIXELS_NNEG, "-windowheight", "windowHeight", "WindowHeight",
	DEF_TAB_HEIGHT, Blt_Offset(Tab, reqHeight), 
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_PIXELS_NNEG, "-windowwidth", "windowWidth", "WindowWidth",
	DEF_TAB_WIDTH, Blt_Offset(Tab, reqWidth), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_END, (char *)NULL, (char *)NULL, (char *)NULL,
	(char *)NULL, 0, 0}
};

/*
 * TabAttributes --
 */
typedef struct {
    Tk_Window tkwin;		/* Default window to map pages. */

    int reqWidth, reqHeight;	/* Requested tab size. */
    int constWidth;
    int borderWidth;		/* Width of 3D border around the tab's
				 * label. */
    int pad;			/* Extra padding of a tab entry */

    XColor *activeFgColor;	/* Active foreground. */
    Tk_3DBorder activeBorder;	/* Active background. */
    XColor *selColor;		/* Selected foreground. */
    Blt_Font font;
    XColor *textColor;

    Blt_Font selFont;
    Tk_3DBorder border;		/* Normal background. */
    Tk_3DBorder selBorder;	/* Selected background. */

    Blt_Dashes dashes;
    GC normalGC, activeGC;
    int relief;
    char *command;
    char *perfCommand;		/* Command (malloc-ed) invoked when the tab
				 * is selected */
    float angle;
    int textSide;

} TabAttributes;

struct NotebookStruct {
    Tk_Window tkwin;		/* Window that embodies the widget.  NULL
                                 * means that the window has been destroyed
                                 * but the data structures haven't yet been
                                 * cleaned up.*/

    Display *display;		/* Display containing widget; needed, among
                                 * other things, to release resources after
                                 * tkwin has already gone away. */

    Tcl_Interp *interp;		/* Interpreter associated with widget. */

    Tcl_Command cmdToken;	/* Token for widget's command. */

    unsigned int flags;		/* For bitfield definitions, see below */

    int inset;			/* Total width of all borders, including
				 * traversal highlight and 3-D border.
				 * Indicates how much interior stuff must be
				 * offset from outside edges to leave room for
				 * borders. */

    int inset2;			/* Total width of 3-D folder border + corner,
				 * Indicates how much interior stuff must
				 * be offset from outside edges of folder.*/

    int yPad;			/* Extra offset for selected tab. Only for
				 * single tiers. */

    int pageTop;		/* Offset from top of notebook to the start of
				 * the page. */

    Tk_Cursor cursor;		/* X Cursor */

    Tk_3DBorder border;		/* 3D border surrounding the window. */
    int borderWidth;		/* Width of 3D border. */
    int relief;			/* 3D border relief. */

    XColor *shadowColor;	/* Shadow color around folder. */
    /*
     * Focus highlight ring
     */
    int highlightWidth;		/* Width in pixels of highlight to draw around
				 * widget when it has the focus.  <= 0 means
				 * don't draw a highlight. */
    XColor *highlightBgColor;	/* Color for drawing traversal highlight area
				 * when highlight is off. */
    XColor *highlightColor;	/* Color for drawing traversal highlight. */

    GC highlightGC;		/* GC for focus highlight. */

    char *takeFocus;		/* Says whether to select this widget during
				 * tab traveral operations.  This value isn't
				 * used in C code, but for the widget's Tcl
				 * bindings. */


    int side;			/* Orientation of the notebook: either
				 * SIDE_LEFT, SIDE_RIGHT, SIDE_TOP, or
				 * SIDE_BOTTOM. */

    int slant;
    int overlap;
    int gap;
    int tabWidth, tabHeight;
    int xSelectPad, ySelectPad;	/* Padding around label of the selected
				 * tab. */
    int outerPad;		/* Padding around the exterior of the notebook
				 * and folder. */

    TabAttributes defTabStyle;	/* Global attribute information specific to
				 * tabs. */
    Blt_Tile tile;

    int reqWidth, reqHeight;	/* Requested dimensions of the notebook
				 * window. */
    int pageWidth, pageHeight;	/* Dimensions of a page in the folder. */
    int reqPageWidth, reqPageHeight; /* Requested dimensions of a page. */

    int lastX, lastY;
    /*
     * Scrolling information:
     */
    int worldWidth;
    int scrollOffset;		/* Offset of viewport in world coordinates. */
    Tcl_Obj *scrollCmdObjPtr;	/* Command strings to control scrollbar.*/

    int scrollUnits;		/* Smallest unit of scrolling for tabs. */

    /*
     * Scanning information:
     */
    int scanAnchor;		/* Scan anchor in screen coordinates. */
    int scanOffset;		/* Offset of the start of the scan in
				 * world coordinates.*/


    int corner;			/* Number of pixels to offset next point when
				 * drawing corners of the folder. */
    int reqTiers;		/* Requested number of tiers. Zero means to
				 * dynamically scroll if there are too many
				 * tabs to be display on a single tier. */
    int nTiers;			/* Actual number of tiers. */

    Blt_HashTable tabImageTable;


    Tab *selectPtr;		/* The currently selected tab.  (i.e. its page
				 * is displayed). */

    Tab *activePtr;		/* Tab last located under the pointer.  It is
				 * displayed with its active foreground /
				 * background colors.  */

    Tab *focusPtr;		/* Tab currently receiving focus. */

    Tab *startPtr;		/* The first tab on the first tier. */

    Blt_Chain chain;		/* List of tab entries. Used to arrange
				 * placement of tabs. */

    Blt_HashTable tabTable;	/* Hash table of tab entries. Used for lookups
				 * of tabs by name. */
    int nextId;

    int nVisible;		/* Number of tabs that are currently visible
				 * in the view port. */

    Blt_BindTable bindTable;	/* Tab binding information */
    Blt_HashTable tagTable;	/* Table of bind tags. */

    int tearoff;
};

static Blt_ConfigSpec configSpecs[] =
{
    {BLT_CONFIG_BORDER, "-activebackground", "activeBackground", 
	"activeBackground", DEF_TNB_ACTIVE_BACKGROUND, 
	Blt_Offset(Notebook, defTabStyle.activeBorder), BLT_CONFIG_COLOR_ONLY},
    {BLT_CONFIG_BORDER, "-activebackground", "activeBackground",
	"activeBackground", DEF_TNB_ACTIVE_BG_MONO, 
	Blt_Offset(Notebook, defTabStyle.activeBorder), BLT_CONFIG_MONO_ONLY},
    {BLT_CONFIG_COLOR, "-activeforeground", "activeForeground",
	"activeForeground", DEF_TNB_ACTIVE_FOREGROUND, 
	Blt_Offset(Notebook, defTabStyle.activeFgColor), BLT_CONFIG_COLOR_ONLY},
    {BLT_CONFIG_COLOR, "-activeforeground", "activeForeground",
	"activeForeground", DEF_TNB_ACTIVE_FG_MONO, 
	Blt_Offset(Notebook, defTabStyle.activeFgColor), BLT_CONFIG_MONO_ONLY},
    {BLT_CONFIG_BORDER, "-background", "background", "Background",
	DEF_TNB_BG_MONO, Blt_Offset(Notebook, border), BLT_CONFIG_MONO_ONLY},
    {BLT_CONFIG_BORDER, "-background", "background", "Background",
	DEF_TNB_BACKGROUND, Blt_Offset(Notebook, border), BLT_CONFIG_COLOR_ONLY},
    {BLT_CONFIG_SYNONYM, "-bd", "borderWidth", (char *)NULL, (char *)NULL, 0, 0},
    {BLT_CONFIG_SYNONYM, "-bg", "background", (char *)NULL, (char *)NULL, 0, 0},
    {BLT_CONFIG_ACTIVE_CURSOR, "-cursor", "cursor", "Cursor",
	DEF_TNB_CURSOR, Blt_Offset(Notebook, cursor), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_PIXELS_NNEG, "-borderwidth", "borderWidth", "BorderWidth",
	DEF_TNB_BORDERWIDTH, Blt_Offset(Notebook, borderWidth),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_DASHES, "-dashes", "dashes", "Dashes",
	DEF_TNB_DASHES, Blt_Offset(Notebook, defTabStyle.dashes),
	BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_SYNONYM, "-fg", "tabForeground", (char *)NULL, 
	(char *)NULL, 0, 0},
    {BLT_CONFIG_FONT, "-font", "font", "Font",
	DEF_TNB_FONT, Blt_Offset(Notebook, defTabStyle.font), 0},
    {BLT_CONFIG_SYNONYM, "-foreground", "tabForeground", (char *)NULL, 
	(char *)NULL, 0, 0},
    {BLT_CONFIG_PIXELS_NNEG, "-gap", "gap", "Gap",
	DEF_TNB_GAP, Blt_Offset(Notebook, gap), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_PIXELS_NNEG, "-height", "height", "Height",
	DEF_TNB_HEIGHT, Blt_Offset(Notebook, reqHeight),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_COLOR, "-highlightbackground", "highlightBackground",
	"HighlightBackground",
	DEF_TNB_HIGHLIGHT_BACKGROUND, Blt_Offset(Notebook, highlightBgColor),
	BLT_CONFIG_COLOR_ONLY},
    {BLT_CONFIG_COLOR, "-highlightbackground", "highlightBackground",
	"HighlightBackground",
	DEF_TNB_HIGHLIGHT_BG_MONO, Blt_Offset(Notebook, highlightBgColor),
	BLT_CONFIG_MONO_ONLY},
    {BLT_CONFIG_COLOR, "-highlightcolor", "highlightColor", "HighlightColor",
	DEF_TNB_HIGHLIGHT_COLOR, Blt_Offset(Notebook, highlightColor), 0},
    {BLT_CONFIG_PIXELS_NNEG, "-highlightthickness", "highlightThickness",
	"HighlightThickness",
	DEF_TNB_HIGHLIGHT_WIDTH, Blt_Offset(Notebook, highlightWidth),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_PIXELS_NNEG, "-outerpad", "outerPad", "OuterPad",
	DEF_TNB_OUTER_PAD, Blt_Offset(Notebook, outerPad),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_PIXELS_NNEG, "-pageheight", "pageHeight", "PageHeight",
	DEF_TNB_PAGE_HEIGHT, Blt_Offset(Notebook, reqPageHeight),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_PIXELS_NNEG, "-pagewidth", "pageWidth", "PageWidth",
	DEF_TNB_PAGE_WIDTH, Blt_Offset(Notebook, reqPageWidth),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_UID, "-perforationcommand", "perforationcommand", 
	"PerforationCommand", DEF_TAB_PERF_COMMAND, 
     Blt_Offset(Notebook, defTabStyle.perfCommand), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_RELIEF, "-relief", "relief", "Relief",
	DEF_TNB_RELIEF, Blt_Offset(Notebook, relief), 0},
    {BLT_CONFIG_DOUBLE, "-rotate", "rotate", "Rotate",
	DEF_TNB_ANGLE, Blt_Offset(Notebook, defTabStyle.angle),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_BOOLEAN, "-samewidth", "sameWidth", "SameWidth",
	DEF_TNB_SAME_WIDTH, Blt_Offset(Notebook, defTabStyle.constWidth),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_OBJ, "-scrollcommand", "scrollCommand", "ScrollCommand",
	(char *)NULL, Blt_Offset(Notebook, scrollCmdObjPtr),BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_PIXELS_POS, "-scrollincrement", "scrollIncrement",
	"ScrollIncrement", DEF_TNB_SCROLL_INCREMENT, 
	Blt_Offset(Notebook, scrollUnits), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_BORDER, "-selectbackground", "selectBackground", "Foreground",
	DEF_TNB_SELECT_BG_MONO, Blt_Offset(Notebook, defTabStyle.selBorder),
	BLT_CONFIG_MONO_ONLY},
    {BLT_CONFIG_BORDER, "-selectbackground", "selectBackground", "Foreground",
	DEF_TNB_SELECT_BACKGROUND, Blt_Offset(Notebook, defTabStyle.selBorder),
	BLT_CONFIG_COLOR_ONLY},
    {BLT_CONFIG_STRING, "-selectcommand", "selectCommand", "SelectCommand",
	DEF_TNB_SELECT_CMD, Blt_Offset(Notebook, defTabStyle.command),
	BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_FONT, "-selectfont", "selectFont", "Font", DEF_TNB_SELECT_FONT,
	Blt_Offset(Notebook, defTabStyle.selFont), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_COLOR, "-selectforeground", "selectForeground", "Background",
	DEF_TNB_SELECT_FG_MONO, Blt_Offset(Notebook, defTabStyle.selColor),
	BLT_CONFIG_MONO_ONLY},
    {BLT_CONFIG_COLOR, "-selectforeground", "selectForeground", "Background",
	DEF_TNB_SELECT_FOREGROUND, Blt_Offset(Notebook, defTabStyle.selColor),
	BLT_CONFIG_COLOR_ONLY},
    {BLT_CONFIG_PIXELS_NNEG, "-selectpad", "selectPad", "SelectPad",
	DEF_TNB_SELECT_PAD, Blt_Offset(Notebook, xSelectPad),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_COLOR, "-shadowcolor", "shadowColor", "ShadowColor",
	DEF_TNB_SHADOW_COLOR, Blt_Offset(Notebook, shadowColor), 0},
    {BLT_CONFIG_SIDE, "-side", "side", "side",
	DEF_TNB_SIDE, Blt_Offset(Notebook, side), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_CUSTOM, "-slant", "slant", "Slant", DEF_TNB_SLANT, 
	Blt_Offset(Notebook, slant), BLT_CONFIG_DONT_SET_DEFAULT, &slantOption},
    {BLT_CONFIG_BORDER, "-tabbackground", "tabBackground", "Background",
	DEF_TNB_TAB_BG_MONO, Blt_Offset(Notebook, defTabStyle.border),
	BLT_CONFIG_MONO_ONLY},
    {BLT_CONFIG_BORDER, "-tabbackground", "tabBackground", "Background",
	DEF_TNB_TAB_BACKGROUND, Blt_Offset(Notebook, defTabStyle.border),
	BLT_CONFIG_COLOR_ONLY},
    {BLT_CONFIG_PIXELS_NNEG, "-tabborderwidth", "tabBorderWidth", "BorderWidth",
	DEF_TNB_BORDERWIDTH, Blt_Offset(Notebook, defTabStyle.borderWidth),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_COLOR, "-tabforeground", "tabForeground", "Foreground",
	DEF_TNB_TEXT_COLOR, Blt_Offset(Notebook, defTabStyle.textColor),
	BLT_CONFIG_COLOR_ONLY},
    {BLT_CONFIG_COLOR, "-tabforeground", "tabForeground", "Foreground",
	DEF_TNB_TEXT_MONO, Blt_Offset(Notebook, defTabStyle.textColor),
	BLT_CONFIG_MONO_ONLY},
    {BLT_CONFIG_RELIEF, "-tabrelief", "tabRelief", "TabRelief",
	DEF_TNB_TAB_RELIEF, Blt_Offset(Notebook, defTabStyle.relief), 0},
    {BLT_CONFIG_STRING, "-takefocus", "takeFocus", "TakeFocus",
	DEF_TNB_TAKE_FOCUS, Blt_Offset(Notebook, takeFocus), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_BOOLEAN, "-tearoff", "tearoff", "Tearoff",
	DEF_TNB_TEAROFF, Blt_Offset(Notebook, tearoff),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_SIDE, "-textside", "textSide", "TextSide",
	DEF_TNB_TEXT_SIDE, Blt_Offset(Notebook, defTabStyle.textSide),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_INT_POS, "-tiers", "tiers", "Tiers",
	DEF_TNB_TIERS, Blt_Offset(Notebook, reqTiers), 
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_TILE, "-tile", "tile", "Tile",
	(char *)NULL, Blt_Offset(Notebook, tile), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_PIXELS_NNEG, "-width", "width", "Width", 
	DEF_TNB_WIDTH, Blt_Offset(Notebook, reqWidth), 
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_END, (char *)NULL, (char *)NULL, (char *)NULL,
	(char *)NULL, 0, 0}
};

/* Forward Declarations */
static Blt_BindPickProc PickTab;
static Blt_BindTagProc GetTags;
static Blt_TileChangedProc TileChangedProc;
static Tcl_CmdDeleteProc NotebookInstDeletedCmd;
static Tcl_FreeProc DestroyNotebook;
static Tcl_FreeProc DestroyTearoff;
static Tcl_IdleProc AdoptWindow;
static Tcl_IdleProc DisplayNotebook;
static Tcl_IdleProc DisplayTearoff;
static Tcl_ObjCmdProc NotebookCmd;
static Tcl_ObjCmdProc NotebookInstCmd;
static Tk_EventProc EmbeddedWidgetEventProc;
static Tk_EventProc NotebookEventProc;
static Tk_EventProc TearoffEventProc;
static Tk_ImageChangedProc ImageChangedProc;

static void DrawLabel _ANSI_ARGS_((Notebook *nbPtr, Tab *tabPtr,
	Drawable drawable));
static void DrawFolder _ANSI_ARGS_((Notebook *nbPtr, Tab *tabPtr,
	Drawable drawable));

static void GetWindowRectangle _ANSI_ARGS_((Tab *tabPtr, Tk_Window parent,
	int tearOff, XRectangle *rectPtr));
static void ArrangeWindow _ANSI_ARGS_((Tk_Window tkwin, XRectangle *rectPtr,
	int force));
static void EventuallyRedraw _ANSI_ARGS_((Notebook *nbPtr));
static void EventuallyRedrawTearoff _ANSI_ARGS_((Tab *tabPtr));
static void ComputeLayout _ANSI_ARGS_((Notebook *nbPtr));
static void DrawOuterBorders _ANSI_ARGS_((Notebook *nbPtr, 
	Drawable drawable));

typedef int (NotebookCmdProc)(Notebook *nbPtr, Tcl_Interp *interp, 
	int objc, Tcl_Obj *const *objv);

static ClientData
MakeTag(
    Notebook *nbPtr,
    const char *tagName)
{
    Blt_HashEntry *hPtr;
    int isNew;

    hPtr = Blt_CreateHashEntry(&nbPtr->tagTable, tagName, &isNew);
    return Blt_GetHashKey(&nbPtr->tagTable, hPtr);
}

/*
 *---------------------------------------------------------------------------
 *
 * WorldToScreen --
 *
 *	Converts world coordinates to screen coordinates. Note that the world
 *	view is always tabs up.
 *
 * Results:
 *	The screen coordinates are returned via *xScreenPtr and *yScreenPtr.
 *
 *---------------------------------------------------------------------------
 */
static void
WorldToScreen(
    Notebook *nbPtr,
    int x, int y,
    int *xScreenPtr, 
    int *yScreenPtr)
{
    int sx, sy;

    sx = sy = 0;		/* Suppress compiler warning. */

    /* Translate world X-Y to screen coordinates */
    /*
     * Note that the world X-coordinate is translated by the selected label's
     * X padding. This is done only to keep the scroll range is between 0.0
     * and 1.0, rather adding/subtracting the pad in various locations.  It
     * may be changed back in the future.
     */
    x += (nbPtr->inset + nbPtr->xSelectPad - nbPtr->scrollOffset);
    y += nbPtr->inset + nbPtr->yPad;

    switch (nbPtr->side) {
    case SIDE_TOP:
	sx = x, sy = y;		/* Do nothing */
	break;
    case SIDE_RIGHT:
	sx = Tk_Width(nbPtr->tkwin) - y;
	sy = x;
	break;
    case SIDE_LEFT:
	sx = y, sy = x;		/* Flip coordinates */
	break;
    case SIDE_BOTTOM:
	sx = x;
	sy = Tk_Height(nbPtr->tkwin) - y;
	break;
    }
    *xScreenPtr = sx;
    *yScreenPtr = sy;
}

/*
 *---------------------------------------------------------------------------
 *
 * EventuallyRedraw --
 *
 *	Queues a request to redraw the widget at the next idle point.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Information gets redisplayed.  Right now we don't do selective
 *	redisplays: the whole window will be redrawn.
 *
 *---------------------------------------------------------------------------
 */
static void
EventuallyRedraw(Notebook *nbPtr)
{
    if ((nbPtr->tkwin != NULL) && !(nbPtr->flags & TNB_REDRAW)) {
	nbPtr->flags |= TNB_REDRAW;
	Tcl_DoWhenIdle(DisplayNotebook, nbPtr);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * EventuallyRedrawTearoff --
 *
 *	Queues a request to redraw the tearoff at the next idle point.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Information gets redisplayed.  Right now we don't do selective
 *	redisplays:  the whole window will be redrawn.
 *
 *---------------------------------------------------------------------------
 */
static void
EventuallyRedrawTearoff(Tab *tabPtr)
{
    if ((tabPtr->tkwin != NULL) && !(tabPtr->flags & TAB_REDRAW)) {
	tabPtr->flags |= TAB_REDRAW;
	Tcl_DoWhenIdle(DisplayTearoff, tabPtr);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * ImageChangedProc
 *
 *	This routine is called whenever an image displayed in a tab changes.
 *	In this case, we assume that everything will change and queue a
 *	request to re-layout and redraw the entire notebook.
 *
 * Results:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
/* ARGSUSED */
static void
ImageChangedProc(
    ClientData clientData,
    int x, int y,		/* Not used. */
    int width, int height,	/* Not used. */
    int imageWidth, int imageHeight) /* Not used. */
{
    Notebook *nbPtr = clientData;

    nbPtr->flags |= (TNB_LAYOUT | TNB_SCROLL);
    EventuallyRedraw(nbPtr);
}

/*
 *---------------------------------------------------------------------------
 *
 * GetImage --
 *
 *	This is a wrapper procedure for Tk_GetImage. The problem is that if
 *	the same image is used repeatedly in the same widget, the separate
 *	instances are saved in a linked list.  This makes it especially slow
 *	to destroy the widget.  As a workaround, this routine hashes the image
 *	and maintains a reference count for it.
 *
 * Results:
 *	Returns a pointer to the new image.
 *
 *---------------------------------------------------------------------------
 */
static TabImage *
GetImage(
    Notebook *nbPtr,
    Tcl_Interp *interp,
    Tk_Window tkwin,
    char *name)
{
    TabImage *tabImgPtr;
    int isNew;
    Blt_HashEntry *hPtr;

    hPtr = Blt_CreateHashEntry(&nbPtr->tabImageTable, name, &isNew);
    if (isNew) {
	Tk_Image tkImage;
	int width, height;

	tkImage = Tk_GetImage(interp, tkwin, name, ImageChangedProc, nbPtr);
	if (tkImage == NULL) {
	    Blt_DeleteHashEntry(&nbPtr->tabImageTable, hPtr);
	    return NULL;
	}
	Tk_SizeOfImage(tkImage, &width, &height);
	tabImgPtr = Blt_MallocAssert(sizeof(TabImage));
	tabImgPtr->tkImage = tkImage;
	tabImgPtr->hashPtr = hPtr;
	tabImgPtr->refCount = 1;
	tabImgPtr->width = width;
	tabImgPtr->height = height;
	Blt_SetHashValue(hPtr, tabImgPtr);
    } else {
	tabImgPtr = Blt_GetHashValue(hPtr);
	tabImgPtr->refCount++;
    }
    return tabImgPtr;
}

/*
 *---------------------------------------------------------------------------
 *
 * FreeImage --
 *
 *	Releases the image if it's not being used anymore by this widget.
 *	Note there may be several uses of the same image by many tabs.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The reference count is decremented and the image is freed is it's not
 *	being used anymore.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static void
FreeImage(
    ClientData clientData,
    Display *display,		/* Not used. */
    char *widgRec,
    int offset)
{
    TabImage **tabImgPtrPtr = (TabImage **)(widgRec + offset);
    Notebook *nbPtr = clientData;

    if (*tabImgPtrPtr != NULL) {
	TabImage *tabImgPtr = *tabImgPtrPtr;
	tabImgPtr->refCount--;

	if (tabImgPtr->refCount == 0) {
	    Blt_DeleteHashEntry(&nbPtr->tabImageTable, tabImgPtr->hashPtr);
	    Tk_FreeImage(tabImgPtr->tkImage);
	    Blt_Free(tabImgPtr);
	    *tabImgPtrPtr = NULL;
	}
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * ObjToImage --
 *
 *	Converts an image name into a Tk image token.
 *
 * Results:
 *	If the string is successfully converted, TCL_OK is returned.
 *	Otherwise, TCL_ERROR is returned and an error message is left in
 *	interpreter's result field.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToImage(
    ClientData clientData,	/* Contains a pointer to the notebook containing
				 * this image. */
    Tcl_Interp *interp,		/* Interpreter to send results back to */
    Tk_Window tkwin,		/* Window associated with the notebook. */
    Tcl_Obj *objPtr,		/* String representation */
    char *widgRec,		/* Widget record */
    int offset,			/* Offset to field in structure */
    int flags)	
{
    Notebook *nbPtr = clientData;
    TabImage **imagePtrPtr = (TabImage **) (widgRec + offset);
    TabImage *tabImgPtr;
    char *string;

    tabImgPtr = NULL;
    string = Tcl_GetString(objPtr);
    if ((string != NULL) && (*string != '\0')) {
	tabImgPtr = GetImage(nbPtr, interp, tkwin, string);
	if (tabImgPtr == NULL) {
	    return TCL_ERROR;
	}
    }
    *imagePtrPtr = tabImgPtr;
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ImageToObj --
 *
 *	Converts the Tk image back to a Tcl_Obj representation (i.e.  its
 *	name).
 *
 * Results:
 *	The name of the image is returned as a Tcl_Obj.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
ImageToObj(
    ClientData clientData,	/* Pointer to notebook containing image. */
    Tcl_Interp *interp,
    Tk_Window tkwin,		/* Not used. */
    char *widgRec,		/* Widget record */
    int offset,			/* Offset to field in structure */
    int flags)	
{
    Notebook *nbPtr = clientData;
    TabImage **imagePtrPtr = (TabImage **) (widgRec + offset);
    Tcl_Obj *objPtr;

    if (*imagePtrPtr == NULL) {
	objPtr = Tcl_NewStringObj("", -1);
    } else {
	char *imgName;

	imgName = Blt_GetHashKey(&nbPtr->tabImageTable, 
		 (*imagePtrPtr)->hashPtr);
	objPtr = Tcl_NewStringObj(imgName, -1);
    }
    return objPtr;
}

/*
 *---------------------------------------------------------------------------
 *
 * ObjToSlant --
 *
 *	Converts the slant style string into its numeric representation.
 *
 *	Valid style strings are:
 *
 *	  "none"   Both sides are straight.
 * 	  "left"   Left side is slanted.
 *	  "right"  Right side is slanted.
 *	  "both"   Both sides are slanted.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToSlant(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,		/* Interpreter to send results back to */
    Tk_Window tkwin,		/* Not used. */
    Tcl_Obj *objPtr,		/* String representation of attribute. */
    char *widgRec,		/* Widget record */
    int offset,			/* Offset to field in structure */
    int flags)	
{
    int *slantPtr = (int *)(widgRec + offset);
    size_t length;
    char c;
    char *string;

    string = Tcl_GetString(objPtr);
    c = string[0];
    length = strlen(string);
    if ((c == 'n') && (strncmp(string, "none", length) == 0)) {
	*slantPtr = SLANT_NONE;
    } else if ((c == 'l') && (strncmp(string, "left", length) == 0)) {
	*slantPtr = SLANT_LEFT;
    } else if ((c == 'r') && (strncmp(string, "right", length) == 0)) {
	*slantPtr = SLANT_RIGHT;
    } else if ((c == 'b') && (strncmp(string, "both", length) == 0)) {
	*slantPtr = SLANT_BOTH;
    } else {
	Tcl_AppendResult(interp, "bad argument \"", string,
	    "\": should be \"none\", \"left\", \"right\", or \"both\"",
	    (char *)NULL);
	return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * SlantToString --
 *
 *	Returns the slant style string based upon the slant flags.
 *
 * Results:
 *	The slant style string is returned.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
SlantToObj(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,
    Tk_Window tkwin,		/* Not used. */
    char *widgRec,		/* Widget structure record. */
    int offset,			/* Offset to field in structure */
    int flags)	
{
    int slant = *(int *)(widgRec + offset);
    const char *string;

    switch (slant) {
    case SLANT_LEFT:
	string = "left";
	break;

    case SLANT_RIGHT:
	string = "right";
	break;

    case SLANT_NONE:
	string = "none";
	break;

    case SLANT_BOTH:
	string = "both";
	break;

    default:
	string = "unknown value";
	break;
    }
    return Tcl_NewStringObj(string, -1);
}

/*
 *---------------------------------------------------------------------------
 *
 * ObjToChild --
 *
 *	Converts a window name into Tk window.
 *
 * Results:
 *	If the string is successfully converted, TCL_OK is returned.
 *	Otherwise, TCL_ERROR is returned and an error message is left
 *	in interpreter's result field.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToChild(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,		/* Interpreter to send results back to */
    Tk_Window parent,		/* Parent window */
    Tcl_Obj *objPtr,		/* String representation. */
    char *widgRec,		/* Widget record */
    int offset,			/* Offset to field in structure */
    int flags)	
{
    Tab *tabPtr = (Tab *)widgRec;
    Tk_Window *tkwinPtr = (Tk_Window *)(widgRec + offset);
    Tk_Window old, tkwin;
    Notebook *nbPtr;
    char *string;

    old = *tkwinPtr;
    tkwin = NULL;
    nbPtr = tabPtr->nbPtr;
    string = Tcl_GetString(objPtr);
    if (string[0] != '\0') {
	tkwin = Tk_NameToWindow(interp, string, parent);
	if (tkwin == NULL) {
	    return TCL_ERROR;
	}
	if (tkwin == old) {
	    return TCL_OK;
	}
	/*
	 * Allow only widgets that are children of the notebook to be embedded
	 * into the page.  This way we can make assumptions about the window
	 * based upon its parent; either it's the notebook window or it has
	 * been torn off.
	 */
	parent = Tk_Parent(tkwin);
	if (parent != nbPtr->tkwin) {
	    Tcl_AppendResult(interp, "can't manage \"", Tk_PathName(tkwin),
		"\" in notebook \"", Tk_PathName(nbPtr->tkwin), "\"",
		(char *)NULL);
	    return TCL_ERROR;
	}
	Tk_ManageGeometry(tkwin, &tabMgrInfo, tabPtr);
	Tk_CreateEventHandler(tkwin, StructureNotifyMask, 
		EmbeddedWidgetEventProc, tabPtr);

	/*
	 * We need to make the window to exist immediately.  If the window is
	 * torn off (placed into another container window), the timing between
	 * the container and the its new child (this window) gets tricky.
	 * This should work for Tk 4.2.
	 */
	Tk_MakeWindowExist(tkwin);
    }
    if (old != NULL) {
	if (tabPtr->container != NULL) {
	    Tcl_EventuallyFree(tabPtr, DestroyTearoff);
	}
	Tk_DeleteEventHandler(old, StructureNotifyMask, 
	      EmbeddedWidgetEventProc, tabPtr);
	Tk_ManageGeometry(old, (Tk_GeomMgr *) NULL, tabPtr);
	Tk_UnmapWindow(old);
    }
    *tkwinPtr = tkwin;
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ChildToObj --
 *
 *	Converts the Tk window back to a Tcl_Obj (i.e. its name).
 *
 * Results:
 *	The name of the window is returned.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
ChildToObj(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,
    Tk_Window parent,		/* Not used. */
    char *widgRec,		/* Widget record */
    int offset,			/* Offset to field in structure */
    int flags)	
{
    Tk_Window tkwin = *(Tk_Window *)(widgRec + offset);
    Tcl_Obj *objPtr;

    if (tkwin == NULL) {
	objPtr = Tcl_NewStringObj("", -1);
    } else {
	objPtr = Tcl_NewStringObj(Tk_PathName(tkwin), -1);
    }
    return objPtr;
}

static int
WorldY(Tab *tabPtr)
{
    int tier;

    tier = tabPtr->nbPtr->nTiers - tabPtr->tier;
    return tier * tabPtr->nbPtr->tabHeight;
}

static int
TabIndex(Notebook *nbPtr, Tab *tabPtr)
{
    Tab *t2Ptr;
    int count;
    Blt_ChainLink link;
    
    count = 0;
    for (link = Blt_ChainFirstLink(nbPtr->chain); link != NULL;
	link = Blt_ChainNextLink(link)) {
	t2Ptr = Blt_ChainGetValue(link);
	if (t2Ptr == tabPtr) {
	    return count;
	}
	count++;
    }
    return -1;
}

/*
 *---------------------------------------------------------------------------
 *
 * RenumberTiers --
 *
 *	In multi-tier mode, we need to find the start of the tier containing
 *	the newly selected tab.
 *
 *	Tiers are draw from the last tier to the first, so that the the
 *	lower-tiered tabs will partially cover the bottoms of tab directly
 *	above it.  This simplifies the drawing of tabs because we don't worry
 *	how tabs are clipped by their neighbors.
 *
 *	In addition, tabs are re-marked with the correct tier number.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Renumbering the tab's tier will change the vertical placement
 *	of the tab (i.e. shift tiers).
 *
 *---------------------------------------------------------------------------
 */
static void
RenumberTiers(Notebook *nbPtr, Tab *tabPtr)
{
    int tier;
    Blt_ChainLink link, last;

    nbPtr->focusPtr = nbPtr->selectPtr = tabPtr;
    Blt_SetFocusItem(nbPtr->bindTable, nbPtr->focusPtr, NULL);

    tier = tabPtr->tier;
    for (link = Blt_ChainPrevLink(tabPtr->link); link != NULL; link = last) {
	Tab *prevPtr;

	last = Blt_ChainPrevLink(link);
	prevPtr = Blt_ChainGetValue(link);
	if ((prevPtr == NULL) || (prevPtr->tier != tier)) {
	    break;
	}
	tabPtr = prevPtr;
    }
    nbPtr->startPtr = tabPtr;
    for (link = Blt_ChainFirstLink(nbPtr->chain); link != NULL;
	link = Blt_ChainNextLink(link)) {
	tabPtr = Blt_ChainGetValue(link);
	tabPtr->tier = (tabPtr->tier - tier + 1);
	if (tabPtr->tier < 1) {
	    tabPtr->tier += nbPtr->nTiers;
	}
	tabPtr->worldY = WorldY(tabPtr);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * PickTab --
 *
 *	Searches the tab located within the given screen X-Y coordinates in
 *	the viewport.  Note that tabs overlap slightly, so that its important
 *	to search from the innermost tier out.
 *
 * Results:
 *	Returns the pointer to the tab.  If the pointer isn't contained by any
 *	tab, NULL is returned.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static ClientData
PickTab(ClientData clientData, int x, int y, ClientData *contextPtr)
{
    Notebook *nbPtr = clientData;
    Tab *tabPtr;
    Blt_ChainLink link;

    if (contextPtr != NULL) {
	*contextPtr = NULL;
    }
    tabPtr = nbPtr->selectPtr;
    if ((nbPtr->tearoff) && (tabPtr != NULL) && 
	(tabPtr->container == NULL) && (tabPtr->tkwin != NULL)) {
	int top, bottom, left, right;
	int sx, sy;

	/* Check first for perforation on the selected tab. */
	WorldToScreen(nbPtr, tabPtr->worldX + 2, 
	      tabPtr->worldY + tabPtr->worldHeight + 4, &sx, &sy);
	if (nbPtr->side & TAB_HORIZONTAL) {
	    left = sx - 2;
	    right = left + tabPtr->screenWidth;
	    top = sy - 4;
	    bottom = sy + 4;
	} else {
	    left = sx - 4;
	    right = sx + 4;
	    top = sy - 2;
	    bottom = top + tabPtr->screenHeight;
	}
	if ((x >= left) && (y >= top) && (x < right) && (y < bottom)) {
	    if (contextPtr != NULL) {
		*contextPtr = TAB_PERFORATION;
	    }
	    return nbPtr->selectPtr;
	}
    } 
    for (link = Blt_ChainFirstLink(nbPtr->chain); link != NULL;
	link = Blt_ChainNextLink(link)) {
	tabPtr = Blt_ChainGetValue(link);
	if (!(tabPtr->flags & TAB_VISIBLE)) {
	    continue;
	}
	if ((x >= tabPtr->screenX) && (y >= tabPtr->screenY) &&
	    (x <= (tabPtr->screenX + tabPtr->screenWidth)) &&
	    (y < (tabPtr->screenY + tabPtr->screenHeight))) {
	    if (contextPtr != NULL) {
		*contextPtr = TAB_LABEL;
	    }
	    return tabPtr;
	}
    }
    return NULL;
}

static Tab *
TabLeft(Tab *tabPtr)
{
    Blt_ChainLink link;

    link = Blt_ChainPrevLink(tabPtr->link);
    if (link != NULL) {
	Tab *newPtr;

	newPtr = Blt_ChainGetValue(link);
	/* Move only if the next tab is on another tier. */
	if (newPtr->tier == tabPtr->tier) {
	    tabPtr = newPtr;
	}
    }
    return tabPtr;
}

static Tab *
TabRight(Tab *tabPtr)
{
    Blt_ChainLink link;

    link = Blt_ChainNextLink(tabPtr->link);
    if (link != NULL) {
	Tab *newPtr;

	newPtr = Blt_ChainGetValue(link);
	/* Move only if the next tab is on another tier. */
	if (newPtr->tier == tabPtr->tier) {
	    tabPtr = newPtr;
	}
    }
    return tabPtr;
}

static Tab *
TabUp(Tab *tabPtr)
{
    if (tabPtr != NULL) {
	Notebook *nbPtr;
	int x, y;
	int worldX, worldY;
	
	nbPtr = tabPtr->nbPtr;
	worldX = tabPtr->worldX + (tabPtr->worldWidth / 2);
	worldY = tabPtr->worldY - (nbPtr->tabHeight / 2);
	WorldToScreen(nbPtr, worldX, worldY, &x, &y);
	
	tabPtr = (Tab *)PickTab(nbPtr, x, y, NULL);
	if (tabPtr == NULL) {
	    /*
	     * We might have inadvertly picked the gap between two tabs, so if
	     * the first pick fails, try again a little to the left.
	     */
	    WorldToScreen(nbPtr, worldX + nbPtr->gap, worldY, &x, &y);
	    tabPtr = (Tab *)PickTab(nbPtr, x, y, NULL);
	}
	if ((tabPtr == NULL) &&
	    (nbPtr->focusPtr->tier < (nbPtr->nTiers - 1))) {
	    worldY -= nbPtr->tabHeight;
	    WorldToScreen(nbPtr, worldX, worldY, &x, &y);
	    tabPtr = (Tab *)PickTab(nbPtr, x, y, NULL);
	}
	if (tabPtr == NULL) {
	    tabPtr = nbPtr->focusPtr;
	}
    }
    return tabPtr;
}

static Tab *
TabDown(Tab *tabPtr)
{
    if (tabPtr != NULL) {
	Notebook *nbPtr;
	int x, y;
	int worldX, worldY;

	nbPtr = tabPtr->nbPtr;
	worldX = tabPtr->worldX + (tabPtr->worldWidth / 2);
	worldY = tabPtr->worldY + (3 * nbPtr->tabHeight) / 2;
	WorldToScreen(nbPtr, worldX, worldY, &x, &y);
	tabPtr = (Tab *)PickTab(nbPtr, x, y, NULL);
	if (tabPtr == NULL) {
	    /*
	     * We might have inadvertly picked the gap between two tabs, so if
	     * the first pick fails, try again a little to the left.
	     */
	    WorldToScreen(nbPtr, worldX - nbPtr->gap, worldY, &x, &y);
	    tabPtr = (Tab *)PickTab(nbPtr, x, y, NULL);
	}
	if ((tabPtr == NULL) && (nbPtr->focusPtr->tier > 2)) {
	    worldY += nbPtr->tabHeight;
	    WorldToScreen(nbPtr, worldX, worldY, &x, &y);
	    tabPtr = (Tab *)PickTab(nbPtr, x, y, NULL);
	}
	if (tabPtr == NULL) {
	    tabPtr = nbPtr->focusPtr;
	}
    }
    return tabPtr;
}

/*
 *---------------------------------------------------------------------------
 *
 * GetTabFromObj --
 *
 *	Converts a string representing a tab index into a tab pointer.  The
 *	index may be in one of the following forms:
 *
 *	 number		Tab at position in the list of tabs.
 *	 @x,y		Tab closest to the specified X-Y screen coordinates.
 *	 "active"	Tab mouse is located over.
 *	 "focus"	Tab is the widget's focus.
 *	 "select"	Currently selected tab.
 *	 "right"	Next tab from the focus tab.
 *	 "left"		Previous tab from the focus tab.
 *	 "up"		Next tab from the focus tab.
 *	 "down"		Previous tab from the focus tab.
 *	 "end"		Last tab in list.
 *
 * Results:
 *	If the string is successfully converted, TCL_OK is returned.  The
 *	pointer to the node is returned via tabPtrPtr.  Otherwise, TCL_ERROR
 *	is returned and an error message is left in interpreter's result
 *	field.
 *
 *---------------------------------------------------------------------------
 */
static int
GetTabFromObj(Notebook *nbPtr, Tcl_Obj *objPtr, Tab **tabPtrPtr, int allowNull)
{
    Tab *tabPtr;
    Blt_ChainLink link;
    int position;
    char c;
    char *string;

    string = Tcl_GetString(objPtr);
    c = string[0];
    tabPtr = NULL;
    if (nbPtr->focusPtr == NULL) {
	nbPtr->focusPtr = nbPtr->selectPtr;
	Blt_SetFocusItem(nbPtr->bindTable, nbPtr->focusPtr, NULL);
    }
    if ((isdigit(UCHAR(c))) &&
	(Tcl_GetIntFromObj(nbPtr->interp, objPtr, &position) == TCL_OK)) {
	link = Blt_ChainGetNthLink(nbPtr->chain, position);
	if (link == NULL) {
	    Tcl_AppendResult(nbPtr->interp, "can't find tab \"", string,
		"\" in \"", Tk_PathName(nbPtr->tkwin), 
		"\": no such index", (char *)NULL);
	    return TCL_ERROR;
	}
	tabPtr = Blt_ChainGetValue(link);
    } else if ((c == 'a') && (strcmp(string, "active") == 0)) {
	tabPtr = nbPtr->activePtr;
    } else if ((c == 'c') && (strcmp(string, "current") == 0)) {
	tabPtr = (Tab *)Blt_GetCurrentItem(nbPtr->bindTable);
    } else if ((c == 's') && (strcmp(string, "select") == 0)) {
	tabPtr = nbPtr->selectPtr;
    } else if ((c == 'f') && (strcmp(string, "focus") == 0)) {
	tabPtr = nbPtr->focusPtr;
    } else if ((c == 'u') && (strcmp(string, "up") == 0)) {
	switch (nbPtr->side) {
	case SIDE_LEFT:
	case SIDE_RIGHT:
	    tabPtr = TabLeft(nbPtr->focusPtr);
	    break;
	    
	case SIDE_BOTTOM:
	    tabPtr = TabDown(nbPtr->focusPtr);
	    break;
	    
	case SIDE_TOP:
	    tabPtr = TabUp(nbPtr->focusPtr);
	    break;
	}
    } else if ((c == 'd') && (strcmp(string, "down") == 0)) {
	switch (nbPtr->side) {
	case SIDE_LEFT:
	case SIDE_RIGHT:
	    tabPtr = TabRight(nbPtr->focusPtr);
	    break;
	    
	case SIDE_BOTTOM:
	    tabPtr = TabUp(nbPtr->focusPtr);
	    break;
	    
	case SIDE_TOP:
	    tabPtr = TabDown(nbPtr->focusPtr);
	    break;
	}
    } else if ((c == 'l') && (strcmp(string, "left") == 0)) {
	switch (nbPtr->side) {
	case SIDE_LEFT:
	    tabPtr = TabUp(nbPtr->focusPtr);
	    break;
	    
	case SIDE_RIGHT:
	    tabPtr = TabDown(nbPtr->focusPtr);
	    break;
	    
	case SIDE_BOTTOM:
	case SIDE_TOP:
	    tabPtr = TabLeft(nbPtr->focusPtr);
	    break;
	}
    } else if ((c == 'r') && (strcmp(string, "right") == 0)) {
	switch (nbPtr->side) {
	case SIDE_LEFT:
	    tabPtr = TabDown(nbPtr->focusPtr);
	    break;
	    
	case SIDE_RIGHT:
	    tabPtr = TabUp(nbPtr->focusPtr);
	    break;
	    
	case SIDE_BOTTOM:
	case SIDE_TOP:
	    tabPtr = TabRight(nbPtr->focusPtr);
	    break;
	}
    } else if ((c == 'e') && (strcmp(string, "end") == 0)) {
	link = Blt_ChainLastLink(nbPtr->chain);
	if (link != NULL) {
	    tabPtr = Blt_ChainGetValue(link);
	}
    } else if (c == '@') {
	int x, y;

	if (Blt_GetXY(nbPtr->interp, nbPtr->tkwin, string, &x, &y) 
	    != TCL_OK) {
	    return TCL_ERROR;
	}
	tabPtr = (Tab *)PickTab(nbPtr, x, y, NULL);
    } else {
	Blt_HashEntry *hPtr;

	hPtr = Blt_FindHashEntry(&nbPtr->tabTable, string);
	if (hPtr != NULL) {
	    tabPtr = Blt_GetHashValue(hPtr);
	}
    }
    *tabPtrPtr = tabPtr;
    Tcl_ResetResult(nbPtr->interp);

    if ((!allowNull) && (tabPtr == NULL)) {
	Tcl_AppendResult(nbPtr->interp, "can't find tab \"", string,
	    "\" in \"", Tk_PathName(nbPtr->tkwin), "\"", (char *)NULL);
	return TCL_ERROR;
    }	
    return TCL_OK;
}

static Tab *
NextOrLastTab(Tab *tabPtr)
{
    if (tabPtr->link != NULL) {
	Blt_ChainLink link;

	link = Blt_ChainNextLink(tabPtr->link);
	if (link == NULL) {
	    link = Blt_ChainPrevLink(tabPtr->link);
	}
	if (link != NULL) {
	    return Blt_ChainGetValue(link);
	}
    }
    return NULL;
}

/*
 *---------------------------------------------------------------------------
 *
 * EmbeddedWidgetEventProc --
 *
 * 	This procedure is invoked by the Tk dispatcher for various events on
 * 	embedded widgets contained in the notebook.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	When an embedded widget gets deleted, internal structures get cleaned
 *	up.  When it gets resized, the notebook is redisplayed.
 *
 *---------------------------------------------------------------------------
 */
static void
EmbeddedWidgetEventProc(ClientData clientData, XEvent *eventPtr)
{
    Tab *tabPtr = clientData;

    if ((tabPtr == NULL) || (tabPtr->tkwin == NULL)) {
	return;
    }
    switch (eventPtr->type) {
    case ConfigureNotify:
	/*
	 * If the window's requested size changes, redraw the window.  But
	 * only if it's currently the selected page.
	 */
	if ((tabPtr->container == NULL) && (Tk_IsMapped(tabPtr->tkwin)) &&
	    (tabPtr->nbPtr->selectPtr == tabPtr)) {
	    EventuallyRedraw(tabPtr->nbPtr);
	}
	break;

    case DestroyNotify:
	/*
	 * Mark the tab as deleted by dereferencing the Tk window
	 * pointer. Redraw the window only if the tab is currently visible.
	 */
	if ((Tk_IsMapped(tabPtr->tkwin)) &&
	    (tabPtr->nbPtr->selectPtr == tabPtr)) {
	    EventuallyRedraw(tabPtr->nbPtr);
	}
	Tk_DeleteEventHandler(tabPtr->tkwin, StructureNotifyMask,
	    EmbeddedWidgetEventProc, tabPtr);
	tabPtr->tkwin = NULL;
	break;

    }
}

/*
 *---------------------------------------------------------------------------
 *
 * EmbeddedWidgetCustodyProc --
 *
 *	This procedure is invoked when a tab window has been stolen by another
 *	geometry manager.  The information and memory associated with the tab
 *	window is released.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Arranges for the widget formerly associated with the tab window to
 *	have its layout re-computed and arranged at the next idle point.
 *
 *---------------------------------------------------------------------------
 */
 /* ARGSUSED */
static void
EmbeddedWidgetCustodyProc(ClientData clientData, Tk_Window tkwin)
{
    Tab *tabPtr = clientData;
    Notebook *nbPtr;

    if ((tabPtr == NULL) || (tabPtr->tkwin == NULL)) {
	return;
    }
    nbPtr = tabPtr->nbPtr;
    if (tabPtr->container != NULL) {
	Tcl_EventuallyFree(tabPtr, DestroyTearoff);
    }
    /*
     * Mark the tab as deleted by dereferencing the Tk window pointer. Redraw
     * the window only if the tab is currently visible.
     */
    if (tabPtr->tkwin != NULL) {
	if (Tk_IsMapped(tabPtr->tkwin) && (nbPtr->selectPtr == tabPtr)) {
	    nbPtr->flags |= (TNB_LAYOUT | TNB_SCROLL);
	    EventuallyRedraw(nbPtr);
	}
	Tk_DeleteEventHandler(tabPtr->tkwin, StructureNotifyMask,
	    EmbeddedWidgetEventProc, tabPtr);
	tabPtr->tkwin = NULL;
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * EmbeddedWidgetGeometryProc --
 *
 *	This procedure is invoked by Tk_GeometryRequest for tab windows
 *	managed by the widget.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Arranges for tkwin, and all its managed siblings, to be repacked and
 *	drawn at the next idle point.
 *
 *---------------------------------------------------------------------------
 */
 /* ARGSUSED */
static void
EmbeddedWidgetGeometryProc(ClientData clientData, Tk_Window tkwin)
{
    Tab *tabPtr = clientData;

    if ((tabPtr == NULL) || (tabPtr->tkwin == NULL)) {
	fprintf(stderr, "%s: line %d \"tkwin is null\"", __FILE__, __LINE__);
	return;
    }
    tabPtr->nbPtr->flags |= (TNB_LAYOUT | TNB_SCROLL);
    EventuallyRedraw(tabPtr->nbPtr);
}

/*
 *---------------------------------------------------------------------------
 *
 * DestroyTab --
 *
 *---------------------------------------------------------------------------
 */
static void
DestroyTab(Notebook *nbPtr, Tab *tabPtr)
{
    Blt_HashEntry *hPtr;

    if (tabPtr->flags & TAB_REDRAW) {
	Tcl_CancelIdleCall(DisplayTearoff, tabPtr);
    }
    if (tabPtr->container != NULL) {
	Tk_DestroyWindow(tabPtr->container);
    }
    if (tabPtr->tkwin != NULL) {
	Tk_ManageGeometry(tabPtr->tkwin, (Tk_GeomMgr *)NULL, tabPtr);
	Tk_DeleteEventHandler(tabPtr->tkwin, StructureNotifyMask, 
		EmbeddedWidgetEventProc, tabPtr);
	if (Tk_IsMapped(tabPtr->tkwin)) {
	    Tk_UnmapWindow(tabPtr->tkwin);
	}
    }
    if (tabPtr == nbPtr->activePtr) {
	nbPtr->activePtr = NULL;
    }
    if (tabPtr == nbPtr->selectPtr) {
	nbPtr->selectPtr = NextOrLastTab(tabPtr);
    }
    if (tabPtr == nbPtr->focusPtr) {
	nbPtr->focusPtr = nbPtr->selectPtr;
	Blt_SetFocusItem(nbPtr->bindTable, nbPtr->focusPtr, NULL);
    }
    if (tabPtr == nbPtr->startPtr) {
	nbPtr->startPtr = NULL;
    }
    Blt_FreeOptions(tabConfigSpecs, (char *)tabPtr, nbPtr->display, 0);
    hPtr = Blt_FindHashEntry(&nbPtr->tabTable, tabPtr->name);
    assert(hPtr);
    Blt_DeleteHashEntry(&(nbPtr->tabTable), hPtr);

    if (tabPtr->name != NULL) {
	Blt_Free(tabPtr->name);
    }
    if (tabPtr->textGC != NULL) {
	Tk_FreeGC(nbPtr->display, tabPtr->textGC);
    }
    if (tabPtr->backGC != NULL) {
	Tk_FreeGC(nbPtr->display, tabPtr->backGC);
    }
    if (tabPtr->link != NULL) {
	Blt_ChainDeleteLink(nbPtr->chain, tabPtr->link);
    }
    Blt_DeleteBindings(nbPtr->bindTable, tabPtr);
    Blt_Free(tabPtr);
}

/*
 *---------------------------------------------------------------------------
 *
 * CreateTab --
 *
 *	Creates a new tab structure.  A tab contains information about the
 *	state of the tab and its embedded window.
 *
 * Results:
 *	Returns a pointer to the new tab structure.
 *
 *---------------------------------------------------------------------------
 */
static Tab *
CreateTab(Notebook *nbPtr)
{
    Tab *tabPtr;
    Blt_HashEntry *hPtr;
    int isNew;
    char string[200];

    tabPtr = Blt_CallocAssert(1, sizeof(Tab));
    tabPtr->nbPtr = nbPtr;
    sprintf_s(string, 200, "tab%d", nbPtr->nextId++);
    tabPtr->name = Blt_StrdupAssert(string);
    tabPtr->text = Blt_GetUid(string);
    tabPtr->fill = FILL_NONE;
    tabPtr->anchor = TK_ANCHOR_CENTER;
    tabPtr->container = NULL;
    tabPtr->state = STATE_NORMAL;
    hPtr = Blt_CreateHashEntry(&nbPtr->tabTable, string, &isNew);
    Blt_SetHashValue(hPtr, tabPtr);
    return tabPtr;
}

/*
 *---------------------------------------------------------------------------
 *
 * TileChangedProc
 *
 *	Stub for image change notifications.  Since we immediately draw the
 *	image into a pixmap, we don't really care about image changes.
 *
 *	It would be better if Tk checked for NULL proc pointers.
 *
 * Results:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static void
TileChangedProc(ClientData clientData, Blt_Tile tile)
{
    Notebook *nbPtr = clientData;

    if (nbPtr->tkwin != NULL) {
	EventuallyRedraw(nbPtr);
    }
}

static int
ConfigureTab(Notebook *nbPtr, Tab *tabPtr)
{
    GC newGC;
    XGCValues gcValues;
    unsigned long gcMask;
    int labelWidth, labelHeight;
    Blt_Font font;
    Tk_3DBorder border;

    font = GETATTR(tabPtr, font);
    labelWidth = labelHeight = 0;
    if (tabPtr->text != NULL) {
	TextStyle ts;
	double rotWidth, rotHeight;

	Blt_Ts_InitStyle(ts);
	Blt_Ts_SetFont(ts, font);
	Blt_Ts_SetPadding(ts, 2, 2, 0, 0);
	Blt_Ts_GetExtents(&ts, tabPtr->text, &labelWidth, &labelHeight);
	Blt_GetBoundingBox(labelWidth, labelHeight, nbPtr->defTabStyle.angle,
	    &rotWidth, &rotHeight, (Point2d *)NULL);
	labelWidth = ROUND(rotWidth);
	labelHeight = ROUND(rotHeight);
    }
    tabPtr->textWidth = (short int)labelWidth;
    tabPtr->textHeight = (short int)labelHeight;
    if (tabPtr->tabImagePtr != NULL) {
	int width, height;

	width = ImageWidth(tabPtr->tabImagePtr) + 2 * IMAGE_PAD;
	height = ImageHeight(tabPtr->tabImagePtr) + 2 * IMAGE_PAD;
	if (nbPtr->defTabStyle.textSide & TAB_VERTICAL) {
	    labelWidth += width;
	    labelHeight = MAX(labelHeight, height);
	} else {
	    labelHeight += height;
	    labelWidth = MAX(labelWidth, width);
	}
    }
    labelWidth += PADDING(tabPtr->iPadX);
    labelHeight += PADDING(tabPtr->iPadY);

    tabPtr->labelWidth = ODD(labelWidth);
    tabPtr->labelHeight = ODD(labelHeight);

    newGC = NULL;
    if (tabPtr->text != NULL) {
	XColor *colorPtr;

	gcMask = GCForeground | GCFont;
	colorPtr = GETATTR(tabPtr, textColor);
	gcValues.foreground = colorPtr->pixel;
	gcValues.font = Blt_FontId(font);
	newGC = Tk_GetGC(nbPtr->tkwin, gcMask, &gcValues);
    }
    if (tabPtr->textGC != NULL) {
	Tk_FreeGC(nbPtr->display, tabPtr->textGC);
    }
    tabPtr->textGC = newGC;

    gcMask = GCForeground | GCStipple | GCFillStyle;
    gcValues.fill_style = FillStippled;
    border = GETATTR(tabPtr, border);
    gcValues.foreground = Tk_3DBorderColor(border)->pixel;
    gcValues.stipple = tabPtr->stipple;
    newGC = Tk_GetGC(nbPtr->tkwin, gcMask, &gcValues);
    if (tabPtr->backGC != NULL) {
	Tk_FreeGC(nbPtr->display, tabPtr->backGC);
    }
    tabPtr->backGC = newGC;
    /*
     * GC for tiled background.
     */
    if (tabPtr->tile != NULL) {
	Blt_SetTileChangedProc(tabPtr->tile, TileChangedProc, nbPtr);
    }
    if (tabPtr->flags & TAB_VISIBLE) {
	EventuallyRedraw(nbPtr);
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * TearoffEventProc --
 *
 * 	This procedure is invoked by the Tk dispatcher for various events on
 * 	the tearoff widget.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	When the tearoff gets deleted, internal structures get cleaned up.
 *	When it gets resized or exposed, it's redisplayed.
 *
 *---------------------------------------------------------------------------
 */
static void
TearoffEventProc(ClientData clientData, XEvent *eventPtr)
{
    Tab *tabPtr = clientData;

    if ((tabPtr == NULL) || (tabPtr->tkwin == NULL) ||
	(tabPtr->container == NULL)) {
	return;
    }
    switch (eventPtr->type) {
    case Expose:
	if (eventPtr->xexpose.count == 0) {
	    EventuallyRedrawTearoff(tabPtr);
	}
	break;

    case ConfigureNotify:
	EventuallyRedrawTearoff(tabPtr);
	break;

    case DestroyNotify:
	if (tabPtr->flags & TAB_REDRAW) {
	    tabPtr->flags &= ~TAB_REDRAW;
	    Tcl_CancelIdleCall(DisplayTearoff, clientData);
	}
	Tk_DestroyWindow(tabPtr->container);
	tabPtr->container = NULL;
	break;

    }
}

/*
 *---------------------------------------------------------------------------
 *
 * GetReqWidth --
 *
 *	Returns the width requested by the embedded tab window and any
 *	requested padding around it. This represents the requested width of
 *	the page.
 *
 * Results:
 *	Returns the requested width of the page.
 *
 *---------------------------------------------------------------------------
 */
static int
GetReqWidth(Tab *tabPtr)
{
    int width;

    if (tabPtr->reqWidth > 0) {
	width = tabPtr->reqWidth;
    } else {
	width = Tk_ReqWidth(tabPtr->tkwin);
    }
    width += PADDING(tabPtr->padX) +
	2 * Tk_Changes(tabPtr->tkwin)->border_width;
    if (width < 1) {
	width = 1;
    }
    return width;
}

/*
 *---------------------------------------------------------------------------
 *
 * GetReqHeight --
 *
 *	Returns the height requested by the window and padding around the
 *	window. This represents the requested height of the page.
 *
 * Results:
 *	Returns the requested height of the page.
 *
 *---------------------------------------------------------------------------
 */
static int
GetReqHeight(Tab *tabPtr)
{
    int height;

    if (tabPtr->reqHeight > 0) {
	height = tabPtr->reqHeight;
    } else {
	height = Tk_ReqHeight(tabPtr->tkwin);
    }
    height += PADDING(tabPtr->padY) +
	2 * Tk_Changes(tabPtr->tkwin)->border_width;
    if (height < 1) {
	height = 1;
    }
    return height;
}

/*
 *---------------------------------------------------------------------------
 *
 * TranslateAnchor --
 *
 * 	Translate the coordinates of a given bounding box based upon the
 * 	anchor specified.  The anchor indicates where the given xy position
 * 	is in relation to the bounding box.
 *
 *  		nw --- n --- ne
 *  		|            |     x,y ---+
 *  		w   center   e      |     |
 *  		|            |      +-----+
 *  		sw --- s --- se
 *
 * Results:
 *	The translated coordinates of the bounding box are returned.
 *
 *---------------------------------------------------------------------------
 */
static void
TranslateAnchor(int dx, int dy, Tk_Anchor anchor, int *xPtr, int *yPtr)
{
    int x, y;

    x = y = 0;
    switch (anchor) {
    case TK_ANCHOR_NW:		/* Upper left corner */
	break;
    case TK_ANCHOR_W:		/* Left center */
	y = (dy / 2);
	break;
    case TK_ANCHOR_SW:		/* Lower left corner */
	y = dy;
	break;
    case TK_ANCHOR_N:		/* Top center */
	x = (dx / 2);
	break;
    case TK_ANCHOR_CENTER:	/* Centered */
	x = (dx / 2);
	y = (dy / 2);
	break;
    case TK_ANCHOR_S:		/* Bottom center */
	x = (dx / 2);
	y = dy;
	break;
    case TK_ANCHOR_NE:		/* Upper right corner */
	x = dx;
	break;
    case TK_ANCHOR_E:		/* Right center */
	x = dx;
	y = (dy / 2);
	break;
    case TK_ANCHOR_SE:		/* Lower right corner */
	x = dx;
	y = dy;
	break;
    }
    *xPtr = (*xPtr) + x;
    *yPtr = (*yPtr) + y;
}


static void
GetWindowRectangle(Tab *tabPtr, Tk_Window parent, int tearoff, 
		   XRectangle *rectPtr)
{
    int pad;
    Notebook *nbPtr;
    int cavityWidth, cavityHeight;
    int width, height;
    int dx, dy;
    int x, y;

    nbPtr = tabPtr->nbPtr;
    pad = nbPtr->inset + nbPtr->inset2;

    x = y = 0; 			/* Suppress compiler warning. */
    if (!tearoff) {
	switch (nbPtr->side) {
	case SIDE_RIGHT:
	case SIDE_BOTTOM:
	    x = nbPtr->inset + nbPtr->inset2;
	    y = nbPtr->inset + nbPtr->inset2;
	    break;

	case SIDE_LEFT:
	    x = nbPtr->pageTop;
	    y = nbPtr->inset + nbPtr->inset2;
	    break;

	case SIDE_TOP:
	    x = nbPtr->inset + nbPtr->inset2;
	    y = nbPtr->pageTop;
	    break;
	}

	if (nbPtr->side & TAB_VERTICAL) {
	    cavityWidth = Tk_Width(nbPtr->tkwin) - (nbPtr->pageTop + pad);
	    cavityHeight = Tk_Height(nbPtr->tkwin) - (2 * pad);
	} else {
	    cavityWidth = Tk_Width(nbPtr->tkwin) - (2 * pad);
	    cavityHeight = Tk_Height(nbPtr->tkwin) - (nbPtr->pageTop + pad);
	}

    } else {
	x = nbPtr->inset + nbPtr->inset2;
#define TEAR_OFF_TAB_SIZE	5
	y = nbPtr->inset + nbPtr->inset2 + nbPtr->yPad + nbPtr->outerPad +
	    TEAR_OFF_TAB_SIZE;
	cavityWidth = Tk_Width(parent) - (2 * pad);
	cavityHeight = Tk_Height(parent) - (y + pad);
    }
    cavityWidth -= PADDING(tabPtr->padX);
    cavityHeight -= PADDING(tabPtr->padY);
    if (cavityWidth < 1) {
	cavityWidth = 1;
    }
    if (cavityHeight < 1) {
	cavityHeight = 1;
    }
    width = GetReqWidth(tabPtr);
    height = GetReqHeight(tabPtr);

    /*
     * Resize the embedded window is of the following is true:
     *
     *	1) It's been torn off.
     *  2) The -fill option (horizontal or vertical) is set.
     *  3) the window is bigger than the cavity.
     */
    if ((tearoff) || (cavityWidth < width) || (tabPtr->fill & FILL_X)) {
	width = cavityWidth;
    }
    if ((tearoff) || (cavityHeight < height) || (tabPtr->fill & FILL_Y)) {
	height = cavityHeight;
    }
    dx = (cavityWidth - width);
    dy = (cavityHeight - height);
    if ((dx > 0) || (dy > 0)) {
	TranslateAnchor(dx, dy, tabPtr->anchor, &x, &y);
    }
    /* Remember that X11 windows must be at least 1 pixel. */
    if (width < 1) {
	width = 1;
    }
    if (height < 1) {
	height = 1;
    }
    rectPtr->x = (short)(x + tabPtr->padLeft);
    rectPtr->y = (short)(y + tabPtr->padTop);
    rectPtr->width = (short)width;
    rectPtr->height = (short)height;
}

static void
ArrangeWindow(Tk_Window tkwin, XRectangle *rectPtr, int force)
{
    if ((force) ||
	(rectPtr->x != Tk_X(tkwin)) || 
	(rectPtr->y != Tk_Y(tkwin)) ||
	(rectPtr->width != Tk_Width(tkwin)) ||
	(rectPtr->height != Tk_Height(tkwin))) {
	Tk_MoveResizeWindow(tkwin, rectPtr->x, rectPtr->y, 
			    rectPtr->width, rectPtr->height);
    }
    if (!Tk_IsMapped(tkwin)) {
	Tk_MapWindow(tkwin);
    }
}


/*ARGSUSED*/
static void
GetTags(Blt_BindTable table, ClientData object, ClientData context, 
	Blt_List list)
{
    Tab *tabPtr = (Tab *)object;
    Notebook *nbPtr;

    nbPtr = table->clientData;
    if (context == TAB_PERFORATION) {
	Blt_ListAppend(list, MakeTag(nbPtr, "Perforation"), 0);
    } else if (context == TAB_LABEL) {
	ClientData tag;

	tag = MakeTag(nbPtr, tabPtr->name);
	Blt_ListAppend(list, tag, 0);
	if (tabPtr->tags != NULL) {
	    int argc;
	    const char **argv;
	    
	    /* 
	     * This is a space/time trade-off in favor of space.  The
	     * tags are stored as character strings in a hash table.
	     * That way, tabs can share the strings. It's likely that
	     * they will.  The down side is that the same string is
	     * split over and over again.
	     */
	    if (Tcl_SplitList(NULL, tabPtr->tags, &argc, &argv) == TCL_OK) {
		const char **p;

		for (p = argv; *p != NULL; p++) {
		    Blt_ListAppend(list, MakeTag(nbPtr, *p), 0);
		}
		Blt_Free(argv);
	    }
	}
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * NotebookEventProc --
 *
 * 	This procedure is invoked by the Tk dispatcher for various events on
 * 	notebook widgets.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	When the window gets deleted, internal structures get cleaned up.
 *	When it gets exposed, it is redisplayed.
 *
 *---------------------------------------------------------------------------
 */
static void
NotebookEventProc(ClientData clientData, XEvent *eventPtr)
{
    Notebook *nbPtr = clientData;

    switch (eventPtr->type) {
    case Expose:
	if (eventPtr->xexpose.count == 0) {
	    EventuallyRedraw(nbPtr);
	}
	break;

    case ConfigureNotify:
	nbPtr->flags |= (TNB_LAYOUT | TNB_SCROLL);
	EventuallyRedraw(nbPtr);
	break;

    case FocusIn:
    case FocusOut:
	if (eventPtr->xfocus.detail != NotifyInferior) {
	    if (eventPtr->type == FocusIn) {
		nbPtr->flags |= TNB_FOCUS;
	    } else {
		nbPtr->flags &= ~TNB_FOCUS;
	    }
	    EventuallyRedraw(nbPtr);
	}
	break;

    case DestroyNotify:
	if (nbPtr->tkwin != NULL) {
	    nbPtr->tkwin = NULL;
	    Tcl_DeleteCommandFromToken(nbPtr->interp, nbPtr->cmdToken);
	}
	if (nbPtr->flags & TNB_REDRAW) {
	    Tcl_CancelIdleCall(DisplayNotebook, nbPtr);
	}
	Tcl_EventuallyFree(nbPtr, DestroyNotebook);
	break;

    }
}

/*
 *---------------------------------------------------------------------------
 *
 * DestroyNotebook --
 *
 * 	This procedure is invoked by Tcl_EventuallyFree or Tcl_Release to
 * 	clean up the internal structure of the widget at a safe time (when
 * 	no-one is using it anymore).
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Everything associated with the widget is freed up.
 *
 *---------------------------------------------------------------------------
 */
static void
DestroyNotebook(DestroyData dataPtr)
{
    Notebook *nbPtr = (Notebook *)dataPtr;
    Tab *tabPtr;
    Blt_ChainLink link;

    if (nbPtr->highlightGC != NULL) {
	Tk_FreeGC(nbPtr->display, nbPtr->highlightGC);
    }
    if (nbPtr->tile != NULL) {
	Blt_FreeTile(nbPtr->tile);
    }
    if (nbPtr->defTabStyle.activeGC != NULL) {
	Blt_FreePrivateGC(nbPtr->display, nbPtr->defTabStyle.activeGC);
    }
    for (link = Blt_ChainFirstLink(nbPtr->chain); link != NULL;
	link = Blt_ChainNextLink(link)) {
	tabPtr = Blt_ChainGetValue(link);
	tabPtr->link = NULL;
	DestroyTab(nbPtr, tabPtr);
    }
    Blt_ChainDestroy(nbPtr->chain);
    Blt_DestroyBindingTable(nbPtr->bindTable);
    Blt_DeleteHashTable(&nbPtr->tabTable);
    Blt_DeleteHashTable(&nbPtr->tagTable);
    Blt_FreeOptions(configSpecs, (char *)nbPtr, nbPtr->display, 0);
    Blt_Free(nbPtr);
}

/*
 *---------------------------------------------------------------------------
 *
 * CreateNotebook --
 *
 *---------------------------------------------------------------------------
 */
static Notebook *
CreateNotebook(Tcl_Interp *interp, Tk_Window tkwin)
{
    Notebook *nbPtr;

    nbPtr = Blt_CallocAssert(1, sizeof(Notebook));
    Tk_SetClass(tkwin, "Tabnotebook");
    nbPtr->tkwin = tkwin;
    nbPtr->display = Tk_Display(tkwin);
    nbPtr->interp = interp;

    nbPtr->flags |= (TNB_LAYOUT | TNB_SCROLL);
    nbPtr->side = SIDE_TOP;
    nbPtr->borderWidth = nbPtr->highlightWidth = 2;
    nbPtr->ySelectPad = SELECT_PADY;
    nbPtr->xSelectPad = SELECT_PADX;
    nbPtr->relief = TK_RELIEF_SUNKEN;
    nbPtr->defTabStyle.relief = TK_RELIEF_RAISED;
    nbPtr->defTabStyle.borderWidth = 1;
    nbPtr->defTabStyle.constWidth = TRUE;
    nbPtr->defTabStyle.textSide = SIDE_LEFT;
    nbPtr->scrollUnits = 2;
    nbPtr->corner = CORNER_OFFSET;
    nbPtr->gap = GAP;
    nbPtr->outerPad = OUTER_PAD;
    nbPtr->slant = SLANT_NONE;
    nbPtr->overlap = 0;
    nbPtr->tearoff = TRUE;
    nbPtr->bindTable = Blt_CreateBindingTable(interp, tkwin, nbPtr, PickTab, 
	GetTags);
    nbPtr->chain = Blt_ChainCreate();
    Blt_InitHashTable(&nbPtr->tabTable, BLT_STRING_KEYS);
    Blt_InitHashTable(&nbPtr->tabImageTable, BLT_STRING_KEYS);
    Blt_InitHashTable(&nbPtr->tagTable, BLT_STRING_KEYS);
    Blt_SetWindowInstanceData(tkwin, nbPtr);
    return nbPtr;
}

/*
 *---------------------------------------------------------------------------
 *
 * ConfigureNotebook --
 *
 * 	This procedure is called to process an objv/objc list, plus the Tk
 * 	option database, in order to configure (or reconfigure) the widget.
 *
 * Results:

 *	The return value is a standard Tcl result.  If TCL_ERROR is returned,
 *	then interp->result contains an error message.
 *
 * Side Effects:
 *	Configuration information, such as text string, colors, font, etc. get
 *	set for nbPtr; old resources get freed, if there were any.  The widget
 *	is redisplayed.
 *
 *---------------------------------------------------------------------------
 */
static int
ConfigureNotebook(
    Tcl_Interp *interp,		/* Interpreter to report errors. */
    Notebook *nbPtr,		/* Information about widget; may or may not
			         * already have values for some fields. */
    int objc,
    Tcl_Obj *const *objv,
    int flags)
{
    XGCValues gcValues;
    unsigned long gcMask;
    GC newGC;

    imageOption.clientData = nbPtr;
    if (Blt_ConfigureWidgetFromObj(interp, nbPtr->tkwin, configSpecs, 
	   objc, objv, (char *)nbPtr, flags) != TCL_OK) {
	return TCL_ERROR;
    }
    if (Blt_ConfigModified(configSpecs, "-width", "-height", "-side", "-gap",
	    "-slant", (char *)NULL)) {
	nbPtr->flags |= (TNB_LAYOUT | TNB_SCROLL);
    }
    if ((nbPtr->reqHeight > 0) && (nbPtr->reqWidth > 0)) {
	Tk_GeometryRequest(nbPtr->tkwin, nbPtr->reqWidth, 
		nbPtr->reqHeight);
    }
    /*
     * GC for focus highlight.
     */
    gcMask = GCForeground;
    gcValues.foreground = nbPtr->highlightColor->pixel;
    newGC = Tk_GetGC(nbPtr->tkwin, gcMask, &gcValues);
    if (nbPtr->highlightGC != NULL) {
	Tk_FreeGC(nbPtr->display, nbPtr->highlightGC);
    }
    nbPtr->highlightGC = newGC;

    /*
     * GC for tiled background.
     */
    if (nbPtr->tile != NULL) {
	Blt_SetTileChangedProc(nbPtr->tile, TileChangedProc, nbPtr);
    }
    /*
     * GC for active line.
     */
    gcMask = GCForeground | GCLineWidth | GCLineStyle | GCCapStyle;
    gcValues.foreground = nbPtr->defTabStyle.activeFgColor->pixel;
    gcValues.line_width = 0;
    gcValues.cap_style = CapProjecting;
    gcValues.line_style = (LineIsDashed(nbPtr->defTabStyle.dashes))
	? LineOnOffDash : LineSolid;

    newGC = Blt_GetPrivateGC(nbPtr->tkwin, gcMask, &gcValues);
    if (LineIsDashed(nbPtr->defTabStyle.dashes)) {
	nbPtr->defTabStyle.dashes.offset = 2;
	Blt_SetDashes(nbPtr->display, newGC, &(nbPtr->defTabStyle.dashes));
    }
    if (nbPtr->defTabStyle.activeGC != NULL) {
	Blt_FreePrivateGC(nbPtr->display, nbPtr->defTabStyle.activeGC);
    }
    nbPtr->defTabStyle.activeGC = newGC;

    nbPtr->defTabStyle.angle = FMOD(nbPtr->defTabStyle.angle, 360.0);
    if (nbPtr->defTabStyle.angle < 0.0) {
	nbPtr->defTabStyle.angle += 360.0;
    }
    nbPtr->inset = nbPtr->highlightWidth + nbPtr->borderWidth + nbPtr->outerPad;
    if (Blt_ConfigModified(configSpecs, "-font", "-*foreground", "-rotate",
	    "-*background", "-side", (char *)NULL)) {
	Blt_ChainLink link;
	Tab *tabPtr;

	for (link = Blt_ChainFirstLink(nbPtr->chain); 
	     link != NULL; link = Blt_ChainNextLink(link)) {
	    tabPtr = Blt_ChainGetValue(link);
	    ConfigureTab(nbPtr, tabPtr);
	}
	nbPtr->flags |= (TNB_LAYOUT | TNB_SCROLL);
    }
    nbPtr->inset2 = nbPtr->defTabStyle.borderWidth + nbPtr->corner;
    EventuallyRedraw(nbPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * Notebook operations
 *
 *---------------------------------------------------------------------------
 */
/*
 *---------------------------------------------------------------------------
 *
 * ActivateOp --
 *
 *	Selects the tab to appear active.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ActivateOp(Notebook *nbPtr, Tcl_Interp *interp, int objc, Tcl_Obj *const *objv)
{
    Tab *tabPtr;
    char *string;

    string = Tcl_GetString(objv[2]);
    if (string[0] == '\0') {
	tabPtr = NULL;
    } else if (GetTabFromObj(nbPtr, objv[2], &tabPtr, INVALID_OK) != TCL_OK) {
	return TCL_ERROR;
    }
    if ((tabPtr != NULL) && (tabPtr->state == STATE_DISABLED)) {
	tabPtr = NULL;
    }
    if (tabPtr != nbPtr->activePtr) {
	nbPtr->activePtr = tabPtr;
	EventuallyRedraw(nbPtr);
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * BindOp --
 *
 *	  .t bind index sequence command
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
BindOp(
    Notebook *nbPtr,
    Tcl_Interp *interp,
    int objc,			/* Not used. */
    Tcl_Obj *const *objv)
{
    ClientData tag;

    if (objc == 2) {
	Blt_HashEntry *hPtr;
	Blt_HashSearch cursor;
	Tcl_Obj *listObjPtr, *objPtr;

	listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
	for (hPtr = Blt_FirstHashEntry(&nbPtr->tagTable, &cursor);
	    hPtr != NULL; hPtr = Blt_NextHashEntry(&cursor)) {
	    objPtr = Blt_GetHashValue(hPtr);
	    Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
	}
	Tcl_SetObjResult(interp, listObjPtr);
	return TCL_OK;
    }
    tag = MakeTag(nbPtr, Tcl_GetString(objv[2]));
    return Blt_ConfigureBindingsFromObj(interp, nbPtr->bindTable, tag, 
	objc - 3, objv + 3);
}

/*
 *---------------------------------------------------------------------------
 *
 * CgetOp --
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
CgetOp(
    Notebook *nbPtr,
    Tcl_Interp *interp,
    int objc,			/* Not used. */
    Tcl_Obj *const *objv)
{
    imageOption.clientData = nbPtr;
    return Blt_ConfigureValueFromObj(interp, nbPtr->tkwin, configSpecs,
	(char *)nbPtr, objv[2], 0);
}

/*
 *---------------------------------------------------------------------------
 *
 * ConfigureOp --
 *
 * 	This procedure is called to process an objv/objc list, plus the Tk
 * 	option database, in order to configure (or reconfigure) the widget.
 *
 * Results:
 *	A standard Tcl result.  If TCL_ERROR is returned, then interp->result
 *	contains an error message.
 *
 * Side Effects:
 *	Configuration information, such as text string, colors, font, etc. get
 *	set for nbPtr; old resources get freed, if there were any.  The widget
 *	is redisplayed.
 *
 *---------------------------------------------------------------------------
 */
static int
ConfigureOp(
    Notebook *nbPtr,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{

    imageOption.clientData = nbPtr;
    if (objc == 2) {
	return Blt_ConfigureInfoFromObj(interp, nbPtr->tkwin, configSpecs,
	    (char *)nbPtr, (Tcl_Obj *)NULL, 0);
    } else if (objc == 3) {
	return Blt_ConfigureInfoFromObj(interp, nbPtr->tkwin, configSpecs,
	    (char *)nbPtr, objv[2], 0);
    }
    if (ConfigureNotebook(interp, nbPtr, objc - 2, objv + 2,
	    BLT_CONFIG_OBJV_ONLY) != TCL_OK) {
	return TCL_ERROR;
    }
    EventuallyRedraw(nbPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * DeleteOp --
 *
 *	Deletes tab from the set. Deletes either a range of tabs or a single
 *	node.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
DeleteOp(
    Notebook *nbPtr,
    Tcl_Interp *interp,
    int objc,			/* Not used. */
    Tcl_Obj *const *objv)
{
    Tab *firstPtr, *lastPtr;

    lastPtr = NULL;
    if (GetTabFromObj(nbPtr, objv[2], &firstPtr, INVALID_FAIL) != TCL_OK) {
	return TCL_ERROR;
    }
    if ((objc == 4) && 
	(GetTabFromObj(nbPtr, objv[3], &lastPtr, INVALID_FAIL) != TCL_OK)) {
	return TCL_ERROR;
    }
    if (lastPtr == NULL) {
	DestroyTab(nbPtr, firstPtr);
    } else {
	Tab *tabPtr;
	Blt_ChainLink link, next;

	tabPtr = NULL;		/* Suppress compiler warning. */

	/* Make sure that the first tab is before the last. */
	for (link = firstPtr->link; link != NULL;
	    link = Blt_ChainNextLink(link)) {
	    tabPtr = Blt_ChainGetValue(link);
	    if (tabPtr == lastPtr) {
		break;
	    }
	}
	if (tabPtr != lastPtr) {
	    return TCL_OK;
	}
	link = firstPtr->link;
	while (link != NULL) {
	    next = Blt_ChainNextLink(link);
	    tabPtr = Blt_ChainGetValue(link);
	    DestroyTab(nbPtr, tabPtr);
	    link = next;
	    if (tabPtr == lastPtr) {
		break;
	    }
	}
    }
    nbPtr->flags |= (TNB_LAYOUT | TNB_SCROLL);
    EventuallyRedraw(nbPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * FocusOp --
 *
 *	Selects the tab to get focus.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
FocusOp(
    Notebook *nbPtr,
    Tcl_Interp *interp,
    int objc,			/* Not used. */
    Tcl_Obj *const *objv)
{
    Tab *tabPtr;

    if (GetTabFromObj(nbPtr, objv[2], &tabPtr, INVALID_FAIL) != TCL_OK) {
	return TCL_ERROR;
    }
    if (tabPtr != NULL) {
	nbPtr->focusPtr = tabPtr;
	Blt_SetFocusItem(nbPtr->bindTable, nbPtr->focusPtr, NULL);
	EventuallyRedraw(nbPtr);
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * IndexOp --
 *
 *	Converts a string representing a tab index.
 *
 * Results:
 *	A standard Tcl result.  Interp->result will contain the identifier of
 *	each index found. If an index could not be found, then the serial
 *	identifier will be the empty string.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
IndexOp(
    Notebook *nbPtr,
    Tcl_Interp *interp,
    int objc,			
    Tcl_Obj *const *objv)
{
    Tab *tabPtr;

    if (GetTabFromObj(nbPtr, objv[2], &tabPtr, INVALID_OK) != TCL_OK) {
	return TCL_ERROR;
    }
    if (tabPtr != NULL) {
	Tcl_SetIntObj(Tcl_GetObjResult(interp), TabIndex(nbPtr, tabPtr));
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * IdOp --
 *
 *	Converts a tab index into the tab identifier.
 *
 * Results:
 *	A standard Tcl result.  Interp->result will contain the identifier of
 *	each index found. If an index could not be found, then the serial
 *	identifier will be the empty string.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
IdOp(
    Notebook *nbPtr,
    Tcl_Interp *interp,
    int objc,			/* Not used. */
    Tcl_Obj *const *objv)
{
    Tab *tabPtr;

    if (GetTabFromObj(nbPtr, objv[2], &tabPtr, INVALID_OK) != TCL_OK) {
	return TCL_ERROR;
    }
    if (tabPtr == NULL) {
	Tcl_SetStringObj(Tcl_GetObjResult(interp), "", -1);
    } else {
	Tcl_SetStringObj(Tcl_GetObjResult(interp), tabPtr->name, -1);
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * InsertOp --
 *
 *	Add new entries into a tab set.
 *
 *	.t insert end label option-value label option-value...
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
InsertOp(
    Notebook *nbPtr,
    Tcl_Interp *interp,
    int objc,			/* Not used. */
    Tcl_Obj *const *objv)
{
    Tab *tabPtr;
    Blt_ChainLink link, before;
    char c;
    char *string;

    string = Tcl_GetString(objv[2]);
    c = string[0];
    if ((c == 'e') && (strcmp(string, "end") == 0)) {
	before = NULL;
    } else if (isdigit(UCHAR(c))) {
	int position;

	if (Tcl_GetIntFromObj(interp, objv[2], &position) != TCL_OK) {
	    return TCL_ERROR;
	}
	if (position < 0) {
	    before = Blt_ChainFirstLink(nbPtr->chain);
	} else if (position > Blt_ChainGetLength(nbPtr->chain)) {
	    before = NULL;
	} else {
	    before = Blt_ChainGetNthLink(nbPtr->chain, position);
	}
    } else {
	Tab *beforePtr;

	if (GetTabFromObj(nbPtr, objv[2], &beforePtr, INVALID_FAIL) != TCL_OK) {
	    return TCL_ERROR;
	}
	before = beforePtr->link;
    }
    nbPtr->flags |= (TNB_LAYOUT | TNB_SCROLL);
    EventuallyRedraw(nbPtr);
    tabPtr = CreateTab(nbPtr);
    if (tabPtr == NULL) {
	return TCL_ERROR;
    }
    imageOption.clientData = nbPtr;
    if (Blt_ConfigureComponentFromObj(interp, nbPtr->tkwin, tabPtr->name, "Tab",
	tabConfigSpecs, objc - 3, objv + 3, (char *)tabPtr, 0) != TCL_OK) {
	DestroyTab(nbPtr, tabPtr);
	return TCL_ERROR;
    }
    if (ConfigureTab(nbPtr, tabPtr) != TCL_OK) {
	DestroyTab(nbPtr, tabPtr);
	return TCL_ERROR;
    }
    link = Blt_ChainNewLink();
    if (before == NULL) {
	Blt_ChainAppendLink(nbPtr->chain, link);
    } else {
	Blt_ChainLinkBefore(nbPtr->chain, link, before);
    }
    tabPtr->link = link;
    Blt_ChainSetValue(link, tabPtr);
    Tcl_SetStringObj(Tcl_GetObjResult(interp), tabPtr->name, -1);
    return TCL_OK;

}

/*
 * Preprocess the command string for percent substitution.
 */
static void
PercentSubst(
    Notebook *nbPtr,
    Tab *tabPtr,
    char *command,
    Tcl_DString *resultPtr)
{
    char *last, *p;
    /*
     * Get the full path name of node, in case we need to substitute for it.
     */
    Tcl_DStringInit(resultPtr);
    for (last = p = command; *p != '\0'; p++) {
	if (*p == '%') {
	    const char *string;
	    char buf[3];

	    if (p > last) {
		*p = '\0';
		Tcl_DStringAppend(resultPtr, last, -1);
		*p = '%';
	    }
	    switch (*(p + 1)) {
	    case '%':		/* Percent sign */
		string = "%";
		break;
	    case 'W':		/* Widget name */
		string = Tk_PathName(nbPtr->tkwin);
		break;
	    case 'i':		/* Tab Index */
		string = Blt_Itoa(TabIndex(nbPtr, tabPtr));
		break;
	    case 'n':		/* Tab name */
		string = tabPtr->name;
		break;
	    default:
		if (*(p + 1) == '\0') {
		    p--;
		}
		buf[0] = *p, buf[1] = *(p + 1), buf[2] = '\0';
		string = buf;
		break;
	    }
	    Tcl_DStringAppend(resultPtr, string, -1);
	    p++;
	    last = p + 1;
	}
    }
    if (p > last) {
	*p = '\0';
	Tcl_DStringAppend(resultPtr, last, -1);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * InvokeOp --
 *
 * 	This procedure is called to invoke a selection command.
 *
 *	  .h invoke index
 *
 * Results:
 *	A standard Tcl result.  If TCL_ERROR is returned, then interp->result
 *	contains an error message.
 *
 * Side Effects:
 *	Configuration information, such as text string, colors, font, etc. get
 *	set; old resources get freed, if there were any.  The widget is
 *	redisplayed if needed.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
InvokeOp(
    Notebook *nbPtr,
    Tcl_Interp *interp,		/* Not used. */
    int objc,
    Tcl_Obj *const *objv)
{
    Tab *tabPtr;
    char *command;

    if (GetTabFromObj(nbPtr, objv[2], &tabPtr, INVALID_OK) != TCL_OK) {
	return TCL_ERROR;
    }
    if ((tabPtr == NULL) || (tabPtr->state == STATE_DISABLED)) {
	return TCL_OK;
    }
    Tcl_Preserve(tabPtr);
    command = GETATTR(tabPtr, command);
    if (command != NULL) {
	Tcl_DString dString;
	int result;

	PercentSubst(nbPtr, tabPtr, command, &dString);
	result = Tcl_GlobalEval(nbPtr->interp, 
		Tcl_DStringValue(&dString));
	Tcl_DStringFree(&dString);
	if (result != TCL_OK) {
	    return TCL_ERROR;
	}
    }
    Tcl_Release(tabPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * MoveOp --
 *
 *	Moves a tab to a new location.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
MoveOp(
    Notebook *nbPtr,
    Tcl_Interp *interp,
    int objc,			/* Not used. */
    Tcl_Obj *const *objv)
{
    Tab *tabPtr, *link;
    int before;
    char *string;

    if (GetTabFromObj(nbPtr, objv[2], &tabPtr, INVALID_OK) != TCL_OK) {
	return TCL_ERROR;
    }
    if ((tabPtr == NULL) || (tabPtr->state == STATE_DISABLED)) {
	return TCL_OK;
    }
    string = Tcl_GetString(objv[3]);
    if ((string[0] == 'b') && (strcmp(string, "before") == 0)) {
	before = 1;
    } else if ((string[0] == 'a') && (strcmp(string, "after") == 0)) {
	before = 0;
    } else {
	Tcl_AppendResult(interp, "bad key word \"", string,
	    "\": should be \"after\" or \"before\"", (char *)NULL);
	return TCL_ERROR;
    }
    if (GetTabFromObj(nbPtr, objv[4], &link, INVALID_FAIL) != TCL_OK) {
	return TCL_ERROR;
    }
    if (tabPtr == link) {
	return TCL_OK;
    }
    Blt_ChainUnlinkLink(nbPtr->chain, tabPtr->link);
    if (before) {
	Blt_ChainLinkBefore(nbPtr->chain, tabPtr->link, link->link);
    } else {
	Blt_ChainLinkAfter(nbPtr->chain, tabPtr->link, link->link);
    }
    nbPtr->flags |= (TNB_LAYOUT | TNB_SCROLL);
    EventuallyRedraw(nbPtr);
    return TCL_OK;
}

/*ARGSUSED*/
static int
NearestOp(
    Notebook *nbPtr,
    Tcl_Interp *interp,
    int objc,			/* Not used. */
    Tcl_Obj *const *objv)
{
    int x, y;			/* Screen coordinates of the test point. */
    Tab *tabPtr;

    if ((Tk_GetPixelsFromObj(interp, nbPtr->tkwin, objv[2], &x) != TCL_OK) ||
	(Tk_GetPixelsFromObj(interp, nbPtr->tkwin, objv[3], &y) != TCL_OK)) {
	return TCL_ERROR;
    }
    if (nbPtr->nVisible > 0) {
	tabPtr = (Tab *)PickTab(nbPtr, x, y, NULL);
	if (tabPtr != NULL) {
	    Tcl_SetStringObj(Tcl_GetObjResult(interp), tabPtr->name, -1);
	}
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * SelectOp --
 *
 * 	This procedure is called to select a tab.
 *
 *	  .h select index
 *
 * Results:
 *	A standard Tcl result.  If TCL_ERROR is returned, then
 *	interp->result contains an error message.
 *
 * Side Effects:
 *	Configuration information, such as text string, colors, font,
 * 	etc. get set;  old resources get freed, if there were any.
 * 	The widget is redisplayed if needed.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
SelectOp(
    Notebook *nbPtr,
    Tcl_Interp *interp,		/* Not used. */
    int objc,
    Tcl_Obj *const *objv)
{
    Tab *tabPtr;

    if (GetTabFromObj(nbPtr, objv[2], &tabPtr, INVALID_OK) != TCL_OK) {
	return TCL_ERROR;
    }
    if ((tabPtr == NULL) || (tabPtr->state == STATE_DISABLED)) {
	return TCL_OK;
    }
    if ((nbPtr->selectPtr != NULL) && (nbPtr->selectPtr != tabPtr) &&
	(nbPtr->selectPtr->tkwin != NULL)) {
	if (nbPtr->selectPtr->container == NULL) {
	    if (Tk_IsMapped(nbPtr->selectPtr->tkwin)) {
		Tk_UnmapWindow(nbPtr->selectPtr->tkwin);
	    }
	} else {
	    /* Redraw now unselected container. */
	    EventuallyRedrawTearoff(nbPtr->selectPtr);
	}
    }
    nbPtr->selectPtr = tabPtr;
    if ((nbPtr->nTiers > 1) && (tabPtr->tier != nbPtr->startPtr->tier)) {
	RenumberTiers(nbPtr, tabPtr);
	Blt_PickCurrentItem(nbPtr->bindTable);
    }
    nbPtr->flags |= (TNB_SCROLL);
    if (tabPtr->container != NULL) {
	EventuallyRedrawTearoff(tabPtr);
    }
    EventuallyRedraw(nbPtr);
    return TCL_OK;
}

static int
ViewOp(
    Notebook *nbPtr,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    int width;

    width = VPORTWIDTH(nbPtr);
    if (objc == 2) {
	double fract;

	/*
	 * Note: we are bounding the fractions between 0.0 and 1.0 to support
	 * the "canvas"-style of scrolling.
	 */

	fract = (double)nbPtr->scrollOffset / nbPtr->worldWidth;
	Tcl_AppendElement(interp, Blt_Dtoa(interp, FCLAMP(fract)));
	fract = (double)(nbPtr->scrollOffset + width) / nbPtr->worldWidth;
	Tcl_AppendElement(interp, Blt_Dtoa(interp, FCLAMP(fract)));
	return TCL_OK;
    }
    if (Blt_GetScrollInfoFromObj(interp, objc - 2, objv + 2, 
		&nbPtr->scrollOffset, nbPtr->worldWidth, width, 
		nbPtr->scrollUnits, BLT_SCROLL_MODE_CANVAS) != TCL_OK) {
	return TCL_ERROR;
    }
    nbPtr->flags |= TNB_SCROLL;
    EventuallyRedraw(nbPtr);
    return TCL_OK;
}


static void
AdoptWindow(ClientData clientData)
{
    Tab *tabPtr = clientData;
    int x, y;
    Notebook *nbPtr = tabPtr->nbPtr;

    x = nbPtr->inset + nbPtr->inset2 + tabPtr->padLeft;
#define TEAR_OFF_TAB_SIZE	5
    y = nbPtr->inset + nbPtr->inset2 + nbPtr->yPad +
	nbPtr->outerPad + TEAR_OFF_TAB_SIZE + tabPtr->padTop;
    Blt_RelinkWindow(tabPtr->tkwin, tabPtr->container, x, y);
    Tk_MapWindow(tabPtr->tkwin);
}

static void
DestroyTearoff(DestroyData dataPtr)
{
    Tab *tabPtr = (Tab *)dataPtr;

    if (tabPtr->container != NULL) {
	Notebook *nbPtr;
	Tk_Window tkwin;
	nbPtr = tabPtr->nbPtr;

	tkwin = tabPtr->container;
	if (tabPtr->flags & TAB_REDRAW) {
	    Tcl_CancelIdleCall(DisplayTearoff, tabPtr);
	}
	Tk_DeleteEventHandler(tkwin, StructureNotifyMask, TearoffEventProc,
	    tabPtr);
	if (tabPtr->tkwin != NULL) {
	    XRectangle rect;

	    GetWindowRectangle(tabPtr, nbPtr->tkwin, FALSE, &rect);
	    Blt_RelinkWindow(tabPtr->tkwin, nbPtr->tkwin, rect.x, rect.y);
	    if (tabPtr == nbPtr->selectPtr) {
		ArrangeWindow(tabPtr->tkwin, &rect, TRUE);
	    } else {
		Tk_UnmapWindow(tabPtr->tkwin);
	    }
	}
	Tk_DestroyWindow(tkwin);
	tabPtr->container = NULL;
    }
}

static int
CreateTearoff(
    Notebook *nbPtr,
    Tcl_Obj *objPtr,
    Tab *tabPtr)
{
    Tk_Window tkwin;
    int width, height;
    char *name;

    name = Tcl_GetString(objPtr);
    tkwin = Tk_CreateWindowFromPath(nbPtr->interp, nbPtr->tkwin, name,
	(char *)NULL);
    if (tkwin == NULL) {
	return TCL_ERROR;
    }
    tabPtr->container = tkwin;
    if (Tk_WindowId(tkwin) == None) {
	Tk_MakeWindowExist(tkwin);
    }
    Tk_SetClass(tkwin, "Tearoff");
    Tk_CreateEventHandler(tkwin, (ExposureMask | StructureNotifyMask),
	TearoffEventProc, tabPtr);
    if (Tk_WindowId(tabPtr->tkwin) == None) {
	Tk_MakeWindowExist(tabPtr->tkwin);
    }
    width = Tk_Width(tabPtr->tkwin);
    if (width < 2) {
	width = (tabPtr->reqWidth > 0)
	    ? tabPtr->reqWidth : Tk_ReqWidth(tabPtr->tkwin);
    }
    width += PADDING(tabPtr->padX) + 2 *
	Tk_Changes(tabPtr->tkwin)->border_width;
    width += 2 * (nbPtr->inset2 + nbPtr->inset);
#define TEAR_OFF_TAB_SIZE	5
    height = Tk_Height(tabPtr->tkwin);
    if (height < 2) {
	height = (tabPtr->reqHeight > 0)
	    ? tabPtr->reqHeight : Tk_ReqHeight(tabPtr->tkwin);
    }
    height += PADDING(tabPtr->padY) +
	2 * Tk_Changes(tabPtr->tkwin)->border_width;
    height += nbPtr->inset + nbPtr->inset2 + nbPtr->yPad +
	TEAR_OFF_TAB_SIZE + nbPtr->outerPad;
    Tk_GeometryRequest(tkwin, width, height);
    Tk_UnmapWindow(tabPtr->tkwin);
    /* Tk_MoveWindow(tabPtr->tkwin, 0, 0); */
#ifdef WIN32
    AdoptWindow(tabPtr);
#else
    Tcl_DoWhenIdle(AdoptWindow, tabPtr);
#endif
    Tcl_SetStringObj(Tcl_GetObjResult(nbPtr->interp), Tk_PathName(tkwin), -1);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * TabCgetOp --
 *
 *	  .h tab cget index option
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
TabCgetOp(
    Notebook *nbPtr,
    Tcl_Interp *interp,
    int objc,			/* Not used. */
    Tcl_Obj *const *objv)
{
    Tab *tabPtr;

    if (GetTabFromObj(nbPtr, objv[3], &tabPtr, INVALID_FAIL) != TCL_OK) {
	return TCL_ERROR;
    }
    imageOption.clientData = nbPtr;
    return Blt_ConfigureValueFromObj(interp, nbPtr->tkwin, tabConfigSpecs,
	(char *)tabPtr, objv[4], 0);
}

/*
 *---------------------------------------------------------------------------
 *
 * TabConfigureOp --
 *
 * 	This procedure is called to process a list of configuration options
 * 	database, in order to reconfigure the options for one or more tabs in
 * 	the widget.
 *
 *	  .h tab configure index ?index...? ?option value?...
 *
 * Results:
 *	A standard Tcl result.  If TCL_ERROR is returned, then interp->result
 *	contains an error message.
 *
 * Side Effects:
 *	Configuration information, such as text string, colors, font, etc. get
 *	set; old resources get freed, if there were any.  The widget is
 *	redisplayed if needed.
 *
 *---------------------------------------------------------------------------
 */
static int
TabConfigureOp(
    Notebook *nbPtr,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    int nTabs, nOpts, result;
    Tcl_Obj *const *options;
    int i;
    Tab *tabPtr;
    char *string;

    /* Figure out where the option value pairs begin */
    objc -= 3;
    objv += 3;
    for (i = 0; i < objc; i++) {
	string = Tcl_GetString(objv[i]);
	if (string[0] == '-') {
	    break;
	}
	if (GetTabFromObj(nbPtr, objv[i], &tabPtr, INVALID_FAIL) != TCL_OK) {
	    return TCL_ERROR;	/* Can't find node. */
	}
    }
    nTabs = i;			/* Number of tab indices specified */
    nOpts = objc - i;		/* Number of options specified */
    options = objv + i;		/* Start of options in objv  */

    for (i = 0; i < nTabs; i++) {
	GetTabFromObj(nbPtr, objv[i], &tabPtr, INVALID_FAIL);
	if (objc == 1) {
	    return Blt_ConfigureInfoFromObj(interp, nbPtr->tkwin, 
		tabConfigSpecs, (char *)tabPtr, (Tcl_Obj *)NULL, 0);
	} else if (objc == 2) {
	    return Blt_ConfigureInfoFromObj(interp, nbPtr->tkwin, 
		tabConfigSpecs, (char *)tabPtr, objv[2], 0);
	}
	Tcl_Preserve(tabPtr);
	imageOption.clientData = nbPtr;
	result = Blt_ConfigureWidgetFromObj(interp, nbPtr->tkwin, 
		tabConfigSpecs, nOpts, options, (char *)tabPtr, 
		BLT_CONFIG_OBJV_ONLY);
	if (result == TCL_OK) {
	    result = ConfigureTab(nbPtr, tabPtr);
	}
	Tcl_Release(tabPtr);
	if (result == TCL_ERROR) {
	    return TCL_ERROR;
	}
	if (tabPtr->flags & TAB_VISIBLE) {
	    nbPtr->flags |= (TNB_LAYOUT | TNB_SCROLL);
	    EventuallyRedraw(nbPtr);
	}
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * TabDockallOp --
 *
 *	  .h tab dockall
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
TabDockallOp(
    Notebook *nbPtr,
    Tcl_Interp *interp,
    int objc,			/* Not used. */
    Tcl_Obj *const *objv)	/* Not used. */
{
    Blt_ChainLink link;

    for (link = Blt_ChainFirstLink(nbPtr->chain); link != NULL;
	 link = Blt_ChainNextLink(link)) {
	Tab *tabPtr;

	tabPtr = Blt_ChainGetValue(link);
	if (tabPtr->container != NULL) {
	    Tcl_EventuallyFree(tabPtr, DestroyTearoff);
	}
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * TabNamesOp --
 *
 *	  .h tab names pattern
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
TabNamesOp(
    Notebook *nbPtr,
    Tcl_Interp *interp,
    int objc,			
    Tcl_Obj *const *objv)	
{
    Tcl_Obj *listObjPtr;

    listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
    if (objc == 3) {
	Blt_ChainLink link;

	for (link = Blt_ChainFirstLink(nbPtr->chain); link != NULL;
	    link = Blt_ChainNextLink(link)) {
	    Tab *tabPtr;

	    tabPtr = Blt_ChainGetValue(link);
	    Tcl_ListObjAppendElement(interp, listObjPtr, 
			Tcl_NewStringObj(tabPtr->name, -1));
	}
    } else {
	Blt_ChainLink link;

	for (link = Blt_ChainFirstLink(nbPtr->chain); link != NULL;
	     link = Blt_ChainNextLink(link)) {
	    Tab *tabPtr;
	    int i;

	    tabPtr = Blt_ChainGetValue(link);
	    for (i = 3; i < objc; i++) {
		if (Tcl_StringMatch(tabPtr->name, Tcl_GetString(objv[i]))) {
		    Tcl_ListObjAppendElement(interp, listObjPtr, 
				Tcl_NewStringObj(tabPtr->name, -1));
		    break;
		}
	    }
	}
    }
    Tcl_SetObjResult(interp, listObjPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * TabTearoffOp --
 *
 *	  .h tab tearoff index ?title?
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
TabTearoffOp(
    Notebook *nbPtr,
    Tcl_Interp *interp,
    int objc,			/* Not used. */
    Tcl_Obj *const *objv)
{
    Tab *tabPtr;
    int result;
    Tk_Window tkwin;
    char *string;

    if (GetTabFromObj(nbPtr, objv[3], &tabPtr, INVALID_OK) != TCL_OK) {
	return TCL_ERROR;
    }
    if ((tabPtr == NULL) || (tabPtr->tkwin == NULL) ||
	(tabPtr->state == STATE_DISABLED)) {
	return TCL_OK;		/* No-op */
    }
    if (objc == 4) {
	Tk_Window parent;

	parent = (tabPtr->container == NULL) ? nbPtr->tkwin : tabPtr->container;
	Tcl_SetStringObj(Tcl_GetObjResult(interp), Tk_PathName(parent), -1);
	return TCL_OK;
    }
    Tcl_Preserve(tabPtr);
    result = TCL_OK;

    string = Tcl_GetString(objv[4]);
    tkwin = Tk_NameToWindow(interp, string, nbPtr->tkwin);
    Tcl_ResetResult(interp);

    if (tabPtr->container != NULL) {
	Tcl_EventuallyFree(tabPtr, DestroyTearoff);
    }
    if ((tkwin != nbPtr->tkwin) && (tabPtr->container == NULL)) {
	result = CreateTearoff(nbPtr, objv[4], tabPtr);
    }
    Tcl_Release(tabPtr);
    EventuallyRedraw(nbPtr);
    return result;
}

/*
 *---------------------------------------------------------------------------
 *
 * TabOp --
 *
 *	This procedure handles tab operations.
 *
 * Results:
 *	A standard Tcl result.
 *
 *---------------------------------------------------------------------------
 */
static Blt_OpSpec tabOps[] =
{
    {"cget", 2, TabCgetOp, 5, 5, "nameOrIndex option",},
    {"configure", 2, TabConfigureOp, 4, 0,
	"nameOrIndex ?option value?...",},
    {"dockall", 1, TabDockallOp, 3, 3, "" }, 
    {"names", 1, TabNamesOp, 3, 0, "?pattern...?",},
    {"tearoff", 1, TabTearoffOp, 4, 5, "index ?parent?",},
};

static int nTabOps = sizeof(tabOps) / sizeof(Blt_OpSpec);

static int
TabOp(
    Notebook *nbPtr,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    NotebookCmdProc *proc;
    int result;

    proc = Blt_GetOpFromObj(interp, nTabOps, tabOps, BLT_OP_ARG2, 
		    objc, objv, 0);
    if (proc == NULL) {
	return TCL_ERROR;
    }
    result = (*proc) (nbPtr, interp, objc, objv);
    return result;
}

/*
 *---------------------------------------------------------------------------
 *
 * PerforationActivateOp --
 *
 * 	This procedure is called to highlight the perforation.
 *
 *	  .h perforation highlight boolean
 *
 * Results:
 *	A standard Tcl result.  If TCL_ERROR is returned, then interp->result
 *	contains an error message.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
PerforationActivateOp(
    Notebook *nbPtr,
    Tcl_Interp *interp,		/* Not used. */
    int objc,
    Tcl_Obj *const *objv)
{
    int bool;

    if (Tcl_GetBooleanFromObj(interp, objv[3], &bool) != TCL_OK) {
	return TCL_ERROR;
    }
    if (bool) {
	nbPtr->flags |= PERFORATION_ACTIVE;
    } else {
	nbPtr->flags &= ~PERFORATION_ACTIVE;
    }
    EventuallyRedraw(nbPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * PerforationInvokeOp --
 *
 * 	This procedure is called to invoke a perforation command.
 *
 *	  .t perforation invoke
 *
 * Results:
 *	A standard Tcl result.  If TCL_ERROR is returned, then
 *	interp->result contains an error message.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
PerforationInvokeOp(
    Notebook *nbPtr,
    Tcl_Interp *interp,		/* Not used. */
    int objc,
    Tcl_Obj *const *objv)
{

    if (nbPtr->selectPtr != NULL) {
	char *cmd;
	
	cmd = GETATTR(nbPtr->selectPtr, perfCommand);
	if (cmd != NULL) {
	    Tcl_DString dString;
	    int result;
	    
	    PercentSubst(nbPtr, nbPtr->selectPtr, cmd, &dString);
	    Tcl_Preserve(nbPtr);
	    result = Tcl_GlobalEval(interp, Tcl_DStringValue(&dString));
	    Tcl_Release(nbPtr);
	    Tcl_DStringFree(&dString);
	    if (result != TCL_OK) {
		return TCL_ERROR;
	    }
	}
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * PerforationOp --
 *
 *	This procedure handles tab operations.
 *
 * Results:
 *	A standard Tcl result.
 *
 *---------------------------------------------------------------------------
 */
static Blt_OpSpec perforationOps[] =
{
    {"activate", 1, PerforationActivateOp, 4, 4, "boolean" }, 
    {"invoke", 1, PerforationInvokeOp, 3, 3, "",},
};

static int nPerforationOps = sizeof(perforationOps) / sizeof(Blt_OpSpec);

static int
PerforationOp(
    Notebook *nbPtr,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    NotebookCmdProc *proc;
    int result;

    proc = Blt_GetOpFromObj(interp, nPerforationOps, perforationOps, 
	BLT_OP_ARG2, objc, objv, 0);
    if (proc == NULL) {
	return TCL_ERROR;
    }
    result = (*proc) (nbPtr, interp, objc, objv);
    return result;
}

/*
 *---------------------------------------------------------------------------
 *
 * ScanOp --
 *
 *	Implements the quick scan.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ScanOp(
    Notebook *nbPtr,
    Tcl_Interp *interp,
    int objc,			/* Not used. */
    Tcl_Obj *const *objv)
{
    char *string;
    char c;
    int oper;
    int x, y;
    size_t length;

#define SCAN_MARK	1
#define SCAN_DRAGTO	2
    string = Tcl_GetString(objv[2]);
    c = string[0];
    length = strlen(string);
    if ((c == 'm') && (strncmp(string, "mark", length) == 0)) {
	oper = SCAN_MARK;
    } else if ((c == 'd') && (strncmp(string, "dragto", length) == 0)) {
	oper = SCAN_DRAGTO;
    } else {
	Tcl_AppendResult(interp, "bad scan operation \"", string,
	    "\": should be either \"mark\" or \"dragto\"", (char *)NULL);
	return TCL_ERROR;
    }
    if ((Tk_GetPixelsFromObj(interp, nbPtr->tkwin, objv[3], &x) != TCL_OK) ||
	(Tk_GetPixelsFromObj(interp, nbPtr->tkwin, objv[4], &y) != TCL_OK)) {
	return TCL_ERROR;
    }
    if (oper == SCAN_MARK) {
	if (nbPtr->side & TAB_VERTICAL) {
	    nbPtr->scanAnchor = y;
	} else {
	    nbPtr->scanAnchor = x;
	}
	nbPtr->scanOffset = nbPtr->scrollOffset;
    } else {
	int offset, delta;

	if (nbPtr->side & TAB_VERTICAL) {
	    delta = nbPtr->scanAnchor - y;
	} else {
	    delta = nbPtr->scanAnchor - x;
	}
	offset = nbPtr->scanOffset + (10 * delta);
	offset = Blt_AdjustViewport(offset, nbPtr->worldWidth,
	    VPORTWIDTH(nbPtr), nbPtr->scrollUnits, BLT_SCROLL_MODE_CANVAS);
	nbPtr->scrollOffset = offset;
	nbPtr->flags |= TNB_SCROLL;
	EventuallyRedraw(nbPtr);
    }
    return TCL_OK;
}

/*ARGSUSED*/
static int
SeeOp(
    Notebook *nbPtr,
    Tcl_Interp *interp,		/* Not used. */
    int objc,
    Tcl_Obj *const *objv)
{
    Tab *tabPtr;

    if (GetTabFromObj(nbPtr, objv[2], &tabPtr, INVALID_OK) != TCL_OK) {
	return TCL_ERROR;
    }
    if (tabPtr != NULL) {
	int left, right, width;

	width = VPORTWIDTH(nbPtr);
	left = nbPtr->scrollOffset + nbPtr->xSelectPad;
	right = nbPtr->scrollOffset + width - nbPtr->xSelectPad;

	/* If the tab is partially obscured, scroll so that it's
	 * entirely in view. */
	if (tabPtr->worldX < left) {
	    nbPtr->scrollOffset = tabPtr->worldX;
	    if (TabIndex(nbPtr, tabPtr) > 0) {
		nbPtr->scrollOffset -= TAB_SCROLL_OFFSET;
	    }
	} else if ((tabPtr->worldX + tabPtr->worldWidth) >= right) {
	    Blt_ChainLink link;

	    nbPtr->scrollOffset = tabPtr->worldX + tabPtr->worldWidth -
		(width - 2 * nbPtr->xSelectPad);
	    link = Blt_ChainNextLink(tabPtr->link); 
	    if (link != NULL) {
		Tab *nextPtr;

		nextPtr = Blt_ChainGetValue(link);
		if (nextPtr->tier == tabPtr->tier) {
		    nbPtr->scrollOffset += TAB_SCROLL_OFFSET;
		}
	    }
	}
	nbPtr->flags |= TNB_SCROLL;
	EventuallyRedraw(nbPtr);
    }
    return TCL_OK;
}

/*ARGSUSED*/
static int
SizeOp(
    Notebook *nbPtr,
    Tcl_Interp *interp,
    int objc,			/* Not used. */
    Tcl_Obj *const *objv)	/* Not used. */
{
    int nTabs;

    nTabs = Blt_ChainGetLength(nbPtr->chain);
    Tcl_SetIntObj(Tcl_GetObjResult(interp), nTabs);
    return TCL_OK;
}


static int
CountTabs(Notebook *nbPtr)
{
    int count;
    int width, height;
    Blt_ChainLink link;
    Tab *tabPtr;
    int pageWidth, pageHeight;
    int labelWidth, labelHeight;
    int tabWidth, tabHeight;

    pageWidth = pageHeight = 0;
    count = 0;

    labelWidth = labelHeight = 0;

    /*
     * Pass 1:  Figure out the maximum area needed for a label and a
     *		page.  Both the label and page dimensions are adjusted
     *		for orientation.  In addition, reset the visibility
     *		flags and reorder the tabs.
     */
    for (link = Blt_ChainFirstLink(nbPtr->chain); link != NULL;
	link = Blt_ChainNextLink(link)) {
	tabPtr = Blt_ChainGetValue(link);

	/* Reset visibility flag and order of tabs. */

	tabPtr->flags &= ~TAB_VISIBLE;
	count++;

	if (tabPtr->tkwin != NULL) {
	    width = GetReqWidth(tabPtr);
	    if (pageWidth < width) {
		pageWidth = width;
	    }
	    height = GetReqHeight(tabPtr);
	    if (pageHeight < height) {
		pageHeight = height;
	    }
	}
	if (labelWidth < tabPtr->labelWidth) {
	    labelWidth = tabPtr->labelWidth;
	}
	if (labelHeight < tabPtr->labelHeight) {
	    labelHeight = tabPtr->labelHeight;
	}
    }

    nbPtr->overlap = 0;

    /*
     * Pass 2:	Set the individual sizes of each tab.  This is different
     *		for constant and variable width tabs.  Add the extra space
     *		needed for slanted tabs, now that we know maximum tab
     *		height.
     */
    if (nbPtr->defTabStyle.constWidth) {
	int slant;

	tabWidth = 2 * nbPtr->inset2;
	tabHeight = nbPtr->inset2 /* + 4 */;

	if (nbPtr->side & TAB_VERTICAL) {
	    tabWidth += labelHeight;
	    tabHeight += labelWidth;
	    slant = labelWidth;
	} else {
	    tabWidth += labelWidth;
	    tabHeight += labelHeight;
	    slant = labelHeight;
	}
	if (nbPtr->slant & SLANT_LEFT) {
	    tabWidth += slant;
	    nbPtr->overlap += tabHeight / 2;
	}
	if (nbPtr->slant & SLANT_RIGHT) {
	    tabWidth += slant;
	    nbPtr->overlap += tabHeight / 2;
	}
	for (link = Blt_ChainFirstLink(nbPtr->chain); link != NULL;
	    link = Blt_ChainNextLink(link)) {
	    tabPtr = Blt_ChainGetValue(link);
	    tabPtr->worldWidth = tabWidth;
	    tabPtr->worldHeight = tabHeight;
	}
    } else {
	int slant;

	tabWidth = tabHeight = 0;
	for (link = Blt_ChainFirstLink(nbPtr->chain); link != NULL;
	    link = Blt_ChainNextLink(link)) {
	    tabPtr = Blt_ChainGetValue(link);

	    width = 2 * nbPtr->inset2;
	    height = nbPtr->inset2 /* + 4 */;
	    if (nbPtr->side & TAB_VERTICAL) {
		width += tabPtr->labelHeight;
		height += labelWidth;
		slant = labelWidth;
	    } else {
		width += tabPtr->labelWidth;
		height += labelHeight;
		slant = labelHeight;
	    }
	    width += (nbPtr->slant & SLANT_LEFT) ? slant : nbPtr->corner;
	    width += (nbPtr->slant & SLANT_RIGHT) ? slant : nbPtr->corner;

	    tabPtr->worldWidth = width; /* + 2 * (nbPtr->corner + nbPtr->xSelectPad) */ ;
	    tabPtr->worldHeight = height;

	    if (tabWidth < width) {
		tabWidth = width;
	    }
	    if (tabHeight < height) {
		tabHeight = height;
	    }
	}
	if (nbPtr->slant & SLANT_LEFT) {
	    nbPtr->overlap += tabHeight / 2;
	}
	if (nbPtr->slant & SLANT_RIGHT) {
	    nbPtr->overlap += tabHeight / 2;
	}
    }

    nbPtr->tabWidth = tabWidth;
    nbPtr->tabHeight = tabHeight;

    /*
     * Let the user override any page dimension.
     */
    nbPtr->pageWidth = pageWidth;
    nbPtr->pageHeight = pageHeight;
    if (nbPtr->reqPageWidth > 0) {
	nbPtr->pageWidth = nbPtr->reqPageWidth;
    }
    if (nbPtr->reqPageHeight > 0) {
	nbPtr->pageHeight = nbPtr->reqPageHeight;
    }
    return count;
}


static void
WidenTabs(Notebook *nbPtr, Tab *startPtr, int nTabs, int adjustment)
{
    Tab *tabPtr;
    int i;
    int ration;
    Blt_ChainLink link;
    int x;

    x = startPtr->tier;
    while (adjustment > 0) {
	ration = adjustment / nTabs;
	if (ration == 0) {
	    ration = 1;
	}
	link = startPtr->link;
	for (i = 0; (link != NULL) && (i < nTabs) && (adjustment > 0); i++) {
	    tabPtr = Blt_ChainGetValue(link);
	    adjustment -= ration;
	    tabPtr->worldWidth += ration;
	    assert(x == tabPtr->tier);
	    link = Blt_ChainNextLink(link);
	}
    }
    /*
     * Go back and reset the world X-coordinates of the tabs,
     * now that their widths have changed.
     */
    x = 0;
    link = startPtr->link;
    for (i = 0; (i < nTabs) && (link != NULL); i++) {
	tabPtr = Blt_ChainGetValue(link);
	tabPtr->worldX = x;
	x += tabPtr->worldWidth + nbPtr->gap - nbPtr->overlap;
	link = Blt_ChainNextLink(link);
    }
}


static void
AdjustTabSizes(Notebook *nbPtr, int nTabs)
{
    int tabsPerTier;
    int total, count, extra;
    Tab *startPtr, *nextPtr;
    Blt_ChainLink link;
    Tab *tabPtr;
    int x, maxWidth;

    tabsPerTier = (nTabs + (nbPtr->nTiers - 1)) / nbPtr->nTiers;
    x = 0;
    maxWidth = 0;
    if (nbPtr->defTabStyle.constWidth) {
	link = Blt_ChainFirstLink(nbPtr->chain);
	count = 1;
	while (link != NULL) {
	    int i;

	    for (i = 0; i < tabsPerTier; i++) {
		tabPtr = Blt_ChainGetValue(link);
		tabPtr->tier = count;
		tabPtr->worldX = x;
		x += tabPtr->worldWidth + nbPtr->gap - nbPtr->overlap;
		link = Blt_ChainNextLink(link);
		if (x > maxWidth) {
		    maxWidth = x;
		}
		if (link == NULL) {
		    goto done;
		}
	    }
	    count++;
	    x = 0;
	}
    }
  done:
    /* Add to tab widths to fill out row. */
    if (((nTabs % tabsPerTier) != 0) && (nbPtr->defTabStyle.constWidth)) {
	return;
    }
    startPtr = NULL;
    count = total = 0;
    for (link = Blt_ChainFirstLink(nbPtr->chain); link != NULL;
	/*empty*/ ) {
	tabPtr = Blt_ChainGetValue(link);
	if (startPtr == NULL) {
	    startPtr = tabPtr;
	}
	count++;
	total += tabPtr->worldWidth + nbPtr->gap - nbPtr->overlap;
	link = Blt_ChainNextLink(link);
	if (link != NULL) {
	    nextPtr = Blt_ChainGetValue(link);
	    if (tabPtr->tier == nextPtr->tier) {
		continue;
	    }
	}
	total += nbPtr->overlap;
	extra = nbPtr->worldWidth - total;
	assert(count > 0);
	if (extra > 0) {
	    WidenTabs(nbPtr, startPtr, count, extra);
	}
	count = total = 0;
	startPtr = NULL;
    }
}

/*
 *
 * tabWidth = textWidth + gap + (2 * (pad + outerBW));
 *
 * tabHeight = textHeight + 2 * (pad + outerBW) + topMargin;
 *
 */
static void
ComputeLayout(Notebook *nbPtr)
{
    int width;
    Blt_ChainLink link;
    Tab *tabPtr;
    int x, extra;
    int nTiers, nTabs;

    nbPtr->nTiers = 0;
    nbPtr->pageTop = 0;
    nbPtr->worldWidth = 1;
    nbPtr->yPad = 0;

    nTabs = CountTabs(nbPtr);
    if (nTabs == 0) {
	return;
    }
    /* Reset the pointers to the selected and starting tab. */
    if (nbPtr->selectPtr == NULL) {
	link = Blt_ChainFirstLink(nbPtr->chain);
	if (link != NULL) {
	    nbPtr->selectPtr = Blt_ChainGetValue(link);
	}
    }
    if (nbPtr->startPtr == NULL) {
	nbPtr->startPtr = nbPtr->selectPtr;
    }
    if (nbPtr->focusPtr == NULL) {
	nbPtr->focusPtr = nbPtr->selectPtr;
	Blt_SetFocusItem(nbPtr->bindTable, nbPtr->focusPtr, NULL);
    }

    if (nbPtr->side & TAB_VERTICAL) {
        width = Tk_Height(nbPtr->tkwin) - 2 * 
	    (nbPtr->corner + nbPtr->xSelectPad);
    } else {
        width = Tk_Width(nbPtr->tkwin) - (2 * nbPtr->inset) -
		nbPtr->xSelectPad - nbPtr->corner;
    }
    nbPtr->flags |= TNB_STATIC;
    if (nbPtr->reqTiers > 1) {
	int total, maxWidth;

	/* Static multiple tier mode. */

	/* Sum tab widths and determine the number of tiers needed. */
	nTiers = 1;
	total = x = 0;
	for (link = Blt_ChainFirstLink(nbPtr->chain); link != NULL;
	    link = Blt_ChainNextLink(link)) {
	    tabPtr = Blt_ChainGetValue(link);
	    if ((x + tabPtr->worldWidth) > width) {
		nTiers++;
		x = 0;
	    }
	    tabPtr->worldX = x;
	    tabPtr->tier = nTiers;
	    extra = tabPtr->worldWidth + nbPtr->gap - nbPtr->overlap;
	    total += extra, x += extra;
	}
	maxWidth = width;

	if (nTiers > nbPtr->reqTiers) {
	    /*
	     * The tabs do not fit into the requested number of tiers.
             * Go into scrolling mode.
	     */
	    width = ((total + nbPtr->tabWidth) / nbPtr->reqTiers);
	    x = 0;
	    nTiers = 1;
	    for (link = Blt_ChainFirstLink(nbPtr->chain);
		link != NULL; link = Blt_ChainNextLink(link)) {
		tabPtr = Blt_ChainGetValue(link);
		tabPtr->tier = nTiers;
		/*
		 * Keep adding tabs to a tier until we overfill it.
		 */
		tabPtr->worldX = x;
		x += tabPtr->worldWidth + nbPtr->gap - nbPtr->overlap;
		if (x > width) {
		    nTiers++;
		    if (x > maxWidth) {
			maxWidth = x;
		    }
		    x = 0;
		}
	    }
	    nbPtr->flags &= ~TNB_STATIC;
	}
	nbPtr->worldWidth = maxWidth;
	nbPtr->nTiers = nTiers;

	if (nTiers > 1) {
	    AdjustTabSizes(nbPtr, nTabs);
	}
	if (nbPtr->flags & TNB_STATIC) {
	    nbPtr->worldWidth = VPORTWIDTH(nbPtr);
	} else {
	    /* Do you add an offset ? */
	    nbPtr->worldWidth += (nbPtr->xSelectPad + nbPtr->corner);
	}
	nbPtr->worldWidth += nbPtr->overlap;
	if (nbPtr->selectPtr != NULL) {
	    RenumberTiers(nbPtr, nbPtr->selectPtr);
	}
    } else {
	/*
	 * Scrollable single tier mode.
	 */
	nTiers = 1;
	x = 0;
	for (link = Blt_ChainFirstLink(nbPtr->chain); link != NULL;
	    link = Blt_ChainNextLink(link)) {
	    tabPtr = Blt_ChainGetValue(link);
	    tabPtr->tier = nTiers;
	    tabPtr->worldX = x;
	    tabPtr->worldY = 0;
	    x += tabPtr->worldWidth + nbPtr->gap - nbPtr->overlap;
	}
	nbPtr->worldWidth = x + nbPtr->corner - nbPtr->gap +
	    nbPtr->xSelectPad + nbPtr->overlap;
	nbPtr->flags &= ~TNB_STATIC;
    }
    if (nTiers == 1) {
	nbPtr->yPad = nbPtr->ySelectPad;
    }
    nbPtr->nTiers = nTiers;
    nbPtr->pageTop = nbPtr->inset + nbPtr->yPad /* + 4 */ +
	(nbPtr->nTiers * nbPtr->tabHeight) + nbPtr->inset2;

    if (nbPtr->side & TAB_VERTICAL) {
	for (link = Blt_ChainFirstLink(nbPtr->chain); link != NULL;
	    link = Blt_ChainNextLink(link)) {
	    tabPtr = Blt_ChainGetValue(link);
	    tabPtr->screenWidth = (short int)nbPtr->tabHeight;
	    tabPtr->screenHeight = (short int)tabPtr->worldWidth;
	}
    } else {
	for (link = Blt_ChainFirstLink(nbPtr->chain); link != NULL;
	    link = Blt_ChainNextLink(link)) {
	    tabPtr = Blt_ChainGetValue(link);
	    tabPtr->screenWidth = (short int)tabPtr->worldWidth;
	    tabPtr->screenHeight = (short int)nbPtr->tabHeight;
	}
    }
}

static void
ComputeVisibleTabs(Notebook *nbPtr)
{
    int nVisibleTabs;
    Tab *tabPtr;
    Blt_ChainLink link;

    nbPtr->nVisible = 0;
    if (Blt_ChainGetLength(nbPtr->chain) == 0) {
	return;
    }
    nVisibleTabs = 0;
    if (nbPtr->flags & TNB_STATIC) {

	/* Static multiple tier mode. */

	for (link = Blt_ChainFirstLink(nbPtr->chain); link != NULL;
	    link = Blt_ChainNextLink(link)) {
	    tabPtr = Blt_ChainGetValue(link);
	    tabPtr->flags |= TAB_VISIBLE;
	    nVisibleTabs++;
	}
    } else {
	int width, offset;
	/*
	 * Scrollable (single or multiple) tier mode.
	 */
	offset = nbPtr->scrollOffset - (nbPtr->outerPad + nbPtr->xSelectPad);
	width = VPORTWIDTH(nbPtr) + nbPtr->scrollOffset +
	    2 * nbPtr->outerPad;
	for (link = Blt_ChainFirstLink(nbPtr->chain); link != NULL;
	    link = Blt_ChainNextLink(link)) {
	    tabPtr = Blt_ChainGetValue(link);
	    if ((tabPtr->worldX >= width) ||
		((tabPtr->worldX + tabPtr->worldWidth) < offset)) {
		tabPtr->flags &= ~TAB_VISIBLE;
	    } else {
		tabPtr->flags |= TAB_VISIBLE;
		nVisibleTabs++;
	    }
	}
    }
    for (link = Blt_ChainFirstLink(nbPtr->chain); link != NULL;
	link = Blt_ChainNextLink(link)) {
	tabPtr = Blt_ChainGetValue(link);
	tabPtr->screenX = tabPtr->screenY = -1000;
	if (tabPtr->flags & TAB_VISIBLE) {
	    WorldToScreen(nbPtr, tabPtr->worldX, tabPtr->worldY,
		&(tabPtr->screenX), &(tabPtr->screenY));
	    switch (nbPtr->side) {
	    case SIDE_RIGHT:
		tabPtr->screenX -= nbPtr->tabHeight;
		break;

	    case SIDE_BOTTOM:
		tabPtr->screenY -= nbPtr->tabHeight;
		break;
	    }
	}
    }
    nbPtr->nVisible = nVisibleTabs;
    Blt_PickCurrentItem(nbPtr->bindTable);
}


static void
Draw3DFolder(
    Notebook *nbPtr,
    Tab *tabPtr,
    Drawable drawable,
    int side,
    XPoint pointArr[],
    int nPoints)
{
    GC gc;
    int relief, borderWidth;
    Tk_3DBorder border;

    if (tabPtr == nbPtr->selectPtr) {
	border = GETATTR(tabPtr, selBorder);
    } else if (tabPtr->border != NULL) {
	border = tabPtr->border;
    } else {
	border = nbPtr->defTabStyle.border;
    }
    relief = nbPtr->defTabStyle.relief;
    if ((side == SIDE_RIGHT) || (side == SIDE_TOP)) {
	borderWidth = -nbPtr->defTabStyle.borderWidth;
	if (relief == TK_RELIEF_SUNKEN) {
	    relief = TK_RELIEF_RAISED;
	} else if (relief == TK_RELIEF_RAISED) {
	    relief = TK_RELIEF_SUNKEN;
	}
    } else {
	borderWidth = nbPtr->defTabStyle.borderWidth;
    }
    {
	int i;

#ifndef notdef
	int dx, dy;
	int oldType, newType;
	int start;

	dx = pointArr[0].x - pointArr[1].x;
	dy = pointArr[0].y - pointArr[1].y;
	oldType = ((dy < 0) || (dx > 0));
	start = 0;
	for (i = 1; i < nPoints; i++) {
	    dx = pointArr[i - 1].x - pointArr[i].x;
	    dy = pointArr[i - 1].y - pointArr[i].y;
	    newType = ((dy < 0) || (dx > 0));
	    if (newType != oldType) {
		if (oldType) {
		    gc = Tk_GCForColor(nbPtr->shadowColor, drawable);
		}  else {
		    gc = Tk_3DBorderGC(nbPtr->tkwin, border, TK_3D_FLAT_GC);
		}		    
		XDrawLines(nbPtr->display, drawable, gc, pointArr + start, 
			   i - start, CoordModeOrigin);
		start = i - 1;
		oldType = newType;
	    }
	}
	if (start != i) {
	    if (oldType) {
		gc = Tk_GCForColor(nbPtr->shadowColor, drawable);
	    }  else {
		gc = Tk_3DBorderGC(nbPtr->tkwin, border, TK_3D_FLAT_GC);
	    }		    
	    XDrawLines(nbPtr->display, drawable, gc, pointArr + start, 
		       i - start, CoordModeOrigin);
	}
#else
	/* Draw the outline of the folder. */
	gc = Tk_GCForColor(nbPtr->shadowColor, drawable);
	XDrawLines(nbPtr->display, drawable, gc, pointArr, nPoints, 
		   CoordModeOrigin);
#endif
    }
    /* And the folder itself. */
    if (tabPtr->tile != NULL) {
#ifdef notdef
	Tk_Fill3DPolygon(nbPtr->tkwin, drawable, border, pointArr, nPoints,
	    borderWidth, relief);
#endif
	Blt_TilePolygon(nbPtr->tkwin, drawable, tabPtr->tile, pointArr, 
			nPoints);
#ifdef notdef
	Tk_Draw3DPolygon(nbPtr->tkwin, drawable, border, pointArr, nPoints,
	    borderWidth, relief);
#endif
    } else {
	Tk_Fill3DPolygon(nbPtr->tkwin, drawable, border, pointArr, nPoints,
	    borderWidth, relief);
    }
}

/*
 *   x,y
 *    |1|2|3|   4    |3|2|1|
 *
 *   1. tab border width
 *   2. corner offset
 *   3. label pad
 *   4. label width
 *
 *
 */
static void
DrawLabel(
    Notebook *nbPtr,
    Tab *tabPtr,
    Drawable drawable)
{
    int x, y, dx, dy;
    int tx, ty, ix, iy;
    int imgWidth, imgHeight;
    int active, selected;
    XColor *fgColor, *bgColor;
    Tk_3DBorder border;
    GC gc;

    if (!(tabPtr->flags & TAB_VISIBLE)) {
	return;
    }
    x = tabPtr->screenX;
    y = tabPtr->screenY;

    active = (nbPtr->activePtr == tabPtr);
    selected = (nbPtr->selectPtr == tabPtr);

    fgColor = GETATTR(tabPtr, textColor);
    border = GETATTR(tabPtr, border);
    if (selected) {
	border = GETATTR(tabPtr, selBorder);
    }
    bgColor = Tk_3DBorderColor(border);
    if (active) {
	Tk_3DBorder activeBorder;

	activeBorder = GETATTR(tabPtr, activeBorder);
	bgColor = Tk_3DBorderColor(activeBorder);
    }
    dx = (tabPtr->screenWidth - tabPtr->labelWidth) / 2;
    dy = (tabPtr->screenHeight - tabPtr->labelHeight) / 2;


    /*
     * The label position is computed with screen coordinates.  This
     * is because both text and image components are oriented in
     * screen space, and not according to the orientation of the tabs
     * themselves.  That's why we have to consider the side when
     * correcting for left/right slants.
     */
    switch (nbPtr->side) {
    case SIDE_TOP:
    case SIDE_BOTTOM:
	if (nbPtr->slant == SLANT_LEFT) {
	    x += nbPtr->overlap;
	} else if (nbPtr->slant == SLANT_RIGHT) {
	    x -= nbPtr->overlap;
	}
	break;
    case SIDE_LEFT:
    case SIDE_RIGHT:
	if (nbPtr->slant == SLANT_LEFT) {
	    y += nbPtr->overlap;
	} else if (nbPtr->slant == SLANT_RIGHT) {
	    y -= nbPtr->overlap;
	}
	break;
    }

    /*
     * Draw the active or normal background color over the entire
     * label area.  This includes both the tab's text and image.
     * The rectangle should be 2 pixels wider/taller than this
     * area. So if the label consists of just an image, we get an
     * halo around the image when the tab is active.
     */
    gc = Tk_GCForColor(bgColor, drawable);
    XFillRectangle(nbPtr->display, drawable, gc, x + dx, y + dy,
	tabPtr->labelWidth, tabPtr->labelHeight);

    if ((nbPtr->flags & TNB_FOCUS) && (nbPtr->focusPtr == tabPtr)) {
	XDrawRectangle(nbPtr->display, drawable, nbPtr->defTabStyle.activeGC,
	    x + dx, y + dy, tabPtr->labelWidth - 1, tabPtr->labelHeight - 1);
    }
    tx = ty = ix = iy = 0;	/* Suppress compiler warning. */

    imgWidth = imgHeight = 0;
    if (tabPtr->tabImagePtr != NULL) {
	imgWidth = ImageWidth(tabPtr->tabImagePtr);
	imgHeight = ImageHeight(tabPtr->tabImagePtr);
    }
    switch (nbPtr->defTabStyle.textSide) {
    case SIDE_LEFT:
	tx = x + dx + tabPtr->iPadX.side1;
	ty = y + (tabPtr->screenHeight - tabPtr->textHeight) / 2;
	ix = tx + tabPtr->textWidth + IMAGE_PAD;
	iy = y + (tabPtr->screenHeight - imgHeight) / 2;
	break;
    case SIDE_RIGHT:
	ix = x + dx + tabPtr->iPadX.side1 + IMAGE_PAD;
	iy = y + (tabPtr->screenHeight - imgHeight) / 2;
	tx = ix + imgWidth;
	ty = y + (tabPtr->screenHeight - tabPtr->textHeight) / 2;
	break;
    case SIDE_BOTTOM:
	iy = y + dy + tabPtr->iPadY.side1 + IMAGE_PAD;
	ix = x + (tabPtr->screenWidth - imgWidth) / 2;
	ty = iy + imgHeight;
	tx = x + (tabPtr->screenWidth - tabPtr->textWidth) / 2;
	break;
    case SIDE_TOP:
	tx = x + (tabPtr->screenWidth - tabPtr->textWidth) / 2;
	ty = y + dy + tabPtr->iPadY.side1 + IMAGE_PAD;
	ix = x + (tabPtr->screenWidth - imgWidth) / 2;
	iy = ty + tabPtr->textHeight;
	break;
    }
    if (tabPtr->tabImagePtr != NULL) {
	Tk_RedrawImage(ImageBits(tabPtr->tabImagePtr), 0, 0, imgWidth, 
		imgHeight, drawable, ix, iy);
    }
    if (tabPtr->text != NULL) {
	TextStyle ts;
	XColor *activeColor;
	Blt_Font font;

	activeColor = fgColor;
	if (selected) {
	    activeColor = GETATTR(tabPtr, selColor);
	    font = GETATTR(tabPtr, selFont);
	    if (font == NULL) {
		font = GETATTR(tabPtr, font);
	    }
	} else if (active) {
	    activeColor = GETATTR(tabPtr, activeFgColor);
	}
	if (selected) {
	    font = GETATTR(tabPtr, selFont);
	    if (font == NULL) {
		font = GETATTR(tabPtr, font);
	    }
	} else {
	    font = GETATTR(tabPtr, font);
	}
	Blt_Ts_InitStyle(ts);
	Blt_Ts_SetAngle(ts, nbPtr->defTabStyle.angle);
	Blt_Ts_SetBorder(ts, border);
	Blt_Ts_SetFont(ts, font);
	Blt_Ts_SetPadding(ts, 2, 2, 0, 0);
	Blt_Ts_SetState(ts, tabPtr->state);
	if (selected || active) {
	    Blt_Ts_SetForeground(ts, activeColor);
	} else {
	    Blt_Ts_SetForeground(ts, fgColor);
	}
	Blt_DrawText(nbPtr->tkwin, drawable, tabPtr->text, &ts, tx, ty);
    }
}

static void
DrawPerforation(
    Notebook *nbPtr,
    Tab *tabPtr,
    Drawable drawable)
{
    XPoint pointArr[2];
    int x, y;
    int segmentWidth, max;
    Tk_3DBorder border, perfBorder;

    if ((tabPtr->container != NULL) || (tabPtr->tkwin == NULL)) {
	return;
    }
    WorldToScreen(nbPtr, tabPtr->worldX + 2, 
	  tabPtr->worldY + tabPtr->worldHeight + 2, &x, &y);
    border = GETATTR(tabPtr, selBorder);
    segmentWidth = 3;
    if (nbPtr->flags & PERFORATION_ACTIVE) {
	perfBorder = GETATTR(tabPtr, activeBorder);
    } else {
	perfBorder = GETATTR(tabPtr, selBorder);
    }	
    if (nbPtr->side & TAB_HORIZONTAL) {
	pointArr[0].x = x;
	pointArr[0].y = pointArr[1].y = y;
	max = tabPtr->screenX + tabPtr->screenWidth - 2;
	Blt_Fill3DRectangle(nbPtr->tkwin, drawable, perfBorder, x - 2, y - 4, 
		tabPtr->screenWidth, 8, 0, TK_RELIEF_FLAT);
	while (pointArr[0].x < max) {
	    pointArr[1].x = pointArr[0].x + segmentWidth;
	    if (pointArr[1].x > max) {
		pointArr[1].x = max;
	    }
	    Tk_Draw3DPolygon(nbPtr->tkwin, drawable, border, pointArr, 2, 1,
		TK_RELIEF_RAISED);
	    pointArr[0].x += 2 * segmentWidth;
	}
    } else {
	pointArr[0].x = pointArr[1].x = x;
	pointArr[0].y = y;
	max  = tabPtr->screenY + tabPtr->screenHeight - 2;
	Blt_Fill3DRectangle(nbPtr->tkwin, drawable, perfBorder,
	       x - 4, y - 2, 8, tabPtr->screenHeight, 0, TK_RELIEF_FLAT);
	while (pointArr[0].y < max) {
	    pointArr[1].y = pointArr[0].y + segmentWidth;
	    if (pointArr[1].y > max) {
		pointArr[1].y = max;
	    }
	    Tk_Draw3DPolygon(nbPtr->tkwin, drawable, border, pointArr, 2, 1,
		TK_RELIEF_RAISED);
	    pointArr[0].y += 2 * segmentWidth;
	}
    }
}

#define NextPoint(px, py) \
	pointPtr->x = (px), pointPtr->y = (py), pointPtr++, nPoints++
#define EndPoint(px, py) \
	pointPtr->x = (px), pointPtr->y = (py), nPoints++

#define BottomLeft(px, py) \
	NextPoint((px) + nbPtr->corner, (py)), \
	NextPoint((px), (py) - nbPtr->corner)

#define TopLeft(px, py) \
	NextPoint((px), (py) + nbPtr->corner), \
	NextPoint((px) + nbPtr->corner, (py))

#define TopRight(px, py) \
	NextPoint((px) - nbPtr->corner, (py)), \
	NextPoint((px), (py) + nbPtr->corner)

#define BottomRight(px, py) \
	NextPoint((px), (py) - nbPtr->corner), \
	NextPoint((px) - nbPtr->corner, (py))


/*
 * From the left edge:
 *
 *   |a|b|c|d|e| f |d|e|g|h| i |h|g|e|d|f|    j    |e|d|c|b|a|
 *
 *	a. highlight ring
 *	b. notebook 3D border
 *	c. outer gap
 *      d. page border
 *	e. page corner
 *	f. gap + select pad
 *	g. label pad x (worldX)
 *	h. internal pad x
 *	i. label width
 *	j. rest of page width
 *
 *  worldX, worldY
 *          |
 *          |
 *          * 4+ . . +5
 *          3+         +6
 *           .         .
 *           .         .
 *   1+. . .2+         +7 . . . .+8
 * 0+                              +9
 *  .                              .
 *  .                              .
 *13+                              +10
 *  12+-------------------------+11
 *
 */
static void
DrawFolder(
    Notebook *nbPtr,
    Tab *tabPtr,
    Drawable drawable)
{
    XPoint pointArr[16];
    XPoint *pointPtr;
    int width, height;
    int left, bottom, right, top, yBot, yTop;
    int x, y;
    int i;
    int nPoints;

    width = VPORTWIDTH(nbPtr);
    height = VPORTHEIGHT(nbPtr);

    x = tabPtr->worldX;
    y = tabPtr->worldY;

    nPoints = 0;
    pointPtr = pointArr;

    /* Remember these are all world coordinates. */
    /*
     * x	Left side of tab.
     * y	Top of tab.
     * yTop	Top of folder.
     * yBot	Bottom of the tab.
     * left	Left side of the folder.
     * right	Right side of the folder.
     * top	Top of folder.
     * bottom	Bottom of folder.
     */
    left = nbPtr->scrollOffset - nbPtr->xSelectPad;
    right = left + width;
    yTop = y + tabPtr->worldHeight;
    yBot = nbPtr->pageTop - (nbPtr->inset + nbPtr->yPad);
    top = yBot - nbPtr->inset2 /* - 4 */;

    if (nbPtr->pageHeight == 0) {
	bottom = yBot + 2 * nbPtr->corner;
    } else {
	bottom = height - nbPtr->yPad - 1;
    }
    if (tabPtr != nbPtr->selectPtr) {

	/*
	 * Case 1: Unselected tab
	 *
	 * * 3+ . . +4
	 * 2+         +5
	 *  .         .
	 * 1+         +6
	 *   0+ . . +7
	 *
	 */
	
	if (nbPtr->slant & SLANT_LEFT) {
	    NextPoint(x, yBot);
	    NextPoint(x, yTop);
	    NextPoint(x + nbPtr->tabHeight, y);
	} else {
	    BottomLeft(x, yBot);
	    TopLeft(x, y);
	}
	x += tabPtr->worldWidth;
	if (nbPtr->slant & SLANT_RIGHT) {
	    NextPoint(x - nbPtr->tabHeight, y);
	    NextPoint(x, yTop);
	    NextPoint(x, yBot);
	} else {
	    TopRight(x, y);
	    BottomRight(x, yBot);
	}
    } else if (!(tabPtr->flags & TAB_VISIBLE)) {

	/*
	 * Case 2: Selected tab not visible in viewport.  Draw folder only.
	 *
	 * * 3+ . . +4
	 * 2+         +5
	 *  .         .
	 * 1+         +6
	 *   0+------+7
	 *
	 */

	TopLeft(left, top);
	TopRight(right, top);
	BottomRight(right, bottom);
	BottomLeft(left, bottom);
    } else {
	int flags;
	int tabWidth;

	x -= nbPtr->xSelectPad;
	y -= nbPtr->yPad;
	tabWidth = tabPtr->worldWidth + 2 * nbPtr->xSelectPad;

#define CLIP_NONE	0
#define CLIP_LEFT	(1<<0)
#define CLIP_RIGHT	(1<<1)
	flags = 0;
	if (x < left) {
	    flags |= CLIP_LEFT;
	}
	if ((x + tabWidth) > right) {
	    flags |= CLIP_RIGHT;
	}
	switch (flags) {
	case CLIP_NONE:

	    /*
	     *  worldX, worldY
	     *          |
	     *          * 4+ . . +5
	     *          3+         +6
	     *           .         .
	     *           .         .
	     *   1+. . .2+---------+7 . . . .+8
	     * 0+                              +9
	     *  .                              .
	     *  .                              .
	     *13+                              +10
	     *  12+ . . . . . . . . . . . . +11
	     */

	    if (x < (left + nbPtr->corner)) {
		NextPoint(left, top);
	    } else {
		TopLeft(left, top);
	    }
	    if (nbPtr->slant & SLANT_LEFT) {
		NextPoint(x, yTop);
		NextPoint(x + nbPtr->tabHeight + nbPtr->yPad, y);
	    } else {
		NextPoint(x, top);
		TopLeft(x, y);
	    }
	    x += tabWidth;
	    if (nbPtr->slant & SLANT_RIGHT) {
		NextPoint(x - nbPtr->tabHeight - nbPtr->yPad, y);
		NextPoint(x, yTop);
	    } else {
		TopRight(x, y);
		NextPoint(x, top);
	    }
	    if (x > (right - nbPtr->corner)) {
		NextPoint(right, top + nbPtr->corner);
	    } else {
		TopRight(right, top);
	    }
	    BottomRight(right, bottom);
	    BottomLeft(left, bottom);
	    break;

	case CLIP_LEFT:

	    /*
	     *  worldX, worldY
	     *          |
	     *          * 4+ . . +5
	     *          3+         +6
	     *           .         .
	     *           .         .
	     *          2+--------+7 . . . .+8
	     *            1+ . . . +0          +9
	     *                     .           .
	     *                     .           .
	     *                   13+           +10
	     *                     12+ . . . .+11
	     */

	    NextPoint(left, yBot);
	    if (nbPtr->slant & SLANT_LEFT) {
		NextPoint(x, yBot);
		NextPoint(x, yTop);
		NextPoint(x + nbPtr->tabHeight + nbPtr->yPad, y);
	    } else {
		BottomLeft(x, yBot);
		TopLeft(x, y);
	    }

	    x += tabWidth;
	    if (nbPtr->slant & SLANT_RIGHT) {
		NextPoint(x - nbPtr->tabHeight - nbPtr->yPad, y);
		NextPoint(x, yTop);
		NextPoint(x, top);
	    } else {
		TopRight(x, y);
		NextPoint(x, top);
	    }
	    if (x > (right - nbPtr->corner)) {
		NextPoint(right, top + nbPtr->corner);
	    } else {
		TopRight(right, top);
	    }
	    BottomRight(right, bottom);
	    BottomLeft(left, bottom);
	    break;

	case CLIP_RIGHT:

	    /*
	     *              worldX, worldY
	     *                     |
	     *                     * 9+ . . +10
	     *                     8+         +11
	     *                      .         .
	     *                      .         .
	     *           6+ . . . .7+---------+12
	     *         5+          0+ . . . +13
	     *          .           .
	     *          .           .
	     *         4+           +1
	     *           3+ . . . +2
	     */

	    NextPoint(right, yBot);
	    BottomRight(right, bottom);
	    BottomLeft(left, bottom);
	    if (x < (left + nbPtr->corner)) {
		NextPoint(left, top);
	    } else {
		TopLeft(left, top);
	    }
	    NextPoint(x, top);

	    if (nbPtr->slant & SLANT_LEFT) {
		NextPoint(x, yTop);
		NextPoint(x + nbPtr->tabHeight + nbPtr->yPad, y);
	    } else {
		TopLeft(x, y);
	    }
	    x += tabWidth;
	    if (nbPtr->slant & SLANT_RIGHT) {
		NextPoint(x - nbPtr->tabHeight - nbPtr->yPad, y);
		NextPoint(x, yTop);
		NextPoint(x, yBot);
	    } else {
		TopRight(x, y);
		BottomRight(x, yBot);
	    }
	    break;

	case (CLIP_LEFT | CLIP_RIGHT):

	    /*
	     *  worldX, worldY
	     *     |
	     *     * 4+ . . . . . . . . +5
	     *     3+                     +6
	     *      .                     .
	     *      .                     .
	     *     1+---------------------+7
	     *       2+ 0+          +9 .+8
	     *           .          .
	     *           .          .
	     *         13+          +10
	     *          12+ . . . +11
	     */

	    NextPoint(left, yBot);
	    if (nbPtr->slant & SLANT_LEFT) {
		NextPoint(x, yBot);
		NextPoint(x, yTop);
		NextPoint(x + nbPtr->tabHeight + nbPtr->yPad, y);
	    } else {
		BottomLeft(x, yBot);
		TopLeft(x, y);
	    }
	    x += tabPtr->worldWidth;
	    if (nbPtr->slant & SLANT_RIGHT) {
		NextPoint(x - nbPtr->tabHeight - nbPtr->yPad, y);
		NextPoint(x, yTop);
		NextPoint(x, yBot);
	    } else {
		TopRight(x, y);
		BottomRight(x, yBot);
	    }
	    NextPoint(right, yBot);
	    BottomRight(right, bottom);
	    BottomLeft(left, bottom);
	    break;
	}
    }
    EndPoint(pointArr[0].x, pointArr[0].y);
    for (i = 0; i < nPoints; i++) {
	WorldToScreen(nbPtr, pointArr[i].x, pointArr[i].y, &x, &y);
	pointArr[i].x = x;
	pointArr[i].y = y;
    }
    Draw3DFolder(nbPtr, tabPtr, drawable, nbPtr->side, pointArr, nPoints);
    DrawLabel(nbPtr, tabPtr, drawable);
    if (tabPtr->container != NULL) {
	XRectangle rect;

	/* Draw a rectangle covering the spot representing the window  */
	GetWindowRectangle(tabPtr, nbPtr->tkwin, FALSE, &rect);
	XFillRectangles(nbPtr->display, drawable, tabPtr->backGC,
	    &rect, 1);
    }
}

static void
DrawOuterBorders(Notebook *nbPtr, Drawable drawable)
{
    /*
     * Draw 3D border just inside of the focus highlight ring.  We
     * draw the border even if the relief is flat so that any tabs
     * that hang over the edge will be clipped.
     */
    if (nbPtr->borderWidth > 0) {
	Blt_Draw3DRectangle(nbPtr->tkwin, drawable, nbPtr->border,
	    nbPtr->highlightWidth, nbPtr->highlightWidth,
	    Tk_Width(nbPtr->tkwin) - 2 * nbPtr->highlightWidth,
	    Tk_Height(nbPtr->tkwin) - 2 * nbPtr->highlightWidth,
	    nbPtr->borderWidth, nbPtr->relief);
    }
    /* Draw focus highlight ring. */
    if (nbPtr->highlightWidth > 0) {
	XColor *color;
	GC gc;

	color = (nbPtr->flags & TNB_FOCUS)
	    ? nbPtr->highlightColor : nbPtr->highlightBgColor;
	gc = Tk_GCForColor(color, drawable);
	Tk_DrawFocusHighlight(nbPtr->tkwin, gc, nbPtr->highlightWidth, 
	      drawable);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * DisplayNotebook --
 *
 * 	This procedure is invoked to display the widget.
 *
 *      Recomputes the layout of the widget if necessary. This is
 *	necessary if the world coordinate system has changed.
 *	Sets the vertical and horizontal scrollbars.  This is done
 *	here since the window width and height are needed for the
 *	scrollbar calculations.
 *
 * Results:
 *	None.
 *
 * Side effects:
 * 	The widget is redisplayed.
 *
 *---------------------------------------------------------------------------
 */
static void
DisplayNotebook(ClientData clientData) /* Information about widget. */
{
    Notebook *nbPtr = clientData;
    Pixmap drawable;
    int width, height;

    nbPtr->flags &= ~TNB_REDRAW;
    if (nbPtr->tkwin == NULL) {
	return;			/* Window has been destroyed. */
    }
    if (nbPtr->flags & TNB_LAYOUT) {
	ComputeLayout(nbPtr);
	nbPtr->flags &= ~TNB_LAYOUT;
    }
    if ((nbPtr->reqHeight == 0) || (nbPtr->reqWidth == 0)) {
	width = height = 0;
	if (nbPtr->side & TAB_VERTICAL) {
	    height = nbPtr->worldWidth;
	} else {
	    width = nbPtr->worldWidth;
	}
	if (nbPtr->reqWidth > 0) {
	    width = nbPtr->reqWidth;
	} else if (nbPtr->pageWidth > 0) {
	    width = nbPtr->pageWidth;
	}
	if (nbPtr->reqHeight > 0) {
	    height = nbPtr->reqHeight;
	} else if (nbPtr->pageHeight > 0) {
	    height = nbPtr->pageHeight;
	}
	if (nbPtr->side & TAB_VERTICAL) {
	    width += nbPtr->pageTop + nbPtr->inset + nbPtr->inset2;
	    height += nbPtr->inset + nbPtr->inset2;
	} else {
	    height += nbPtr->pageTop + nbPtr->inset + nbPtr->inset2;
	    width += nbPtr->inset + nbPtr->inset2;
	}
	if ((Tk_ReqWidth(nbPtr->tkwin) != width) ||
	    (Tk_ReqHeight(nbPtr->tkwin) != height)) {
	    Tk_GeometryRequest(nbPtr->tkwin, width, height);
	}
    }
    if (nbPtr->flags & TNB_SCROLL) {
	width = VPORTWIDTH(nbPtr);
	nbPtr->scrollOffset = Blt_AdjustViewport(nbPtr->scrollOffset,
	    nbPtr->worldWidth, width, nbPtr->scrollUnits, 
	    BLT_SCROLL_MODE_CANVAS);
	if (nbPtr->scrollCmdObjPtr != NULL) {
	    Blt_UpdateScrollbar(nbPtr->interp, nbPtr->scrollCmdObjPtr,
		(double)nbPtr->scrollOffset / nbPtr->worldWidth,
		(double)(nbPtr->scrollOffset + width) / nbPtr->worldWidth);
	}
	ComputeVisibleTabs(nbPtr);
	nbPtr->flags &= ~TNB_SCROLL;
    }
    if (!Tk_IsMapped(nbPtr->tkwin)) {
	return;
    }
    height = Tk_Height(nbPtr->tkwin);
    drawable = Tk_GetPixmap(nbPtr->display, Tk_WindowId(nbPtr->tkwin),
	Tk_Width(nbPtr->tkwin), Tk_Height(nbPtr->tkwin),
	Tk_Depth(nbPtr->tkwin));
    /*
     * Clear the background either by tiling a pixmap or filling with
     * a solid color. Tiling takes precedence.
     */
    if (nbPtr->tile != NULL) {
	Blt_SetTileOrigin(nbPtr->tkwin, nbPtr->tile, 0, 0);
	Blt_TileRectangle(nbPtr->tkwin, drawable, nbPtr->tile, 0, 0,
	    Tk_Width(nbPtr->tkwin), height);
    } else {
	Blt_Fill3DRectangle(nbPtr->tkwin, drawable, nbPtr->border, 0, 0,
	    Tk_Width(nbPtr->tkwin), height, 0, TK_RELIEF_FLAT);
    }

    if (nbPtr->nVisible > 0) {
	int i;
	Tab *tabPtr;
	Blt_ChainLink link;

	link = nbPtr->startPtr->link;
	for (i = 0; i < Blt_ChainGetLength(nbPtr->chain); i++) {
	    link = Blt_ChainPrevLink(link);
	    if (link == NULL) {
		link = Blt_ChainLastLink(nbPtr->chain);
	    }
	    tabPtr = Blt_ChainGetValue(link);
	    if ((tabPtr != nbPtr->selectPtr) &&
		(tabPtr->flags & TAB_VISIBLE)) {
		DrawFolder(nbPtr, tabPtr, drawable);
	    }
	}
	DrawFolder(nbPtr, nbPtr->selectPtr, drawable);
	if (nbPtr->tearoff) {
	    DrawPerforation(nbPtr, nbPtr->selectPtr, drawable);
	}

	if ((nbPtr->selectPtr->tkwin != NULL) &&
	    (nbPtr->selectPtr->container == NULL)) {
	    XRectangle rect;

	    GetWindowRectangle(nbPtr->selectPtr, nbPtr->tkwin, FALSE, &rect);
	    ArrangeWindow(nbPtr->selectPtr->tkwin, &rect, 0);
	}
    }
    DrawOuterBorders(nbPtr, drawable);
    XCopyArea(nbPtr->display, drawable, Tk_WindowId(nbPtr->tkwin),
	nbPtr->highlightGC, 0, 0, Tk_Width(nbPtr->tkwin), height, 0, 0);
    Tk_FreePixmap(nbPtr->display, drawable);
}

/*
 * From the left edge:
 *
 *   |a|b|c|d|e| f |d|e|g|h| i |h|g|e|d|f|    j    |e|d|c|b|a|
 *
 *	a. highlight ring
 *	b. notebook 3D border
 *	c. outer gap
 *      d. page border
 *	e. page corner
 *	f. gap + select pad
 *	g. label pad x (worldX)
 *	h. internal pad x
 *	i. label width
 *	j. rest of page width
 *
 *  worldX, worldY
 *          |
 *          |
 *          * 4+ . . +5
 *          3+         +6
 *           .         .
 *           .         .
 *   1+. . .2+         +7 . . . .+8
 * 0+                              +9
 *  .                              .
 *  .                              .
 *13+                              +10
 *  12+-------------------------+11
 *
 */
static void
DisplayTearoff(ClientData clientData)
{
    Notebook *nbPtr;
    Tab *tabPtr;
    Drawable drawable;
    XPoint pointArr[16];
    XPoint *pointPtr;
    int width, height;
    int left, bottom, right, top;
    int x, y;
    int nPoints;
    Tk_Window tkwin;
    Tk_Window parent;
    XRectangle rect;

    tabPtr = clientData;
    if (tabPtr == NULL) {
	return;
    }
    tabPtr->flags &= ~TAB_REDRAW;
    nbPtr = tabPtr->nbPtr;
    if (nbPtr->tkwin == NULL) {
	return;
    }
    tkwin = tabPtr->container;
    drawable = Tk_WindowId(tkwin);
    /*
     * Clear the background either by tiling a pixmap or filling with
     * a solid color. Tiling takes precedence.
     */
    if (nbPtr->tile != NULL) {
	Blt_SetTileOrigin(tkwin, nbPtr->tile, 0, 0);
	Blt_TileRectangle(tkwin, drawable, nbPtr->tile, 0, 0, Tk_Width(tkwin), 
		Tk_Height(tkwin));
    } else {
	Blt_Fill3DRectangle(tkwin, drawable, nbPtr->border, 0, 0, 
		Tk_Width(tkwin), Tk_Height(tkwin), 0, TK_RELIEF_FLAT);
    }

    width = Tk_Width(tkwin) - 2 * nbPtr->inset;
    height = Tk_Height(tkwin) - 2 * nbPtr->inset;
    x = nbPtr->inset + nbPtr->gap + nbPtr->corner;
    y = nbPtr->inset;

    left = nbPtr->inset;
    right = nbPtr->inset + width;
    top = nbPtr->inset + nbPtr->corner + nbPtr->xSelectPad;
    bottom = nbPtr->inset + height;

    /*
     *  worldX, worldY
     *          |
     *          * 4+ . . +5
     *          3+         +6
     *           .         .
     *           .         .
     *   1+. . .2+         +7 . . . .+8
     * 0+                              +9
     *  .                              .
     *  .                              .
     *13+                              +10
     *  12+-------------------------+11
     */

    nPoints = 0;
    pointPtr = pointArr;

    TopLeft(left, top);
    NextPoint(x, top);
    TopLeft(x, y);
    x += tabPtr->worldWidth;
    TopRight(x, y);
    NextPoint(x, top);
    TopRight(right, top);
    BottomRight(right, bottom);
    BottomLeft(left, bottom);
    EndPoint(pointArr[0].x, pointArr[0].y);
    Draw3DFolder(nbPtr, tabPtr, drawable, SIDE_TOP, pointArr, nPoints);

    parent = (tabPtr->container == NULL) ? nbPtr->tkwin : tabPtr->container;
    GetWindowRectangle(tabPtr, parent, TRUE, &rect);
    ArrangeWindow(tabPtr->tkwin, &rect, TRUE);

    /* Draw 3D border. */
    if ((nbPtr->borderWidth > 0) && (nbPtr->relief != TK_RELIEF_FLAT)) {
	Blt_Draw3DRectangle(tkwin, drawable, nbPtr->border, 0, 0,
	    Tk_Width(tkwin), Tk_Height(tkwin), nbPtr->borderWidth,
	    nbPtr->relief);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * NotebookCmd --
 *
 * 	This procedure is invoked to process the "notebook" command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *---------------------------------------------------------------------------
 */
static Blt_OpSpec notebookOps[] =
{
    {"activate", 1, ActivateOp, 3, 3, "index",},
    {"bind", 1, BindOp, 2, 5, "index ?sequence command?",},
    {"cget", 2, CgetOp, 3, 3, "option",},
    {"configure", 2, ConfigureOp, 2, 0, "?option value?...",},
    {"delete", 1, DeleteOp, 2, 0, "first ?last?",},
    {"focus", 1, FocusOp, 3, 3, "index",},
    {"highlight", 1, ActivateOp, 3, 3, "index",},
    {"id", 2, IdOp, 3, 3, "index",},
    {"index", 3, IndexOp, 3, 5, "string",},
    {"insert", 3, InsertOp, 3, 0, "index ?option value?",},
    {"invoke", 3, InvokeOp, 3, 3, "index",},
    {"move", 1, MoveOp, 5, 5, "name after|before index",},
    {"nearest", 1, NearestOp, 4, 4, "x y",},
    {"perforation", 1, PerforationOp, 2, 0, "args",},
    {"scan", 2, ScanOp, 5, 5, "dragto|mark x y",},
    {"see", 3, SeeOp, 3, 3, "index",},
    {"select", 3, SelectOp, 3, 3, "index",},
    {"size", 2, SizeOp, 2, 2, "",},
    {"tab", 1, TabOp, 2, 0, "oper args",},
    {"view", 1, ViewOp, 2, 5,
	"?moveto fract? ?scroll number what?",},
};

static int nNotebookOps = sizeof(notebookOps) / sizeof(Blt_OpSpec);

static int
NotebookInstCmd(
    ClientData clientData,	/* Information about the widget. */
    Tcl_Interp *interp,		/* Interpreter to report errors back to. */
    int objc,			/* Number of arguments. */
    Tcl_Obj *const *objv)	/* Vector of argument strings. */
{
    NotebookCmdProc *proc;
    Notebook *nbPtr = clientData;
    int result;

    proc = Blt_GetOpFromObj(interp, nNotebookOps, notebookOps, BLT_OP_ARG1, 
	objc, objv, 0);
    if (proc == NULL) {
	return TCL_ERROR;
    }
    Tcl_Preserve(nbPtr);
    result = (*proc) (nbPtr, interp, objc, objv);
    Tcl_Release(nbPtr);
    return result;
}

/*
 *---------------------------------------------------------------------------
 *
 * NotebookInstDeletedCmd --
 *
 *	This procedure can be called if the window was destroyed
 *	(tkwin will be NULL) and the command was deleted
 *	automatically.  In this case, we need to do nothing.
 *
 *	Otherwise this routine was called because the command was
 *	deleted.  Then we need to clean-up and destroy the widget.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The widget is destroyed.
 *
 *---------------------------------------------------------------------------
 */
static void
NotebookInstDeletedCmd(ClientData clientData)
{
    Notebook *nbPtr = clientData;

    if (nbPtr->tkwin != NULL) {
	Tk_Window tkwin;

	tkwin = nbPtr->tkwin;
	nbPtr->tkwin = NULL;
	Tk_DestroyWindow(tkwin);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * NotebookCmd --
 *
 * 	This procedure is invoked to process the Tcl command that
 * 	corresponds to a widget managed by this module. See the user
 * 	documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side Effects:
 *	See the user documentation.
 *
 *---------------------------------------------------------------------------
 */
/* ARGSUSED */
static int
NotebookCmd(
    ClientData clientData,	/* Main window associated with interpreter. */
    Tcl_Interp *interp,		/* Current interpreter. */
    int objc,			/* Number of arguments. */
    Tcl_Obj *const *objv)	/* Argument strings. */
{
    Notebook *nbPtr;
    Tk_Window tkwin;
    unsigned int mask;
    Tcl_CmdInfo cmdInfo;
    
    if (objc < 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", 
		Tcl_GetString(objv[0]), " pathName ?option value?...\"", 
		(char *)NULL);
	return TCL_ERROR;
    }
    tkwin = Tk_CreateWindowFromPath(interp, Tk_MainWindow(interp), 
	    Tcl_GetString(objv[1]), (char *)NULL);
    if (tkwin == NULL) {
	return TCL_ERROR;
    }
    nbPtr = CreateNotebook(interp, tkwin);
    if (ConfigureNotebook(interp, nbPtr, objc - 2, objv + 2, 0) != TCL_OK) {
	Tk_DestroyWindow(nbPtr->tkwin);
	return TCL_ERROR;
    }
    mask = (ExposureMask | StructureNotifyMask | FocusChangeMask);
    Tk_CreateEventHandler(tkwin, mask, NotebookEventProc, nbPtr);
    nbPtr->cmdToken = Tcl_CreateObjCommand(interp, Tcl_GetString(objv[1]), 
	NotebookInstCmd, nbPtr, NotebookInstDeletedCmd);

    /*
     * Try to invoke a procedure to initialize various bindings on
     * tabs.  Source the file containing the procedure now if the
     * procedure isn't currently defined.  We deferred this to now so
     * that the user could set the variable "blt_library" within the
     * script.
     */
    if (!Tcl_GetCommandInfo(interp, "::blt::TabnotebookInit", &cmdInfo)) {
	static char initCmd[] = 
	    "source [file join $blt_library tabnotebook.tcl]";

	if (Tcl_GlobalEval(interp, initCmd) != TCL_OK) {
	    char info[200];

	    sprintf_s(info, 200, "\n    (while loading bindings for %s)", 
		Tcl_GetString(objv[0]));
	    Tcl_AddErrorInfo(interp, info);
	    Tk_DestroyWindow(nbPtr->tkwin);
	    return TCL_ERROR;
	}
    }
    if (Tcl_VarEval(interp, "::blt::TabnotebookInit ", 
		    Tk_PathName(nbPtr->tkwin), (char *)NULL) != TCL_OK) {
	Tk_DestroyWindow(nbPtr->tkwin);
	return TCL_ERROR;
    }
    Tcl_SetObjResult(interp, objv[1]);
    return TCL_OK;
}

int
Blt_TabnotebookCmdInitProc(Tcl_Interp *interp)
{
    static Blt_InitCmdSpec cmdSpec = { "tabnotebook", NotebookCmd, };

    return Blt_InitCmd(interp, "::blt", &cmdSpec);
}

#endif /* NO_TABNOTEBOOK */
