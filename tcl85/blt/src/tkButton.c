/*
 * tkButton.c --
 *
 *	This module implements a collection of button-like
 *	widgets for the Tk toolkit.  The widgets implemented
 *	include labels, buttons, check buttons, and radio
 *	buttons.
 *
 * Copyright (c) 1990-1994 The Regents of the University of California.
 * Copyright (c) 1994-1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * SCCS: @(#) tkButton.c 1.128 96/03/01 17:34:49
 */

#include "bltInt.h"

#ifndef NO_TILEBUTTON

#include "bltTile.h"
#include "bltFont.h"
#include "bltText.h"

/*
 * The definitions below provide symbolic names for the default colors.
 * NORMAL_BG -		Normal background color.
 * ACTIVE_BG -		Background color when widget is active.
 * SELECT_BG -		Background color for selected text.
 * TROUGH -		Background color for troughs in scales and scrollbars.
 * INDICATOR -		Color for indicator when button is selected.
 * DISABLED -		Foreground color when widget is disabled.
 */

#define NORMAL_BG	"#d9d9d9"
#define ACTIVE_BG	"#ececec"
#define SELECT_BG	"#c3c3c3"
#define TROUGH		"#c3c3c3"
#define INDICATOR	"#b03060"
#define DISABLED	"#a3a3a3"

#define DEF_BUTTON_ANCHOR		"center"
#define DEF_BUTTON_ACTIVE_BACKGROUND	STD_ACTIVE_BACKGROUND
#define DEF_BUTTON_ACTIVE_BG_MONO	RGB_BLACK
#define DEF_BUTTON_ACTIVE_FOREGROUND	RGB_BLACK
#define DEF_BUTTON_ACTIVE_FG_MONO	RGB_WHITE
#define DEF_BUTTON_BACKGROUND		STD_NORMAL_BACKGROUND
#define DEF_BUTTON_BG_MONO		RGB_WHITE
#define DEF_BUTTON_BITMAP		""
#define DEF_BUTTON_BORDERWIDTH		"2"
#define DEF_BUTTON_CURSOR		""
#define DEF_BUTTON_COMMAND		""
#define DEF_BUTTON_COMPOUND		"none"
#define DEF_BUTTON_DEFAULT              "disabled"
#define DEF_BUTTON_DISABLED_FOREGROUND	STD_DISABLED_FOREGROUND
#define DEF_BUTTON_DISABLED_FG_MONO	""
#define DEF_BUTTON_FG			RGB_BLACK
#define DEF_BUTTON_FONT			STD_FONT
#define DEF_BUTTON_HEIGHT		"0"
#define DEF_BUTTON_HIGHLIGHT_BG		STD_NORMAL_BACKGROUND
#define DEF_BUTTON_HIGHLIGHT		RGB_BLACK
#define DEF_LABEL_HIGHLIGHT_WIDTH	"0"
#define DEF_BUTTON_HIGHLIGHT_WIDTH	"2"
#define DEF_BUTTON_IMAGE		(char *) NULL
#define DEF_BUTTON_INDICATOR		"1"
#define DEF_BUTTON_JUSTIFY		"center"
#define DEF_BUTTON_OFF_VALUE		"0"
#define DEF_BUTTON_ON_VALUE		"1"
#define DEF_BUTTON_OVER_RELIEF		"raised"
#define DEF_BUTTON_PADX			"3m"
#define DEF_LABCHKRAD_PADX		"1"
#define DEF_BUTTON_PADY			"1m"
#define DEF_LABCHKRAD_PADY		"1"
#define DEF_BUTTON_RELIEF		"raised"
#define DEF_BUTTON_REPEAT_DELAY		"0"
#define DEF_LABCHKRAD_RELIEF		"flat"
#define DEF_BUTTON_SELECT_COLOR		STD_INDICATOR_COLOR
#define DEF_BUTTON_SELECT_MONO		RGB_BLACK
#define DEF_BUTTON_SELECT_IMAGE		(char *) NULL
#define DEF_BUTTON_STATE		"normal"
#define DEF_LABEL_TAKE_FOCUS		"0"
#define DEF_BUTTON_TAKE_FOCUS		(char *) NULL
#define DEF_BUTTON_TEXT			""
#define DEF_BUTTON_TEXT_VARIABLE	""
#define DEF_BUTTON_UNDERLINE		"-1"
#define DEF_BUTTON_VALUE		""
#define DEF_BUTTON_WIDTH		"0"
#define DEF_BUTTON_WRAP_LENGTH		"0"
#define DEF_RADIOBUTTON_VARIABLE	"selectedButton"
#define DEF_CHECKBUTTON_VARIABLE	""

/*
 * A data structure of the following type is kept for each
 * widget managed by this file:
 */

typedef struct {
    Tk_Window tkwin;		/* Window that embodies the button.  NULL
				 * means that the window has been destroyed. */
    Display *display;		/* Display containing widget.  Needed to
				 * free up resources after tkwin is gone. */
    Tcl_Interp *interp;		/* Interpreter associated with button. */
    Tcl_Command widgetCmd;	/* Token for button's widget command. */
    int type;			/* Type of widget:  restricts operations
				 * that may be performed on widget.  See
				 * below for possible values. */

    /*
     * Information about what's in the button.
     */

    const char *text;		/* Text to display in button (malloc'ed)
				 * or NULL. */
    int numChars;		/* # of characters in text. */
    int underline;		/* Index of character to underline.  < 0 means
				 * don't underline anything. */
    const char *textVarName;	/* Name of variable (malloc'ed) or NULL.
				 * If non-NULL, button displays the contents
				 * of this variable. */
    Pixmap bitmap;		/* Bitmap to display or None.  If not None
				 * then text and textVar are ignored. */
    const char *imageString;	/* Name of image to display (malloc'ed), or
				 * NULL.  If non-NULL, bitmap, text, and
				 * textVarName are ignored. */
    Tk_Image image;		/* Image to display in window, or NULL if
				 * none. */
    const char *selectImageString; /* Name of image to display when selected
				 * (malloc'ed), or NULL. */
    Tk_Image selectImage;	/* Image to display in window when selected,
				 * or NULL if none.  Ignored if image is
				 * NULL. */

    /*
     * Information used when displaying widget:
     */

    int state;			/* State of button for display purposes:
				 * normal, active, or disabled. */
    Tk_3DBorder normalBorder;	/* Structure used to draw 3-D
				 * border and background when window
				 * isn't active.  NULL means no such
				 * border exists. */
    Tk_3DBorder activeBorder;	/* Structure used to draw 3-D
				 * border and background when window
				 * is active.  NULL means no such
				 * border exists. */
    int borderWidth;		/* Width of border. */
    int relief;			/* 3-d effect: TK_RELIEF_RAISED, etc. */
    int overRelief;		/* Value of -overrelief option: specifies a 3-d
				 * effect for the border, such as
				 * TK_RELIEF_RAISED, to be used when the mouse
				 * is over the button. */
    int highlightWidth;		/* Width in pixels of highlight to draw
				 * around widget when it has the focus.
				 * <= 0 means don't draw a highlight. */
    XColor *highlightBgColorPtr;
    /* Color for drawing traversal highlight
				 * area when highlight is off. */
    XColor *highlightColorPtr;	/* Color for drawing traversal highlight. */
    int inset;			/* Total width of all borders, including
				 * traversal highlight and 3-D border.
				 * Indicates how much interior stuff must
				 * be offset from outside edges to leave
				 * room for borders. */
    Blt_Font font;		/* Information about text font, or NULL. */
    XColor *normalFg;		/* Foreground color in normal mode. */
    XColor *activeFg;		/* Foreground color in active mode.  NULL
				 * means use normalFg instead. */
    XColor *disabledFg;		/* Foreground color when disabled.  NULL
				 * means use normalFg with a 50% stipple
				 * instead. */
    GC normalTextGC;		/* GC for drawing text in normal mode.  Also
				 * used to copy from off-screen pixmap onto
				 * screen. */
    GC activeTextGC;		/* GC for drawing text in active mode (NULL
				 * means use normalTextGC). */
    Pixmap gray;		/* Pixmap for displaying disabled text if
				 * disabledFg is NULL. */
    GC disabledGC;		/* Used to produce disabled effect.  If
				 * disabledFg isn't NULL, this GC is used to
				 * draw button text or icon.  Otherwise
				 * text or icon is drawn with normalGC and
				 * this GC is used to stipple background
				 * across it.  For labels this is None. */
    GC copyGC;			/* Used for copying information from an
				 * off-screen pixmap to the screen. */
    const char *widthString;	/* Value of -width option.  Malloc'ed. */
    const char *heightString;	/* Value of -height option.  Malloc'ed. */
    int width, height;		/* If > 0, these specify dimensions to request
				 * for window, in characters for text and in
				 * pixels for bitmaps.  In this case the actual
				 * size of the text string or bitmap is
				 * ignored in computing desired window size. */
    int wrapLength;		/* Line length (in pixels) at which to wrap
				 * onto next line.  <= 0 means don't wrap
				 * except at newlines. */
    int padX, padY;		/* Extra space around text (pixels to leave
				 * on each side).  Ignored for bitmaps and
				 * images. */
    Tk_Anchor anchor;		/* Where text/bitmap should be displayed
				 * inside button region. */
    Tk_Justify justify;		/* Justification to use for multi-line text. */
    int indicatorOn;		/* True means draw indicator, false means
				 * don't draw it. */
    Tk_3DBorder selectBorder;	/* For drawing indicator background, or perhaps
				 * widget background, when selected. */
    int textWidth;		/* Width needed to display text as requested,
				 * in pixels. */
    int textHeight;		/* Height needed to display text as requested,
				 * in pixels. */
    Tk_TextLayout textLayout;	/* Saved text layout information. */
    int indicatorSpace;		/* Horizontal space (in pixels) allocated for
				 * display of indicator. */
    int indicatorDiameter;	/* Diameter of indicator, in pixels. */

    int defaultState;		/* Used in 8.0 (not here)  */

    /*
     * For check and radio buttons, the fields below are used
     * to manage the variable indicating the button's state.
     */

    const char *selVarName;	/* Name of variable used to control selected
				 * state of button.  Malloc'ed (if
				 * not NULL). */
    const char *onValue;	/* Value to store in variable when
				 * this button is selected.  Malloc'ed (if
				 * not NULL). */
    const char *offValue;	/* Value to store in variable when this
				 * button isn't selected.  Malloc'ed
				 * (if not NULL).  Valid only for check
				 * buttons. */

    /*
     * Miscellaneous information:
     */

    Tk_Cursor cursor;		/* Current cursor for window, or None. */
    const char *takeFocus;	/* Value of -takefocus option;  not used in
				 * the C code, but used by keyboard traversal
				 * scripts.  Malloc'ed, but may be NULL. */
    const char *command;	/* Command to execute when button is
				 * invoked; valid for buttons only.
				 * If not NULL, it's malloc-ed. */
    const char *compound;      /* Value of -compound option; specifies whether
				 * the button should show both an image and
				 * text, and, if so, how. */
    int repeatDelay;		/* Value of -repeatdelay option; specifies
				 * the number of ms after which the button will
				 * start to auto-repeat its command. */
    int repeatInterval;		/* Value of -repeatinterval option; specifies
				 * the number of ms between auto-repeat
				 * invocataions of the button command. */
    int flags;			/* Various flags;  see below for
				 * definitions. */
    Blt_Tile tile, activeTile;
} Button;

/*
 * Possible "type" values for buttons.  These are the kinds of
 * widgets supported by this file.  The ordering of the type
 * numbers is significant:  greater means more features and is
 * used in the code.
 */

#define TYPE_LABEL		0
#define TYPE_BUTTON		1
#define TYPE_CHECK_BUTTON	2
#define TYPE_RADIO_BUTTON	3

/*
 * Class names for buttons, indexed by one of the type values above.
 */

static const char *classNames[] = {
    "Label", "Button", "Checkbutton", "Radiobutton"
};

/*
 * Flag bits for buttons:
 *
 * REDRAW_PENDING:		Non-zero means a DoWhenIdle handler
 *				has already been queued to redraw
 *				this window.
 * SELECTED:			Non-zero means this button is selected,
 *				so special highlight should be drawn.
 * GOT_FOCUS:			Non-zero means this button currently
 *				has the input focus.
 */

#define REDRAW_PENDING		1
#define SELECTED		2
#define GOT_FOCUS		4

/*
 * Mask values used to selectively enable entries in the
 * configuration specs:
 */

#define LABEL_MASK		BLT_CONFIG_USER_BIT
#define BUTTON_MASK		BLT_CONFIG_USER_BIT << 1
#define CHECK_BUTTON_MASK	BLT_CONFIG_USER_BIT << 2
#define RADIO_BUTTON_MASK	BLT_CONFIG_USER_BIT << 3
#define ALL_MASK		(LABEL_MASK | BUTTON_MASK \
	| CHECK_BUTTON_MASK | RADIO_BUTTON_MASK)

static int configFlags[] =
{LABEL_MASK, BUTTON_MASK,
    CHECK_BUTTON_MASK, RADIO_BUTTON_MASK};

/*
 * Information used for parsing configuration specs:
 */

static Blt_ConfigSpec configSpecs[] =
{
    {BLT_CONFIG_BORDER, "-activebackground", "activeBackground", "Foreground",
	DEF_BUTTON_ACTIVE_BACKGROUND, Blt_Offset(Button, activeBorder),
	BUTTON_MASK | CHECK_BUTTON_MASK | RADIO_BUTTON_MASK | 
        BLT_CONFIG_COLOR_ONLY},
    {BLT_CONFIG_BORDER, "-activebackground", "activeBackground", "Foreground",
	DEF_BUTTON_ACTIVE_BG_MONO, Blt_Offset(Button, activeBorder),
	BUTTON_MASK | CHECK_BUTTON_MASK | RADIO_BUTTON_MASK
	| BLT_CONFIG_MONO_ONLY},
    {BLT_CONFIG_COLOR, "-activeforeground", "activeForeground", "Background",
	DEF_BUTTON_ACTIVE_FOREGROUND, Blt_Offset(Button, activeFg),
	BUTTON_MASK | CHECK_BUTTON_MASK | RADIO_BUTTON_MASK
	| BLT_CONFIG_COLOR_ONLY},
    {BLT_CONFIG_COLOR, "-activeforeground", "activeForeground", "Background",
	DEF_BUTTON_ACTIVE_FG_MONO, Blt_Offset(Button, activeFg),
	BUTTON_MASK | CHECK_BUTTON_MASK | RADIO_BUTTON_MASK
	| BLT_CONFIG_MONO_ONLY},
    {BLT_CONFIG_TILE, "-activetile", "activeTile", "Tile", (char *)NULL, 
	Blt_Offset(Button, activeTile), ALL_MASK | BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_ANCHOR, "-anchor", "anchor", "Anchor",
	DEF_BUTTON_ANCHOR, Blt_Offset(Button, anchor), ALL_MASK},
    {BLT_CONFIG_BORDER, "-background", "background", "Background",
	DEF_BUTTON_BACKGROUND, Blt_Offset(Button, normalBorder),
	ALL_MASK | BLT_CONFIG_COLOR_ONLY},
    {BLT_CONFIG_BORDER, "-background", "background", "Background",
	DEF_BUTTON_BG_MONO, Blt_Offset(Button, normalBorder),
	ALL_MASK | BLT_CONFIG_MONO_ONLY},
    {BLT_CONFIG_SYNONYM, "-bd", "borderWidth", (char *)NULL,
	(char *)NULL, 0, ALL_MASK},
    {BLT_CONFIG_SYNONYM, "-bg", "background", (char *)NULL,
	(char *)NULL, 0, ALL_MASK},
    {BLT_CONFIG_BITMAP, "-bitmap", "bitmap", "Bitmap",
	DEF_BUTTON_BITMAP, Blt_Offset(Button, bitmap),
	ALL_MASK | BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_PIXELS_NNEG, "-borderwidth", "borderWidth", "BorderWidth",
	DEF_BUTTON_BORDERWIDTH, Blt_Offset(Button, borderWidth), ALL_MASK},
    {BLT_CONFIG_STRING, "-command", "command", "Command",
	DEF_BUTTON_COMMAND, Blt_Offset(Button, command),
	BUTTON_MASK | CHECK_BUTTON_MASK | RADIO_BUTTON_MASK | BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_STRING, "-compound", "compound", "Compound",
	DEF_BUTTON_COMPOUND, Blt_Offset(Button, compound), 
	ALL_MASK | BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_ACTIVE_CURSOR, "-cursor", "cursor", "Cursor",
	DEF_BUTTON_CURSOR, Blt_Offset(Button, cursor),
	ALL_MASK | BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_STATE, "-default", "default", "Default",
	DEF_BUTTON_DEFAULT, Blt_Offset(Button, defaultState), BUTTON_MASK},
    {BLT_CONFIG_COLOR, "-disabledforeground", "disabledForeground",
	"DisabledForeground", DEF_BUTTON_DISABLED_FOREGROUND,
	Blt_Offset(Button, disabledFg), BUTTON_MASK | CHECK_BUTTON_MASK
	| RADIO_BUTTON_MASK | BLT_CONFIG_COLOR_ONLY | BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_COLOR, "-disabledforeground", "disabledForeground",
	"DisabledForeground", DEF_BUTTON_DISABLED_FG_MONO,
	Blt_Offset(Button, disabledFg), BUTTON_MASK | CHECK_BUTTON_MASK
	| RADIO_BUTTON_MASK | BLT_CONFIG_MONO_ONLY | BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_SYNONYM, "-fg", "foreground", (char *)NULL,
	(char *)NULL, 0, ALL_MASK},
    {BLT_CONFIG_FONT, "-font", "font", "Font", DEF_BUTTON_FONT, 
	Blt_Offset(Button, font), ALL_MASK},
    {BLT_CONFIG_COLOR, "-foreground", "foreground", "Foreground",
	DEF_BUTTON_FG, Blt_Offset(Button, normalFg), ALL_MASK},
    {BLT_CONFIG_STRING, "-height", "height", "Height",
	DEF_BUTTON_HEIGHT, Blt_Offset(Button, heightString), ALL_MASK},
    {BLT_CONFIG_COLOR, "-highlightbackground", "highlightBackground",
	"HighlightBackground", DEF_BUTTON_HIGHLIGHT_BG,
	Blt_Offset(Button, highlightBgColorPtr), ALL_MASK},
    {BLT_CONFIG_COLOR, "-highlightcolor", "highlightColor", "HighlightColor",
	DEF_BUTTON_HIGHLIGHT, Blt_Offset(Button, highlightColorPtr),
	ALL_MASK},
    {BLT_CONFIG_PIXELS_NNEG, "-highlightthickness", "highlightThickness",
	"HighlightThickness",
	DEF_LABEL_HIGHLIGHT_WIDTH, Blt_Offset(Button, highlightWidth),
	LABEL_MASK},
    {BLT_CONFIG_PIXELS_NNEG, "-highlightthickness", "highlightThickness",
	"HighlightThickness",
	DEF_BUTTON_HIGHLIGHT_WIDTH, Blt_Offset(Button, highlightWidth),
	BUTTON_MASK | CHECK_BUTTON_MASK | RADIO_BUTTON_MASK},
    {BLT_CONFIG_STRING, "-image", "image", "Image",
	DEF_BUTTON_IMAGE, Blt_Offset(Button, imageString),
	ALL_MASK | BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_BOOLEAN, "-indicatoron", "indicatorOn", "IndicatorOn",
	DEF_BUTTON_INDICATOR, Blt_Offset(Button, indicatorOn),
	CHECK_BUTTON_MASK | RADIO_BUTTON_MASK},
    {BLT_CONFIG_JUSTIFY, "-justify", "justify", "Justify",
	DEF_BUTTON_JUSTIFY, Blt_Offset(Button, justify), ALL_MASK},
    {BLT_CONFIG_STRING, "-offvalue", "offValue", "Value",
	DEF_BUTTON_OFF_VALUE, Blt_Offset(Button, offValue),
	CHECK_BUTTON_MASK},
    {BLT_CONFIG_STRING, "-onvalue", "onValue", "Value",
	DEF_BUTTON_ON_VALUE, Blt_Offset(Button, onValue),
	CHECK_BUTTON_MASK},
    {BLT_CONFIG_RELIEF, "-overrelief", "overRelief", "OverRelief",
	DEF_BUTTON_OVER_RELIEF, Blt_Offset(Button, overRelief),
	BUTTON_MASK | CHECK_BUTTON_MASK | RADIO_BUTTON_MASK | BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_PIXELS_NNEG, "-padx", "padX", "Pad",
	DEF_BUTTON_PADX, Blt_Offset(Button, padX), BUTTON_MASK},
    {BLT_CONFIG_PIXELS_NNEG, "-padx", "padX", "Pad",
	DEF_LABCHKRAD_PADX, Blt_Offset(Button, padX),
	LABEL_MASK | CHECK_BUTTON_MASK | RADIO_BUTTON_MASK},
    {BLT_CONFIG_PIXELS_NNEG, "-pady", "padY", "Pad",
	DEF_BUTTON_PADY, Blt_Offset(Button, padY), BUTTON_MASK},
    {BLT_CONFIG_PIXELS_NNEG, "-pady", "padY", "Pad",
	DEF_LABCHKRAD_PADY, Blt_Offset(Button, padY),
	LABEL_MASK | CHECK_BUTTON_MASK | RADIO_BUTTON_MASK},
    {BLT_CONFIG_RELIEF, "-relief", "relief", "Relief",
	DEF_BUTTON_RELIEF, Blt_Offset(Button, relief), BUTTON_MASK},
    {BLT_CONFIG_RELIEF, "-relief", "relief", "Relief",
	DEF_LABCHKRAD_RELIEF, Blt_Offset(Button, relief),
	LABEL_MASK | CHECK_BUTTON_MASK | RADIO_BUTTON_MASK},
    {BLT_CONFIG_INT, "-repeatdelay", "repeatDelay", "RepeatDelay",
	DEF_BUTTON_REPEAT_DELAY, Blt_Offset(Button, repeatDelay),
 	BUTTON_MASK | CHECK_BUTTON_MASK | RADIO_BUTTON_MASK},
    {BLT_CONFIG_BORDER, "-selectcolor", "selectColor", "Background",
	DEF_BUTTON_SELECT_COLOR, Blt_Offset(Button, selectBorder),
	CHECK_BUTTON_MASK | RADIO_BUTTON_MASK | BLT_CONFIG_COLOR_ONLY
	| BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_BORDER, "-selectcolor", "selectColor", "Background",
	DEF_BUTTON_SELECT_MONO, Blt_Offset(Button, selectBorder),
	CHECK_BUTTON_MASK | RADIO_BUTTON_MASK | BLT_CONFIG_MONO_ONLY
	| BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_STRING, "-selectimage", "selectImage", "SelectImage",
	DEF_BUTTON_SELECT_IMAGE, Blt_Offset(Button, selectImageString),
	CHECK_BUTTON_MASK | RADIO_BUTTON_MASK | BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_STATE, "-state", "state", "State",
	DEF_BUTTON_STATE, Blt_Offset(Button, state),
	BUTTON_MASK | CHECK_BUTTON_MASK | RADIO_BUTTON_MASK},
    {BLT_CONFIG_STRING, "-takefocus", "takeFocus", "TakeFocus",
	DEF_LABEL_TAKE_FOCUS, Blt_Offset(Button, takeFocus),
	LABEL_MASK | BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_STRING, "-takefocus", "takeFocus", "TakeFocus",
	DEF_BUTTON_TAKE_FOCUS, Blt_Offset(Button, takeFocus),
	BUTTON_MASK | CHECK_BUTTON_MASK | RADIO_BUTTON_MASK | BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_STRING, "-text", "text", "Text",
	DEF_BUTTON_TEXT, Blt_Offset(Button, text), ALL_MASK},
    {BLT_CONFIG_STRING, "-textvariable", "textVariable", "Variable",
	DEF_BUTTON_TEXT_VARIABLE, Blt_Offset(Button, textVarName),
	ALL_MASK | BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_TILE, "-tile", "tile", "Tile", (char *)NULL, 
	Blt_Offset(Button, tile), ALL_MASK | BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_INT, "-underline", "underline", "Underline",
	DEF_BUTTON_UNDERLINE, Blt_Offset(Button, underline), ALL_MASK},
    {BLT_CONFIG_STRING, "-value", "value", "Value",
	DEF_BUTTON_VALUE, Blt_Offset(Button, onValue),
	RADIO_BUTTON_MASK},
    {BLT_CONFIG_STRING, "-variable", "variable", "Variable",
	DEF_RADIOBUTTON_VARIABLE, Blt_Offset(Button, selVarName),
	RADIO_BUTTON_MASK},
    {BLT_CONFIG_STRING, "-variable", "variable", "Variable",
	DEF_CHECKBUTTON_VARIABLE, Blt_Offset(Button, selVarName),
	CHECK_BUTTON_MASK | BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_STRING, "-width", "width", "Width",
	DEF_BUTTON_WIDTH, Blt_Offset(Button, widthString), ALL_MASK},
    {BLT_CONFIG_PIXELS_NNEG, "-wraplength", "wrapLength", "WrapLength",
	DEF_BUTTON_WRAP_LENGTH, Blt_Offset(Button, wrapLength), ALL_MASK},
    {BLT_CONFIG_END, (char *)NULL, (char *)NULL, (char *)NULL,
	(char *)NULL, 0, 0}
};

/*
 * String to print out in error messages, identifying options for
 * widget commands for different types of labels or buttons:
 */

static const char *optionStrings[] =
{
    "cget or configure",
    "cget, configure, flash, or invoke",
    "cget, configure, deselect, flash, invoke, select, or toggle",
    "cget, configure, deselect, flash, invoke, or select"
};

/*
 * Forward declarations for procedures defined later in this file:
 */

static void ButtonCmdDeletedProc _ANSI_ARGS_((
	ClientData clientData));
static int ButtonCreate _ANSI_ARGS_((ClientData clientData,
	Tcl_Interp *interp, int objc, Tcl_Obj *const *objv, int type));
static void ButtonEventProc _ANSI_ARGS_((ClientData clientData,
	XEvent *eventPtr));
static void ButtonImageProc _ANSI_ARGS_((ClientData clientData,
	int x, int y, int width, int height,
	int imgWidth, int imgHeight));
static void ButtonSelectImageProc _ANSI_ARGS_((
	ClientData clientData, int x, int y, int width,
	int height, int imgWidth, int imgHeight));
static char *ButtonTextVarProc _ANSI_ARGS_((ClientData clientData,
	Tcl_Interp *interp, const char *name1, const char *name2,
	int flags));
static char *ButtonVarProc _ANSI_ARGS_((ClientData clientData,
	Tcl_Interp *interp, const char *name1, const char *name2,
	int flags));
static int ButtonWidgetCmd _ANSI_ARGS_((ClientData clientData,
	Tcl_Interp *interp, int objc, Tcl_Obj *const *objv));
static void ComputeButtonGeometry _ANSI_ARGS_((Button *butPtr));
static int ConfigureButton _ANSI_ARGS_((Tcl_Interp *interp,
	Button *butPtr, int objc, Tcl_Obj *const *objv,
	int flags));
static void DestroyButton _ANSI_ARGS_((Button *butPtr));
static void DisplayButton _ANSI_ARGS_((ClientData clientData));
static int InvokeButton _ANSI_ARGS_((Button *butPtr));

static Blt_TileChangedProc TileChangedProc;
static Tcl_ObjCmdProc ButtonCmd, LabelCmd, CheckbuttonCmd, RadiobuttonCmd;

#ifndef USE_TK_STUBS
BLT_EXTERN int TkCopyAndGlobalEval _ANSI_ARGS_((Tcl_Interp *interp, char *script));

BLT_EXTERN void TkComputeAnchor _ANSI_ARGS_((Tk_Anchor anchor, Tk_Window tkwin, 
	int padX, int padY, int innerWidth, int innerHeight, int *xPtr, 
	int *yPtr));
#endif

/*
 *---------------------------------------------------------------------------
 *
 * Tk_ButtonCmd, Tk_CheckbuttonCmd, Tk_LabelCmd, Tk_RadiobuttonCmd --
 *
 *	These procedures are invoked to process the "button", "label",
 *	"radiobutton", and "checkbutton" Tcl commands.  See the
 *	user documentation for details on what they do.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.  These procedures are just wrappers;
 *	they call ButtonCreate to do all of the real work.
 *
 *---------------------------------------------------------------------------
 */

static int
ButtonCmd(
    ClientData clientData,	/* Main window associated with
				 * interpreter. */
    Tcl_Interp *interp,		/* Current interpreter. */
    int objc,			/* Number of arguments. */
    Tcl_Obj *const *objv)	/* Argument strings. */
{
    return ButtonCreate(clientData, interp, objc, objv, TYPE_BUTTON);
}

static int
CheckbuttonCmd(
    ClientData clientData,	/* Main window associated with
				 * interpreter. */
    Tcl_Interp *interp,		/* Current interpreter. */
    int objc,			/* Number of arguments. */
    Tcl_Obj *const *objv)	/* Argument strings. */
{
    return ButtonCreate(clientData, interp, objc, objv, TYPE_CHECK_BUTTON);
}

static int
LabelCmd(
    ClientData clientData,	/* Main window associated with
				 * interpreter. */
    Tcl_Interp *interp,		/* Current interpreter. */
    int objc,			/* Number of arguments. */
    Tcl_Obj *const *objv)	/* Argument strings. */
{
    return ButtonCreate(clientData, interp, objc, objv, TYPE_LABEL);
}

static int
RadiobuttonCmd(
    ClientData clientData,	/* Main window associated with
				 * interpreter. */
    Tcl_Interp *interp,		/* Current interpreter. */
    int objc,			/* Number of arguments. */
    Tcl_Obj *const *objv)	/* Argument strings. */
{
    return ButtonCreate(clientData, interp, objc, objv, TYPE_RADIO_BUTTON);
}

/*
 *---------------------------------------------------------------------------
 *
 * ButtonCreate --
 *
 *	This procedure does all the real work of implementing the
 *	"button", "label", "radiobutton", and "checkbutton" Tcl
 *	commands.  See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *---------------------------------------------------------------------------
 */

/*ARGSUSED*/
static int
ButtonCreate(
    ClientData clientData,	/* Main window associated with
				 * interpreter. */
    Tcl_Interp *interp,		/* Current interpreter. */
    int objc,			/* Number of arguments. */
    Tcl_Obj *const *objv,	/* Argument strings. */
    int type)			/* Type of button to create: TYPE_LABEL,
				 * TYPE_BUTTON, TYPE_CHECK_BUTTON, or
				 * TYPE_RADIO_BUTTON. */
{
    Button *butPtr;
    Tk_Window tkwin;
    char *path;

    if (objc < 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
		Tcl_GetString(objv[0]), " pathName ?options?\"", (char *)NULL);
	return TCL_ERROR;
    }
    /*
     * Create the new window.
     */

    path = Tcl_GetString(objv[1]);
    tkwin = Tk_CreateWindowFromPath(interp, Tk_MainWindow(interp), path, 
	(char *)NULL);
    if (tkwin == NULL) {
	return TCL_ERROR;
    }
    /*
     * Initialize the data structure for the button.
     */

    butPtr = Blt_MallocAssert(sizeof(Button));
    butPtr->tkwin = tkwin;
    butPtr->display = Tk_Display(tkwin);
    butPtr->widgetCmd = Tcl_CreateObjCommand(interp, Tk_PathName(butPtr->tkwin),
	ButtonWidgetCmd, butPtr, ButtonCmdDeletedProc);

    butPtr->interp = interp;
    butPtr->type = type;
    butPtr->text = NULL;
    butPtr->numChars = 0;
    butPtr->underline = -1;
    butPtr->textVarName = NULL;
    butPtr->bitmap = None;
    butPtr->imageString = NULL;
    butPtr->image = NULL;
    butPtr->selectImageString = NULL;
    butPtr->selectImage = NULL;
    butPtr->state = STATE_NORMAL;
    butPtr->normalBorder = NULL;
    butPtr->activeBorder = NULL;
    butPtr->borderWidth = 0;
    butPtr->relief = TK_RELIEF_FLAT;
    butPtr->highlightWidth = 0;
    butPtr->highlightBgColorPtr = NULL;
    butPtr->highlightColorPtr = NULL;
    butPtr->inset = 0;
    butPtr->font = NULL;
    butPtr->normalFg = NULL;
    butPtr->activeFg = NULL;
    butPtr->disabledFg = NULL;
    butPtr->normalTextGC = None;
    butPtr->activeTextGC = None;
    butPtr->gray = None;
    butPtr->disabledGC = None;
    butPtr->copyGC = None;
    butPtr->widthString = NULL;
    butPtr->heightString = NULL;
    butPtr->width = 0;
    butPtr->height = 0;
    butPtr->wrapLength = 0;
    butPtr->padX = 0;
    butPtr->padY = 0;
    butPtr->anchor = TK_ANCHOR_CENTER;
    butPtr->justify = TK_JUSTIFY_CENTER;
    butPtr->textLayout = NULL;
    butPtr->indicatorOn = 0;
    butPtr->selectBorder = NULL;
    butPtr->indicatorSpace = 0;
    butPtr->indicatorDiameter = 0;
    butPtr->selVarName = NULL;
    butPtr->onValue = NULL;
    butPtr->offValue = NULL;
    butPtr->cursor = None;
    butPtr->command = NULL;
    butPtr->takeFocus = NULL;
    butPtr->flags = 0;
    butPtr->tile = butPtr->activeTile = NULL;
    butPtr->defaultState = STATE_DISABLED;
    butPtr->compound = NULL;
    butPtr->repeatDelay = 0;
    butPtr->overRelief = TK_RELIEF_RAISED;

    Tk_SetClass(tkwin, classNames[type]);
    Tk_CreateEventHandler(butPtr->tkwin,
	ExposureMask | StructureNotifyMask | FocusChangeMask,
	ButtonEventProc, butPtr);
    if (ConfigureButton(interp, butPtr, objc - 2, objv + 2,
	    configFlags[type]) != TCL_OK) {
	Tk_DestroyWindow(butPtr->tkwin);
	return TCL_ERROR;
    }
    Tcl_SetStringObj(Tcl_GetObjResult(interp), Tk_PathName(butPtr->tkwin), -1);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ButtonWidgetCmd --
 *
 *	This procedure is invoked to process the Tcl command
 *	that corresponds to a widget managed by this module.
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

static int
ButtonWidgetCmd(
    ClientData clientData,	/* Information about button widget. */
    Tcl_Interp *interp,		/* Current interpreter. */
    int objc,			/* Number of arguments. */
    Tcl_Obj *const *objv)	/* Argument strings. */
{
    Button *butPtr = clientData;
    int result = TCL_OK;
    size_t length;
    int c;
    char *string;

    if (objc < 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", 
		Tcl_GetString(objv[0]), " option ?arg arg ...?\"", 
		(char *)NULL);
	return TCL_ERROR;
    }
    Tcl_Preserve(butPtr);
    string = Tcl_GetString(objv[1]);
    c = string[0];
    length = strlen(string);
    if ((c == 'c') && (length >= 2) && (strncmp(string, "cget", length) == 0)) {
	if (objc != 3) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
		Tcl_GetString(objv[0]), " cget option\"", (char *)NULL);
	    goto error;
	}
	result = Blt_ConfigureValueFromObj(interp, butPtr->tkwin, configSpecs,
	    (char *)butPtr, objv[2], configFlags[butPtr->type]);
    } else if ((c == 'c') && (length >= 2) && 
	       (strncmp(string, "configure", length) == 0)) {
	if (objc == 2) {
	    result = Blt_ConfigureInfoFromObj(interp, butPtr->tkwin, 
		configSpecs, (char *)butPtr, (Tcl_Obj *)NULL, 
		configFlags[butPtr->type]);
	} else if (objc == 3) {
	    result = Blt_ConfigureInfoFromObj(interp, butPtr->tkwin, 
		configSpecs, (char *)butPtr, objv[2], 
		configFlags[butPtr->type]);
	} else {
	    result = ConfigureButton(interp, butPtr, objc - 2, objv + 2,
		configFlags[butPtr->type] | BLT_CONFIG_OBJV_ONLY);
	}
    } else if ((c == 'd') && (strncmp(string, "deselect", length) == 0)
	&& (butPtr->type >= TYPE_CHECK_BUTTON)) {
	if (objc > 2) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"", 
		Tcl_GetString(objv[0]), " deselect\"", (char *)NULL);
	    goto error;
	}
	if (butPtr->type == TYPE_CHECK_BUTTON) {
	    if (Tcl_SetVar(interp, butPtr->selVarName, butPtr->offValue,
		    TCL_GLOBAL_ONLY | TCL_LEAVE_ERR_MSG) == NULL) {
		result = TCL_ERROR;
	    }
	} else if (butPtr->flags & SELECTED) {
	    if (Tcl_SetVar(interp, butPtr->selVarName, "",
		    TCL_GLOBAL_ONLY | TCL_LEAVE_ERR_MSG) == NULL) {
		result = TCL_ERROR;
	    };
	}
    } else if ((c == 'f') && (strncmp(string, "flash", length) == 0)
	&& (butPtr->type != TYPE_LABEL)) {
	int i;

	if (objc > 2) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"", 
		Tcl_GetString(objv[0]), " flash\"", (char *)NULL);
	    goto error;
	}
	if (butPtr->state != STATE_DISABLED) {
	    for (i = 0; i < 4; i++) {
		butPtr->state = (butPtr->state == STATE_NORMAL)
		    ? STATE_ACTIVE : STATE_NORMAL;
		Tk_SetBackgroundFromBorder(butPtr->tkwin,
		    (butPtr->state == STATE_ACTIVE) ? butPtr->activeBorder
		    : butPtr->normalBorder);
		DisplayButton(butPtr);

		/*
		 * Special note: must cancel any existing idle handler
		 * for DisplayButton;  it's no longer needed, and DisplayButton
		 * cleared the REDRAW_PENDING flag.
		 */

		Tcl_CancelIdleCall(DisplayButton, butPtr);
#if !defined(WIN32) && !defined(MACOSX)
		XFlush(butPtr->display);
#endif
		Tcl_Sleep(50);
	    }
	}
    } else if ((c == 'i') && (strncmp(string, "invoke", length) == 0)
	&& (butPtr->type > TYPE_LABEL)) {
	if (objc > 2) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
		Tcl_GetString(objv[0]), " invoke\"", (char *)NULL);
	    goto error;
	}
	if (butPtr->state != STATE_DISABLED) {
	    result = InvokeButton(butPtr);
	}
    } else if ((c == 's') && (strncmp(string, "select", length) == 0)
	&& (butPtr->type >= TYPE_CHECK_BUTTON)) {
	if (objc > 2) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"", 
		Tcl_GetString(objv[0]), " select\"", (char *)NULL);
	    goto error;
	}
	if (Tcl_SetVar(interp, butPtr->selVarName, butPtr->onValue,
		TCL_GLOBAL_ONLY | TCL_LEAVE_ERR_MSG) == NULL) {
	    result = TCL_ERROR;
	}
    } else if ((c == 't') && (strncmp(string, "toggle", length) == 0)
	&& (length >= 2) && (butPtr->type == TYPE_CHECK_BUTTON)) {
	if (objc > 2) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"", 
		Tcl_GetString(objv[0]), " toggle\"", (char *)NULL);
	    goto error;
	}
	if (butPtr->flags & SELECTED) {
	    if (Tcl_SetVar(interp, butPtr->selVarName, butPtr->offValue,
		    TCL_GLOBAL_ONLY) == NULL) {
		result = TCL_ERROR;
	    }
	} else {
	    if (Tcl_SetVar(interp, butPtr->selVarName, butPtr->onValue,
		    TCL_GLOBAL_ONLY) == NULL) {
		result = TCL_ERROR;
	    }
	}
    } else {
	Tcl_AppendResult(interp, "bad option \"", Tcl_GetString(objv[1]), 
		"\": must be ", optionStrings[butPtr->type], (char *)NULL);
	goto error;
    }
    Tcl_Release(butPtr);
    return result;

  error:
    Tcl_Release(butPtr);
    return TCL_ERROR;
}

/*
 *---------------------------------------------------------------------------
 *
 * DestroyButton --
 *
 *	This procedure is invoked by Tcl_EventuallyFree or Tcl_Release
 *	to clean up the internal structure of a button at a safe time
 *	(when no-one is using it anymore).
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Everything associated with the widget is freed up.
 *
 *---------------------------------------------------------------------------
 */

static void
DestroyButton(Button *butPtr)
{
    /*
     * Free up all the stuff that requires special handling, then
     * let Blt_FreeOptions handle all the standard option-related
     * stuff.
     */

    if (butPtr->textVarName != NULL) {
	Tcl_UntraceVar(butPtr->interp, butPtr->textVarName,
	    TCL_GLOBAL_ONLY | TCL_TRACE_WRITES | TCL_TRACE_UNSETS,
	    ButtonTextVarProc, butPtr);
    }
    if (butPtr->image != NULL) {
	Tk_FreeImage(butPtr->image);
    }
    if (butPtr->selectImage != NULL) {
	Tk_FreeImage(butPtr->selectImage);
    }
    if (butPtr->normalTextGC != None) {
	Tk_FreeGC(butPtr->display, butPtr->normalTextGC);
    }
    if (butPtr->activeTextGC != None) {
	Tk_FreeGC(butPtr->display, butPtr->activeTextGC);
    }
    if (butPtr->gray != None) {
	Tk_FreeBitmap(butPtr->display, butPtr->gray);
    }
    if (butPtr->disabledGC != None) {
	Tk_FreeGC(butPtr->display, butPtr->disabledGC);
    }
    if (butPtr->copyGC != None) {
	Tk_FreeGC(butPtr->display, butPtr->copyGC);
    }
    if (butPtr->selVarName != NULL) {
	Tcl_UntraceVar(butPtr->interp, butPtr->selVarName,
	    TCL_GLOBAL_ONLY | TCL_TRACE_WRITES | TCL_TRACE_UNSETS,
	    ButtonVarProc, (ClientData)butPtr);
    }
    if (butPtr->activeTile != NULL) {
	Blt_FreeTile(butPtr->activeTile);
    }
    if (butPtr->tile != NULL) {
	Blt_FreeTile(butPtr->tile);
    }
    Tk_FreeTextLayout(butPtr->textLayout);
    Blt_FreeOptions(configSpecs, (char *)butPtr, butPtr->display,
	configFlags[butPtr->type]);
    Tcl_EventuallyFree((ClientData)butPtr, TCL_DYNAMIC);
}

/*ARGSUSED*/
static void
TileChangedProc(ClientData clientData, Blt_Tile tile)
{
    Button *butPtr = clientData;

    if (butPtr->tkwin != NULL) {
	/*
	 * Arrange for the button to be redisplayed.
	 */
	if (Tk_IsMapped(butPtr->tkwin) && !(butPtr->flags & REDRAW_PENDING)) {
	    Tcl_DoWhenIdle(DisplayButton, (ClientData)butPtr);
	    butPtr->flags |= REDRAW_PENDING;
	}
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * ConfigureButton --
 *
 *	This procedure is called to process an objv/objc list, plus
 *	the Tk option database, in order to configure (or
 *	reconfigure) a button widget.
 *
 * Results:
 *	The return value is a standard Tcl result.  If TCL_ERROR is
 *	returned, then interp->result contains an error message.
 *
 * Side effects:
 *	Configuration information, such as text string, colors, font,
 *	etc. get set for butPtr;  old resources get freed, if there
 *	were any.  The button is redisplayed.
 *
 *---------------------------------------------------------------------------
 */

static int
ConfigureButton(
    Tcl_Interp *interp,		/* Used for error reporting. */
    Button *butPtr,		/* Information about widget;  may or may
				 * not already have values for some fields. */
    int objc,			/* Number of valid entries in objv. */
    Tcl_Obj *const *objv,	/* Arguments. */
    int flags)			/* Flags to pass to Blt_ConfigureWidget. */
{
    XGCValues gcValues;
    GC newGC;
    unsigned long mask;
    Tk_Image image;

    /*
     * Eliminate any existing trace on variables monitored by the button.
     */

    if (butPtr->textVarName != NULL) {
	Tcl_UntraceVar(interp, butPtr->textVarName,
	    TCL_GLOBAL_ONLY | TCL_TRACE_WRITES | TCL_TRACE_UNSETS,
	    ButtonTextVarProc, (ClientData)butPtr);
    }
    if (butPtr->selVarName != NULL) {
	Tcl_UntraceVar(interp, butPtr->selVarName,
	    TCL_GLOBAL_ONLY | TCL_TRACE_WRITES | TCL_TRACE_UNSETS,
	    ButtonVarProc, (ClientData)butPtr);
    }
    if (Blt_ConfigureWidgetFromObj(interp, butPtr->tkwin, configSpecs,
	    objc, objv, (char *)butPtr, flags) != TCL_OK) {
	return TCL_ERROR;
    }
    /*
     * A few options need special processing, such as setting the
     * background from a 3-D border, or filling in complicated
     * defaults that couldn't be specified to Blt_ConfigureWidget.
     */

    if ((butPtr->state == STATE_ACTIVE) && !Tk_StrictMotif(butPtr->tkwin)) {
	Tk_SetBackgroundFromBorder(butPtr->tkwin, butPtr->activeBorder);
    } else {
	Tk_SetBackgroundFromBorder(butPtr->tkwin, butPtr->normalBorder);
	if ((butPtr->state != STATE_NORMAL) && (butPtr->state != STATE_ACTIVE)
	    && (butPtr->state != STATE_DISABLED)) {
	    Tcl_AppendResult(interp, "bad state value \"", 
		Blt_Itoa(butPtr->state), 
		"\": must be normal, active, or disabled", (char *)NULL);
	    butPtr->state = STATE_NORMAL;
	    return TCL_ERROR;
	}
    }

    if ((butPtr->defaultState != STATE_ACTIVE)
	&& (butPtr->defaultState != STATE_DISABLED)
	&& (butPtr->defaultState != STATE_NORMAL)) {
	Tcl_AppendResult(interp, "bad -default value \"", butPtr->defaultState,
	    "\": must be normal, active, or disabled", (char *)NULL);
	butPtr->defaultState = STATE_DISABLED;
	return TCL_ERROR;
    }
    if (butPtr->highlightWidth < 0) {
	butPtr->highlightWidth = 0;
    }
    gcValues.font = Blt_FontId(butPtr->font);
    gcValues.foreground = butPtr->normalFg->pixel;
    gcValues.background = Tk_3DBorderColor(butPtr->normalBorder)->pixel;

    if (butPtr->tile != NULL) {
	Blt_SetTileChangedProc(butPtr->tile, TileChangedProc,
		(ClientData)butPtr);
    }
    if (butPtr->activeTile != NULL) {
	Blt_SetTileChangedProc(butPtr->activeTile, TileChangedProc,
		(ClientData)butPtr);
    }
    /*
     * Note: GraphicsExpose events are disabled in normalTextGC because it's
     * used to copy stuff from an off-screen pixmap onto the screen (we know
     * that there's no problem with obscured areas).
     */

    gcValues.graphics_exposures = False;
    newGC = Tk_GetGC(butPtr->tkwin,
	GCForeground | GCBackground | GCFont | GCGraphicsExposures,
	&gcValues);
    if (butPtr->normalTextGC != None) {
	Tk_FreeGC(butPtr->display, butPtr->normalTextGC);
    }
    butPtr->normalTextGC = newGC;

    if (butPtr->activeFg != NULL) {
	gcValues.font = Blt_FontId(butPtr->font);
	gcValues.foreground = butPtr->activeFg->pixel;
	gcValues.background = Tk_3DBorderColor(butPtr->activeBorder)->pixel;
	newGC = Tk_GetGC(butPtr->tkwin,
	    GCForeground | GCBackground | GCFont, &gcValues);
	if (butPtr->activeTextGC != None) {
	    Tk_FreeGC(butPtr->display, butPtr->activeTextGC);
	}
	butPtr->activeTextGC = newGC;
    }
    if (butPtr->type != TYPE_LABEL) {
	gcValues.font = Blt_FontId(butPtr->font);
	gcValues.background = Tk_3DBorderColor(butPtr->normalBorder)->pixel;
	if ((butPtr->disabledFg != NULL) && (butPtr->imageString == NULL)) {
	    gcValues.foreground = butPtr->disabledFg->pixel;
	    mask = GCForeground | GCBackground | GCFont;
	} else {
	    gcValues.foreground = gcValues.background;
	    if (butPtr->gray == None) {
		butPtr->gray = Tk_GetBitmap(interp, butPtr->tkwin,
		    Tk_GetUid("gray50"));
		if (butPtr->gray == None) {
		    return TCL_ERROR;
		}
	    }
	    gcValues.fill_style = FillStippled;
	    gcValues.stipple = butPtr->gray;
	    mask = GCForeground | GCFillStyle | GCStipple;
	}
	newGC = Tk_GetGC(butPtr->tkwin, mask, &gcValues);
	if (butPtr->disabledGC != None) {
	    Tk_FreeGC(butPtr->display, butPtr->disabledGC);
	}
	butPtr->disabledGC = newGC;
    }
    if (butPtr->copyGC == None) {
	butPtr->copyGC = Tk_GetGC(butPtr->tkwin, 0, &gcValues);
    }
    if (butPtr->padX < 0) {
	butPtr->padX = 0;
    }
    if (butPtr->padY < 0) {
	butPtr->padY = 0;
    }
    if (butPtr->type >= TYPE_CHECK_BUTTON) {
	const char *value;

	if (butPtr->selVarName == NULL) {
	    butPtr->selVarName = Blt_StrdupAssert(Tk_Name(butPtr->tkwin));
	}
	/*
	 * Select the button if the associated variable has the
	 * appropriate value, initialize the variable if it doesn't
	 * exist, then set a trace on the variable to monitor future
	 * changes to its value.
	 */

	value = Tcl_GetVar(interp, butPtr->selVarName, TCL_GLOBAL_ONLY);
	butPtr->flags &= ~SELECTED;
	if (value != NULL) {
	    if (strcmp(value, butPtr->onValue) == 0) {
		butPtr->flags |= SELECTED;
	    }
	} else {
	    if (Tcl_SetVar(interp, butPtr->selVarName,
		    (butPtr->type == TYPE_CHECK_BUTTON) ? butPtr->offValue : "",
		    TCL_GLOBAL_ONLY | TCL_LEAVE_ERR_MSG) == NULL) {
		return TCL_ERROR;
	    }
	}
	Tcl_TraceVar(interp, butPtr->selVarName,
	    TCL_GLOBAL_ONLY | TCL_TRACE_WRITES | TCL_TRACE_UNSETS,
	    ButtonVarProc, (ClientData)butPtr);
    }
    /*
     * Get the images for the widget, if there are any.  Allocate the
     * new images before freeing the old ones, so that the reference
     * counts don't go to zero and cause image data to be discarded.
     */

    if (butPtr->imageString != NULL) {
	image = Tk_GetImage(butPtr->interp, butPtr->tkwin,
	    butPtr->imageString, ButtonImageProc, (ClientData)butPtr);
	if (image == NULL) {
	    return TCL_ERROR;
	}
    } else {
	image = NULL;
    }
    if (butPtr->image != NULL) {
	Tk_FreeImage(butPtr->image);
    }
    butPtr->image = image;
    if (butPtr->selectImageString != NULL) {
	image = Tk_GetImage(butPtr->interp, butPtr->tkwin,
	    butPtr->selectImageString, ButtonSelectImageProc,
	    (ClientData)butPtr);
	if (image == NULL) {
	    return TCL_ERROR;
	}
    } else {
	image = NULL;
    }
    if (butPtr->selectImage != NULL) {
	Tk_FreeImage(butPtr->selectImage);
    }
    butPtr->selectImage = image;

    if ((butPtr->image == NULL) && (butPtr->bitmap == None)
	&& (butPtr->textVarName != NULL)) {
	/*
	 * The button must display the value of a variable: set up a trace
	 * on the variable's value, create the variable if it doesn't
	 * exist, and fetch its current value.
	 */

	const char *value;

	value = Tcl_GetVar(interp, butPtr->textVarName, TCL_GLOBAL_ONLY);
	if (value == NULL) {
	    if (Tcl_SetVar(interp, butPtr->textVarName, butPtr->text,
		    TCL_GLOBAL_ONLY | TCL_LEAVE_ERR_MSG) == NULL) {
		return TCL_ERROR;
	    }
	} else {
	    if (butPtr->text != NULL) {
		Blt_Free(butPtr->text);
	    }
	    butPtr->text = Blt_StrdupAssert(value);
	}
	Tcl_TraceVar(interp, butPtr->textVarName,
	    TCL_GLOBAL_ONLY | TCL_TRACE_WRITES | TCL_TRACE_UNSETS,
	    ButtonTextVarProc, (ClientData)butPtr);
    }
    if ((butPtr->bitmap != None) || (butPtr->image != NULL)) {
	if (Tk_GetPixels(interp, butPtr->tkwin, butPtr->widthString,
		&butPtr->width) != TCL_OK) {
	  widthError:
	    Tcl_AddErrorInfo(interp, "\n    (processing -width option)");
	    return TCL_ERROR;
	}
	if (Tk_GetPixels(interp, butPtr->tkwin, butPtr->heightString,
		&butPtr->height) != TCL_OK) {
	  heightError:
	    Tcl_AddErrorInfo(interp, "\n    (processing -height option)");
	    return TCL_ERROR;
	}
    } else {
	if (Tcl_GetInt(interp, butPtr->widthString, &butPtr->width)
	    != TCL_OK) {
	    goto widthError;
	}
	if (Tcl_GetInt(interp, butPtr->heightString, &butPtr->height)
	    != TCL_OK) {
	    goto heightError;
	}
    }
    ComputeButtonGeometry(butPtr);

    /*
     * Lastly, arrange for the button to be redisplayed.
     */

    if (Tk_IsMapped(butPtr->tkwin) && !(butPtr->flags & REDRAW_PENDING)) {
	Tcl_DoWhenIdle(DisplayButton, (ClientData)butPtr);
	butPtr->flags |= REDRAW_PENDING;
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * DisplayButton --
 *
 *	This procedure is invoked to display a button widget.  It is
 *	normally invoked as an idle handler.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Commands are output to X to display the button in its
 *	current mode.  The REDRAW_PENDING flag is cleared.
 *
 *---------------------------------------------------------------------------
 */

static void
DisplayButton(ClientData clientData)
{
    Button *butPtr = clientData;
    GC gc;
    Tk_3DBorder border;
    Pixmap pixmap;
    int x = 0;			/* Initialization only needed to stop
				 * compiler warning. */
    int y, relief;
    Tk_Window tkwin = butPtr->tkwin;
    int width, height;
    int offset;			/* 0 means this is a label widget.  1 means
				 * it is a flavor of button, so we offset
				 * the text to make the button appear to
				 * move up and down as the relief changes. */
    Blt_Tile tile;

    butPtr->flags &= ~REDRAW_PENDING;
    if ((butPtr->tkwin == NULL) || !Tk_IsMapped(tkwin)) {
	return;
    }
    if ((Tk_Width(butPtr->tkwin) < 2) || (Tk_Height(butPtr->tkwin) < 2)) {
	return;
    }
    tile = butPtr->tile;
    border = butPtr->normalBorder;
    if ((butPtr->state == STATE_DISABLED) && (butPtr->disabledFg != NULL)) {
	gc = butPtr->disabledGC;
    } else if ((butPtr->state == STATE_ACTIVE)
	&& !Tk_StrictMotif(butPtr->tkwin)) {
	gc = butPtr->activeTextGC;
	border = butPtr->activeBorder;
	tile = butPtr->activeTile;
    } else {
	gc = butPtr->normalTextGC;
    }
    if ((butPtr->flags & SELECTED) && (butPtr->state != STATE_ACTIVE)
	&& (butPtr->selectBorder != NULL) && !butPtr->indicatorOn) {
	border = butPtr->selectBorder;
    }
    /*
     * Override the relief specified for the button if this is a
     * checkbutton or radiobutton and there's no indicator.
     */

    relief = butPtr->relief;
    if ((butPtr->type >= TYPE_CHECK_BUTTON) && !butPtr->indicatorOn) {
	relief = (butPtr->flags & SELECTED) ? TK_RELIEF_SUNKEN
	    : TK_RELIEF_RAISED;
    }
    offset = (butPtr->type == TYPE_BUTTON) && !Tk_StrictMotif(butPtr->tkwin);

    /*
     * In order to avoid screen flashes, this procedure redraws
     * the button in a pixmap, then copies the pixmap to the
     * screen in a single operation.  This means that there's no
     * point in time where the on-sreen image has been cleared.
     */

    pixmap = Tk_GetPixmap(butPtr->display, Tk_WindowId(tkwin),
	Tk_Width(tkwin), Tk_Height(tkwin), Tk_Depth(tkwin));
    if (tile != NULL) {
	Blt_SetTileOrigin(tkwin, tile, 0, 0);
	Blt_TileRectangle(tkwin, pixmap, tile, 0, 0, Tk_Width(tkwin),
	    Tk_Height(tkwin));
    } else {
	Blt_Fill3DRectangle(tkwin, pixmap, border, 0, 0, Tk_Width(tkwin),
	    Tk_Height(tkwin), 0, TK_RELIEF_FLAT);
    }

    /*
     * Display image or bitmap or text for button.
     */

    if (butPtr->image != None) {
	Tk_SizeOfImage(butPtr->image, &width, &height);

      imageOrBitmap:
	TkComputeAnchor(butPtr->anchor, tkwin, 0, 0,
	    butPtr->indicatorSpace + width, height, &x, &y);
	x += butPtr->indicatorSpace;

	x += offset;
	y += offset;
	if (relief == TK_RELIEF_RAISED) {
	    x -= offset;
	    y -= offset;
	} else if (relief == TK_RELIEF_SUNKEN) {
	    x += offset;
	    y += offset;
	}
	if (butPtr->image != NULL) {
	    if ((butPtr->selectImage != NULL) && (butPtr->flags & SELECTED)) {
		Tk_RedrawImage(butPtr->selectImage, 0, 0, width, height, pixmap,
		    x, y);
	    } else {
		Tk_RedrawImage(butPtr->image, 0, 0, width, height, pixmap,
		    x, y);
	    }
	} else {
	    XSetClipOrigin(butPtr->display, gc, x, y);
	    if (tile != NULL) {
		XSetClipMask(butPtr->display, gc, butPtr->bitmap);
	    }
	    XCopyPlane(butPtr->display, butPtr->bitmap, pixmap, gc, 0, 0,
		(unsigned int)width, (unsigned int)height, x, y, 1);
	    if (tile != NULL) {
		XSetClipMask(butPtr->display, gc, None);
	    }
	    XSetClipOrigin(butPtr->display, gc, 0, 0);
	}
	y += height / 2;
    } else if (butPtr->bitmap != None) {
	Tk_SizeOfBitmap(butPtr->display, butPtr->bitmap, &width, &height);
	goto imageOrBitmap;
    } else {
	TkComputeAnchor(butPtr->anchor, tkwin, butPtr->padX, butPtr->padY,
	    butPtr->indicatorSpace + butPtr->textWidth,
	    butPtr->textHeight, &x, &y);

	x += butPtr->indicatorSpace;

	x += offset;
	y += offset;
	if (relief == TK_RELIEF_RAISED) {
	    x -= offset;
	    y -= offset;
	} else if (relief == TK_RELIEF_SUNKEN) {
	    x += offset;
	    y += offset;
	}
	Blt_DrawTextLayout(butPtr->display, pixmap, gc, butPtr->textLayout,
	    x, y, 0, -1);
	Blt_UnderlineTextLayout(butPtr->display, pixmap, gc,
	    butPtr->textLayout, x, y, butPtr->underline);
	y += butPtr->textHeight / 2;
    }

    /*
     * Draw the indicator for check buttons and radio buttons.  At this
     * point x and y refer to the top-left corner of the text or image
     * or bitmap.
     */

    if ((butPtr->type == TYPE_CHECK_BUTTON) && butPtr->indicatorOn) {
	int dim;

	dim = butPtr->indicatorDiameter;
	x -= butPtr->indicatorSpace;
	y -= dim / 2;
	if (dim > 2 * butPtr->borderWidth) {
	    Blt_Draw3DRectangle(tkwin, pixmap, border, x, y, dim, dim,
		butPtr->borderWidth,
		(butPtr->flags & SELECTED) ? TK_RELIEF_SUNKEN :
		TK_RELIEF_RAISED);
	    x += butPtr->borderWidth;
	    y += butPtr->borderWidth;
	    dim -= 2 * butPtr->borderWidth;
	    if (butPtr->flags & SELECTED) {
		GC borderGC;

		borderGC = Tk_3DBorderGC(tkwin, (butPtr->selectBorder != NULL)
		    ? butPtr->selectBorder : butPtr->normalBorder,
		    TK_3D_FLAT_GC);
		XFillRectangle(butPtr->display, pixmap, borderGC, x, y,
		    (unsigned int)dim, (unsigned int)dim);
	    } else {
		Blt_Fill3DRectangle(tkwin, pixmap, butPtr->normalBorder, x, y,
		    dim, dim, butPtr->borderWidth, TK_RELIEF_FLAT);
	    }
	}
    } else if ((butPtr->type == TYPE_RADIO_BUTTON) && butPtr->indicatorOn) {
	XPoint points[4];
	int radius;

	radius = butPtr->indicatorDiameter / 2;
	points[0].x = x - butPtr->indicatorSpace;
	points[0].y = y;
	points[1].x = points[0].x + radius;
	points[1].y = points[0].y + radius;
	points[2].x = points[1].x + radius;
	points[2].y = points[0].y;
	points[3].x = points[1].x;
	points[3].y = points[0].y - radius;
	if (butPtr->flags & SELECTED) {
	    GC borderGC;

	    borderGC = Tk_3DBorderGC(tkwin, (butPtr->selectBorder != NULL)
		? butPtr->selectBorder : butPtr->normalBorder,
		TK_3D_FLAT_GC);
	    XFillPolygon(butPtr->display, pixmap, borderGC, points, 4, Convex,
		CoordModeOrigin);
	} else {
	    Tk_Fill3DPolygon(tkwin, pixmap, butPtr->normalBorder, points,
		4, butPtr->borderWidth, TK_RELIEF_FLAT);
	}
	Tk_Draw3DPolygon(tkwin, pixmap, border, points, 4, butPtr->borderWidth,
	    (butPtr->flags & SELECTED) ? TK_RELIEF_SUNKEN :
	    TK_RELIEF_RAISED);
    }
    /*
     * If the button is disabled with a stipple rather than a special
     * foreground color, generate the stippled effect.  If the widget
     * is selected and we use a different background color when selected,
     * must temporarily modify the GC.
     */

    if ((butPtr->state == STATE_DISABLED)
	&& ((butPtr->disabledFg == NULL) || (butPtr->image != NULL))) {
	if ((butPtr->flags & SELECTED) && !butPtr->indicatorOn
	    && (butPtr->selectBorder != NULL)) {
	    XSetForeground(butPtr->display, butPtr->disabledGC,
		Tk_3DBorderColor(butPtr->selectBorder)->pixel);
	}
	XFillRectangle(butPtr->display, pixmap, butPtr->disabledGC,
	    butPtr->inset, butPtr->inset,
	    (unsigned)(Tk_Width(tkwin) - 2 * butPtr->inset),
	    (unsigned)(Tk_Height(tkwin) - 2 * butPtr->inset));
	if ((butPtr->flags & SELECTED) && !butPtr->indicatorOn
	    && (butPtr->selectBorder != NULL)) {
	    XSetForeground(butPtr->display, butPtr->disabledGC,
		Tk_3DBorderColor(butPtr->normalBorder)->pixel);
	}
    }
    /*
     * Draw the border and traversal highlight last.  This way, if the
     * button's contents overflow they'll be covered up by the border.
     */

    if (relief != TK_RELIEF_FLAT) {
	int inset = butPtr->highlightWidth;
	if (butPtr->defaultState == STATE_ACTIVE) {
	    inset += 2;
	    Blt_Draw3DRectangle(tkwin, pixmap, border, inset, inset,
		Tk_Width(tkwin) - 2 * inset, Tk_Height(tkwin) - 2 * inset,
		1, TK_RELIEF_SUNKEN);
	    inset += 3;
	}
	Blt_Draw3DRectangle(tkwin, pixmap, border, inset, inset,
	    Tk_Width(tkwin) - 2 * inset, Tk_Height(tkwin) - 2 * inset,
	    butPtr->borderWidth, relief);
    }
    if (butPtr->highlightWidth != 0) {
	GC highlightGC;

	if (butPtr->flags & GOT_FOCUS) {
	    highlightGC = Tk_GCForColor(butPtr->highlightColorPtr, pixmap);
	} else {
	    highlightGC = Tk_GCForColor(butPtr->highlightBgColorPtr, pixmap);
	}
	Tk_DrawFocusHighlight(tkwin, highlightGC, butPtr->highlightWidth, pixmap);
    }
    /*
     * Copy the information from the off-screen pixmap onto the screen,
     * then delete the pixmap.
     */

    XCopyArea(butPtr->display, pixmap, Tk_WindowId(tkwin),
	butPtr->copyGC, 0, 0, (unsigned)Tk_Width(tkwin),
	(unsigned)Tk_Height(tkwin), 0, 0);
    Tk_FreePixmap(butPtr->display, pixmap);
}

/*
 *---------------------------------------------------------------------------
 *
 * ComputeButtonGeometry --
 *
 *	After changes in a button's text or bitmap, this procedure
 *	recomputes the button's geometry and passes this information
 *	along to the geometry manager for the window.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The button's window may change size.
 *
 *---------------------------------------------------------------------------
 */

static void
ComputeButtonGeometry(Button *butPtr)
{
    int width, height;

    if (butPtr->highlightWidth < 0) {
	butPtr->highlightWidth = 0;
    }
    butPtr->inset = butPtr->highlightWidth + butPtr->borderWidth;

    /*
     * Leave room for the default ring if needed.
     */

    if (butPtr->defaultState == STATE_ACTIVE) {
	butPtr->inset += 5;
    }
    butPtr->indicatorSpace = 0;
    if (butPtr->image != NULL) {
	Tk_SizeOfImage(butPtr->image, &width, &height);
      imageOrBitmap:
	if (butPtr->width > 0) {
	    width = butPtr->width;
	}
	if (butPtr->height > 0) {
	    height = butPtr->height;
	}
	if ((butPtr->type >= TYPE_CHECK_BUTTON) && butPtr->indicatorOn) {
	    butPtr->indicatorSpace = height;
	    if (butPtr->type == TYPE_CHECK_BUTTON) {
		butPtr->indicatorDiameter = (65 * height) / 100;
	    } else {
		butPtr->indicatorDiameter = (75 * height) / 100;
	    }
	}
    } else if (butPtr->bitmap != None) {
	Tk_SizeOfBitmap(butPtr->display, butPtr->bitmap, &width, &height);
	goto imageOrBitmap;
    } else {
	int avgWidth;
	Blt_FontMetrics fm;

	Tk_FreeTextLayout(butPtr->textLayout);
	butPtr->textLayout = Blt_ComputeTextLayout(butPtr->font,
	    butPtr->text, -1, butPtr->wrapLength, butPtr->justify, 0,
	    &butPtr->textWidth, &butPtr->textHeight);
	width = butPtr->textWidth;
	height = butPtr->textHeight;
	avgWidth = Blt_TextWidth(butPtr->font, "0", 1);
	Blt_GetFontMetrics(butPtr->font, &fm);

	if (butPtr->width > 0) {
	    width = butPtr->width * avgWidth;
	}
	if (butPtr->height > 0) {
	    height = butPtr->height * fm.linespace;
	}
	if ((butPtr->type >= TYPE_CHECK_BUTTON) && butPtr->indicatorOn) {
	    butPtr->indicatorDiameter = fm.linespace;
	    if (butPtr->type == TYPE_CHECK_BUTTON) {
		butPtr->indicatorDiameter = 
		    (80 * butPtr->indicatorDiameter) / 100;
	    }
	    butPtr->indicatorSpace = butPtr->indicatorDiameter + avgWidth;
	}
    }

    /*
     * When issuing the geometry request, add extra space for the indicator,
     * if any, and for the border and padding, plus two extra pixels so the
     * display can be offset by 1 pixel in either direction for the raised
     * or lowered effect.
     */

    if ((butPtr->image == NULL) && (butPtr->bitmap == None)) {
	width += 2 * butPtr->padX;
	height += 2 * butPtr->padY;
    }
    if ((butPtr->type == TYPE_BUTTON) && !Tk_StrictMotif(butPtr->tkwin)) {
	width += 2;
	height += 2;
    }
    Tk_GeometryRequest(butPtr->tkwin, (int)(width + butPtr->indicatorSpace
	    + 2 * butPtr->inset), (int)(height + 2 * butPtr->inset));
    Tk_SetInternalBorder(butPtr->tkwin, butPtr->inset);
}

/*
 *---------------------------------------------------------------------------
 *
 * ButtonEventProc --
 *
 *	This procedure is invoked by the Tk dispatcher for various
 *	events on buttons.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	When the window gets deleted, internal structures get
 *	cleaned up.  When it gets exposed, it is redisplayed.
 *
 *---------------------------------------------------------------------------
 */

static void
ButtonEventProc(ClientData clientData, XEvent *eventPtr)
{
    Button *butPtr = clientData;
    if ((eventPtr->type == Expose) && (eventPtr->xexpose.count == 0)) {
	goto redraw;
    } else if (eventPtr->type == ConfigureNotify) {
	/*
	 * Must redraw after size changes, since layout could have changed
	 * and borders will need to be redrawn.
	 */

	goto redraw;
    } else if (eventPtr->type == DestroyNotify) {
	if (butPtr->tkwin != NULL) {
	    butPtr->tkwin = NULL;
	    Tcl_DeleteCommandFromToken(butPtr->interp, butPtr->widgetCmd);
	}
	if (butPtr->flags & REDRAW_PENDING) {
	    Tcl_CancelIdleCall(DisplayButton, (ClientData)butPtr);
	}
	/* This is a hack to workaround a bug in 8.3.3. */
	DestroyButton((ClientData)butPtr);
	/* Tcl_EventuallyFree((ClientData)butPtr, (Tcl_FreeProc *)Blt_Free); */
    } else if (eventPtr->type == FocusIn) {
	if (eventPtr->xfocus.detail != NotifyInferior) {
	    butPtr->flags |= GOT_FOCUS;
	    if (butPtr->highlightWidth > 0) {
		goto redraw;
	    }
	}
    } else if (eventPtr->type == FocusOut) {
	if (eventPtr->xfocus.detail != NotifyInferior) {
	    butPtr->flags &= ~GOT_FOCUS;
	    if (butPtr->highlightWidth > 0) {
		goto redraw;
	    }
	}
    }
    return;

  redraw:
    if ((butPtr->tkwin != NULL) && !(butPtr->flags & REDRAW_PENDING)) {
	Tcl_DoWhenIdle(DisplayButton, (ClientData)butPtr);
	butPtr->flags |= REDRAW_PENDING;
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * ButtonCmdDeletedProc --
 *
 *	This procedure is invoked when a widget command is deleted.  If
 *	the widget isn't already in the process of being destroyed,
 *	this command destroys it.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The widget is destroyed.
 *
 *---------------------------------------------------------------------------
 */

static void
ButtonCmdDeletedProc(ClientData clientData)
{
    Button *butPtr = clientData;
    Tk_Window tkwin = butPtr->tkwin;

    /*
     * This procedure could be invoked either because the window was
     * destroyed and the command was then deleted (in which case tkwin
     * is NULL) or because the command was deleted, and then this procedure
     * destroys the widget.
     */

    if (tkwin != NULL) {
	butPtr->tkwin = NULL;
	Tk_DestroyWindow(tkwin);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * InvokeButton --
 *
 *	This procedure is called to carry out the actions associated
 *	with a button, such as invoking a Tcl command or setting a
 *	variable.  This procedure is invoked, for example, when the
 *	button is invoked via the mouse.
 *
 * Results:
 *	A standard Tcl return value.  Information is also left in
 *	interp->result.
 *
 * Side effects:
 *	Depends on the button and its associated command.
 *
 *---------------------------------------------------------------------------
 */

static int
InvokeButton(Button *butPtr)
{
    if (butPtr->type == TYPE_CHECK_BUTTON) {
	if (butPtr->flags & SELECTED) {
	    if (Tcl_SetVar(butPtr->interp, butPtr->selVarName, 
			   butPtr->offValue,
		    TCL_GLOBAL_ONLY | TCL_LEAVE_ERR_MSG) == NULL) {
		return TCL_ERROR;
	    }
	} else {
	    if (Tcl_SetVar(butPtr->interp, butPtr->selVarName, butPtr->onValue,
		    TCL_GLOBAL_ONLY | TCL_LEAVE_ERR_MSG) == NULL) {
		return TCL_ERROR;
	    }
	}
    } else if (butPtr->type == TYPE_RADIO_BUTTON) {
	if (Tcl_SetVar(butPtr->interp, butPtr->selVarName, butPtr->onValue,
		TCL_GLOBAL_ONLY | TCL_LEAVE_ERR_MSG) == NULL) {
	    return TCL_ERROR;
	}
    }
    if ((butPtr->type != TYPE_LABEL) && (butPtr->command != NULL)) {
	return TkCopyAndGlobalEval(butPtr->interp, butPtr->command);
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ButtonVarProc --
 *
 *	This procedure is invoked when someone changes the
 *	state variable associated with a radio button.  Depending
 *	on the new value of the button's variable, the button
 *	may be selected or deselected.
 *
 * Results:
 *	NULL is always returned.
 *
 * Side effects:
 *	The button may become selected or deselected.
 *
 *---------------------------------------------------------------------------
 */

 /* ARGSUSED */
static char *
ButtonVarProc(
    ClientData clientData,	/* Information about button. */
    Tcl_Interp *interp,		/* Interpreter containing variable. */
    const char *name1,		/* Name of variable. */
    const char *name2,		/* Second part of variable name. */
    int flags)			/* Information about what happened. */
{
    Button *butPtr = clientData;
    const char *value;

    /*
     * If the variable is being unset, then just re-establish the
     * trace unless the whole interpreter is going away.
     */

    if (flags & TCL_TRACE_UNSETS) {
	butPtr->flags &= ~SELECTED;
	if ((flags & TCL_TRACE_DESTROYED) && !(flags & TCL_INTERP_DESTROYED)) {
	    Tcl_TraceVar(interp, butPtr->selVarName,
		TCL_GLOBAL_ONLY | TCL_TRACE_WRITES | TCL_TRACE_UNSETS,
		ButtonVarProc, clientData);
	}
	goto redisplay;
    }
    /*
     * Use the value of the variable to update the selected status of
     * the button.
     */

    value = Tcl_GetVar(interp, butPtr->selVarName, TCL_GLOBAL_ONLY);
    if (value == NULL) {
	value = "";
    }
    if (strcmp(value, butPtr->onValue) == 0) {
	if (butPtr->flags & SELECTED) {
	    return (char *) NULL;
	}
	butPtr->flags |= SELECTED;
    } else if (butPtr->flags & SELECTED) {
	butPtr->flags &= ~SELECTED;
    } else {
	return (char *) NULL;
    }

  redisplay:
    if ((butPtr->tkwin != NULL) && Tk_IsMapped(butPtr->tkwin)
	&& !(butPtr->flags & REDRAW_PENDING)) {
	Tcl_DoWhenIdle(DisplayButton, (ClientData)butPtr);
	butPtr->flags |= REDRAW_PENDING;
    }
    return (char *) NULL;
}

/*
 *---------------------------------------------------------------------------
 *
 * ButtonTextVarProc --
 *
 *	This procedure is invoked when someone changes the variable
 *	whose contents are to be displayed in a button.
 *
 * Results:
 *	NULL is always returned.
 *
 * Side effects:
 *	The text displayed in the button will change to match the
 *	variable.
 *
 *---------------------------------------------------------------------------
 */

 /* ARGSUSED */
static char *
ButtonTextVarProc(
    ClientData clientData,	/* Information about button. */
    Tcl_Interp *interp,		/* Interpreter containing variable. */
    const char *name1,		/* Not used. */
    const char *name2,		/* Not used. */
    int flags)			/* Information about what happened. */
{
    Button *butPtr = clientData;
    const char *value;

    /*
     * If the variable is unset, then immediately recreate it unless
     * the whole interpreter is going away.
     */

    if (flags & TCL_TRACE_UNSETS) {
	if ((flags & TCL_TRACE_DESTROYED) && !(flags & TCL_INTERP_DESTROYED)) {
	    Tcl_SetVar(interp, butPtr->textVarName, butPtr->text,
		TCL_GLOBAL_ONLY);
	    Tcl_TraceVar(interp, butPtr->textVarName,
		TCL_GLOBAL_ONLY | TCL_TRACE_WRITES | TCL_TRACE_UNSETS,
		ButtonTextVarProc, clientData);
	}
	return (char *) NULL;
    }
    value = Tcl_GetVar(interp, butPtr->textVarName, TCL_GLOBAL_ONLY);
    if (value == NULL) {
	value = "";
    }
    if (butPtr->text != NULL) {
	Blt_Free(butPtr->text);
    }
    butPtr->text = Blt_StrdupAssert(value);
    ComputeButtonGeometry(butPtr);

    if ((butPtr->tkwin != NULL) && Tk_IsMapped(butPtr->tkwin)
	&& !(butPtr->flags & REDRAW_PENDING)) {
	Tcl_DoWhenIdle(DisplayButton, (ClientData)butPtr);
	butPtr->flags |= REDRAW_PENDING;
    }
    return (char *) NULL;
}

/*
 *---------------------------------------------------------------------------
 *
 * ButtonImageProc --
 *
 *	This procedure is invoked by the image code whenever the manager
 *	for an image does something that affects the size of contents
 *	of an image displayed in a button.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Arranges for the button to get redisplayed.
 *
 *---------------------------------------------------------------------------
 */

/*ARGSUSED*/
static void
ButtonImageProc(
    ClientData clientData,	/* Pointer to widget record. */
    int x, int y,		/* Upper left pixel (within image)
				 * that must be redisplayed. */
    int width, int height,	/* Dimensions of area to redisplay
				 * (may be <= 0). */
    int imgWidth, int imgHeight)/* New dimensions of image. */
{
    Button *butPtr = clientData;

    if (butPtr->tkwin != NULL) {
	ComputeButtonGeometry(butPtr);
	if (Tk_IsMapped(butPtr->tkwin) && !(butPtr->flags & REDRAW_PENDING)) {
	    Tcl_DoWhenIdle(DisplayButton, (ClientData)butPtr);
	    butPtr->flags |= REDRAW_PENDING;
	}
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * ButtonSelectImageProc --
 *
 *	This procedure is invoked by the image code whenever the manager
 *	for an image does something that affects the size of contents
 *	of the image displayed in a button when it is selected.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	May arrange for the button to get redisplayed.
 *
 *---------------------------------------------------------------------------
 */

/*ARGSUSED*/
static void
ButtonSelectImageProc(
    ClientData clientData,	/* Pointer to widget record. */
    int x, int y,		/* Upper left pixel (within image)
				 * that must be redisplayed. */
    int width, int height,	/* Dimensions of area to redisplay
				 * (may be <= 0). */
    int imgWidth, int imgHeight)/* New dimensions of image. */
{
    Button *butPtr = clientData;

    /*
     * Don't recompute geometry:  it's controlled by the primary image.
     */

    if ((butPtr->flags & SELECTED) && (butPtr->tkwin != NULL)
	&& Tk_IsMapped(butPtr->tkwin)
	&& !(butPtr->flags & REDRAW_PENDING)) {
	Tcl_DoWhenIdle(DisplayButton, (ClientData)butPtr);
	butPtr->flags |= REDRAW_PENDING;
    }
}

int
Blt_ButtonCmdInitProc(Tcl_Interp *interp)
{
    static Blt_InitCmdSpec cmdSpecs[4] = {
	{"button", ButtonCmd,},
	{"checkbutton", CheckbuttonCmd,},
	{"radiobutton", RadiobuttonCmd,},
	{"label", LabelCmd,},
    };
    return Blt_InitCmds(interp, "::blt::tile", cmdSpecs, 4);
}

#endif /* NO_TILEBUTTON */
