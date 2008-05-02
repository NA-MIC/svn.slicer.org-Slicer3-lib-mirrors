
/*
 * bltTvCol.c --
 *
 * This module implements an hierarchy widget for the BLT toolkit.
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

/*
 * TODO:
 *
 * BUGS:
 *   1.  "open" operation should change scroll offset so that as many
 *	 new entries (up to half a screen) can be seen.
 *   2.  "open" needs to adjust the scrolloffset so that the same entry
 *	 is seen at the same place.
 */
#include "bltInt.h"

#ifndef NO_TREEVIEW
#include "bltOp.h"
#include "bltTreeView.h"
#include <X11/Xutil.h>

#define RULE_AREA		(8)

typedef int (TvCmdProc)(Blt_Tv *tvPtr, Tcl_Interp *interp, int objc, 
	Tcl_Obj *const *objv);

static const char *sortTypeStrings[] = {
    "dictionary", "ascii", "integer", "real", "command", "none", NULL
};

enum SortTypeValues { 
    SORT_DICTIONARY, SORT_ASCII, SORT_INTEGER, 
    SORT_REAL, SORT_COMMAND, SORT_NONE
};

#define DEF_SORT_COLUMN		(char *)NULL
#define DEF_SORT_COMMAND	(char *)NULL
#define DEF_SORT_DECREASING	"no"
#define DEF_SORT_TYPE		"dictionary"

#ifdef WIN32
#define DEF_COLUMN_ACTIVE_TITLE_BG	RGB_GREY85
#else
#define DEF_COLUMN_ACTIVE_TITLE_BG	RGB_GREY90
#endif
#define DEF_COLUMN_ACTIVE_TITLE_FG	STD_ACTIVE_FOREGROUND
#define DEF_COLUMN_BACKGROUND		(char *)NULL
#define DEF_COLUMN_BIND_TAGS		"all"
#define DEF_COLUMN_BORDERWIDTH		STD_BORDERWIDTH
#define DEF_COLUMN_COLOR		RGB_BLACK
#define DEF_COLUMN_EDIT			"yes"
#define DEF_COLUMN_FONT			STD_FONT
#define DEF_COLUMN_COMMAND		(char *)NULL
#define DEF_COLUMN_FORMAT_COMMAND	(char *)NULL
#define DEF_COLUMN_HIDE			"no"
#define DEF_COLUMN_JUSTIFY		"center"
#define DEF_COLUMN_MAX			"0"
#define DEF_COLUMN_MIN			"0"
#define DEF_COLUMN_PAD			"2"
#define DEF_COLUMN_RELIEF		"flat"
#define DEF_COLUMN_STATE		"normal"
#define DEF_COLUMN_STYLE		"text"
#define DEF_COLUMN_TITLE_BACKGROUND	STD_NORMAL_BACKGROUND
#define DEF_COLUMN_TITLE_BORDERWIDTH	STD_BORDERWIDTH
#define DEF_COLUMN_TITLE_FONT		STD_FONT_SMALL
#define DEF_COLUMN_TITLE_FOREGROUND	STD_NORMAL_FOREGROUND
#define DEF_COLUMN_TITLE_RELIEF		"raised"
#define DEF_COLUMN_WEIGHT		"1.0"
#define DEF_COLUMN_WIDTH		"0"
#define DEF_COLUMN_RULE_DASHES		"dot"

static Blt_OptionParseProc ObjToEnum;
static Blt_OptionPrintProc EnumToObj;
static Blt_CustomOption typeOption =
{
    ObjToEnum, EnumToObj, NULL, (ClientData)sortTypeStrings
};

static Blt_OptionParseProc ObjToColumn;
static Blt_OptionPrintProc ColumnToObj;
static Blt_CustomOption columnOption =
{
    ObjToColumn, ColumnToObj, NULL, (ClientData)0
};

static Blt_OptionParseProc ObjToData;
static Blt_OptionPrintProc DataToObj;
Blt_CustomOption bltTvDataOption =
{
    ObjToData, DataToObj, NULL, (ClientData)0,
};

static Blt_OptionParseProc ObjToStyle;
static Blt_OptionPrintProc StyleToObj;
static Blt_OptionFreeProc FreeStyle;
static Blt_CustomOption styleOption =
{
    /* Contains a pointer to the widget that's currently being
     * configured.  This is used in the custom configuration parse
     * routine for icons.  */
    ObjToStyle, StyleToObj, FreeStyle, NULL,
};

BLT_EXTERN Blt_CustomOption bltTvUidOption;
BLT_EXTERN Blt_CustomOption bltTvIconOption;

static Blt_ConfigSpec columnSpecs[] =
{
    {BLT_CONFIG_BACKGROUND, "-activetitlebackground", "activeTitleBackground", 
	"Background", DEF_COLUMN_ACTIVE_TITLE_BG, 
	Blt_Offset(Blt_Tv_Column, activeTitleBg), 0},
    {BLT_CONFIG_COLOR, "-activetitleforeground", "activeTitleForeground", 
	"Foreground", DEF_COLUMN_ACTIVE_TITLE_FG, 
	Blt_Offset(Blt_Tv_Column, activeTitleFgColor), 0},
    {BLT_CONFIG_BACKGROUND, "-background", "background", "Background",
	DEF_COLUMN_BACKGROUND, Blt_Offset(Blt_Tv_Column, bg), 
	BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_SYNONYM, "-bd", "borderWidth", (char *)NULL, (char *)NULL, 
	0, 0},
    {BLT_CONFIG_SYNONYM, "-bg", "background", (char *)NULL, (char *)NULL, 
	0, 0},
    {BLT_CONFIG_CUSTOM, "-bindtags", "bindTags", "BindTags",
	DEF_COLUMN_BIND_TAGS, Blt_Offset(Blt_Tv_Column, tagsUid),
	BLT_CONFIG_NULL_OK, &bltTvUidOption},
    {BLT_CONFIG_PIXELS_NNEG, "-borderwidth", "borderWidth", "BorderWidth",
	DEF_COLUMN_BORDERWIDTH, Blt_Offset(Blt_Tv_Column, borderWidth),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_STRING, "-command", "command", "Command",
	DEF_COLUMN_COMMAND, Blt_Offset(Blt_Tv_Column, titleCmd),
	BLT_CONFIG_DONT_SET_DEFAULT | BLT_CONFIG_NULL_OK}, 
    {BLT_CONFIG_BOOLEAN, "-edit", "edit", "Edit",
	DEF_COLUMN_STATE, Blt_Offset(Blt_Tv_Column, editable), 
        BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_BOOLEAN, "-hide", "hide", "Hide",
	DEF_COLUMN_HIDE, Blt_Offset(Blt_Tv_Column, hidden),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_CUSTOM, "-icon", "icon", "icon",
	(char *)NULL, Blt_Offset(Blt_Tv_Column, titleIcon),
        BLT_CONFIG_NULL_OK | BLT_CONFIG_DONT_SET_DEFAULT, 
	&bltTvIconOption},
    {BLT_CONFIG_JUSTIFY, "-justify", "justify", "Justify",
	DEF_COLUMN_JUSTIFY, Blt_Offset(Blt_Tv_Column, justify), 
        BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_PIXELS_NNEG, "-max", "max", "Max",
	DEF_COLUMN_MAX, Blt_Offset(Blt_Tv_Column, reqMax), 
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_PIXELS_NNEG, "-min", "min", "Min",
	DEF_COLUMN_MIN, Blt_Offset(Blt_Tv_Column, reqMin), 
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_PAD, "-pad", "pad", "Pad",
	DEF_COLUMN_PAD, Blt_Offset(Blt_Tv_Column, pad), 
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_RELIEF, "-relief", "relief", "Relief",
	DEF_COLUMN_RELIEF, Blt_Offset(Blt_Tv_Column, relief), 
        BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_DASHES, "-ruledashes", "ruleDashes", "RuleDashes",
	DEF_COLUMN_RULE_DASHES, Blt_Offset(Blt_Tv_Column, ruleDashes),
	BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_STRING, "-sortcommand", "sortCommand", "SortCommand",
	DEF_SORT_COMMAND, Blt_Offset(Blt_Tv_Column, sortCmd), 
	BLT_CONFIG_NULL_OK}, 
    {BLT_CONFIG_STATE, "-state", "state", "State",
	DEF_COLUMN_STATE, Blt_Offset(Blt_Tv_Column, state), 
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_CUSTOM, "-style", "style", "Style", DEF_COLUMN_STYLE, 
	Blt_Offset(Blt_Tv_Column, stylePtr), BLT_CONFIG_NULL_OK, &styleOption},
    {BLT_CONFIG_STRING, "-text", "text", "Text",
	(char *)NULL, Blt_Offset(Blt_Tv_Column, title), 0},
    {BLT_CONFIG_STRING, "-title", "title", "Title",
	(char *)NULL, Blt_Offset(Blt_Tv_Column, title), 0},
    {BLT_CONFIG_BACKGROUND, "-titlebackground", "titleBackground", 
	"TitleBackground", DEF_COLUMN_TITLE_BACKGROUND, 
	Blt_Offset(Blt_Tv_Column, titleBg),0},
    {BLT_CONFIG_PIXELS_NNEG, "-titleborderwidth", "BorderWidth", 
	"TitleBorderWidth", DEF_COLUMN_TITLE_BORDERWIDTH, 
	Blt_Offset(Blt_Tv_Column, titleBorderWidth), 
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_FONT, "-titlefont", "titleFont", "Font",
	DEF_COLUMN_TITLE_FONT, Blt_Offset(Blt_Tv_Column, titleFont), 0},
    {BLT_CONFIG_COLOR, "-titleforeground", "titleForeground", "TitleForeground",
	DEF_COLUMN_TITLE_FOREGROUND, 
	Blt_Offset(Blt_Tv_Column, titleFgColor), 0},
    {BLT_CONFIG_RELIEF, "-titlerelief", "titleRelief", "TitleRelief",
	DEF_COLUMN_TITLE_RELIEF, Blt_Offset(Blt_Tv_Column, titleRelief), 
        BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_DOUBLE, "-weight", (char *)NULL, (char *)NULL,
	DEF_COLUMN_WEIGHT, Blt_Offset(Blt_Tv_Column, weight), 
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_PIXELS_NNEG, "-width", "width", "Width",
	DEF_COLUMN_WIDTH, Blt_Offset(Blt_Tv_Column, reqWidth), 
        BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_END, (char *)NULL, (char *)NULL, (char *)NULL,
	(char *)NULL, 0, 0}
};

static Blt_ConfigSpec sortSpecs[] =
{
    {BLT_CONFIG_STRING, "-command", "command", "Command",
	DEF_SORT_COMMAND, Blt_Offset(Blt_Tv, sortCmd),
	BLT_CONFIG_DONT_SET_DEFAULT | BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_CUSTOM, "-column", "column", "Column",
	DEF_SORT_COLUMN, Blt_Offset(Blt_Tv, sortColumnPtr),
        BLT_CONFIG_NULL_OK | BLT_CONFIG_DONT_SET_DEFAULT, &columnOption},
    {BLT_CONFIG_BOOLEAN, "-decreasing", "decreasing", "Decreasing",
	DEF_SORT_DECREASING, Blt_Offset(Blt_Tv, sortDecreasing),
        BLT_CONFIG_DONT_SET_DEFAULT}, 
    {BLT_CONFIG_CUSTOM, "-mode", "mode", "Mode", DEF_SORT_TYPE, 
	Blt_Offset(Blt_Tv, sortType), 0, &typeOption},
    {BLT_CONFIG_END, (char *)NULL, (char *)NULL, (char *)NULL,
	(char *)NULL, 0, 0}
};

static Blt_TreeApplyProc SortApplyProc;
static Blt_TreeCompareNodesProc CompareNodes;

/*
 *---------------------------------------------------------------------------
 *
 * ObjToEnum --
 *
 *	Converts the string into its enumerated type.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToEnum(
    ClientData clientData,	/* Vectors of valid strings. */
    Tcl_Interp *interp,		/* Interpreter to send results back to */
    Tk_Window tkwin,		/* Not used. */
    Tcl_Obj *objPtr,
    char *widgRec,		/* Widget record. */
    int offset,			/* Offset of field in record */
    int flags)			
{
    int *enumPtr = (int *)(widgRec + offset);
    char c;
    char **p;
    int i;
    int count;
    char *string;

    string = Tcl_GetString(objPtr);
    c = string[0];
    count = 0;
    for (p = (char **)clientData; *p != NULL; p++) {
	if ((c == p[0][0]) && (strcmp(string, *p) == 0)) {
	    *enumPtr = count;
	    return TCL_OK;
	}
	count++;
    }
    *enumPtr = -1;

    Tcl_AppendResult(interp, "bad value \"", string, "\": should be ", 
	(char *)NULL);
    p = (char **)clientData; 
    if (count > 0) {
	Tcl_AppendResult(interp, p[0], (char *)NULL);
    }
    for (i = 1; i < (count - 1); i++) {
	Tcl_AppendResult(interp, " ", p[i], ", ", (char *)NULL);
    }
    if (count > 1) {
	Tcl_AppendResult(interp, " or ", p[count - 1], ".", (char *)NULL);
    }
    return TCL_ERROR;
}

/*
 *---------------------------------------------------------------------------
 *
 * EnumToObj --
 *
 *	Returns the string associated with the enumerated type.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
EnumToObj(
    ClientData clientData,	/* List of strings. */
    Tcl_Interp *interp,
    Tk_Window tkwin,		/* Not used. */
    char *widgRec,		/* Widget record */
    int offset,			/* Offset to field in structure */
    int flags)	
{
    int value = *(int *)(widgRec + offset);
    char **strings = (char **)clientData;
    char **p;
    int count;

    count = 0;
    for (p = strings; *p != NULL; p++) {
	if (value == count) {
	    return Tcl_NewStringObj(*p, -1);
	}
	count++;
    }
    return Tcl_NewStringObj("unknown value", -1);
}

/*
 *---------------------------------------------------------------------------
 *
 * ObjToColumn --
 *
 *	Convert the string reprsenting a column, to its numeric
 *	form.
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
ObjToColumn(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,		/* Interpreter to send results back to */
    Tk_Window tkwin,		/* Not used. */
    Tcl_Obj *objPtr,		/* New legend position string */
    char *widgRec,
    int offset,			/* Offset to field in structure */
    int flags)	
{
    Blt_Tv *tvPtr = (Blt_Tv *)widgRec;
    Blt_Tv_Column **columnPtrPtr = (Blt_Tv_Column **)(widgRec + offset);
    char *string;

    string = Tcl_GetString(objPtr);
    if (string[0] == '\0') {
	*columnPtrPtr = &tvPtr->treeColumn;
    } else {
	if (Blt_Tv_GetColumn(interp, tvPtr, objPtr, columnPtrPtr) 
	    != TCL_OK) {
	    return TCL_ERROR;
	}
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ColumnToString --
 *
 * Results:
 *	The string representation of the button boolean is returned.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
ColumnToObj(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,
    Tk_Window tkwin,		/* Not used. */
    char *widgRec,
    int offset,			/* Offset to field in structure */
    int flags)	
{
    Blt_Tv_Column *columnPtr = *(Blt_Tv_Column **)(widgRec + offset);

    if (columnPtr == NULL) {
	return Tcl_NewStringObj("", -1);
    } else {
	return Tcl_NewStringObj(columnPtr->key, -1);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * ObjToData --
 *
 *	Convert the string reprsenting a scroll mode, to its numeric
 *	form.
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
ObjToData(
    ClientData clientData,	/* Node of entry. */
    Tcl_Interp *interp,		/* Interpreter to send results back to */
    Tk_Window tkwin,		/* Not used. */
    Tcl_Obj *objPtr,		/* Tcl_Obj representing new data. */
    char *widgRec,
    int offset,			/* Offset to field in structure */
    int flags)	
{
    Tcl_Obj **objv;
    Blt_Tv_Entry *entryPtr = (Blt_Tv_Entry *)widgRec;
    char *string;
    int objc;
    int i;

    string = Tcl_GetString(objPtr);
    if (*string == '\0') {
	return TCL_OK;
    } 
    if (Tcl_ListObjGetElements(interp, objPtr, &objc, &objv) != TCL_OK) {
	return TCL_ERROR;
    }
    if (objc == 0) {
	return TCL_OK;
    }
    if (objc & 0x1) {
	Tcl_AppendResult(interp, "data \"", string, 
		 "\" must be in even name-value pairs", (char *)NULL);
	return TCL_ERROR;
    }
    for (i = 0; i < objc; i += 2) {
	Blt_Tv_Column *columnPtr;
	Blt_Tv *tvPtr = entryPtr->tvPtr;

	if (Blt_Tv_GetColumn(interp, tvPtr, objv[i], &columnPtr) 
	    != TCL_OK) {
	    return TCL_ERROR;
	}
	if (Blt_TreeSetValueByKey(tvPtr->interp, tvPtr->tree, entryPtr->node, 
		columnPtr->key, objv[i + 1]) != TCL_OK) {
	    return TCL_ERROR;
	}
	Blt_Tv_AddValue(entryPtr, columnPtr);
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * DataToObj --
 *
 * Results:
 *	The string representation of the data is returned.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
DataToObj(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,
    Tk_Window tkwin,		/* Not used. */
    char *widgRec,
    int offset,			/* Offset to field in structure */
    int flags)	
{
    Tcl_Obj *listObjPtr, *objPtr;
    Blt_Tv_Entry *entryPtr = (Blt_Tv_Entry *)widgRec;
    Blt_Tv_Value *valuePtr;

    /* Add the key-value pairs to a new Tcl_Obj */
    listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
    for (valuePtr = entryPtr->values; valuePtr != NULL; 
	valuePtr = valuePtr->nextPtr) {
	objPtr = Tcl_NewStringObj(valuePtr->columnPtr->key, -1);
	Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
	if (Blt_Tv_GetData(entryPtr, valuePtr->columnPtr->key, &objPtr)
	    != TCL_OK) {
	    objPtr = Blt_EmptyStringObj();
	    Tcl_IncrRefCount(objPtr);
	} 
	Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
    }
    return listObjPtr;
}

int
Blt_Tv_GetColumn(Tcl_Interp *interp, Blt_Tv *tvPtr, Tcl_Obj *objPtr, 
		 Blt_Tv_Column **columnPtrPtr)
{
    char *string;

    string = Tcl_GetString(objPtr);
    if (strcmp(string, "treeView") == 0) {
	*columnPtrPtr = &tvPtr->treeColumn;
    } else {
	Blt_HashEntry *hPtr;
    
	hPtr = Blt_FindHashEntry(&tvPtr->columnTable, 
		Blt_TreeGetKey(tvPtr->tree, string));
	if (hPtr == NULL) {
	    if (interp != NULL) {
		Tcl_AppendResult(interp, "can't find column \"", string, 
			"\" in \"", Tk_PathName(tvPtr->tkwin), "\"", 
			(char *)NULL);
	    }
	    return TCL_ERROR;
	} 
	*columnPtrPtr = Blt_GetHashValue(hPtr);
    }
    return TCL_OK;
}


/*ARGSUSED*/
static void
FreeStyle(ClientData clientData, Display *display, char *widgRec, int offset)
{
    Blt_Tv *tvPtr = clientData;
    Blt_Tv_Style **stylePtrPtr = (Blt_Tv_Style **)(widgRec + offset);

    if (*stylePtrPtr != NULL) {
	Blt_Tv_FreeStyle(tvPtr, *stylePtrPtr);
	*stylePtrPtr = NULL;
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * ObjToStyle --
 *
 *	Convert the name of an icon into a treeview style.
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
ObjToStyle(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,		/* Interpreter to send results back to */
    Tk_Window tkwin,		/* Not used. */
    Tcl_Obj *objPtr,		/* Tcl_Obj representing the new value. */
    char *widgRec,
    int offset,			/* Offset to field in structure */
    int flags)	
{
    Blt_Tv *tvPtr = clientData;
    Blt_Tv_Style **stylePtrPtr = (Blt_Tv_Style **)(widgRec + offset);
    Blt_Tv_Style *stylePtr;

    if (Blt_Tv_GetStyle(interp, tvPtr, Tcl_GetString(objPtr), 
	     &stylePtr) != TCL_OK) {
	return TCL_ERROR;
    }
    stylePtr->flags |= STYLE_DIRTY;
    tvPtr->flags |= (TV_LAYOUT | TV_DIRTY);
    if (*stylePtrPtr != NULL) {
	Blt_Tv_FreeStyle(tvPtr, *stylePtrPtr);
    }
    *stylePtrPtr = stylePtr;
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
StyleToObj(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,
    Tk_Window tkwin,		/* Not used. */
    char *widgRec,
    int offset,			/* Offset to field in structure */
    int flags)	
{
    Blt_Tv_Style *stylePtr = *(Blt_Tv_Style **)(widgRec + offset);

    if (stylePtr == NULL) {
	return Tcl_NewStringObj("", -1);
    } else {
	return Tcl_NewStringObj(stylePtr->name, -1);
    }
}


void
Blt_Tv_UpdateColumnGCs(Blt_Tv *tvPtr, Blt_Tv_Column *columnPtr)
{
    Drawable drawable;
    GC newGC;
    Blt_Background bg;
    XGCValues gcValues;
    int ruleDrawn;
    unsigned long gcMask;
    int iconWidth, iconHeight;
    int textWidth, textHeight;

    gcMask = GCForeground | GCFont;
    gcValues.font = Blt_FontId(columnPtr->titleFont);

    /* Normal title text */
    gcValues.foreground = columnPtr->titleFgColor->pixel;
    newGC = Tk_GetGC(tvPtr->tkwin, gcMask, &gcValues);
    if (columnPtr->titleGC != NULL) {
	Tk_FreeGC(tvPtr->display, columnPtr->titleGC);
    }
    columnPtr->titleGC = newGC;

    /* Active title text */
    gcValues.foreground = columnPtr->activeTitleFgColor->pixel;
    newGC = Tk_GetGC(tvPtr->tkwin, gcMask, &gcValues);
    if (columnPtr->activeTitleGC != NULL) {
	Tk_FreeGC(tvPtr->display, columnPtr->activeTitleGC);
    }
    columnPtr->activeTitleGC = newGC;

    columnPtr->titleWidth = 2 * columnPtr->titleBorderWidth;
    iconWidth = iconHeight = 0;
    if (columnPtr->titleIcon != NULL) {
	iconWidth = Blt_Tv_IconWidth(columnPtr->titleIcon);
	iconHeight = Blt_Tv_IconHeight(columnPtr->titleIcon);
	columnPtr->titleWidth += iconWidth;
    }
    if (columnPtr->titleTextPtr != NULL) {
	Blt_Free(columnPtr->titleTextPtr);
	columnPtr->titleTextPtr = NULL;
    }
    textWidth = textHeight = 0;
    if (columnPtr->title != NULL) {
	TextStyle ts;

	Blt_Ts_InitStyle(ts);
	Blt_Ts_SetFont(ts, columnPtr->titleFont);
	columnPtr->titleTextPtr = Blt_Ts_CreateLayout(columnPtr->title, -1,&ts);
	textHeight = columnPtr->titleTextPtr->height;
	textWidth = columnPtr->titleTextPtr->width;
	columnPtr->titleWidth += textWidth;
    }
    if ((iconWidth > 0) && (textWidth > 0)) {
	columnPtr->titleWidth += 8;
    }
    columnPtr->titleWidth += TV_ARROW_WIDTH + 2;
    columnPtr->titleHeight = MAX(iconHeight, textHeight);
    gcMask = (GCFunction | GCLineWidth | GCLineStyle | GCForeground);

    /* 
     * If the rule is active, turn it off (i.e. draw again to erase
     * it) before changing the GC.  If the color changes, we won't be
     * able to erase the old line, since it will no longer be
     * correctly XOR-ed with the background.
     */
    drawable = Tk_WindowId(tvPtr->tkwin);
    ruleDrawn = ((tvPtr->flags & TV_RULE_ACTIVE) &&
		 (tvPtr->activeTitleColumnPtr == columnPtr) && 
		 (drawable != None));
    if (ruleDrawn) {
	Blt_Tv_DrawRule(tvPtr, columnPtr, drawable);
    }
    /* XOR-ed rule column divider */ 
    gcValues.line_width = LineWidth(columnPtr->ruleLineWidth);
    gcValues.foreground = 
	Blt_Tv_GetStyleFg(tvPtr, columnPtr->stylePtr)->pixel;
    if (LineIsDashed(columnPtr->ruleDashes)) {
	gcValues.line_style = LineOnOffDash;
    } else {
	gcValues.line_style = LineSolid;
    }
    gcValues.function = GXxor;

    bg = CHOOSE(tvPtr->bg, columnPtr->bg);
    gcValues.foreground ^= Blt_BackgroundBorderColor(bg)->pixel; 
    newGC = Blt_GetPrivateGC(tvPtr->tkwin, gcMask, &gcValues);
    if (columnPtr->ruleGC != NULL) {
	Blt_FreePrivateGC(tvPtr->display, columnPtr->ruleGC);
    }
    if (LineIsDashed(columnPtr->ruleDashes)) {
	Blt_SetDashes(tvPtr->display, newGC, &columnPtr->ruleDashes);
    }
    columnPtr->ruleGC = newGC;
    if (ruleDrawn) {
	Blt_Tv_DrawRule(tvPtr, columnPtr, drawable);
    }
    columnPtr->flags |= COLUMN_DIRTY;
    tvPtr->flags |= TV_UPDATE;
}

static void
DestroyColumn(Blt_Tv *tvPtr, Blt_Tv_Column *columnPtr)
{
    Blt_HashEntry *hPtr;

    bltTvUidOption.clientData = tvPtr;
    bltTvIconOption.clientData = tvPtr;
    styleOption.clientData = tvPtr;
    Blt_FreeOptions(columnSpecs, (char *)columnPtr, tvPtr->display, 0);
    if (columnPtr->titleGC != NULL) {
	Tk_FreeGC(tvPtr->display, columnPtr->titleGC);
    }
    if (columnPtr->ruleGC != NULL) {
	Blt_FreePrivateGC(tvPtr->display, columnPtr->ruleGC);
    }
    hPtr = Blt_FindHashEntry(&tvPtr->columnTable, columnPtr->key);
    if (hPtr != NULL) {
	Blt_DeleteHashEntry(&tvPtr->columnTable, hPtr);
    }
    if (columnPtr->link != NULL) {
	Blt_ChainDeleteLink(tvPtr->columns, columnPtr->link);
    }
    if (columnPtr->titleTextPtr != NULL) {
	Blt_Free(columnPtr->titleTextPtr);
    }
    if (columnPtr == &tvPtr->treeColumn) {
	columnPtr->titleTextPtr = NULL;
	columnPtr->link = NULL;
    } else {
	Blt_Free(columnPtr);
    }
}

void
Blt_Tv_DestroyColumns(Blt_Tv *tvPtr)
{
    if (tvPtr->columns != NULL) {
	Blt_ChainLink link;
	Blt_Tv_Column *columnPtr;
	
	for (link = Blt_ChainFirstLink(tvPtr->columns); link != NULL;
	     link = Blt_ChainNextLink(link)) {
	    columnPtr = Blt_ChainGetValue(link);
	    columnPtr->link = NULL;
	    DestroyColumn(tvPtr, columnPtr);
	}
	Blt_ChainDestroy(tvPtr->columns);
	tvPtr->columns = NULL;
    }
    Blt_DeleteHashTable(&tvPtr->columnTable);
}

int
Blt_Tv_CreateColumn(Blt_Tv *tvPtr, Blt_Tv_Column *columnPtr, const char *name, 
		    const char *defTitle)
{
    Blt_HashEntry *hPtr;
    int isNew;

    columnPtr->key = Blt_TreeGetKeyFromInterp(tvPtr->interp, name);
    columnPtr->title = Blt_StrdupAssert(defTitle);
    columnPtr->justify = TK_JUSTIFY_CENTER;
    columnPtr->relief = TK_RELIEF_FLAT;
    columnPtr->borderWidth = 1;
    columnPtr->pad.side1 = columnPtr->pad.side2 = 2;
    columnPtr->state = STATE_NORMAL;
    columnPtr->weight = 1.0;
    columnPtr->editable = FALSE;
    columnPtr->ruleLineWidth = 1;
    columnPtr->titleBorderWidth = 2;
    columnPtr->titleRelief = TK_RELIEF_RAISED;
    columnPtr->titleIcon = NULL;
    hPtr = Blt_CreateHashEntry(&tvPtr->columnTable, columnPtr->key, &isNew);
    Blt_SetHashValue(hPtr, columnPtr);

    bltTvUidOption.clientData = tvPtr;
    bltTvIconOption.clientData = tvPtr;
    styleOption.clientData = tvPtr;
    if (Blt_ConfigureComponentFromObj(tvPtr->interp, tvPtr->tkwin, name, 
	"Column", columnSpecs, 0, (Tcl_Obj **)NULL, (char *)columnPtr, 0) 
	!= TCL_OK) {
	DestroyColumn(tvPtr, columnPtr);
	return TCL_ERROR;
    }
    return TCL_OK;
}

static Blt_Tv_Column *
CreateColumn(Blt_Tv *tvPtr, Tcl_Obj *nameObjPtr, int objc, Tcl_Obj *const *objv)
{
    Blt_Tv_Column *columnPtr;

    columnPtr = Blt_CallocAssert(1, sizeof(Blt_Tv_Column));
    if (Blt_Tv_CreateColumn(tvPtr, columnPtr, Tcl_GetString(nameObjPtr), 
	Tcl_GetString(nameObjPtr)) != TCL_OK) {
	return NULL;
    }
    bltTvUidOption.clientData = tvPtr;
    bltTvIconOption.clientData = tvPtr;
    styleOption.clientData = tvPtr;
    if (Blt_ConfigureComponentFromObj(tvPtr->interp, tvPtr->tkwin, 
	columnPtr->key, "Column", columnSpecs, objc, objv, (char *)columnPtr, 
	BLT_CONFIG_OBJV_ONLY) != TCL_OK) {
	DestroyColumn(tvPtr, columnPtr);
	return NULL;
    }
    Blt_Tv_UpdateColumnGCs(tvPtr, columnPtr);
    return columnPtr;
}

Blt_Tv_Column *
Blt_Tv_NearestColumn(Blt_Tv *tvPtr, int x, int y, ClientData *contextPtr)
{
    if (tvPtr->nVisible > 0) {
	Blt_ChainLink link;
	Blt_Tv_Column *columnPtr;
	int right;

	/*
	 * Determine if the pointer is over the rightmost portion of the
	 * column.  This activates the rule.
	 */
	x = WORLDX(tvPtr, x);
	for(link = Blt_ChainFirstLink(tvPtr->columns); link != NULL;
	    link = Blt_ChainNextLink(link)) {
	    columnPtr = Blt_ChainGetValue(link);
	    right = columnPtr->worldX + columnPtr->width;
	    if ((x >= columnPtr->worldX) && (x <= right)) {
		if (contextPtr != NULL) {
		    *contextPtr = NULL;
		    if ((tvPtr->flags & TV_SHOW_COLUMN_TITLES) && 
			(y >= tvPtr->inset) &&
			(y < (tvPtr->titleHeight + tvPtr->inset))) {
			*contextPtr = (x >= (right - RULE_AREA)) 
			    ? ITEM_COLUMN_RULE : ITEM_COLUMN_TITLE;
		    } 
		}
		return columnPtr;
	    }
	}
    }
    return NULL;
}

/*
 *---------------------------------------------------------------------------
 *
 * ColumnActivateOp --
 *
 *	Selects the button to appear active.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ColumnActivateOp(Blt_Tv *tvPtr, Tcl_Interp *interp, int objc, 
		 Tcl_Obj *const *objv)
{
    if (objc == 4) {
	Drawable drawable;
	Blt_Tv_Column *columnPtr;
	char *string;

	string = Tcl_GetString(objv[3]);
	if (string[0] == '\0') {
	    columnPtr = NULL;
	} else {
	    if (Blt_Tv_GetColumn(interp, tvPtr, objv[3], &columnPtr) 
		!= TCL_OK) {
		return TCL_ERROR;
	    }
	    if (((tvPtr->flags & TV_SHOW_COLUMN_TITLES) == 0) || 
		(columnPtr->hidden) || (columnPtr->state == STATE_DISABLED)) {
		columnPtr = NULL;
	    }
	}
	tvPtr->activeTitleColumnPtr = tvPtr->activeColumnPtr = columnPtr;
	drawable = Tk_WindowId(tvPtr->tkwin);
	if (drawable != None) {
	    Blt_Tv_DrawHeadings(tvPtr, drawable);
	    Blt_Tv_DrawOuterBorders(tvPtr, drawable);
	}
    }
    if (tvPtr->activeTitleColumnPtr != NULL) {
	Tcl_SetStringObj(Tcl_GetObjResult(interp), 
			 tvPtr->activeTitleColumnPtr->key, -1);
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ColumnBindOp --
 *
 *	  .t bind tag sequence command
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ColumnBindOp(Blt_Tv *tvPtr, Tcl_Interp *interp, int objc, Tcl_Obj *const *objv)
{
    ClientData object;
    Blt_Tv_Column *columnPtr;

    if (Blt_Tv_GetColumn(NULL, tvPtr, objv[3], &columnPtr) == TCL_OK) {
	object = Blt_Tv_ColumnTag(tvPtr, columnPtr->key);
    } else {
	object = Blt_Tv_ColumnTag(tvPtr, Tcl_GetString(objv[3]));
    }
    return Blt_ConfigureBindingsFromObj(interp, tvPtr->bindTable, object,
	objc - 4, objv + 4);
}


/*
 *---------------------------------------------------------------------------
 *
 * ColumnCgetOp --
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ColumnCgetOp(Blt_Tv *tvPtr, Tcl_Interp *interp, int objc, Tcl_Obj *const *objv)
{
    Blt_Tv_Column *columnPtr;

    if (Blt_Tv_GetColumn(interp, tvPtr, objv[3], &columnPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    return Blt_ConfigureValueFromObj(interp, tvPtr->tkwin, columnSpecs, 
	(char *)columnPtr, objv[4], 0);
}

/*
 *---------------------------------------------------------------------------
 *
 * ColumnConfigureOp --
 *
 * 	This procedure is called to process a list of configuration
 *	options database, in order to reconfigure the one of more
 *	entries in the widget.
 *
 *	  .h entryconfigure node node node node option value
 *
 * Results:
 *	A standard Tcl result.  If TCL_ERROR is returned, then
 *	interp->result contains an error message.
 *
 * Side effects:
 *	Configuration information, such as text string, colors, font,
 *	etc. get set for tvPtr; old resources get freed, if there
 *	were any.  The hypertext is redisplayed.
 *
 *---------------------------------------------------------------------------
 */
static int
ColumnConfigureOp(Blt_Tv *tvPtr, Tcl_Interp *interp, int objc, 
		  Tcl_Obj *const *objv)
{
    int nOptions, start;
    int i;

    /* Figure out where the option value pairs begin */
    for(i = 3; i < objc; i++) {
	Blt_Tv_Column *columnPtr;

	if (Blt_ObjIsOption(columnSpecs, objv[i], 0)) {
	    break;
	}
	if (Blt_Tv_GetColumn(interp, tvPtr, objv[i], &columnPtr) 
	    != TCL_OK) {
	    return TCL_ERROR;
	}
    }
    start = i;
    nOptions = objc - start;
    
    bltTvUidOption.clientData = tvPtr;
    bltTvIconOption.clientData = tvPtr;
    styleOption.clientData = tvPtr;
    for (i = 3; i < start; i++) {
	Blt_Tv_Column *columnPtr;

	if (Blt_Tv_GetColumn(interp, tvPtr, objv[i], &columnPtr) 
	    != TCL_OK) {
	    return TCL_ERROR;
	}
	if (nOptions == 0) {
	    return Blt_ConfigureInfoFromObj(interp, tvPtr->tkwin, columnSpecs, 
		(char *)columnPtr, (Tcl_Obj *)NULL, 0);
	} else if (nOptions == 1) {
	    return Blt_ConfigureInfoFromObj(interp, tvPtr->tkwin, columnSpecs, 
		(char *)columnPtr, objv[start], 0);
	}
	if (Blt_ConfigureWidgetFromObj(tvPtr->interp, tvPtr->tkwin, 
	       columnSpecs, nOptions, objv + start, (char *)columnPtr, 
		BLT_CONFIG_OBJV_ONLY) != TCL_OK) {
	    return TCL_ERROR;
	}
	Blt_Tv_UpdateColumnGCs(tvPtr, columnPtr);
    }
    /*FIXME: Makes every change redo everything. */
    tvPtr->flags |= (TV_LAYOUT | TV_DIRTY);
    Blt_Tv_EventuallyRedraw(tvPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ColumnDeleteOp --
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ColumnDeleteOp(
    Blt_Tv *tvPtr,
    Tcl_Interp *interp,		/* Not used. */
    int objc,
    Tcl_Obj *const *objv)
{
    int i;

    for(i = 3; i < objc; i++) {
	Blt_Tv_Column *columnPtr;
	Blt_Tv_Entry *entryPtr;

	if (Blt_Tv_GetColumn(interp, tvPtr, objv[i], &columnPtr) 
	    != TCL_OK) {
	    return TCL_ERROR;
	}
	/* Traverse the tree deleting values associated with the column.  */
	for(entryPtr = tvPtr->rootPtr; entryPtr != NULL;
	    entryPtr = Blt_Tv_NextEntry(entryPtr, 0)) {
	    if (entryPtr != NULL) {
		Blt_Tv_Value *valuePtr, *lastPtr, *nextPtr;
		
		lastPtr = NULL;
		for (valuePtr = entryPtr->values; valuePtr != NULL; 
		     valuePtr = nextPtr) {
		    nextPtr = valuePtr->nextPtr;
		    if (valuePtr->columnPtr == columnPtr) {
			Blt_Tv_DestroyValue(tvPtr, valuePtr);
			if (lastPtr == NULL) {
			    entryPtr->values = nextPtr;
			} else {
			    lastPtr->nextPtr = nextPtr;
			}
			break;
		    }
		    lastPtr = valuePtr;
		}
	    }
	}
	DestroyColumn(tvPtr, columnPtr);
    }
    /* Deleting a column may affect the height of an entry. */
    tvPtr->flags |= (TV_LAYOUT | TV_DIRTY /*| TV_RESORT */);
    Blt_Tv_EventuallyRedraw(tvPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ColumnInsertOp --
 *
 *	Add new columns to the tree.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ColumnInsertOp(Blt_Tv *tvPtr, Tcl_Interp *interp, int objc, 
	       Tcl_Obj *const *objv)
{
    Blt_ChainLink before;
    Tcl_Obj *const *options;
    int i;
    int nOptions;
    int start;
    long insertPos;

    if (Blt_GetPositionFromObj(tvPtr->interp, objv[3], &insertPos) != TCL_OK) {
	return TCL_ERROR;
    }
    if ((insertPos == -1) || 
	(insertPos >= Blt_ChainGetLength(tvPtr->columns))) {
	before = NULL;
    } else {
	before =  Blt_ChainGetNthLink(tvPtr->columns, insertPos);
    }
    /*
     * Count the column names that follow.  Count the arguments until we
     * spot one that looks like a configuration option (i.e. starts
     * with a minus ("-")).
     */
    for (i = 4; i < objc; i++) {
	if (Blt_ObjIsOption(columnSpecs, objv[i], 0)) {
	    break;
	}
    }
    start = i;
    nOptions = objc - i;
    options = objv + start;

    for (i = 4; i < start; i++) {
	Blt_Tv_Column *columnPtr;
	Blt_Tv_Entry *entryPtr;

	if (Blt_Tv_GetColumn(NULL, tvPtr, objv[i], &columnPtr) == TCL_OK) {
	    Tcl_AppendResult(interp, "column \"", Tcl_GetString(objv[i]), 
		"\" already exists", (char *)NULL);
	    return TCL_ERROR;
	}
	columnPtr = CreateColumn(tvPtr, objv[i], nOptions, options);
	if (columnPtr == NULL) {
	    return TCL_ERROR;
	}
	if (before == NULL) {
	    columnPtr->link = Blt_ChainAppend(tvPtr->columns, columnPtr);
	} else {
	    columnPtr->link = Blt_ChainNewLink();
	    Blt_ChainSetValue(columnPtr->link, columnPtr);
	    Blt_ChainLinkBefore(tvPtr->columns, columnPtr->link, before);
	}
	/* 
	 * Traverse the tree adding column entries where needed.
	 */
	for(entryPtr = tvPtr->rootPtr; entryPtr != NULL;
	    entryPtr = Blt_Tv_NextEntry(entryPtr, 0)) {
	    Blt_Tv_AddValue(entryPtr, columnPtr);
	}
	Blt_Tv_TraceColumn(tvPtr, columnPtr);
    }
    Blt_Tv_EventuallyRedraw(tvPtr);
    return TCL_OK;
}



/*
 *---------------------------------------------------------------------------
 *
 * ColumnCurrentOp --
 *
 *	Make the rule to appear active.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ColumnCurrentOp(Blt_Tv *tvPtr, Tcl_Interp *interp, int objc, 
		Tcl_Obj *const *objv)
{
    ClientData context;
    Blt_Tv_Column *columnPtr;

    columnPtr = NULL;
    context = Blt_GetCurrentContext(tvPtr->bindTable);
    if ((context == ITEM_COLUMN_TITLE) || (context == ITEM_COLUMN_RULE)) {
	columnPtr = Blt_GetCurrentItem(tvPtr->bindTable);
    }
    if (context >= ITEM_STYLE) {
	Blt_Tv_Value *valuePtr = context;
	
	columnPtr = valuePtr->columnPtr;
    }
    if (columnPtr != NULL) {
	Tcl_SetStringObj(Tcl_GetObjResult(interp), columnPtr->key, -1);
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ColumnInvokeOp --
 *
 * 	This procedure is called to invoke a column command.
 *
 *	  .h column invoke columnName
 *
 * Results:
 *	A standard Tcl result.  If TCL_ERROR is returned, then
 *	interp->result contains an error message.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ColumnInvokeOp(Blt_Tv *tvPtr, Tcl_Interp *interp, int objc, 
	       Tcl_Obj *const *objv)
{
    Blt_Tv_Column *columnPtr;
    char *string;

    string = Tcl_GetString(objv[3]);
    if (string[0] == '\0') {
	return TCL_OK;
    }
    if (Blt_Tv_GetColumn(interp, tvPtr, objv[3], &columnPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    if ((columnPtr->state == STATE_NORMAL) && (columnPtr->titleCmd != NULL)) {
	int result;

	Tcl_Preserve(tvPtr);
	Tcl_Preserve(columnPtr);
	result = Tcl_GlobalEval(interp, columnPtr->titleCmd);
	Tcl_Release(columnPtr);
	Tcl_Release(tvPtr);
	return result;
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ColumnMoveOp --
 *
 *	Move a column.
 *
 * .h column move field1 position
 *---------------------------------------------------------------------------
 */

/*
 *---------------------------------------------------------------------------
 *
 * ColumnNamesOp --
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ColumnNamesOp(Blt_Tv *tvPtr, Tcl_Interp *interp, int objc, 
	      Tcl_Obj *const *objv)
{
    Blt_ChainLink link;
    Tcl_Obj *listObjPtr, *objPtr;
    Blt_Tv_Column *columnPtr;

    listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
    for(link = Blt_ChainFirstLink(tvPtr->columns); link != NULL;
	link = Blt_ChainNextLink(link)) {
	columnPtr = Blt_ChainGetValue(link);
	objPtr = Tcl_NewStringObj(columnPtr->key, -1);
	Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
    }
    Tcl_SetObjResult(interp, listObjPtr);
    return TCL_OK;
}

/*ARGSUSED*/
static int
ColumnNearestOp(Blt_Tv *tvPtr, Tcl_Interp *interp, int objc, 
		Tcl_Obj *const *objv)
{
    int x, y;			/* Screen coordinates of the test point. */
    Blt_Tv_Column *columnPtr;
    ClientData context;
    int checkTitle;
#ifdef notdef
    int isRoot;

    isRoot = FALSE;
    string = Tcl_GetString(objv[3]);

    if (strcmp("-root", string) == 0) {
	isRoot = TRUE;
	objv++, objc--;
    }
    if (objc != 5) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", 
		Tcl_GetString(objv[0]), " ", Tcl_GetString(objv[1]), 
		Tcl_GetString(objv[2]), " ?-root? x y\"", (char *)NULL);
	return TCL_ERROR;
			 
    }
#endif
    if (Tk_GetPixelsFromObj(interp, tvPtr->tkwin, objv[3], &x) != TCL_OK) {
	return TCL_ERROR;
    } 
    y = 0;
    checkTitle = FALSE;
    if (objc == 5) {
	if (Tk_GetPixelsFromObj(interp, tvPtr->tkwin, objv[4], &y) != TCL_OK) {
	    return TCL_ERROR;
	}
	checkTitle = TRUE;
    }
    columnPtr = Blt_Tv_NearestColumn(tvPtr, x, y, &context);
    if ((checkTitle) && (context == NULL)) {
	columnPtr = NULL;
    }
    if (columnPtr != NULL) {
	Tcl_SetStringObj(Tcl_GetObjResult(interp), columnPtr->key, -1);
    }
    return TCL_OK;
}

static void
UpdateMark(Blt_Tv *tvPtr, int newMark)
{
    Drawable drawable;
    Blt_Tv_Column *cp;
    int dx;
    int width;

    cp = tvPtr->resizeColumnPtr;
    if (cp == NULL) {
	return;
    }
    drawable = Tk_WindowId(tvPtr->tkwin);
    if (drawable == None) {
	return;
    }

    /* Erase any existing rule. */
    if (tvPtr->flags & TV_RULE_ACTIVE) { 
	Blt_Tv_DrawRule(tvPtr, cp, drawable);
    }
    
    dx = newMark - tvPtr->ruleAnchor; 
    width = cp->width - (PADDING(cp->pad) + 2 * cp->borderWidth);
    if ((cp->reqMin > 0) && ((width + dx) < cp->reqMin)) {
	dx = cp->reqMin - width;
    }
    if ((cp->reqMax > 0) && ((width + dx) > cp->reqMax)) {
	dx = cp->reqMax - width;
    }
    if ((width + dx) < 4) {
	dx = 4 - width;
    }
    tvPtr->ruleMark = tvPtr->ruleAnchor + dx;

    /* Redraw the rule if required. */
    if (tvPtr->flags & TV_RULE_NEEDED) {
	Blt_Tv_DrawRule(tvPtr, cp, drawable);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * ResizeActivateOp --
 *
 *	Turns on/off the resize cursor.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ResizeActivateOp(Blt_Tv *tvPtr, Tcl_Interp *interp, int objc, 
		 Tcl_Obj *const *objv)
{
    Blt_Tv_Column *columnPtr;
    char *string;

    string = Tcl_GetString(objv[4]);
    if (string[0] == '\0') {
	if (tvPtr->cursor != None) {
	    Tk_DefineCursor(tvPtr->tkwin, tvPtr->cursor);
	} else {
	    Tk_UndefineCursor(tvPtr->tkwin);
	}
	tvPtr->resizeColumnPtr = NULL;
    } else if (Blt_Tv_GetColumn(interp, tvPtr, objv[4], &columnPtr) 
	       == TCL_OK) {
	if (tvPtr->resizeCursor != None) {
	    Tk_DefineCursor(tvPtr->tkwin, tvPtr->resizeCursor);
	} 
	tvPtr->resizeColumnPtr = columnPtr;
    } else {
	return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ResizeAnchorOp --
 *
 *	Set the anchor for the resize.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ResizeAnchorOp(Blt_Tv *tvPtr, Tcl_Interp *interp, int objc, 
	       Tcl_Obj *const *objv)
{
    int x;

    if (Tcl_GetIntFromObj(NULL, objv[4], &x) != TCL_OK) {
	return TCL_ERROR;
    } 
    tvPtr->ruleAnchor = x;
    tvPtr->flags |= TV_RULE_NEEDED;
    UpdateMark(tvPtr, x);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ResizeMarkOp --
 *
 *	Sets the resize mark.  The distance between the mark and the anchor
 *	is the delta to change the width of the active column.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ResizeMarkOp(Blt_Tv *tvPtr, Tcl_Interp *interp, int objc, Tcl_Obj *const *objv)
{
    int x;

    if (Tcl_GetIntFromObj(NULL, objv[4], &x) != TCL_OK) {
	return TCL_ERROR;
    } 
    tvPtr->flags |= TV_RULE_NEEDED;
    UpdateMark(tvPtr, x);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ResizeSetOp --
 *
 *	Returns the new width of the column including the resize delta.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ResizeSetOp(Blt_Tv *tvPtr, Tcl_Interp *interp, int objc, Tcl_Obj *const *objv)
{
    tvPtr->flags &= ~TV_RULE_NEEDED;
    UpdateMark(tvPtr, tvPtr->ruleMark);
    if (tvPtr->resizeColumnPtr != NULL) {
	int width, delta;
	Blt_Tv_Column *columnPtr;

	columnPtr = tvPtr->resizeColumnPtr;
	delta = (tvPtr->ruleMark - tvPtr->ruleAnchor);
	width = tvPtr->resizeColumnPtr->width + delta - 
	    (PADDING(columnPtr->pad) + 2 * columnPtr->borderWidth) - 1;
	Tcl_SetIntObj(Tcl_GetObjResult(interp), width);
    }
    return TCL_OK;
}

static Blt_OpSpec resizeOps[] =
{ 
    {"activate", 2, ResizeActivateOp, 5, 5, "column"},
    {"anchor", 2, ResizeAnchorOp, 5, 5, "x"},
    {"mark", 1, ResizeMarkOp, 5, 5, "x"},
    {"set", 1, ResizeSetOp, 4, 4, "",},
};

static int nResizeOps = sizeof(resizeOps) / sizeof(Blt_OpSpec);

/*
 *---------------------------------------------------------------------------
 *
 * ColumnResizeOp --
 *
 *---------------------------------------------------------------------------
 */
static int
ColumnResizeOp(Blt_Tv *tvPtr, Tcl_Interp *interp, int objc, 
	       Tcl_Obj *const *objv)
{
    TvCmdProc *proc;
    int result;

    proc = Blt_GetOpFromObj(interp, nResizeOps, resizeOps, BLT_OP_ARG3, 
	objc, objv,0);
    if (proc == NULL) {
	return TCL_ERROR;
    }
    result = (*proc) (tvPtr, interp, objc, objv);
    return result;
}


static Blt_OpSpec columnOps[] =
{
    {"activate", 1, ColumnActivateOp, 3, 4, "?field?",},
    {"bind", 1, ColumnBindOp, 4, 6, "tagName ?sequence command?",},
    {"cget", 2, ColumnCgetOp, 5, 5, "field option",},
    {"configure", 2, ColumnConfigureOp, 4, 0, 
	"field ?option value?...",},
    {"current", 2, ColumnCurrentOp, 3, 3, "",},
    {"delete", 1, ColumnDeleteOp, 3, 0, "?field...?",},
    {"highlight", 1, ColumnActivateOp, 3, 4, "?field?",},
    {"insert", 3, ColumnInsertOp, 5, 0, 
	"position field ?field...? ?option value?...",},
    {"invoke", 3, ColumnInvokeOp, 4, 4, "field",},
    {"names", 2, ColumnNamesOp, 3, 3, "",},
    {"nearest", 2, ColumnNearestOp, 4, 5, "x ?y?",},
    {"resize", 1, ColumnResizeOp, 3, 0, "arg",},
};
static int nColumnOps = sizeof(columnOps) / sizeof(Blt_OpSpec);

/*
 *---------------------------------------------------------------------------
 *
 * Blt_Tv_ColumnOp --
 *
 *---------------------------------------------------------------------------
 */
int
Blt_Tv_ColumnOp(Blt_Tv *tvPtr, Tcl_Interp *interp, int objc, 
		Tcl_Obj *const *objv)
{
    TvCmdProc *proc;
    int result;

    proc = Blt_GetOpFromObj(interp, nColumnOps, columnOps, BLT_OP_ARG2, 
	objc, objv,0);
    if (proc == NULL) {
	return TCL_ERROR;
    }
    result = (*proc) (tvPtr, interp, objc, objv);
    return result;
}


static int
InvokeCompare(Blt_Tv *tvPtr, Blt_Tv_Entry *e1Ptr, Blt_Tv_Entry *e2Ptr, 
	      const char *command)
{
    int result;
    Tcl_Obj *objv[8];
    int i;

    objv[0] = Tcl_NewStringObj(command, -1);
    objv[1] = Tcl_NewStringObj(Tk_PathName(tvPtr->tkwin), -1);
    objv[2] = Tcl_NewLongObj(Blt_TreeNodeId(e1Ptr->node));
    objv[3] = Tcl_NewLongObj(Blt_TreeNodeId(e2Ptr->node));
    objv[4] = Tcl_NewStringObj(tvPtr->sortColumnPtr->key, -1);
	     
    if (tvPtr->flatView) {
	objv[5] = Tcl_NewStringObj(e1Ptr->fullName, -1);
	objv[6] = Tcl_NewStringObj(e2Ptr->fullName, -1);
    } else {
	objv[5] = Tcl_NewStringObj(GETLABEL(e1Ptr), -1);
	objv[6] = Tcl_NewStringObj(GETLABEL(e2Ptr), -1);
    }
    for(i = 0; i < 7; i++) {
	Tcl_IncrRefCount(objv[i]);
    }
    objv[7] = NULL;
    result = Tcl_EvalObjv(tvPtr->interp, 7, objv, TCL_EVAL_GLOBAL);
    if ((result != TCL_OK) ||
	(Tcl_GetIntFromObj(tvPtr->interp, Tcl_GetObjResult(tvPtr->interp), 
			   &result) != TCL_OK)) {
	Tcl_BackgroundError(tvPtr->interp);
    }
    for(i = 0; i < 7; i++) {
	Tcl_DecrRefCount(objv[i]);
    }
    Tcl_ResetResult(tvPtr->interp);
    return result;
}

static Blt_Tv *treeViewInstance;

static int
CompareEntries(const void *a, const void *b)
{
    Blt_Tv *tvPtr;
    Blt_Tv_Entry **e1PtrPtr = (Blt_Tv_Entry **)a;
    Blt_Tv_Entry **e2PtrPtr = (Blt_Tv_Entry **)b;
    Tcl_Obj *obj1, *obj2;
    const char *s1, *s2;
    int result;

    tvPtr = (*e1PtrPtr)->tvPtr;
    obj1 = (*e1PtrPtr)->dataObjPtr;
    obj2 = (*e2PtrPtr)->dataObjPtr;
    s1 = Tcl_GetString(obj1);
    s2 = Tcl_GetString(obj2);
    result = 0;
    switch (tvPtr->sortType) {
    case SORT_ASCII:
	result = strcmp(s1, s2);
	break;

    case SORT_COMMAND:
	{
	    const char *cmd;

	    cmd = tvPtr->sortColumnPtr->sortCmd;
	    if (cmd == NULL) {
		cmd = tvPtr->sortCmd;
	    }
	    if (cmd == NULL) {
		result = Blt_DictionaryCompare(s1, s2);
	    } else {
		result = InvokeCompare(tvPtr, *e1PtrPtr, *e2PtrPtr, cmd);
	    }
	}
	break;

    case SORT_DICTIONARY:
	result = Blt_DictionaryCompare(s1, s2);
	break;

    case SORT_INTEGER:
	{
	    int i1, i2;

	    if (Tcl_GetIntFromObj(NULL, obj1, &i1)==TCL_OK) {
		if (Tcl_GetIntFromObj(NULL, obj2, &i2) == TCL_OK) {
		    result = i1 - i2;
		} else {
		    result = -1;
		} 
	    } else if (Tcl_GetIntFromObj(NULL, obj2, &i2) == TCL_OK) {
		result = 1;
	    } else {
		result = Blt_DictionaryCompare(s1, s2);
	    }
	}
	break;

    case SORT_REAL:
	{
	    double r1, r2;

	    if (Tcl_GetDoubleFromObj(NULL, obj1, &r1) == TCL_OK) {
		if (Tcl_GetDoubleFromObj(NULL, obj2, &r2) == TCL_OK) {
		    result = (r1 < r2) ? -1 : (r1 > r2) ? 1 : 0;
		} else {
		    result = -1;
		} 
	    } else if (Tcl_GetDoubleFromObj(NULL, obj2, &r2) == TCL_OK) {
		result = 1;
	    } else {
		result = Blt_DictionaryCompare(s1, s2);
	    }
	}
	break;
    }
    if (tvPtr->sortDecreasing) {
	return -result;
    } 
    return result;
}


/*
 *---------------------------------------------------------------------------
 *
 * CompareNodes --
 *
 *	Comparison routine (used by qsort) to sort a chain of subnodes.
 *
 * Results:
 *	1 is the first is greater, -1 is the second is greater, 0
 *	if equal.
 *
 *---------------------------------------------------------------------------
 */
static int
CompareNodes(Blt_TreeNode *n1Ptr, Blt_TreeNode *n2Ptr)
{
    Blt_Tv *tvPtr = treeViewInstance;
    Blt_Tv_Entry *e1Ptr, *e2Ptr;

    e1Ptr = Blt_NodeToEntry(tvPtr, *n1Ptr);
    e2Ptr = Blt_NodeToEntry(tvPtr, *n2Ptr);

    /* Fetch the data for sorting. */
    if (tvPtr->sortType == SORT_COMMAND) {
	e1Ptr->dataObjPtr = Tcl_NewLongObj(Blt_TreeNodeId(*n1Ptr));
	e2Ptr->dataObjPtr = Tcl_NewLongObj(Blt_TreeNodeId(*n2Ptr));
    } else if (tvPtr->sortColumnPtr == &tvPtr->treeColumn) {
	Tcl_DString dString;

	Tcl_DStringInit(&dString);
	if (e1Ptr->fullName == NULL) {
	    Blt_Tv_GetFullName(tvPtr, e1Ptr, TRUE, &dString);
	    e1Ptr->fullName = Blt_StrdupAssert(Tcl_DStringValue(&dString));
	}
	e1Ptr->dataObjPtr = Tcl_NewStringObj(e1Ptr->fullName, -1);
	if (e2Ptr->fullName == NULL) {
	    Blt_Tv_GetFullName(tvPtr, e2Ptr, TRUE, &dString);
	    e2Ptr->fullName = Blt_StrdupAssert(Tcl_DStringValue(&dString));
	}
	e2Ptr->dataObjPtr = Tcl_NewStringObj(e2Ptr->fullName, -1);
	Tcl_DStringFree(&dString);
    } else {
	Blt_TreeKey key;
	Tcl_Obj *objPtr;

	key = tvPtr->sortColumnPtr->key;
	if (Blt_Tv_GetData(e1Ptr, key, &objPtr) != TCL_OK) {
	    e1Ptr->dataObjPtr = Blt_EmptyStringObj();
	} else {
	    e1Ptr->dataObjPtr = objPtr;
	}
	if (Blt_Tv_GetData(e2Ptr, key, &objPtr) != TCL_OK) {
	    e2Ptr->dataObjPtr = Blt_EmptyStringObj();
	} else {
	    e2Ptr->dataObjPtr = objPtr;
	}
    }
    return CompareEntries(&e1Ptr, &e2Ptr);
}

static int
SortAutoOp(Blt_Tv *tvPtr, Tcl_Interp *interp, int objc, Tcl_Obj *const *objv)
{

    if (objc == 4) {
	int bool;
	int isAuto;

	isAuto = ((tvPtr->flags & TV_SORT_AUTO) != 0);
	if (Tcl_GetBooleanFromObj(interp, objv[3], &bool) != TCL_OK) {
	    return TCL_ERROR;
	}
	if (isAuto != bool) {
	    tvPtr->flags |= (TV_LAYOUT | TV_DIRTY | TV_RESORT);
	    Blt_Tv_EventuallyRedraw(tvPtr);
	}
	if (bool) {
	    tvPtr->flags |= TV_SORT_AUTO;
	} else {
	    tvPtr->flags &= ~TV_SORT_AUTO;
	}
    }
    Tcl_SetBooleanObj(Tcl_GetObjResult(interp), (tvPtr->flags & TV_SORT_AUTO));
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * SortCgetOp --
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
SortCgetOp(Blt_Tv *tvPtr, Tcl_Interp *interp, int objc, Tcl_Obj *const *objv)
{
    return Blt_ConfigureValueFromObj(interp, tvPtr->tkwin, sortSpecs, 
	(char *)tvPtr, objv[3], 0);
}

/*
 *---------------------------------------------------------------------------
 *
 * SortConfigureOp --
 *
 * 	This procedure is called to process a list of configuration
 *	options database, in order to reconfigure the one of more
 *	entries in the widget.
 *
 *	  .h sort configure option value
 *
 * Results:
 *	A standard Tcl result.  If TCL_ERROR is returned, then
 *	interp->result contains an error message.
 *
 * Side effects:
 *	Configuration information, such as text string, colors, font,
 *	etc. get set for tvPtr; old resources get freed, if there
 *	were any.  The hypertext is redisplayed.
 *
 *---------------------------------------------------------------------------
 */
static int
SortConfigureOp(Blt_Tv *tvPtr, Tcl_Interp *interp, int objc, 
		Tcl_Obj *const *objv)
{
    int oldType;
    const char *oldCommand;
    Blt_Tv_Column *oldColumn;

    if (objc == 3) {
	return Blt_ConfigureInfoFromObj(interp, tvPtr->tkwin, sortSpecs, 
		(char *)tvPtr, (Tcl_Obj *)NULL, 0);
    } else if (objc == 4) {
	return Blt_ConfigureInfoFromObj(interp, tvPtr->tkwin, sortSpecs, 
		(char *)tvPtr, objv[3], 0);
    }
    oldColumn = tvPtr->sortColumnPtr;
    oldType = tvPtr->sortType;
    oldCommand = tvPtr->sortCmd;
    if (Blt_ConfigureWidgetFromObj(interp, tvPtr->tkwin, sortSpecs, 
	objc - 3, objv + 3, (char *)tvPtr, BLT_CONFIG_OBJV_ONLY) != TCL_OK) {
	return TCL_ERROR;
    }
    if ((oldColumn != tvPtr->sortColumnPtr) || (oldType != tvPtr->sortType) ||
	(oldCommand != tvPtr->sortCmd)) {
	tvPtr->flags &= ~TV_SORTED;
	tvPtr->flags |= (TV_DIRTY | TV_RESORT);
    } 
    if (tvPtr->flags & TV_SORT_AUTO) {
	tvPtr->flags |= TV_SORT_PENDING;
    }
    Blt_Tv_EventuallyRedraw(tvPtr);
    return TCL_OK;
}

/*ARGSUSED*/
static int
SortOnceOp(
    Blt_Tv *tvPtr,
    Tcl_Interp *interp,		/* Not used. */
    int objc,
    Tcl_Obj *const *objv)
{
    int recurse;

    recurse = FALSE;
    if (objc > 3) {
	char *string;
	int length;

	string = Tcl_GetStringFromObj(objv[3], &length);
	if ((string[0] == '-') && (length > 1) &&
	    (strncmp(string, "-recurse", length) == 0)) {
	    objv++, objc--;
	    recurse = TRUE;
	}
    }
#ifdef notdef
    { 
	int i;

	treeViewInstance = tvPtr;
	for (i = 3; i < objc; i++) {
	    Blt_Tv_Entry *entryPtr;
	    int result;
	    
	    if (Blt_Tv_GetEntry(tvPtr, objv[i], &entryPtr) != TCL_OK) {
		return TCL_ERROR;
	    }
	    if (recurse) {
		result = Blt_TreeApply(entryPtr->node, SortApplyProc, tvPtr);
	    } else {
		result = SortApplyProc(entryPtr->node, tvPtr, TREE_PREORDER);
	    }
	    if (result != TCL_OK) {
		return TCL_ERROR;
	    }
	}
    }
#endif
    tvPtr->flags |= TV_LAYOUT | TV_DIRTY | TV_UPDATE | TV_SORT_PENDING | TV_RESORT;
    Blt_Tv_EventuallyRedraw(tvPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_Tv_SortOp --
 *
 *	Comparison routine (used by qsort) to sort a chain of subnodes.
 *	A simple string comparison is performed on each node name.
 *
 *	.h sort auto
 *	.h sort once -recurse root
 *
 * Results:
 *	1 is the first is greater, -1 is the second is greater, 0
 *	if equal.
 *
 *---------------------------------------------------------------------------
 */
static Blt_OpSpec sortOps[] =
{
    {"auto", 1, SortAutoOp, 3, 4, "?boolean?",},
    {"cget", 2, SortCgetOp, 4, 4, "option",},
    {"configure", 2, SortConfigureOp, 3, 0, "?option value?...",},
    {"once", 1, SortOnceOp, 3, 0, "?-recurse? node...",},
};
static int nSortOps = sizeof(sortOps) / sizeof(Blt_OpSpec);

/*ARGSUSED*/
int
Blt_Tv_SortOp(Blt_Tv *tvPtr, Tcl_Interp *interp, int objc, Tcl_Obj *const *objv)
{
    TvCmdProc *proc;
    int result;

    proc = Blt_GetOpFromObj(interp, nSortOps, sortOps, BLT_OP_ARG2, objc, 
	    objv, 0);
    if (proc == NULL) {
	return TCL_ERROR;
    }
    result = (*proc) (tvPtr, interp, objc, objv);
    return result;
}

/*
 *---------------------------------------------------------------------------
 *
 * SortApplyProc --
 *
 *	Sorts the subnodes at a given node.
 *
 * Results:
 *	Always returns TCL_OK.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
SortApplyProc(Blt_TreeNode node, ClientData clientData, int order)
{
    Blt_Tv *tvPtr = clientData;

    if (!Blt_TreeIsLeaf(node)) {
	Blt_TreeSortNode(tvPtr->tree, node, CompareNodes);
    }
    return TCL_OK;
}
 
/*
 *---------------------------------------------------------------------------
 *
 * Blt_Tv_SortFlatView --
 *
 *	Sorts the flatten array of entries.
 *
 *---------------------------------------------------------------------------
 */
void
Blt_Tv_SortFlatView(Blt_Tv *tvPtr)
{
    Blt_Tv_Entry *entryPtr, **p;

    tvPtr->flags &= ~TV_SORT_PENDING;
    if ((tvPtr->sortType == SORT_NONE) || (tvPtr->nEntries < 2)) {
	return;
    }
    if (tvPtr->flags & TV_SORTED) {
	int first, last;
	Blt_Tv_Entry *hold;

	if (tvPtr->sortDecreasing == tvPtr->viewIsDecreasing) {
	    return;
	}

	/* 
	 * The view is already sorted but in the wrong direction. 
	 * Reverse the entries in the array.
	 */
 	for (first = 0, last = tvPtr->nEntries - 1; last > first; 
	     first++, last--) {
	    hold = tvPtr->flatArr[first];
	    tvPtr->flatArr[first] = tvPtr->flatArr[last];
	    tvPtr->flatArr[last] = hold;
	}
	tvPtr->viewIsDecreasing = tvPtr->sortDecreasing;
	tvPtr->flags |= TV_SORTED | TV_LAYOUT;
	return;
    }
    /* Fetch each entry's data as Tcl_Objs for sorting. */
    if (tvPtr->sortColumnPtr == &tvPtr->treeColumn) {
	for(p = tvPtr->flatArr; *p != NULL; p++) {
	    entryPtr = *p;
	    if (entryPtr->fullName == NULL) {
		Tcl_DString dString;

		Blt_Tv_GetFullName(tvPtr, entryPtr, TRUE, &dString);
		entryPtr->fullName = 
		    Blt_StrdupAssert(Tcl_DStringValue(&dString));
		Tcl_DStringFree(&dString);
	    }
	    entryPtr->dataObjPtr = Tcl_NewStringObj(entryPtr->fullName, -1);
	    Tcl_IncrRefCount(entryPtr->dataObjPtr);
	}
    } else {
	Blt_TreeKey key;
	Tcl_Obj *objPtr;

	key = tvPtr->sortColumnPtr->key;
	for(p = tvPtr->flatArr; *p != NULL; p++) {
	    entryPtr = *p;
	    if (Blt_Tv_GetData(entryPtr, key, &objPtr) != TCL_OK) {
		objPtr = Blt_EmptyStringObj();
	    }
	    entryPtr->dataObjPtr = objPtr;
	    Tcl_IncrRefCount(entryPtr->dataObjPtr);
	}
    }
    qsort((char *)tvPtr->flatArr, tvPtr->nEntries, sizeof(Blt_Tv_Entry *),
	  (QSortCompareProc *)CompareEntries);

    /* Free all the Tcl_Objs used for comparison data. */
    for(p = tvPtr->flatArr; *p != NULL; p++) {
	Tcl_DecrRefCount((*p)->dataObjPtr);
    }
    tvPtr->viewIsDecreasing = tvPtr->sortDecreasing;
    tvPtr->flags |= TV_SORTED;
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_Tv_SortView --
 *
 *	Sorts the tree array of entries.
 *
 *---------------------------------------------------------------------------
 */
void
Blt_Tv_SortView(Blt_Tv *tvPtr)
{
    tvPtr->flags &= ~TV_SORT_PENDING;
    if (tvPtr->sortType != SORT_NONE) {
	treeViewInstance = tvPtr;
	Blt_TreeApply(tvPtr->rootPtr->node, SortApplyProc, tvPtr);
    }
    tvPtr->viewIsDecreasing = tvPtr->sortDecreasing;
}


#endif /* NO_TREEVIEW */
