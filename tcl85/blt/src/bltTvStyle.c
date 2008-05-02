
/*
 * bltTvStyle.c --
 *
 * This module implements styles for treeview widget cells.
 *
 *	Copyright 1998-2004 George A Howlett.
 *
 *	Permission is hereby granted, free of charge, to any person
 *	obtaining a copy of this software and associated documentation
 *	files (the "Software"), to deal in the Software without
 *	restriction, including without limitation the rights to use,
 *	copy, modify, merge, publish, distribute, sublicense, and/or
 *	sell copies of the Software, and to permit persons to whom the
 *	Software is furnished to do so, subject to the following
 *	conditions:
 *
 *	The above copyright notice and this permission notice shall be
 *	included in all copies or substantial portions of the
 *	Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 *	KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *	WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 *	PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
 *	OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 *	OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 *	OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 *	SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "bltInt.h"

#ifndef NO_TREEVIEW
#include "bltOp.h"
#include "bltTreeView.h"
#include "bltList.h"
#include <X11/Xutil.h>
#include <X11/Xatom.h>

typedef int (TvCmdProc)(Blt_Tv *tvPtr, Tcl_Interp *interp, int objc, 
	Tcl_Obj *const *objv);

#define STYLE_GAP		2
#define ARROW_WIDTH		13

static Blt_OptionParseProc ObjToIcon;
static Blt_OptionPrintProc IconToObj;
static Blt_OptionFreeProc FreeIcon;
Blt_CustomOption bltTvIconOption =
{
    /* Contains a pointer to the widget that's currently being
     * configured.  This is used in the custom configuration parse
     * routine for icons.  */
    ObjToIcon, IconToObj, FreeIcon, NULL,
};

#define DEF_STYLE_HIGHLIGHT_BACKGROUND	STD_NORMAL_BACKGROUND
#define DEF_STYLE_HIGHLIGHT_FOREGROUND	STD_NORMAL_FOREGROUND
#ifdef WIN32
#define DEF_STYLE_ACTIVE_BACKGROUND	RGB_GREY85
#else
#define DEF_STYLE_ACTIVE_BACKGROUND	RGB_GREY95
#endif
#define DEF_STYLE_ACTIVE_FOREGROUND 	STD_ACTIVE_FOREGROUND
#define DEF_STYLE_GAP			"2"

typedef struct {
    int refCount;		/* Usage reference count. */
    unsigned int flags;
    const char *name;			
    Blt_Tv_StyleClass *classPtr; /* Class-specific routines to manage style. */
    Blt_HashEntry *hashPtr;
    Blt_ChainLink link;

    /* General style fields. */
    Tk_Cursor cursor;		/* X Cursor */
    Blt_Tv_Icon icon;		/* If non-NULL, is a Tk_Image to be drawn
				 * in the cell. */
    int gap;			/* # pixels gap between icon and text. */
    Blt_Font font;
    XColor *fgColor;		/* Normal foreground color of cell. */
    XColor *highlightFgColor;	/* Foreground color of cell when
				 * highlighted. */
    XColor *activeFgColor;	/* Foreground color of cell when active. */
    XColor *selInFocusFgColor;	/* Foreground color of a selected
				 * cell. If non-NULL, overrides
				 * default foreground color
				 * specification. */

    Blt_Background bg;		/* Normal background color of cell. */
    Blt_Background highlightBg;	/* Background color of cell when
				 * highlighted. */
    Blt_Background activeBg;	/* Background color of cell when active. */

    Blt_Background selInFocusBg;/* Background color of a selected
				 * cell.  If non-NULL, overrides the
				 * default background color
				 * specification. */
    const char *validateCmd;

    /* TextBox-specific fields */
    GC gc;
    GC highlightGC;
    GC activeGC;
    int side;			/* Position of the text in relation to
				 * the icon.  */
    Blt_TreeKey key;		/* Actual data resides in this tree
				   value. */

} Blt_Tv_TextBox;

#ifdef WIN32
#define DEF_TEXTBOX_CURSOR		"arrow"
#else
#define DEF_TEXTBOX_CURSOR		"hand2"
#endif /*WIN32*/
#define DEF_TEXTBOX_SIDE		"left"
#define DEF_TEXTBOX_VALIDATE_COMMAND	(char *)NULL

static Blt_ConfigSpec textBoxSpecs[] =
{
    {BLT_CONFIG_BACKGROUND, "-activebackground", "activeBackground", 
	"ActiveBackground", DEF_STYLE_ACTIVE_BACKGROUND, 
	Blt_Offset(Blt_Tv_TextBox, activeBg), 0},
    {BLT_CONFIG_SYNONYM, "-activebg", "activeBackground", 
	(char *)NULL, (char *)NULL, 0, 0},
    {BLT_CONFIG_SYNONYM, "-activefg", "activeFackground", 
	(char *)NULL, (char *)NULL, 0, 0},
    {BLT_CONFIG_COLOR, "-activeforeground", "activeForeground", 
	"ActiveForeground", DEF_STYLE_ACTIVE_FOREGROUND, 
	Blt_Offset(Blt_Tv_TextBox, activeFgColor), 0},
    {BLT_CONFIG_BACKGROUND, "-background", "background", "Background",
	(char *)NULL, Blt_Offset(Blt_Tv_TextBox, bg), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_SYNONYM, "-bg", "background", (char *)NULL, (char *)NULL, 
	0, 0},
    {BLT_CONFIG_CURSOR, "-cursor", "cursor", "Cursor",
	DEF_TEXTBOX_CURSOR, Blt_Offset(Blt_Tv_TextBox, cursor), 0},
    {BLT_CONFIG_BITMASK, "-edit", "edit", "Edit", (char *)NULL, 
	Blt_Offset(Blt_Tv_TextBox, flags), BLT_CONFIG_DONT_SET_DEFAULT,
	(Blt_CustomOption *)STYLE_EDITABLE},
    {BLT_CONFIG_SYNONYM, "-fg", "foreground", (char *)NULL, (char *)NULL, 
	0, 0},
    {BLT_CONFIG_FONT, "-font", "font", "Font",
	(char *)NULL, Blt_Offset(Blt_Tv_TextBox, font), 
        BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_COLOR, "-foreground", "foreground", "Foreground",
	(char *)NULL, Blt_Offset(Blt_Tv_TextBox, fgColor),BLT_CONFIG_NULL_OK },
    {BLT_CONFIG_PIXELS_NNEG, "-gap", "gap", "Gap",
	DEF_STYLE_GAP, Blt_Offset(Blt_Tv_TextBox, gap), 
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_BACKGROUND, "-highlightbackground", "highlightBackground",
	"HighlightBackground", DEF_STYLE_HIGHLIGHT_BACKGROUND, 
        Blt_Offset(Blt_Tv_TextBox, highlightBg), BLT_CONFIG_COLOR_ONLY},
    {BLT_CONFIG_COLOR, "-highlightforeground", "highlightForeground", 
	"HighlightForeground", DEF_STYLE_HIGHLIGHT_FOREGROUND, 
	 Blt_Offset(Blt_Tv_TextBox, highlightFgColor), 0},
    {BLT_CONFIG_SYNONYM, "-highlightbg", "highlightBackground", 
	(char *)NULL, (char *)NULL, 0, 0},
    {BLT_CONFIG_SYNONYM, "-highlightfg", "highlightForeground", 
	(char *)NULL, (char *)NULL, 0, 0},
    {BLT_CONFIG_CUSTOM, "-icon", "icon", "Icon", (char *)NULL, 
	Blt_Offset(Blt_Tv_TextBox, icon), BLT_CONFIG_NULL_OK, 
	&bltTvIconOption},
    {BLT_CONFIG_STRING, "-key", "key", "key", 	(char *)NULL, 
	Blt_Offset(Blt_Tv_TextBox, key), BLT_CONFIG_NULL_OK, 0},
    {BLT_CONFIG_BACKGROUND, "-selectbackground", "selectBackground", 
	"Foreground", (char *)NULL, Blt_Offset(Blt_Tv_TextBox, selInFocusBg), 
        0},
    {BLT_CONFIG_COLOR, "-selectforeground", "selectForeground", "Background",
	(char *)NULL, Blt_Offset(Blt_Tv_TextBox, selInFocusFgColor), 0},
    {BLT_CONFIG_SIDE, "-side", "side", "side", DEF_TEXTBOX_SIDE, 
	Blt_Offset(Blt_Tv_TextBox, side), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_STRING, "-validatecommand", "validateCommand", 
	"ValidateCommand", DEF_TEXTBOX_VALIDATE_COMMAND, 
	Blt_Offset(Blt_Tv_TextBox, validateCmd), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_END, (char *)NULL, (char *)NULL, (char *)NULL, (char *)NULL, 
	0, 0}
};

typedef struct {
    int refCount;		/* Usage reference count. */
    unsigned int flags;		/* Contains style type and update flags. */
    const char *name;		/* Instance name. */
    Blt_Tv_StyleClass *classPtr; /* Class-specific routines to manage style. */
    Blt_HashEntry *hashPtr;	
    Blt_ChainLink link;

    /* General style fields. */
    Tk_Cursor cursor;		/* X Cursor */
    Blt_Tv_Icon icon;		/* If non-NULL, is a Tk_Image to be drawn
				 * in the cell. */
    int gap;			/* # pixels gap between icon and text. */
    Blt_Font font;
    XColor *fgColor;		/* Normal foreground color of cell. */
    XColor *highlightFgColor;	/* Foreground color of cell when
				 * highlighted. */
    XColor *activeFgColor;	/* Foreground color of cell when active. */
    XColor *selInFocusFgColor;	/* Foreground color of a selected
				 * cell. If non-NULL, overrides
				 * default foreground color
				 * specification. */

    Blt_Background bg;		/* Normal background color of cell. */
    Blt_Background highlightBg;	/* Background color of cell when
				 * highlighted. */
    Blt_Background activeBg;	/* Background color of cell when active. */

    Blt_Background selInFocusBg;/* Background color of a selected
				 * cell.  If non-NULL, overrides the
				 * default background color
				 * specification. */
    const char *validateCmd;

    /* Checkbox specific fields. */
    GC gc;
    GC highlightGC;
    GC activeGC;

    Blt_TreeKey key;		/* Actual data resides in this tree
				   value. */
    int size;			/* Size of the checkbox. */
    int showValue;		/* If non-zero, display the on/off value.  */
    const char *onValue;
    const char *offValue;
    int lineWidth;		/* Linewidth of the surrounding box. */

    XColor *boxColor;		/* Rectangle (box) color (grey). */
    XColor *fillColor;		/* Fill color (white) */
    XColor *checkColor;		/* Check color (red). */

    GC boxGC;
    GC fillGC;			/* Box fill GC */
    GC checkGC;

    TextLayout *onPtr, *offPtr;
    
} Blt_Tv_CheckBox;

#define DEF_CHECKBOX_BOX_COLOR		"black"
#define DEF_CHECKBOX_CHECK_COLOR	"red"
#define DEF_CHECKBOX_FILL_COLOR		"white"
#define DEF_CHECKBOX_OFFVALUE		"0"
#define DEF_CHECKBOX_ONVALUE		"1"
#define DEF_CHECKBOX_SHOWVALUE		"yes"
#define DEF_CHECKBOX_SIZE		"11"
#define DEF_CHECKBOX_LINEWIDTH		"2"
#define DEF_CHECKBOX_GAP		"4"
#ifdef WIN32
#define DEF_CHECKBOX_CURSOR		"arrow"
#else
#define DEF_CHECKBOX_CURSOR		"hand2"
#endif /*WIN32*/

static Blt_ConfigSpec checkBoxSpecs[] =
{
    {BLT_CONFIG_BACKGROUND, "-activebackground", "activeBackground", 
	"ActiveBackground", DEF_STYLE_ACTIVE_BACKGROUND, 
	Blt_Offset(Blt_Tv_CheckBox, activeBg), 0},
    {BLT_CONFIG_SYNONYM, "-activebg", "activeBackground", 
	(char *)NULL, (char *)NULL, 0, 0},
    {BLT_CONFIG_SYNONYM, "-activefg", "activeFackground", 
	(char *)NULL, (char *)NULL, 0, 0},
    {BLT_CONFIG_COLOR, "-activeforeground", "activeForeground", 
	"ActiveForeground", DEF_STYLE_ACTIVE_FOREGROUND, 
	Blt_Offset(Blt_Tv_CheckBox, activeFgColor), 0},
    {BLT_CONFIG_BACKGROUND, "-background", "background", "Background",
	(char *)NULL, Blt_Offset(Blt_Tv_CheckBox, bg), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_SYNONYM, "-bg", "background", (char *)NULL, (char *)NULL, 
	0, 0},
    {BLT_CONFIG_PIXELS_POS, "-boxsize", "boxSize", "BoxSize",
	DEF_CHECKBOX_SIZE, Blt_Offset(Blt_Tv_CheckBox, size), 
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_CURSOR, "-cursor", "cursor", "Cursor",
	DEF_CHECKBOX_CURSOR, Blt_Offset(Blt_Tv_TextBox, cursor), 0},
    {BLT_CONFIG_SYNONYM, "-fg", "foreground", (char *)NULL, (char *)NULL, 
	0, 0},
    {BLT_CONFIG_FONT, "-font", "font", "Font",
	(char *)NULL, Blt_Offset(Blt_Tv_CheckBox, font), 
        BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_COLOR, "-foreground", "foreground", "Foreground",
	(char *)NULL, Blt_Offset(Blt_Tv_CheckBox, fgColor), 
        BLT_CONFIG_NULL_OK },
    {BLT_CONFIG_PIXELS_NNEG, "-gap", "gap", "Gap",
	DEF_CHECKBOX_GAP, Blt_Offset(Blt_Tv_CheckBox, gap), 
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_BACKGROUND, "-highlightbackground", "highlightBackground",
	"HighlightBackground", DEF_STYLE_HIGHLIGHT_BACKGROUND, 
        Blt_Offset(Blt_Tv_CheckBox, highlightBg), BLT_CONFIG_COLOR_ONLY},
    {BLT_CONFIG_COLOR, "-highlightforeground", "highlightForeground", 
	"HighlightForeground", DEF_STYLE_HIGHLIGHT_FOREGROUND, 
	 Blt_Offset(Blt_Tv_CheckBox, highlightFgColor), 0},
    {BLT_CONFIG_SYNONYM, "-highlightbg", "highlightBackground", 
	(char *)NULL, (char *)NULL, 0, 0},
    {BLT_CONFIG_SYNONYM, "-highlightfg", "highlightForeground", 
	(char *)NULL, (char *)NULL, 0, 0},
    {BLT_CONFIG_CUSTOM, "-icon", "icon", "Icon", (char *)NULL, 
	Blt_Offset(Blt_Tv_CheckBox, icon), BLT_CONFIG_NULL_OK, 
	&bltTvIconOption},
    {BLT_CONFIG_STRING, "-key", "key", "key", (char *)NULL, 
	Blt_Offset(Blt_Tv_CheckBox, key), BLT_CONFIG_NULL_OK, 0},
    {BLT_CONFIG_PIXELS_NNEG, "-linewidth", "lineWidth", "LineWidth",
	DEF_CHECKBOX_LINEWIDTH, Blt_Offset(Blt_Tv_CheckBox, lineWidth),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_COLOR, "-checkcolor", "checkColor", "CheckColor", 
	DEF_CHECKBOX_CHECK_COLOR, Blt_Offset(Blt_Tv_CheckBox, checkColor), 0},
    {BLT_CONFIG_COLOR, "-boxcolor", "boxColor", "BoxColor", 
	DEF_CHECKBOX_BOX_COLOR, Blt_Offset(Blt_Tv_CheckBox, boxColor), 0},
    {BLT_CONFIG_COLOR, "-fillcolor", "fillColor", "FillColor", 
	DEF_CHECKBOX_FILL_COLOR, Blt_Offset(Blt_Tv_CheckBox, fillColor), 0},
    {BLT_CONFIG_STRING, "-offvalue", "offValue", "OffValue",
	DEF_CHECKBOX_OFFVALUE, Blt_Offset(Blt_Tv_CheckBox, offValue), 
        BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_STRING, "-onvalue", "onValue", "OnValue",
	DEF_CHECKBOX_ONVALUE, Blt_Offset(Blt_Tv_CheckBox, onValue), 
        BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_STRING, "-key", "key", "key",
	(char *)NULL, Blt_Offset(Blt_Tv_CheckBox, key), BLT_CONFIG_NULL_OK, 0},
    {BLT_CONFIG_BACKGROUND, "-selectbackground", "selectBackground", 
	"Foreground", (char *)NULL, 
	Blt_Offset(Blt_Tv_CheckBox, selInFocusBg), 0},
    {BLT_CONFIG_COLOR, "-selectforeground", "selectForeground", "Background",
	(char *)NULL, Blt_Offset(Blt_Tv_CheckBox, selInFocusFgColor), 0},
    {BLT_CONFIG_BOOLEAN, "-showvalue", "showValue", "ShowValue",
	DEF_CHECKBOX_SHOWVALUE, Blt_Offset(Blt_Tv_CheckBox, showValue), 
        BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_END, (char *)NULL, (char *)NULL, (char *)NULL,
	(char *)NULL, 0, 0}
};

typedef struct {
    int refCount;		/* Usage reference count. */
    unsigned int flags;
    const char *name;			
    Blt_Tv_StyleClass *classPtr;/* Class-specific style routines. */
    Blt_HashEntry *hashPtr;
    Blt_ChainLink link;

    /* General style fields. */
    Tk_Cursor cursor;		/* X Cursor */
    Blt_Tv_Icon icon;		/* If non-NULL, is a Tk_Image to be drawn
				 * in the cell. */
    int gap;			/* # pixels gap between icon and text. */
    Blt_Font font;
    XColor *fgColor;		/* Normal foreground color of cell. */
    XColor *highlightFgColor;	/* Foreground color of cell when
				 * highlighted. */
    XColor *activeFgColor;	/* Foreground color of cell when active. */
    XColor *selInFocusFgColor;	/* Foreground color of a selected
				 * cell. If non-NULL, overrides
				 * default foreground color
				 * specification. */

    Blt_Background bg;		/* Normal background color of cell. */
    Blt_Background highlightBg;	/* Background color of cell when
				 * highlighted. */
    Blt_Background activeBg;	/* Background color of cell when active. */

    Blt_Background selInFocusBg;/* Background color of a selected
				 * cell.  If non-NULL, overrides the
				 * default background color
				 * specification. */
    const char *validateCmd;

    /* ComboBox-specific fields */
    GC gc;
    GC highlightGC;
    GC activeGC;

    int borderWidth;		/* Width of outer border surrounding
				 * the entire box. */
    int relief;			/* Relief of outer border. */

    Blt_TreeKey key;		/* Actual data resides in this tree
				   value. */

    const char *choices;	/* List of available choices. */
    const char *choiceIcons;	/* List of icons associated with choices. */
    int scrollWidth;
    int button;
    int buttonWidth;
    int buttonBorderWidth;	/* Border width of button. */
    int buttonRelief;		/* Normal relief of button. */

} Blt_Tv_ComboBox;

#define DEF_COMBOBOX_BORDERWIDTH	"1"
#define DEF_COMBOBOX_BUTTON_BORDERWIDTH	"1"
#define DEF_COMBOBOX_BUTTON_RELIEF	"raised"
#define DEF_COMBOBOX_RELIEF		"flat"
#ifdef WIN32
#define DEF_COMBOBOX_CURSOR		"arrow"
#else
#define DEF_COMBOBOX_CURSOR		"hand2"
#endif /*WIN32*/


static Blt_ConfigSpec comboBoxSpecs[] =
{
    {BLT_CONFIG_BACKGROUND, "-activebackground", "activeBackground", 
	"ActiveBackground", DEF_STYLE_ACTIVE_BACKGROUND, 
	Blt_Offset(Blt_Tv_ComboBox, activeBg), 0},
    {BLT_CONFIG_SYNONYM, "-activebg", "activeBackground", 
	(char *)NULL, (char *)NULL, 0, 0},
    {BLT_CONFIG_SYNONYM, "-activefg", "activeFackground", 
	(char *)NULL, (char *)NULL, 0, 0},
    {BLT_CONFIG_COLOR, "-activeforeground", "activeForeground", 
	"ActiveForeground", DEF_STYLE_ACTIVE_FOREGROUND, 
	Blt_Offset(Blt_Tv_ComboBox, activeFgColor), 0},
    {BLT_CONFIG_BACKGROUND, "-background", "background", "Background",
	(char *)NULL, Blt_Offset(Blt_Tv_ComboBox, bg), 
	BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_SYNONYM, "-bd", "borderWidth", (char *)NULL, (char *)NULL, 0, 
	0},
    {BLT_CONFIG_SYNONYM, "-bg", "background", (char *)NULL, (char *)NULL, 
	0, 0},
    {BLT_CONFIG_PIXELS_NNEG, "-borderwidth", "borderWidth", "BorderWidth",
	DEF_COMBOBOX_BORDERWIDTH, Blt_Offset(Blt_Tv_ComboBox, borderWidth),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_RELIEF, "-buttonrelief", "buttonRelief", "ButtonRelief",
	DEF_COMBOBOX_BUTTON_RELIEF, Blt_Offset(Blt_Tv_ComboBox, buttonRelief),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_PIXELS_NNEG, "-buttonborderwidth", "buttonBorderWidth", 
	"ButtonBorderWidth", DEF_COMBOBOX_BUTTON_BORDERWIDTH, 
	Blt_Offset(Blt_Tv_ComboBox, buttonBorderWidth),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_CURSOR, "-cursor", "cursor", "Cursor",
	DEF_COMBOBOX_CURSOR, Blt_Offset(Blt_Tv_TextBox, cursor), 0},
    {BLT_CONFIG_SYNONYM, "-fg", "foreground", (char *)NULL, (char *)NULL, 
	0, 0},
    {BLT_CONFIG_FONT, "-font", "font", "Font",
	(char *)NULL, Blt_Offset(Blt_Tv_ComboBox, font), 
	BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_COLOR, "-foreground", "foreground", "Foreground",
	(char *)NULL, Blt_Offset(Blt_Tv_ComboBox, fgColor), 
	BLT_CONFIG_NULL_OK },
    {BLT_CONFIG_PIXELS_NNEG, "-gap", "gap", "Gap",
	DEF_STYLE_GAP, Blt_Offset(Blt_Tv_ComboBox, gap), 
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_BACKGROUND, "-highlightbackground", "highlightBackground",
	"HighlightBackground", DEF_STYLE_HIGHLIGHT_BACKGROUND, 
        Blt_Offset(Blt_Tv_ComboBox, highlightBg), BLT_CONFIG_COLOR_ONLY},
    {BLT_CONFIG_COLOR, "-highlightforeground", "highlightForeground", 
	"HighlightForeground", DEF_STYLE_HIGHLIGHT_FOREGROUND, 
	 Blt_Offset(Blt_Tv_ComboBox, highlightFgColor), 0},
    {BLT_CONFIG_SYNONYM, "-highlightbg", "highlightBackground", 
	(char *)NULL, (char *)NULL, 0, 0},
    {BLT_CONFIG_SYNONYM, "-highlightfg", "highlightForeground", 
	(char *)NULL, (char *)NULL, 0, 0},
    {BLT_CONFIG_CUSTOM, "-icon", "icon", "Icon", (char *)NULL, 
	Blt_Offset(Blt_Tv_ComboBox, icon), BLT_CONFIG_NULL_OK, 
	&bltTvIconOption},
    {BLT_CONFIG_STRING, "-key", "key", "key", (char *)NULL, 
	Blt_Offset(Blt_Tv_ComboBox, key), BLT_CONFIG_NULL_OK, 0},
    {BLT_CONFIG_RELIEF, "-relief", "relief", "Relief",
	DEF_COMBOBOX_RELIEF, Blt_Offset(Blt_Tv_ComboBox, relief),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_BACKGROUND, "-selectbackground", "selectBackground", 
	"Foreground", (char *)NULL, Blt_Offset(Blt_Tv_ComboBox, selInFocusBg),
	0},
    {BLT_CONFIG_COLOR, "-selectforeground", "selectForeground", "Background",
	(char *)NULL, Blt_Offset(Blt_Tv_ComboBox, selInFocusFgColor), 0},
    {BLT_CONFIG_END, (char *)NULL, (char *)NULL, (char *)NULL,
	(char *)NULL, 0, 0}
};

typedef Blt_Tv_Style *(StyleCreateProc) _ANSI_ARGS_((Blt_Tv *tvPtr, 
	Blt_HashEntry *hPtr));

static StyleConfigProc ConfigureTextBox, ConfigureCheckBox, ConfigureComboBox;
static StyleCreateProc CreateTextBox, CreateCheckBox, CreateComboBox;
static StyleDrawProc DrawTextBox, DrawCheckBox, DrawComboBox;
static StyleEditProc EditTextBox, EditCheckBox, EditComboBox;
static StyleFreeProc FreeTextBox, FreeCheckBox, FreeComboBox;
static StyleMeasureProc MeasureTextBox, MeasureCheckBox, MeasureComboBox;
#ifdef notdef
static StylePickProc PickCheckBox;
#endif
static StylePickProc PickComboBox;

/*
 *---------------------------------------------------------------------------
 *
 * ObjToIcon --
 *
 *	Convert the name of an icon into a Tk image.
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
ObjToIcon(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,		/* Interpreter to send results back to */
    Tk_Window tkwin,		/* Not used. */
    Tcl_Obj *objPtr,		/* Tcl_Obj representing the new value. */
    char *widgRec,
    int offset,			/* Offset to field in structure */
    int flags)	
{
    Blt_Tv *tvPtr = clientData;
    Blt_Tv_Icon *iconPtr = (Blt_Tv_Icon *)(widgRec + offset);
    Blt_Tv_Icon icon;

    icon = Blt_Tv_GetIcon(tvPtr, Tcl_GetString(objPtr));
    if (icon == NULL) {
	return TCL_ERROR;
    }
    *iconPtr = icon;
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * IconToObj --
 *
 *	Converts the icon into its string representation (its name).
 *
 * Results:
 *	The name of the icon is returned.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
IconToObj(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,
    Tk_Window tkwin,		/* Not used. */
    char *widgRec,
    int offset,			/* Offset to field in structure */
    int flags)	
{
    Blt_Tv_Icon icon = *(Blt_Tv_Icon *)(widgRec + offset);

    if (icon == NULL) {
	return Tcl_NewStringObj("", -1);
    } else {
	return Tcl_NewStringObj(Blt_NameOfImage((icon)->tkImage), -1);
    }
}

/*ARGSUSED*/
static void
FreeIcon(
    ClientData clientData,
    Display *display,		/* Not used. */
    char *widgRec,
    int offset)
{
    Blt_Tv_Icon *iconPtr = (Blt_Tv_Icon *)(widgRec + offset);
    Blt_Tv *tvPtr = clientData;

    if (*iconPtr != NULL) {
	Blt_Tv_FreeIcon(tvPtr, *iconPtr);
	*iconPtr = NULL;
    }
}

static Blt_Tv_StyleClass textBoxClass = {
    "TextBoxStyle",
    textBoxSpecs,
    ConfigureTextBox,
    MeasureTextBox,
    DrawTextBox,
    NULL,
    EditTextBox,
    FreeTextBox,
};

static Blt_Tv_StyleClass checkBoxClass = {
    "CheckBoxStyle",
    checkBoxSpecs,
    ConfigureCheckBox,
    MeasureCheckBox,
    DrawCheckBox,
    NULL,
    EditCheckBox,
    FreeCheckBox,
};

static Blt_Tv_StyleClass comboBoxClass = {
    "ComboBoxStyle", 
    comboBoxSpecs,
    ConfigureComboBox,
    MeasureComboBox,
    DrawComboBox,
    PickComboBox,
    EditComboBox,
    FreeComboBox,
};

/*
 *---------------------------------------------------------------------------
 *
 * CreateTextBox --
 *
 *	Creates a "textbox" style.
 *
 * Results:
 *	A pointer to the new style structure.
 *
 *---------------------------------------------------------------------------
 */
static Blt_Tv_Style *
CreateTextBox(Blt_Tv *tvPtr, Blt_HashEntry *hPtr)
{
    Blt_Tv_TextBox *tbPtr;

    tbPtr = Blt_CallocAssert(1, sizeof(Blt_Tv_TextBox));
    tbPtr->classPtr = &textBoxClass;
    tbPtr->side = SIDE_LEFT;
    tbPtr->gap = STYLE_GAP;
    tbPtr->name = Blt_StrdupAssert(Blt_GetHashKey(&tvPtr->styleTable, hPtr));
    tbPtr->hashPtr = hPtr;
    tbPtr->link = NULL;
    tbPtr->flags = STYLE_TEXTBOX;
    tbPtr->refCount = 1;
    Blt_SetHashValue(hPtr, tbPtr);
    return (Blt_Tv_Style *)tbPtr;
}

/*
 *---------------------------------------------------------------------------
 *
 * ConfigureTextBox --
 *
 *	Configures a "textbox" style.  This routine performs 
 *	generates the GCs required for a textbox style.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	GCs are created for the style.
 *
 *---------------------------------------------------------------------------
 */
static void
ConfigureTextBox(Blt_Tv *tvPtr, Blt_Tv_Style *stylePtr)
{
    Blt_Tv_TextBox *tbPtr = (Blt_Tv_TextBox *)stylePtr;
    GC newGC;
    XGCValues gcValues;
    unsigned long gcMask;

    gcMask = GCForeground | GCFont;
    gcValues.font = Blt_FontId(CHOOSE(tvPtr->font, tbPtr->font));

    /* Normal GC */
    gcValues.foreground = CHOOSE(tvPtr->fgColor, tbPtr->fgColor)->pixel;
    newGC = Tk_GetGC(tvPtr->tkwin, gcMask, &gcValues);
    if (tbPtr->gc != NULL) {
	Tk_FreeGC(tvPtr->display, tbPtr->gc);
    }
    tbPtr->gc = newGC;

    /* Highlight GC  */
    gcValues.foreground = tbPtr->highlightFgColor->pixel;
    newGC = Tk_GetGC(tvPtr->tkwin, gcMask, &gcValues);
    if (tbPtr->highlightGC != NULL) {
	Tk_FreeGC(tvPtr->display, tbPtr->highlightGC);
    }
    tbPtr->highlightGC = newGC;

    /* Active GC  */
    gcValues.foreground = tbPtr->activeFgColor->pixel;
    newGC = Tk_GetGC(tvPtr->tkwin, gcMask, &gcValues);
    if (tbPtr->activeGC != NULL) {
	Tk_FreeGC(tvPtr->display, tbPtr->activeGC);
    }
    tbPtr->activeGC = newGC;
    tbPtr->flags |= STYLE_DIRTY;
}

/*
 *---------------------------------------------------------------------------
 *
 * MeasureTextBox --
 *
 *	Determines the space requirements for the "textbox" given
 *	the value to be displayed.  Depending upon whether an icon
 *	or text is displayed and their relative placements, this
 *	routine computes the space needed for the text entry.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The width and height fields of *valuePtr* are set with the
 *	computed dimensions.
 *
 *---------------------------------------------------------------------------
 */
static void
MeasureTextBox(
    Blt_Tv *tvPtr, 
    Blt_Tv_Style *stylePtr, 
    Blt_Tv_Value *valuePtr)
{
    Blt_Tv_TextBox *tbPtr = (Blt_Tv_TextBox *)stylePtr;
    int iconWidth, iconHeight;
    int textWidth, textHeight;
    int gap;

    textWidth = textHeight = 0;
    iconWidth = iconHeight = 0;
    valuePtr->width = valuePtr->height = 0;

    if (tbPtr->icon != NULL) {
	iconWidth = Blt_Tv_IconWidth(tbPtr->icon);
	iconHeight = Blt_Tv_IconHeight(tbPtr->icon);
    } 
    if (valuePtr->textPtr != NULL) {
	Blt_Free(valuePtr->textPtr);
	valuePtr->textPtr = NULL;
    }
    if (valuePtr->string != NULL) {	/* New string defined. */
	TextStyle ts;

	Blt_Ts_InitStyle(ts);
	Blt_Ts_SetFont(ts, CHOOSE(tvPtr->font, tbPtr->font));
	valuePtr->textPtr = Blt_Ts_CreateLayout(valuePtr->string, -1, &ts);
    } 
    gap = 0;
    if (valuePtr->textPtr != NULL) {
	textWidth = valuePtr->textPtr->width;
	textHeight = valuePtr->textPtr->height;
	if (tbPtr->icon != NULL) {
	    gap = tbPtr->gap;
	}
    }
    if (tbPtr->side & (SIDE_TOP | SIDE_BOTTOM)) {
	valuePtr->width = MAX(textWidth, iconWidth);
	valuePtr->height = iconHeight + gap + textHeight;
    } else {
	valuePtr->width = iconWidth + gap + textWidth;
	valuePtr->height = MAX(textHeight, iconHeight);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * DrawTextBox --
 *
 *	Draws the "textbox" given the screen coordinates and the
 *	value to be displayed.  
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The textbox value is drawn.
 *
 *---------------------------------------------------------------------------
 */
static void
DrawTextBox(
    Blt_Tv *tvPtr,
    Drawable drawable,
    Blt_Tv_Entry *entryPtr,
    Blt_Tv_Value *valuePtr,
    Blt_Tv_Style *stylePtr,
    int x, int y)
{
    Blt_Tv_Column *cp;
    Blt_Tv_TextBox *tbPtr = (Blt_Tv_TextBox *)stylePtr;
    int iconX, iconY, iconWidth, iconHeight;
    int textX, textY, textWidth, textHeight;
    int gap, columnWidth;
    Blt_Background bg;
    XColor *fgColor;

    cp = valuePtr->columnPtr;

    if (stylePtr->flags & STYLE_HIGHLIGHT) {
	bg = tbPtr->highlightBg;
	fgColor = tbPtr->highlightFgColor;
    } else {
	if (tbPtr->bg != NULL) {
	    bg = tbPtr->bg;
	} else {
	    bg = ((tvPtr->altBg != NULL) && (entryPtr->flatIndex & 0x1)) 
		? tvPtr->altBg : tvPtr->bg;
	}
	fgColor = CHOOSE(tvPtr->fgColor, tbPtr->fgColor);
    }
    if (Blt_Tv_EntryIsSelected(tvPtr, entryPtr)) {
	if (stylePtr->selInFocusBg != NULL) {
	    bg = stylePtr->selInFocusBg;
	} else {
	    bg = SELECT_FOCUS_BG(tvPtr);
	}
    } 
    /*
     * Draw the active or normal background color over the entire
     * label area.  This includes both the tab's text and image.
     * The rectangle should be 2 pixels wider/taller than this
     * area. So if the label consists of just an image, we get an
     * halo around the image when the tab is active.
     */
    if (bg != NULL) {
	Blt_FillBackgroundRectangle(tvPtr->tkwin, drawable, bg, x, y, 
		cp->width, entryPtr->height + 1, 0, TK_RELIEF_FLAT);
    }

    columnWidth = cp->width - 
	(2 * cp->borderWidth + PADDING(cp->pad));
    if (columnWidth > valuePtr->width) {
	switch(cp->justify) {
	case TK_JUSTIFY_RIGHT:
	    x += (columnWidth - valuePtr->width);
	    break;
	case TK_JUSTIFY_CENTER:
	    x += (columnWidth - valuePtr->width) / 2;
	    break;
	case TK_JUSTIFY_LEFT:
	    break;
	}
    }

    textX = textY = iconX = iconY = 0;	/* Suppress compiler warning. */
    
    iconWidth = iconHeight = 0;
    if (tbPtr->icon != NULL) {
	iconWidth = Blt_Tv_IconWidth(tbPtr->icon);
	iconHeight = Blt_Tv_IconHeight(tbPtr->icon);
    }
    textWidth = textHeight = 0;
    if (valuePtr->textPtr != NULL) {
	textWidth = valuePtr->textPtr->width;
	textHeight = valuePtr->textPtr->height;
    }
    gap = 0;
    if ((tbPtr->icon != NULL) && (valuePtr->textPtr != NULL)) {
	gap = tbPtr->gap;
    }
    switch (tbPtr->side) {
    case SIDE_RIGHT:
	textX = x;
	textY = y + (entryPtr->height - textHeight) / 2;
	iconX = textX + textWidth + gap;
	iconY = y + (entryPtr->height - iconHeight) / 2;
	break;
    case SIDE_LEFT:
	iconX = x;
	iconY = y + (entryPtr->height - iconHeight) / 2;
	textX = iconX + iconWidth + gap;
	textY = y + (entryPtr->height - textHeight) / 2;
	break;
    case SIDE_TOP:
	iconY = y;
	iconX = x + (columnWidth - iconWidth) / 2;
	textY = iconY + iconHeight + gap;
	textX = x + (columnWidth - textWidth) / 2;
	break;
    case SIDE_BOTTOM:
	textY = y;
	textX = x + (columnWidth - textWidth) / 2;
	iconY = textY + textHeight + gap;
	iconX = x + (columnWidth - iconWidth) / 2;
	break;
    }
    if (tbPtr->icon != NULL) {
	Tk_RedrawImage(Blt_Tv_IconBits(tbPtr->icon), 0, 0, iconWidth, 
		       iconHeight, drawable, iconX, iconY);
    }
    if (valuePtr->textPtr != NULL) {
	TextStyle ts;
	XColor *color;
	Blt_Font font;
	int xMax;

	font = CHOOSE(tvPtr->font, tbPtr->font);
	if (Blt_Tv_EntryIsSelected(tvPtr, entryPtr)) {
	    if (stylePtr->selInFocusFgColor != NULL) {
		color = stylePtr->selInFocusFgColor;
	    } else {
		color = SELECT_FG(tvPtr);
	    }
	} else if (entryPtr->color != NULL) {
	    color = entryPtr->color;
	} else {
	    color = fgColor;
	}
	Blt_Ts_InitStyle(ts);
	Blt_Ts_SetFont(ts, font);
	Blt_Ts_SetForeground(ts, color);
	xMax = SCREENX(tvPtr, cp->worldX) + cp->width - 
		cp->titleBorderWidth - cp->pad.side2;
	Blt_Ts_DrawLayout(tvPtr->tkwin, drawable, valuePtr->textPtr, &ts, 
		textX, textY, xMax);
    }
    stylePtr->flags &= ~STYLE_DIRTY;
}

/*
 *---------------------------------------------------------------------------
 *
 * EditCombobox --
 *
 *	Edits the "combobox".
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The checkbox value is drawn.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
EditTextBox(Blt_Tv *tvPtr, Blt_Tv_Entry *entryPtr, Blt_Tv_Value *valuePtr,
	    Blt_Tv_Style *stylePtr)
{
    return Blt_Tv_CreateTextbox(tvPtr, entryPtr, valuePtr->columnPtr);
}


/*
 *---------------------------------------------------------------------------
 *
 * FreeTextBox --
 *
 *	Releases resources allocated for the textbox. The resources
 *	freed by this routine are specific only to the "textbox".   
 *	Other resources (common to all styles) are freed in the 
 *	Blt_Tv_FreeStyle routine.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	GCs allocated for the textbox are freed.
 *
 *---------------------------------------------------------------------------
 */
static void
FreeTextBox(Blt_Tv *tvPtr, Blt_Tv_Style *stylePtr)
{
    Blt_Tv_TextBox *tbPtr = (Blt_Tv_TextBox *)stylePtr;

    if (tbPtr->highlightGC != NULL) {
	Tk_FreeGC(tvPtr->display, tbPtr->highlightGC);
    }
    if (tbPtr->activeGC != NULL) {
	Tk_FreeGC(tvPtr->display, tbPtr->activeGC);
    }
    if (tbPtr->gc != NULL) {
	Tk_FreeGC(tvPtr->display, tbPtr->gc);
    }
    if (tbPtr->icon != NULL) {
	Blt_Tv_FreeIcon(tvPtr, tbPtr->icon);
    }
}


/*
 *---------------------------------------------------------------------------
 *
 * CreateCheckbox --
 *
 *	Creates a "checkbox" style.
 *
 * Results:
 *	A pointer to the new style structure.
 *
 *---------------------------------------------------------------------------
 */
static Blt_Tv_Style *
CreateCheckBox(Blt_Tv *tvPtr, Blt_HashEntry *hPtr)
{
    Blt_Tv_CheckBox *cbPtr;

    cbPtr = Blt_CallocAssert(1, sizeof(Blt_Tv_CheckBox));
    cbPtr->classPtr = &checkBoxClass;
    cbPtr->gap = 4;
    cbPtr->size = 11;
    cbPtr->lineWidth = 2;
    cbPtr->showValue = TRUE;
    cbPtr->name = Blt_StrdupAssert(Blt_GetHashKey(&tvPtr->styleTable, hPtr));
    cbPtr->hashPtr = hPtr;
    cbPtr->link = NULL;
    cbPtr->flags = STYLE_CHECKBOX;
    cbPtr->refCount = 1;
    Blt_SetHashValue(hPtr, cbPtr);
    return (Blt_Tv_Style *)cbPtr;
}

/*
 *---------------------------------------------------------------------------
 *
 * ConfigureCheckbox --
 *
 *	Configures a "checkbox" style.  This routine performs 
 *	generates the GCs required for a checkbox style.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	GCs are created for the style.
 *
 *---------------------------------------------------------------------------
 */
static void
ConfigureCheckBox(Blt_Tv *tvPtr, Blt_Tv_Style *stylePtr)
{
    GC newGC;
    Blt_Tv_CheckBox *cbPtr = (Blt_Tv_CheckBox *)stylePtr;
    XColor *bgColor;
    XGCValues gcValues;
    unsigned long gcMask;

    gcMask = GCForeground | GCBackground | GCFont;
    gcValues.font = Blt_FontId(CHOOSE(tvPtr->font, cbPtr->font));
    bgColor = Blt_BackgroundBorderColor(CHOOSE(tvPtr->bg, cbPtr->bg));

    gcValues.background = bgColor->pixel;
    gcValues.foreground = CHOOSE(tvPtr->fgColor, cbPtr->fgColor)->pixel;
    newGC = Tk_GetGC(tvPtr->tkwin, gcMask, &gcValues);
    if (cbPtr->gc != NULL) {
	Tk_FreeGC(tvPtr->display, cbPtr->gc);
    }
    cbPtr->gc = newGC;
    gcValues.background = Blt_BackgroundBorderColor(cbPtr->highlightBg)->pixel;
    gcValues.foreground = cbPtr->highlightFgColor->pixel;
    newGC = Tk_GetGC(tvPtr->tkwin, gcMask, &gcValues);
    if (cbPtr->highlightGC != NULL) {
	Tk_FreeGC(tvPtr->display, cbPtr->highlightGC);
    }
    cbPtr->highlightGC = newGC;

    gcValues.background = Blt_BackgroundBorderColor(cbPtr->activeBg)->pixel;
    gcValues.foreground = cbPtr->activeFgColor->pixel;
    newGC = Tk_GetGC(tvPtr->tkwin, gcMask, &gcValues);
    if (cbPtr->activeGC != NULL) {
	Tk_FreeGC(tvPtr->display, cbPtr->activeGC);
    }
    cbPtr->activeGC = newGC;

    gcMask = GCForeground;
    gcValues.foreground = cbPtr->fillColor->pixel;
    newGC = Tk_GetGC(tvPtr->tkwin, gcMask, &gcValues);
    if (cbPtr->fillGC != NULL) {
	Tk_FreeGC(tvPtr->display, cbPtr->fillGC);
    }
    cbPtr->fillGC = newGC;

    gcMask = GCForeground | GCLineWidth;
    gcValues.line_width = cbPtr->lineWidth;
    gcValues.foreground = cbPtr->boxColor->pixel;
    newGC = Tk_GetGC(tvPtr->tkwin, gcMask, &gcValues);
    if (cbPtr->boxGC != NULL) {
	Tk_FreeGC(tvPtr->display, cbPtr->boxGC);
    }
    cbPtr->boxGC = newGC;

    gcMask = GCForeground | GCLineWidth;
    gcValues.line_width = 1;
    gcValues.foreground = cbPtr->checkColor->pixel;
    newGC = Tk_GetGC(tvPtr->tkwin, gcMask, &gcValues);
    if (cbPtr->checkGC != NULL) {
	Tk_FreeGC(tvPtr->display, cbPtr->checkGC);
    }
    cbPtr->checkGC = newGC;
    cbPtr->flags |= STYLE_DIRTY;
}

/*
 *---------------------------------------------------------------------------
 *
 * MeasureCheckbox --
 *
 *	Determines the space requirements for the "checkbox" given
 *	the value to be displayed.  Depending upon whether an icon
 *	or text is displayed and their relative placements, this
 *	routine computes the space needed for the text entry.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The width and height fields of *valuePtr* are set with the
 *	computed dimensions.
 *
 *---------------------------------------------------------------------------
 */
static void
MeasureCheckBox(
    Blt_Tv *tvPtr, 
    Blt_Tv_Style *stylePtr, 
    Blt_Tv_Value *valuePtr)
{
    Blt_Tv_CheckBox *cbPtr = (Blt_Tv_CheckBox *)stylePtr;
    int iconWidth, iconHeight;
    int textWidth, textHeight;
    int gap;
    int boxWidth, boxHeight;

    boxWidth = boxHeight = ODD(cbPtr->size);

    textWidth = textHeight = iconWidth = iconHeight = 0;
    valuePtr->width = valuePtr->height = 0;
    if (cbPtr->icon != NULL) {
	iconWidth = Blt_Tv_IconWidth(cbPtr->icon);
	iconHeight = Blt_Tv_IconHeight(cbPtr->icon);
    } 
    if (cbPtr->onPtr != NULL) {
	Blt_Free(cbPtr->onPtr);
	cbPtr->onPtr = NULL;
    }
    if (cbPtr->offPtr != NULL) {
	Blt_Free(cbPtr->offPtr);
	cbPtr->offPtr = NULL;
    }
    gap = 0;
    if (cbPtr->showValue) {
	TextStyle ts;
	const char *string;

	Blt_Ts_InitStyle(ts);
	Blt_Ts_SetFont(ts, CHOOSE(tvPtr->font, cbPtr->font));
	string = (cbPtr->onValue != NULL) ? cbPtr->onValue : valuePtr->string;
	cbPtr->onPtr = Blt_Ts_CreateLayout(string, -1, &ts);
	string = (cbPtr->offValue != NULL) ? cbPtr->offValue : valuePtr->string;
	cbPtr->offPtr = Blt_Ts_CreateLayout(string, -1, &ts);
	textWidth = MAX(cbPtr->offPtr->width, cbPtr->onPtr->width);
	textHeight = MAX(cbPtr->offPtr->height, cbPtr->onPtr->height);
	if (cbPtr->icon != NULL) {
	    gap = cbPtr->gap;
	}
    }
    valuePtr->width = cbPtr->gap * 2 + boxWidth + iconWidth + gap + textWidth;
    valuePtr->height = MAX3(boxHeight, textHeight, iconHeight);
}

/*
 *---------------------------------------------------------------------------
 *
 * DrawCheckbox --
 *
 *	Draws the "checkbox" given the screen coordinates and the
 *	value to be displayed.  
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The checkbox value is drawn.
 *
 *---------------------------------------------------------------------------
 */
static void
DrawCheckBox(
    Blt_Tv *tvPtr,
    Drawable drawable,
    Blt_Tv_Entry *entryPtr,
    Blt_Tv_Value *valuePtr,
    Blt_Tv_Style *stylePtr,
    int x, int y)
{
    Blt_Tv_Column *cp;
    Blt_Tv_CheckBox *cbPtr = (Blt_Tv_CheckBox *)stylePtr;
    int iconX, iconY, iconWidth, iconHeight;
    int textX, textY, textHeight;
    int gap, columnWidth;
    Blt_Background bg;
    XColor *fgColor;
    Blt_Font font;
    int bool;
    int borderWidth, relief;
    TextLayout *textPtr;
    int boxX, boxY, boxWidth, boxHeight;

    font = CHOOSE(tvPtr->font, cbPtr->font);
    cp = valuePtr->columnPtr;
    borderWidth = 0;
    relief = TK_RELIEF_FLAT;
    if (valuePtr == tvPtr->activeValuePtr) {
	bg = cbPtr->activeBg;
	fgColor = cbPtr->activeFgColor;
	borderWidth = 1;
	relief = TK_RELIEF_RAISED;
    } else if (stylePtr->flags & STYLE_HIGHLIGHT) {
	bg = cbPtr->highlightBg;
	fgColor = cbPtr->highlightFgColor;
    } else {
	bg = CHOOSE(tvPtr->bg, cbPtr->bg);
	fgColor = CHOOSE(tvPtr->fgColor, cbPtr->fgColor);
    }
    columnWidth = cp->width - PADDING(cp->pad);
    /* if (valuePtr == tvPtr->activeValuePtr) */ {
	/*
	 * Draw the active or normal background color over the entire
	 * label area.  This includes both the tab's text and image.
	 * The rectangle should be 2 pixels wider/taller than this
	 * area. So if the label consists of just an image, we get an
	 * halo around the image when the tab is active.
	 */
	if (Blt_Tv_EntryIsSelected(tvPtr, entryPtr)) {
	    if (stylePtr->selInFocusBg != NULL) {
		bg = stylePtr->selInFocusBg;
	    } else {
		bg = SELECT_FOCUS_BG(tvPtr);
	    }
	    Blt_FillBackgroundRectangle(tvPtr->tkwin, drawable, bg, x, y, 
		columnWidth, entryPtr->height - 1, borderWidth, relief);
	} else {
	    Blt_FillBackgroundRectangle(tvPtr->tkwin, drawable, bg, x, y, 
		columnWidth, entryPtr->height - 1, borderWidth, relief);
	}
    }    

    if (columnWidth > valuePtr->width) {
	switch(cp->justify) {
	case TK_JUSTIFY_RIGHT:
	    x += (columnWidth - valuePtr->width);
	    break;
	case TK_JUSTIFY_CENTER:
	    x += (columnWidth - valuePtr->width) / 2;
	    break;
	case TK_JUSTIFY_LEFT:
	    break;
	}
    }

    bool = (strcmp(valuePtr->string, cbPtr->onValue) == 0);
    textPtr = (bool) ? cbPtr->onPtr : cbPtr->offPtr;

    /*
     * Draw the box and check. 
     *
     *		+-----------+
     *		|           |
     *		|         * |
     *          |        *  |
     *          | *     *   |
     *          |  *   *    |
     *          |   * *     |
     *		|    *      |
     *		+-----------+
     */
    boxWidth = boxHeight = ODD(cbPtr->size);
    boxX = x + cbPtr->gap;
    boxY = y + (entryPtr->height - boxHeight) / 2;
    XFillRectangle(tvPtr->display, drawable, cbPtr->fillGC, boxX, boxY, 
		       boxWidth, boxHeight);
    XDrawRectangle(tvPtr->display, drawable, cbPtr->boxGC, boxX, boxY, 
	boxWidth, boxHeight);

    if (bool) {
#ifndef notdef
	int ax, ay, bx, by, cx, cy;
	int i;

	ax = boxX + 2, ay = boxY + 2 + (boxHeight / 3);
	bx = boxX + (boxWidth / 2) - 1, by = boxY + boxHeight - 4;
	cx = boxX + boxWidth, cy = boxY;
	for (i = 0; i < 3; i++) {
	    XDrawLine(tvPtr->display, drawable, cbPtr->checkGC, ax, ay, bx, by);
	    XDrawLine(tvPtr->display, drawable, cbPtr->checkGC, bx, by, cx, cy);
	    ay++, by++, cy++;
	}
#else 
	int midX, midY;
	int i;

	for (i = 0; i < 3; i++) {
	    midX = boxX + 2 * boxWidth / 5;
	    midY = boxY + boxHeight - 5 + i;
	    XDrawLine(tvPtr->display, drawable, cbPtr->checkGC, 
		      boxX + 2, boxY + boxHeight / 3 + 1 + i, midX, midY);
	    XDrawLine(tvPtr->display, drawable, cbPtr->checkGC, 
		      midX, midY, boxX + boxWidth - 2, boxY + i + 1);
	}
#endif
    }
#ifdef notdef
    textX = textY = iconX = iconY = 0;	/* Suppress compiler warning. */
#endif
    iconWidth = iconHeight = 0;
    if (cbPtr->icon != NULL) {
	iconWidth = Blt_Tv_IconWidth(cbPtr->icon);
	iconHeight = Blt_Tv_IconHeight(cbPtr->icon);
    }
    textHeight = 0;
    gap = 0;
    if (cbPtr->showValue) {
	textHeight = textPtr->height;
	if (cbPtr->icon != NULL) {
	    gap = cbPtr->gap;
	}
    }
    x = boxX + boxWidth + cbPtr->gap;

    /* The icon sits to the left of the text. */
    iconX = x;
    iconY = y + (entryPtr->height - iconHeight) / 2;
    textX = iconX + iconWidth + gap;
    textY = y + (entryPtr->height - textHeight) / 2;

    if (cbPtr->icon != NULL) {
	Tk_RedrawImage(Blt_Tv_IconBits(cbPtr->icon), 0, 0, iconWidth, 
		       iconHeight, drawable, iconX, iconY);
    }
    if ((cbPtr->showValue) && (textPtr != NULL)) {
	TextStyle ts;
	XColor *color;
	int xMax;

	if (Blt_Tv_EntryIsSelected(tvPtr, entryPtr)) {
	    if (stylePtr->selInFocusFgColor != NULL) {
		color = stylePtr->selInFocusFgColor;
	    } else {
		color = SELECT_FG(tvPtr);
	    }
	} else if (entryPtr->color != NULL) {
	    color = entryPtr->color;
	} else {
	    color = fgColor;
	}
	Blt_Ts_InitStyle(ts);
	Blt_Ts_SetFont(ts, font);
	Blt_Ts_SetForeground(ts, color);
	xMax = SCREENX(tvPtr, cp->worldX) + cp->width - 
		cp->titleBorderWidth - cp->pad.side2;
	Blt_Ts_DrawLayout(tvPtr->tkwin, drawable, textPtr, &ts, textX, textY, 
		xMax);
    }
    stylePtr->flags &= ~STYLE_DIRTY;
}

#ifdef notdef
/*
 *---------------------------------------------------------------------------
 *
 * PickCheckbox --
 *
 *	Draws the "checkbox" given the screen coordinates and the
 *	value to be displayed.  
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The checkbox value is drawn.
 *
 *---------------------------------------------------------------------------
 */
static int
PickCheckBox(Blt_Tv_Entry *entryPtr, Blt_Tv_Value *valuePtr, 
	     Blt_Tv_Style *stylePtr, int worldX, int worldY)
{
    Blt_Tv_Column *cp;
    Blt_Tv_CheckBox *cbPtr = (Blt_Tv_CheckBox *)stylePtr;
    int columnWidth;
    int x, y, width, height;

    cp = valuePtr->columnPtr;
    columnWidth = cp->width - 
	(2 * cp->borderWidth + PADDING(cp->pad));
    if (columnWidth > valuePtr->width) {
	switch(cp->justify) {
	case TK_JUSTIFY_RIGHT:
	    worldX += (columnWidth - valuePtr->width);
	    break;
	case TK_JUSTIFY_CENTER:
	    worldX += (columnWidth - valuePtr->width) / 2;
	    break;
	case TK_JUSTIFY_LEFT:
	    break;
	}
    }
    width = height = ODD(cbPtr->size) + 2 * cbPtr->lineWidth;
    x = cp->worldX + cp->pad.side1 + cbPtr->gap - 
	cbPtr->lineWidth;
    y = entryPtr->worldY + (entryPtr->height - height) / 2;
    if ((worldX >= x) && (worldX < (x + width)) && 
	(worldY >= y) && (worldY < (y + height))) {
	return TRUE;
    }
    return FALSE;
}
#endif

/*
 *---------------------------------------------------------------------------
 *
 * EditCheckbox --
 *
 *	Edits the "checkbox".
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The checkbox value is drawn.
 *
 *---------------------------------------------------------------------------
 */
static int
EditCheckBox(
    Blt_Tv *tvPtr,
    Blt_Tv_Entry *entryPtr,
    Blt_Tv_Value *valuePtr,
    Blt_Tv_Style *stylePtr)
{
    Blt_Tv_Column *cp;
    Blt_Tv_CheckBox *cbPtr = (Blt_Tv_CheckBox *)stylePtr;
    Tcl_Obj *objPtr;

    cp = valuePtr->columnPtr;
    if (Blt_TreeGetValueByKey(tvPtr->interp, tvPtr->tree, 
	      entryPtr->node, cp->key, &objPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    if (strcmp(Tcl_GetString(objPtr), cbPtr->onValue) == 0) {
	objPtr = Tcl_NewStringObj(cbPtr->offValue, -1);
    } else {
	objPtr = Tcl_NewStringObj(cbPtr->onValue, -1);
    }
    entryPtr->flags |= ENTRY_DIRTY;
    tvPtr->flags |= (TV_DIRTY | TV_LAYOUT | TV_SCROLL);
    if (Blt_TreeSetValueByKey(tvPtr->interp, tvPtr->tree, 
	      entryPtr->node, cp->key, objPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * FreeCheckbox --
 *
 *	Releases resources allocated for the checkbox. The resources
 *	freed by this routine are specific only to the "checkbox".   
 *	Other resources (common to all styles) are freed in the 
 *	Blt_Tv_FreeStyle routine.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	GCs allocated for the checkbox are freed.
 *
 *---------------------------------------------------------------------------
 */
static void
FreeCheckBox(Blt_Tv *tvPtr, Blt_Tv_Style *stylePtr)
{
    Blt_Tv_CheckBox *cbPtr = (Blt_Tv_CheckBox *)stylePtr;

    if (cbPtr->highlightGC != NULL) {
	Tk_FreeGC(tvPtr->display, cbPtr->highlightGC);
    }
    if (cbPtr->activeGC != NULL) {
	Tk_FreeGC(tvPtr->display, cbPtr->activeGC);
    }
    if (cbPtr->gc != NULL) {
	Tk_FreeGC(tvPtr->display, cbPtr->gc);
    }
    if (cbPtr->fillGC != NULL) {
	Tk_FreeGC(tvPtr->display, cbPtr->fillGC);
    }
    if (cbPtr->boxGC != NULL) {
	Tk_FreeGC(tvPtr->display, cbPtr->boxGC);
    }
    if (cbPtr->checkGC != NULL) {
	Tk_FreeGC(tvPtr->display, cbPtr->checkGC);
    }
    if (cbPtr->icon != NULL) {
	Blt_Tv_FreeIcon(tvPtr, cbPtr->icon);
    }
    if (cbPtr->offPtr != NULL) {
	Blt_Free(cbPtr->offPtr);
    }
    if (cbPtr->onPtr != NULL) {
	Blt_Free(cbPtr->onPtr);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * CreateComboBox --
 *
 *	Creates a "combobox" style.
 *
 * Results:
 *	A pointer to the new style structure.
 *
 *---------------------------------------------------------------------------
 */
static Blt_Tv_Style *
CreateComboBox(Blt_Tv *tvPtr, Blt_HashEntry *hPtr)
{
    Blt_Tv_ComboBox *cbPtr;

    cbPtr = Blt_CallocAssert(1, sizeof(Blt_Tv_ComboBox));
    cbPtr->classPtr = &comboBoxClass;
    cbPtr->gap = STYLE_GAP;
    cbPtr->buttonRelief = TK_RELIEF_RAISED;
    cbPtr->buttonBorderWidth = 1;
    cbPtr->borderWidth = 1;
    cbPtr->relief = TK_RELIEF_FLAT;
    cbPtr->name = Blt_StrdupAssert(Blt_GetHashKey(&tvPtr->styleTable, hPtr));
    cbPtr->hashPtr = hPtr;
    cbPtr->link = NULL;
    cbPtr->flags = STYLE_COMBOBOX;
    cbPtr->refCount = 1;
    Blt_SetHashValue(hPtr, cbPtr);
    return (Blt_Tv_Style *)cbPtr;
}

/*
 *---------------------------------------------------------------------------
 *
 * ConfigureComboBox --
 *
 *	Configures a "combobox" style.  This routine performs 
 *	generates the GCs required for a combobox style.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	GCs are created for the style.
 *
 *---------------------------------------------------------------------------
 */
static void
ConfigureComboBox(Blt_Tv *tvPtr, Blt_Tv_Style *stylePtr)
{
    GC newGC;
    Blt_Tv_ComboBox *cbPtr = (Blt_Tv_ComboBox *)stylePtr;
    XColor *bgColor;
    XGCValues gcValues;
    unsigned long gcMask;

    gcValues.font = Blt_FontId(CHOOSE(tvPtr->font, cbPtr->font));
    bgColor = Blt_BackgroundBorderColor(CHOOSE(tvPtr->bg, cbPtr->bg));
    gcMask = GCForeground | GCBackground | GCFont;

    /* Normal foreground */
    gcValues.background = bgColor->pixel;
    gcValues.foreground = CHOOSE(tvPtr->fgColor, cbPtr->fgColor)->pixel;
    newGC = Tk_GetGC(tvPtr->tkwin, gcMask, &gcValues);
    if (cbPtr->gc != NULL) {
	Tk_FreeGC(tvPtr->display, cbPtr->gc);
    }
    cbPtr->gc = newGC;

    /* Highlight foreground */
    gcValues.background = Blt_BackgroundBorderColor(cbPtr->highlightBg)->pixel;
    gcValues.foreground = cbPtr->highlightFgColor->pixel;
    newGC = Tk_GetGC(tvPtr->tkwin, gcMask, &gcValues);
    if (cbPtr->highlightGC != NULL) {
	Tk_FreeGC(tvPtr->display, cbPtr->highlightGC);
    }
    cbPtr->highlightGC = newGC;

    /* Active foreground */
    gcValues.background = Blt_BackgroundBorderColor(cbPtr->activeBg)->pixel;
    gcValues.foreground = cbPtr->activeFgColor->pixel;
    newGC = Tk_GetGC(tvPtr->tkwin, gcMask, &gcValues);
    if (cbPtr->activeGC != NULL) {
	Tk_FreeGC(tvPtr->display, cbPtr->activeGC);
    }
    cbPtr->activeGC = newGC;
    cbPtr->flags |= STYLE_DIRTY;
}

/*
 *---------------------------------------------------------------------------
 *
 * MeasureComboBox --
 *
 *	Determines the space requirements for the "combobox" given
 *	the value to be displayed.  Depending upon whether an icon
 *	or text is displayed and their relative placements, this
 *	routine computes the space needed for the text entry.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The width and height fields of *valuePtr* are set with the
 *	computed dimensions.
 *
 *---------------------------------------------------------------------------
 */
static void
MeasureComboBox(
    Blt_Tv *tvPtr, 
    Blt_Tv_Style *stylePtr,
    Blt_Tv_Value *valuePtr)
{
    Blt_Tv_ComboBox *cbPtr = (Blt_Tv_ComboBox *)stylePtr;
    int iconWidth, iconHeight;
    int textWidth, textHeight;
    int gap;
    Blt_Font font;

    textWidth = textHeight = 0;
    iconWidth = iconHeight = 0;
    valuePtr->width = valuePtr->height = 0;

    if (cbPtr->icon != NULL) {
	iconWidth = Blt_Tv_IconWidth(cbPtr->icon);
	iconHeight = Blt_Tv_IconHeight(cbPtr->icon);
    } 
    if (valuePtr->textPtr != NULL) {
	Blt_Free(valuePtr->textPtr);
	valuePtr->textPtr = NULL;
    }
    font = CHOOSE(tvPtr->font, cbPtr->font);
    if (valuePtr->string != NULL) {	/* New string defined. */
	TextStyle ts;

	Blt_Ts_InitStyle(ts);
	Blt_Ts_SetFont(ts, font);
	valuePtr->textPtr = Blt_Ts_CreateLayout(valuePtr->string, -1, &ts);
    } 
    gap = 0;
    if (valuePtr->textPtr != NULL) {
	textWidth = valuePtr->textPtr->width;
	textHeight = valuePtr->textPtr->height;
	if (cbPtr->icon != NULL) {
	    gap = cbPtr->gap;
	}
    }
    cbPtr->buttonWidth = ARROW_WIDTH + 2 * cbPtr->buttonBorderWidth;
    valuePtr->width = 2 * cbPtr->borderWidth + iconWidth + 4 * gap + 
	cbPtr->buttonWidth + textWidth;
    valuePtr->height = MAX(textHeight, iconHeight) + 2 * cbPtr->borderWidth;
}


/*
 *---------------------------------------------------------------------------
 *
 * DrawComboBox --
 *
 *	Draws the "combobox" given the screen coordinates and the
 *	value to be displayed.  
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The combobox value is drawn.
 *
 *---------------------------------------------------------------------------
 */
static void
DrawComboBox(
    Blt_Tv *tvPtr,
    Drawable drawable,
    Blt_Tv_Entry *entryPtr,
    Blt_Tv_Value *valuePtr,
    Blt_Tv_Style *stylePtr,
    int x, int y)
{
    GC gc;
    Blt_Tv_Column *cp;
    Blt_Tv_ComboBox *cbPtr = (Blt_Tv_ComboBox *)stylePtr;
    int iconX, iconY, iconWidth, iconHeight;
    int textX, textY, textHeight;
    int buttonX, buttonY;
    int gap, columnWidth;
    Blt_Background bg;
    XColor *fgColor;

    cp = valuePtr->columnPtr;
    if (stylePtr->flags & STYLE_HIGHLIGHT) {
	gc = cbPtr->highlightGC;
	bg = cbPtr->highlightBg;
	fgColor = cbPtr->highlightFgColor;
    } else {
	gc = cbPtr->gc;
	bg = CHOOSE(tvPtr->bg, cbPtr->bg);
	fgColor = CHOOSE(tvPtr->fgColor, cbPtr->fgColor);
    }
    if (!Blt_Tv_EntryIsSelected(tvPtr, entryPtr)) {
	/*
	 * Draw the active or normal background color over the entire
	 * label area.  This includes both the tab's text and image.
	 * The rectangle should be 2 pixels wider/taller than this
	 * area. So if the label consists of just an image, we get an
	 * halo around the image when the tab is active.
	 */
	if (bg != NULL) {
	    Blt_FillBackgroundRectangle(tvPtr->tkwin, drawable, bg, x, y, 
		cp->width, entryPtr->height, cbPtr->borderWidth, 
		cbPtr->relief);
	}
    }    
    buttonX = x + cp->width;
    buttonX -= cp->pad.side2 + cbPtr->borderWidth  +
	cbPtr->buttonWidth + cbPtr->gap;
    buttonY = y;

    columnWidth = cp->width - 
	(2 * cp->borderWidth + PADDING(cp->pad));
    if (columnWidth > valuePtr->width) {
	switch(cp->justify) {
	case TK_JUSTIFY_RIGHT:
	    x += (columnWidth - valuePtr->width);
	    break;
	case TK_JUSTIFY_CENTER:
	    x += (columnWidth - valuePtr->width) / 2;
	    break;
	case TK_JUSTIFY_LEFT:
	    break;
	}
    }

#ifdef notdef
    textX = textY = iconX = iconY = 0;	/* Suppress compiler warning. */
#endif
    
    iconWidth = iconHeight = 0;
    if (cbPtr->icon != NULL) {
	iconWidth = Blt_Tv_IconWidth(cbPtr->icon);
	iconHeight = Blt_Tv_IconHeight(cbPtr->icon);
    }
    textHeight = 0;
    if (valuePtr->textPtr != NULL) {
	textHeight = valuePtr->textPtr->height;
    }
    gap = 0;
    if ((cbPtr->icon != NULL) && (valuePtr->textPtr != NULL)) {
	gap = cbPtr->gap;
    }

    iconX = x + gap;
    iconY = y + (entryPtr->height - iconHeight) / 2;
    textX = iconX + iconWidth + gap;
    textY = y + (entryPtr->height - textHeight) / 2;

    if (cbPtr->icon != NULL) {
	Tk_RedrawImage(Blt_Tv_IconBits(cbPtr->icon), 0, 0, iconWidth, 
	       iconHeight, drawable, iconX, iconY);
    }
    if (valuePtr->textPtr != NULL) {
	TextStyle ts;
	XColor *color;
	Blt_Font font;
	int xMax;
	
	font = CHOOSE(tvPtr->font, cbPtr->font);
	if (Blt_Tv_EntryIsSelected(tvPtr, entryPtr)) {
	    if (stylePtr->selInFocusFgColor != NULL) {
		color = stylePtr->selInFocusFgColor;
	    } else {
		color = SELECT_FG(tvPtr);
	    }
	} else if (entryPtr->color != NULL) {
	    color = entryPtr->color;
	} else {
	    color = fgColor;
	}
	Blt_Ts_InitStyle(ts);
	Blt_Ts_SetFont(ts, font);
	Blt_Ts_SetForeground(ts, color);

	xMax = SCREENX(tvPtr, cp->worldX) + cp->width - 
		cp->titleBorderWidth - cp->pad.side2 - 
	    cbPtr->buttonWidth;
	Blt_Ts_DrawLayout(tvPtr->tkwin, drawable, valuePtr->textPtr, &ts, 
		textX, textY, xMax);
    }
    if (valuePtr == tvPtr->activeValuePtr) {
	Blt_FillBackgroundRectangle(tvPtr->tkwin, drawable, stylePtr->activeBg, 
	   buttonX, buttonY + cbPtr->borderWidth, cbPtr->buttonWidth, 
	   entryPtr->height - 2 * cbPtr->borderWidth, 
	cbPtr->buttonBorderWidth, cbPtr->buttonRelief); 
    } else {
	Blt_FillBackgroundRectangle(tvPtr->tkwin, drawable, cp->titleBg, 
		buttonX, buttonY + cbPtr->borderWidth, cbPtr->buttonWidth, 
		entryPtr->height - 2 * cbPtr->borderWidth, 
		cbPtr->buttonBorderWidth, cbPtr->buttonRelief); 
    }
    Blt_DrawArrow(tvPtr->display, drawable, gc, buttonX + 1, buttonY + 1, 
	cbPtr->buttonWidth - 2, entryPtr->height - 2, cbPtr->buttonBorderWidth,
	ARROW_DOWN);
    stylePtr->flags &= ~STYLE_DIRTY;
}

/*
 *---------------------------------------------------------------------------
 *
 * PickCombobox --
 *
 *	Draws the "checkbox" given the screen coordinates and the
 *	value to be displayed.  
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The checkbox value is drawn.
 *
 *---------------------------------------------------------------------------
 */
static int
PickComboBox(
    Blt_Tv_Entry *entryPtr,
    Blt_Tv_Value *valuePtr,
    Blt_Tv_Style *stylePtr,
    int worldX, int worldY)
{
    Blt_Tv_Column *cp;
    Blt_Tv_ComboBox *cbPtr = (Blt_Tv_ComboBox *)stylePtr;
    int x, y, width, height;

    cp = valuePtr->columnPtr;
    width = cbPtr->buttonWidth;
    height = entryPtr->height - 4;
    x = cp->worldX + cp->width - cp->pad.side2 - 
	cbPtr->borderWidth - cp->borderWidth - width;
    y = entryPtr->worldY + cbPtr->borderWidth;
    if ((worldX >= x) && (worldX < (x + width)) && 
	(worldY >= y) && (worldY < (y + height))) {
	return TRUE;
    }
    return FALSE;
}

/*
 *---------------------------------------------------------------------------
 *
 * EditCombobox --
 *
 *	Edits the "combobox".
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The checkbox value is drawn.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
EditComboBox(
    Blt_Tv *tvPtr,
    Blt_Tv_Entry *entryPtr,
    Blt_Tv_Value *valuePtr,
    Blt_Tv_Style *stylePtr)	/* Not used. */
{
    return Blt_Tv_CreateTextbox(tvPtr, entryPtr, valuePtr->columnPtr);
}

/*
 *---------------------------------------------------------------------------
 *
 * FreeComboBox --
 *
 *	Releases resources allocated for the combobox. The resources
 *	freed by this routine are specific only to the "combobox".   
 *	Other resources (common to all styles) are freed in the 
 *	Blt_Tv_FreeStyle routine.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	GCs allocated for the combobox are freed.
 *
 *---------------------------------------------------------------------------
 */
static void
FreeComboBox(Blt_Tv *tvPtr, Blt_Tv_Style *stylePtr)
{
    Blt_Tv_ComboBox *cbPtr = (Blt_Tv_ComboBox *)stylePtr;

    if (cbPtr->highlightGC != NULL) {
	Tk_FreeGC(tvPtr->display, cbPtr->highlightGC);
    }
    if (cbPtr->activeGC != NULL) {
	Tk_FreeGC(tvPtr->display, cbPtr->activeGC);
    }
    if (cbPtr->gc != NULL) {
	Tk_FreeGC(tvPtr->display, cbPtr->gc);
    }
    if (cbPtr->icon != NULL) {
	Blt_Tv_FreeIcon(tvPtr, cbPtr->icon);
    }
}

static Blt_Tv_Style *
GetStyle(Tcl_Interp *interp, Blt_Tv *tvPtr, const char *styleName)
{
    Blt_HashEntry *hPtr;

    hPtr = Blt_FindHashEntry(&tvPtr->styleTable, styleName);
    if (hPtr == NULL) {
	if (interp != NULL) {
	    Tcl_AppendResult(interp, "can't find cell style \"", styleName, 
		"\"", (char *)NULL);
	}
	return NULL;
    }
    return Blt_GetHashValue(hPtr);
}

int
Blt_Tv_GetStyle(
    Tcl_Interp *interp,
    Blt_Tv *tvPtr,
    const char *styleName,
    Blt_Tv_Style **stylePtrPtr)
{
    Blt_Tv_Style *stylePtr;

    stylePtr = GetStyle(interp, tvPtr, styleName);
    if (stylePtr == NULL) {
	return TCL_ERROR;
    }
    stylePtr->refCount++;
    *stylePtrPtr = stylePtr;
    return TCL_OK;
}

#ifdef notdef
int
Blt_Tv_GetStyleFromObj(Tcl_Interp *interp, Blt_Tv *tvPtr, Tcl_Obj *objPtr,
		       Blt_Tv_Style **stylePtrPtr)
{
    return Blt_Tv_GetStyle(interp, tvPtr, Tcl_GetString(objPtr), stylePtrPtr);
}
#endif

static Blt_Tv_Style *
CreateStyle(
     Tcl_Interp *interp,
     Blt_Tv *tvPtr,		/* Blt_Tv_ widget. */
     int type,			/* Type of style: either
				 * STYLE_TEXTBOX,
				 * STYLE_COMBOBOX, or
				 * STYLE_CHECKBOX */
    const char *styleName,	/* Name of the new style. */
    int objc,
    Tcl_Obj *const *objv)
{    
    Blt_HashEntry *hPtr;
    int isNew;
    Blt_Tv_Style *stylePtr;
    
    hPtr = Blt_CreateHashEntry(&tvPtr->styleTable, styleName, &isNew);
    if (!isNew) {
	if (interp != NULL) {
	    Tcl_AppendResult(interp, "cell style \"", styleName, 
			     "\" already exists", (char *)NULL);
	}
	return NULL;
    }
    /* Create the new marker based upon the given type */
    switch (type) {
    case STYLE_TEXTBOX:
	stylePtr = CreateTextBox(tvPtr, hPtr);
	break;
    case STYLE_COMBOBOX:
	stylePtr = CreateComboBox(tvPtr, hPtr);
	break;
    case STYLE_CHECKBOX:
	stylePtr = CreateCheckBox(tvPtr, hPtr);
	break;
    default:
	return NULL;
    }
    bltTvIconOption.clientData = tvPtr;
    if (Blt_ConfigureComponentFromObj(interp, tvPtr->tkwin, styleName, 
	stylePtr->classPtr->className, stylePtr->classPtr->specsPtr, 
	objc, objv, (char *)stylePtr, 0) != TCL_OK) {
	Blt_Tv_FreeStyle(tvPtr, stylePtr);
	return NULL;
    }
    return stylePtr;
}

void
Blt_Tv_UpdateStyleGCs(Blt_Tv *tvPtr, Blt_Tv_Style *stylePtr)
{
    (*stylePtr->classPtr->configProc)(tvPtr, stylePtr);
    stylePtr->flags |= STYLE_DIRTY;
    Blt_Tv_EventuallyRedraw(tvPtr);
}

Blt_Tv_Style *
Blt_Tv_CreateStyle(
     Tcl_Interp *interp,
     Blt_Tv *tvPtr,		/* Blt_Tv_ widget. */
     int type,			/* Type of style: either
				 * STYLE_TEXTBOX,
				 * STYLE_COMBOBOX, or
				 * STYLE_CHECKBOX */
    const char *styleName)	/* Name of the new style. */
{    
    Blt_Tv_Style *stylePtr;

    stylePtr = CreateStyle(interp, tvPtr, type, styleName, 0, (Tcl_Obj **)NULL);
    if (stylePtr == NULL) {
	return NULL;
    }
    return stylePtr;
}

void
Blt_Tv_FreeStyle(Blt_Tv *tvPtr, Blt_Tv_Style *stylePtr)
{
    stylePtr->refCount--;
#ifdef notdef
    fprintf(stderr, "Blt_Tv_FreeStyle %s count=%d\n", stylePtr->name,
	     stylePtr->refCount);
#endif
    /* Remove the style from the hash table so that it's name can be used.*/
    /* If no cell is using the style, remove it.*/
    if (stylePtr->refCount <= 0) {
#ifdef notdef
	fprintf(stderr, "freeing %s\n", stylePtr->name);
#endif
	bltTvIconOption.clientData = tvPtr;
	Blt_FreeOptions(stylePtr->classPtr->specsPtr, (char *)stylePtr, 
			   tvPtr->display, 0);
	(*stylePtr->classPtr->freeProc)(tvPtr, stylePtr); 
	if (stylePtr->hashPtr != NULL) {
	    Blt_DeleteHashEntry(&tvPtr->styleTable, stylePtr->hashPtr);
	} 
	if (stylePtr->link != NULL) {
	    /* Only user-generated styles will be in the list. */
	    Blt_ChainDeleteLink(tvPtr->userStyles, stylePtr->link);
	}
	if (stylePtr->name != NULL) {
	    Blt_Free(stylePtr->name);
	}
	Blt_Free(stylePtr);
    } 
}

void
Blt_Tv_SetStyleIcon(
    Blt_Tv *tvPtr, 
    Blt_Tv_Style *stylePtr, 
    Blt_Tv_Icon icon)
{
    Blt_Tv_TextBox *tbPtr = (Blt_Tv_TextBox *)stylePtr;

    if (tbPtr->icon != NULL) {
	Blt_Tv_FreeIcon(tvPtr, tbPtr->icon);
    }
    tbPtr->icon = icon;
}

GC
Blt_Tv_GetStyleGC(Blt_Tv_Style *stylePtr)
{
    Blt_Tv_TextBox *tbPtr = (Blt_Tv_TextBox *)stylePtr;
    return tbPtr->gc;
}

Blt_Background
Blt_Tv_GetStyleBackground(Blt_Tv *tvPtr, Blt_Tv_Style *stylePtr)
{
    Blt_Tv_TextBox *tbPtr = (Blt_Tv_TextBox *)stylePtr;
    Blt_Background bg;

    bg = (tbPtr->flags & STYLE_HIGHLIGHT) ? tbPtr->highlightBg : tbPtr->bg;
    return (bg != NULL) ? bg : tvPtr->bg;
}

Blt_Font
Blt_Tv_GetStyleFont(Blt_Tv *tvPtr, Blt_Tv_Style *stylePtr)
{
    Blt_Tv_TextBox *tbPtr = (Blt_Tv_TextBox *)stylePtr;

    if (tbPtr->font != NULL) {
	return tbPtr->font;
    }
    return tvPtr->font;
}

XColor *
Blt_Tv_GetStyleFg(Blt_Tv *tvPtr, Blt_Tv_Style *stylePtr)
{
    Blt_Tv_TextBox *tbPtr = (Blt_Tv_TextBox *)stylePtr;

    if (tbPtr->fgColor != NULL) {
	return tbPtr->fgColor;
    }
    return tvPtr->fgColor;
}

static void
DrawValue(Blt_Tv *tvPtr, Blt_Tv_Entry *entryPtr, Blt_Tv_Value *valuePtr)
{
    Drawable drawable;
    int sx, sy, dx, dy;
    int width, height;
    int left, right, top, bottom;
    Blt_Tv_Column *cp;
    Blt_Tv_Style *stylePtr;

    stylePtr = valuePtr->stylePtr;
    if (stylePtr == NULL) {
	stylePtr = valuePtr->columnPtr->stylePtr;
    }
    if (stylePtr->cursor != None) {
	if (valuePtr == tvPtr->activeValuePtr) {
	    Tk_DefineCursor(tvPtr->tkwin, stylePtr->cursor);
	} else {
	    if (tvPtr->cursor != None) {
		Tk_DefineCursor(tvPtr->tkwin, tvPtr->cursor);
	    } else {
		Tk_UndefineCursor(tvPtr->tkwin);
	    }
	}
    }
    cp = valuePtr->columnPtr;
    dx = SCREENX(tvPtr, cp->worldX) + cp->pad.side1;
    dy = SCREENY(tvPtr, entryPtr->worldY);
    height = entryPtr->height - 1;
    width = valuePtr->columnPtr->width - PADDING(cp->pad);

    top = tvPtr->titleHeight + tvPtr->inset;
    bottom = Tk_Height(tvPtr->tkwin) - tvPtr->inset;
    left = tvPtr->inset;
    right = Tk_Width(tvPtr->tkwin) - tvPtr->inset;

    if (((dx + width) < left) || (dx > right) ||
	((dy + height) < top) || (dy > bottom)) {
	return;			/* Value is clipped. */
    }

    drawable = Tk_GetPixmap(tvPtr->display, Tk_WindowId(tvPtr->tkwin), 
	width, height, Tk_Depth(tvPtr->tkwin));
    /* Draw the background of the value. */
    if ((valuePtr == tvPtr->activeValuePtr) ||
	(!Blt_Tv_EntryIsSelected(tvPtr, entryPtr))) {
	Blt_Background bg;

	bg = Blt_Tv_GetStyleBackground(tvPtr, tvPtr->stylePtr);
	Blt_FillBackgroundRectangle(tvPtr->tkwin, drawable, bg, 0, 0, 
		width, height, 0, TK_RELIEF_FLAT);
    } else {
	Blt_Background bg;

	if (stylePtr->selInFocusBg != NULL) {
	    bg = stylePtr->selInFocusBg;
	} else {
	    bg = SELECT_FOCUS_BG(tvPtr);
	}
	Blt_FillBackgroundRectangle(tvPtr->tkwin, drawable, bg, 0, 0, 
		width, height, tvPtr->selBorderWidth, tvPtr->selRelief);
    }
    Blt_Tv_DrawValue(tvPtr, entryPtr, valuePtr, drawable, 0, 0);
    
    /* Clip the drawable if necessary */
    sx = sy = 0;
    if (dx < left) {
	width -= left - dx;
	sx += left - dx;
	dx = left;
    }
    if ((dx + width) >= right) {
	width -= (dx + width) - right;
    }
    if (dy < top) {
	height -= top - dy;
	sy += top - dy;
	dy = top;
    }
    if ((dy + height) >= bottom) {
	height -= (dy + height) - bottom;
    }
    XCopyArea(tvPtr->display, drawable, Tk_WindowId(tvPtr->tkwin), 
      tvPtr->lineGC, sx, sy, width,  height, dx, dy);
    Tk_FreePixmap(tvPtr->display, drawable);
}

/*
 *---------------------------------------------------------------------------
 *
 * StyleActivateOp --
 *
 * 	Turns on/off highlighting for a particular style.
 *
 *	  .t style activate entry column
 *
 * Results:
 *	A standard Tcl result.  If TCL_ERROR is returned, then
 *	interp->result contains an error message.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
StyleActivateOp(
    Blt_Tv *tvPtr,

    Tcl_Interp *interp,
    int objc,			/* Not used. */
    Tcl_Obj *const *objv)
{
    Blt_Tv_Value *oldValuePtr;

    oldValuePtr = tvPtr->activeValuePtr;
    if (objc == 3) {
	Tcl_Obj *listObjPtr; 
	Blt_Tv_Entry *ep;
	Blt_Tv_Value *vp;

	vp = tvPtr->activeValuePtr;
	ep = tvPtr->activePtr;
	listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
	if ((ep != NULL) && (vp != NULL)) {
	    Tcl_Obj *objPtr; 
	    objPtr = Tcl_NewLongObj(Blt_TreeNodeId(ep->node));
	    Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
	    objPtr = Tcl_NewStringObj(vp->columnPtr->key, -1);
	    Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
	} 
	Tcl_SetObjResult(interp, listObjPtr);
    } else if (objc == 4) {
	tvPtr->activeValuePtr = NULL;
	if ((oldValuePtr != NULL)  && (tvPtr->activePtr != NULL)) {
	    DrawValue(tvPtr, tvPtr->activePtr, oldValuePtr);
	}
    } else {
	Blt_Tv_Column *cp;
	Blt_Tv_Entry *ep;
	Blt_Tv_Value *vp;

	if (Blt_Tv_GetEntry(tvPtr, objv[3], &ep) != TCL_OK) {
	    return TCL_ERROR;
	}
	if (Blt_Tv_GetColumn(interp, tvPtr, objv[4], &cp) != TCL_OK) {
	    return TCL_ERROR;
	}
	vp = Blt_Tv_FindValue(ep, cp);
	if (vp != NULL) {
	    tvPtr->activePtr = ep;
	    tvPtr->activeColumnPtr = cp;
	    oldValuePtr = tvPtr->activeValuePtr;
	    tvPtr->activeValuePtr = vp;
	    if (vp != oldValuePtr) {
		if (oldValuePtr != NULL) {
		    DrawValue(tvPtr, ep, oldValuePtr);
		}
		DrawValue(tvPtr, ep, vp);
	    }
	}
    }
    return TCL_OK;
}


/*
 *---------------------------------------------------------------------------
 *
 * StyleCgetOp --
 *
 *	  .t style cget "styleName" -background
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
StyleCgetOp(
    Blt_Tv *tvPtr,
    Tcl_Interp *interp,
    int objc,			/* Not used. */
    Tcl_Obj *const *objv)
{
    Blt_Tv_Style *stylePtr;

    stylePtr = GetStyle(interp, tvPtr, Tcl_GetString(objv[3]));
    if (stylePtr == NULL) {
	return TCL_ERROR;
    }
    return Blt_ConfigureValueFromObj(interp, tvPtr->tkwin, 
	stylePtr->classPtr->specsPtr, (char *)tvPtr, objv[4], 0);
}

/*
 *---------------------------------------------------------------------------
 *
 * StyleCheckBoxOp --
 *
 *	  .t style checkbox "styleName" -background blue
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
StyleCheckBoxOp(
    Blt_Tv *tvPtr,
    Tcl_Interp *interp,
    int objc,			/* Not used. */
    Tcl_Obj *const *objv)
{
    Blt_Tv_Style *stylePtr;

    stylePtr = CreateStyle(interp, tvPtr, STYLE_CHECKBOX, 
	Tcl_GetString(objv[3]), objc - 4, objv + 4);
    if (stylePtr == NULL) {
	return TCL_ERROR;
    }
    stylePtr->link = Blt_ChainAppend(tvPtr->userStyles, stylePtr);
    Blt_Tv_UpdateStyleGCs(tvPtr, stylePtr);
    Tcl_SetObjResult(interp, objv[3]);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * StyleComboBoxOp --
 *
 *	  .t style combobox "styleName" -background blue
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
StyleComboBoxOp(
    Blt_Tv *tvPtr,
    Tcl_Interp *interp,
    int objc,			/* Not used. */
    Tcl_Obj *const *objv)
{
    Blt_Tv_Style *stylePtr;

    stylePtr = CreateStyle(interp, tvPtr, STYLE_COMBOBOX, 
	Tcl_GetString(objv[3]), objc - 4, objv + 4);
    if (stylePtr == NULL) {
	return TCL_ERROR;
    }
    stylePtr->link = Blt_ChainAppend(tvPtr->userStyles, stylePtr);
    Blt_Tv_UpdateStyleGCs(tvPtr, stylePtr);
    Tcl_SetObjResult(interp, objv[3]);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * StyleConfigureOp --
 *
 * 	This procedure is called to process a list of configuration
 *	options database, in order to reconfigure a style.
 *
 *	  .t style configure "styleName" option value
 *
 * Results:
 *	A standard Tcl result.  If TCL_ERROR is returned, then
 *	interp->result contains an error message.
 *
 * Side effects:
 *	Configuration information, such as text string, colors, font,
 *	etc. get set for stylePtr; old resources get freed, if there
 *	were any.  
 *
 *---------------------------------------------------------------------------
 */
static int
StyleConfigureOp(
    Blt_Tv *tvPtr,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    Blt_Tv_Style *stylePtr;

    stylePtr = GetStyle(interp, tvPtr, Tcl_GetString(objv[3]));
    if (stylePtr == NULL) {
	return TCL_ERROR;
    }
    if (objc == 4) {
	return Blt_ConfigureInfoFromObj(interp, tvPtr->tkwin, 
	    stylePtr->classPtr->specsPtr, (char *)stylePtr, (Tcl_Obj *)NULL, 0);
    } else if (objc == 5) {
	return Blt_ConfigureInfoFromObj(interp, tvPtr->tkwin, 
		stylePtr->classPtr->specsPtr, (char *)stylePtr, objv[5], 0);
    }
    bltTvIconOption.clientData = tvPtr;
    if (Blt_ConfigureWidgetFromObj(interp, tvPtr->tkwin, 
	stylePtr->classPtr->specsPtr, objc - 4, objv + 4, (char *)stylePtr, 
	BLT_CONFIG_OBJV_ONLY) != TCL_OK) {
	return TCL_ERROR;
    }
    (*stylePtr->classPtr->configProc)(tvPtr, stylePtr);
    stylePtr->flags |= STYLE_DIRTY;
    tvPtr->flags |= (TV_LAYOUT | TV_DIRTY);
    Blt_Tv_EventuallyRedraw(tvPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * StyleForgetOp --
 *
 * 	Eliminates one or more style names.  A style still may be in
 * 	use after its name has been officially removed.  Only its hash
 * 	table entry is removed.  The style itself remains until its
 * 	reference count returns to zero (i.e. no one else is using it).
 *
 *	  .t style forget "styleName"...
 *
 * Results:
 *	A standard Tcl result.  If TCL_ERROR is returned, then
 *	interp->result contains an error message.
 *
 *---------------------------------------------------------------------------
 */
static int
StyleForgetOp(
    Blt_Tv *tvPtr,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    Blt_Tv_Style *stylePtr;
    int i;

    for (i = 3; i < objc; i++) {
	stylePtr = GetStyle(interp, tvPtr, Tcl_GetString(objv[i]));
	if (stylePtr == NULL) {
	    return TCL_ERROR;
	}
	/* 
	 * Removing the style from the hash tables frees up the style
	 * name again.  The style itself may not be removed until it's
	 * been released by everything using it.
	 */
	if (stylePtr->hashPtr != NULL) {
	    Blt_DeleteHashEntry(&tvPtr->styleTable, stylePtr->hashPtr);
	    stylePtr->hashPtr = NULL;
	} 
	Blt_Tv_FreeStyle(tvPtr, stylePtr);
    }
    Blt_Tv_EventuallyRedraw(tvPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * StyleHighlightOp --
 *
 * 	Turns on/off highlighting for a particular style.
 *
 *	  .t style highlight styleName on|off
 *
 * Results:
 *	A standard Tcl result.  If TCL_ERROR is returned, then
 *	interp->result contains an error message.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
StyleHighlightOp(
    Blt_Tv *tvPtr,
    Tcl_Interp *interp,
    int objc,			/* Not used. */
    Tcl_Obj *const *objv)
{
    Blt_Tv_Style *stylePtr;
    int bool, oldBool;

    stylePtr = GetStyle(interp, tvPtr, Tcl_GetString(objv[3]));
    if (stylePtr == NULL) {
	return TCL_ERROR;
    }
    if (Tcl_GetBooleanFromObj(interp, objv[4], &bool) != TCL_OK) {
	return TCL_ERROR;
    }
    oldBool = ((stylePtr->flags & STYLE_HIGHLIGHT) != 0);
    if (oldBool != bool) {
	if (bool) {
	    stylePtr->flags |= STYLE_HIGHLIGHT;
	} else {
	    stylePtr->flags &= ~STYLE_HIGHLIGHT;
	}
	Blt_Tv_EventuallyRedraw(tvPtr);
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * StyleNamesOp --
 *
 * 	Lists the names of all the current styles in the treeview widget.
 *
 *	  .t style names
 *
 * Results:
 *	Always TCL_OK.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
StyleNamesOp(
    Blt_Tv *tvPtr,
    Tcl_Interp *interp,
    int objc,			/* Not used. */
    Tcl_Obj *const *objv)	/* Not used. */
{
    Blt_HashEntry *hPtr;
    Blt_HashSearch cursor;
    Tcl_Obj *listObjPtr, *objPtr;
    Blt_Tv_Style *stylePtr;

    listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
    for (hPtr = Blt_FirstHashEntry(&tvPtr->styleTable, &cursor); hPtr != NULL;
	 hPtr = Blt_NextHashEntry(&cursor)) {
	stylePtr = Blt_GetHashValue(hPtr);
	objPtr = Tcl_NewStringObj(stylePtr->name, -1);
	Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
    }
    Tcl_SetObjResult(interp, listObjPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * StyleSetOp --
 *
 * 	Sets a style for a given key for all the ids given.
 *
 *	  .t style set styleName key node...
 *
 * Results:
 *	A standard Tcl result.  If TCL_ERROR is returned, then
 *	interp->result contains an error message.
 *
 *---------------------------------------------------------------------------
 */
static int
StyleSetOp(
    Blt_Tv *tvPtr,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    Blt_TreeKey key;
    Blt_Tv_Style *stylePtr;
    int i;

    stylePtr = GetStyle(interp, tvPtr, Tcl_GetString(objv[3]));
    if (stylePtr == NULL) {
	return TCL_ERROR;
    }
    key = Blt_TreeGetKey(tvPtr->tree, Tcl_GetString(objv[4]));
    stylePtr->flags |= STYLE_LAYOUT;
    for (i = 5; i < objc; i++) {
	Blt_Tv_Entry *entryPtr;
	Blt_Tv_TagIter iter;

	if (Blt_Tv_FindTaggedEntries(tvPtr, objv[i], &iter) != TCL_OK) {
	    return TCL_ERROR;
	}
	for (entryPtr = Blt_Tv_FirstTaggedEntry(&iter); entryPtr != NULL; 
	     entryPtr = Blt_Tv_NextTaggedEntry(&iter)) {
	    Blt_Tv_Value *vp;

	    for (vp = entryPtr->values; vp != NULL; vp = vp->nextPtr) {
		if (vp->columnPtr->key == key) {
		    Blt_Tv_Style *oldStylePtr;

		    stylePtr->refCount++;
		    oldStylePtr = vp->stylePtr;
		    vp->stylePtr = stylePtr;
		    if (oldStylePtr != NULL) {
			Blt_Tv_FreeStyle(tvPtr, oldStylePtr);
		    }
		    break;
		}
	    }
	}
    }
    Blt_Tv_EventuallyRedraw(tvPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * StyleTextBoxOp --
 *
 *	  .t style text "styleName" -background blue
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
StyleTextBoxOp(
    Blt_Tv *tvPtr,
    Tcl_Interp *interp,
    int objc,			/* Not used. */
    Tcl_Obj *const *objv)
{
    Blt_Tv_Style *stylePtr;

    stylePtr = CreateStyle(interp, tvPtr, STYLE_TEXTBOX, 
	Tcl_GetString(objv[3]), objc - 4, objv + 4);
    if (stylePtr == NULL) {
	return TCL_ERROR;
    }
    stylePtr->link = Blt_ChainAppend(tvPtr->userStyles, stylePtr);
    Blt_Tv_UpdateStyleGCs(tvPtr, stylePtr);
    Tcl_SetObjResult(interp, objv[3]);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * StyleUnsetOp --
 *
 * 	Removes a style for a given key for all the ids given.
 *	The cell's style is returned to its default state.
 *
 *	  .t style unset styleName key node...
 *
 * Results:
 *	A standard Tcl result.  If TCL_ERROR is returned, then
 *	interp->result contains an error message.
 *
 *---------------------------------------------------------------------------
 */
static int
StyleUnsetOp(
    Blt_Tv *tvPtr,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    Blt_TreeKey key;
    Blt_Tv_Style *stylePtr;
    int i;

    stylePtr = GetStyle(interp, tvPtr, Tcl_GetString(objv[3]));
    if (stylePtr == NULL) {
	return TCL_ERROR;
    }
    key = Blt_TreeGetKey(tvPtr->tree, Tcl_GetString(objv[4]));
    stylePtr->flags |= STYLE_LAYOUT;
    for (i = 5; i < objc; i++) {
	Blt_Tv_TagIter iter;
	Blt_Tv_Entry *entryPtr;

	if (Blt_Tv_FindTaggedEntries(tvPtr, objv[i], &iter) != TCL_OK) {
	    return TCL_ERROR;
	}
	for (entryPtr = Blt_Tv_FirstTaggedEntry(&iter); entryPtr != NULL; 
	     entryPtr = Blt_Tv_NextTaggedEntry(&iter)) {
	    Blt_Tv_Value *vp;

	    for (vp = entryPtr->values; vp != NULL; vp = vp->nextPtr) {
		if (vp->columnPtr->key == key) {
		    if (vp->stylePtr != NULL) {
			Blt_Tv_FreeStyle(tvPtr, vp->stylePtr);
			vp->stylePtr = NULL;
		    }
		    break;
		}
	    }
	}
    }
    Blt_Tv_EventuallyRedraw(tvPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * StyleOp --
 *
 *	.t style activate $node $column
 *	.t style activate 
 *	.t style cget "highlight" -foreground
 *	.t style configure "highlight" -fg blue -bg green
 *	.t style checkbox "highlight"
 *	.t style highlight "highlight" on|off
 *	.t style combobox "highlight"
 *	.t style text "highlight"
 *	.t style forget "highlight"
 *	.t style get "mtime" $node
 *	.t style names
 *	.t style set "mtime" "highlight" all
 *	.t style unset "mtime" all
 *
 *---------------------------------------------------------------------------
 */
static Blt_OpSpec styleOps[] = {
    {"activate", 1, StyleActivateOp, 3, 5,"entry column",},
    {"cget", 2, StyleCgetOp, 5, 5, "styleName option",},
    {"checkbox", 2, StyleCheckBoxOp, 4, 0, "styleName options...",},
    {"combobox", 3, StyleComboBoxOp, 4, 0, "styleName options...",},
    {"configure", 3, StyleConfigureOp, 4, 0, "styleName options...",},
    {"forget", 1, StyleForgetOp, 3, 0, "styleName...",},
    {"highlight", 1, StyleHighlightOp, 5, 5, "styleName boolean",},
    {"names", 1, StyleNamesOp, 3, 3, "",}, 
    {"set", 1, StyleSetOp, 6, 6, "key styleName tagOrId...",},
    {"textbox", 1, StyleTextBoxOp, 4, 0, "styleName options...",},
    {"unset", 1, StyleUnsetOp, 5, 5, "key tagOrId",},
};

static int nStyleOps = sizeof(styleOps) / sizeof(Blt_OpSpec);

int
Blt_Tv_StyleOp(
    Blt_Tv *tvPtr,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    TvCmdProc *proc;
    int result;

    proc = Blt_GetOpFromObj(interp, nStyleOps, styleOps, BLT_OP_ARG2, objc, 
	objv, 0);
    if (proc == NULL) {
	return TCL_ERROR;
    }
    result = (*proc)(tvPtr, interp, objc, objv);
    return result;
}
#endif /* NO_TREEVIEW */
