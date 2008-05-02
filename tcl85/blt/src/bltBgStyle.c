
/*
 * bltBgStyle.c --
 *
 * This module creates background styles for the BLT toolkit.
 *
 *	Copyright 1995-2004 George A Howlett.
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
#include "bltOp.h"
#include "bltChain.h"
#include "bltHash.h"
#include "bltPicture.h"
#include "bltPainter.h"
#include <X11/Xutil.h>
#include "bltBgStyle.h"

#define BG_STYLE_THREAD_KEY	"BLT Background Style Data"

/* 
  bgstyle create tile 
	-relativeto self|toplevel|window
	-picture $image
	-bg $color
  bgstyle create picture 
        -picture $image
	-filter $filterName
	-bg $color
  bgstyle create gradient 
	-type radial|xlinear|ylinear|diagonal
	-low $color
	-high $color
	-bg $color
  bgstyle create border 
	-bg $color

  bgstyle create texture -type metal|wind|??? 
	-bg $color

  bgstyle names
  bgstyle configure $tile
  bgstyle delete $tile
*/


enum StyleTypes {
    STYLE_BORDER,		/* Standard 3D border. */
    STYLE_PICTURE,		/* Resizable color picture. */
    STYLE_TILE,			/* Color picture tiled. */
    STYLE_GRADIENT,		/* Color gradient. */
    STYLE_TEXTURE,		/* Procedural texture. */
};

static const char *styleTypes[] = {
    "border",
    "picture",
    "tile",
    "gradient",
    "texture"
};

enum ReferenceTypes {
    REFERENCE_SELF,		/* Current window. */
    REFERENCE_TOPLEVEL,		/* Toplevel of current window. */
    REFERENCE_WINDOW,		/* Specifically named window. */
    REFERENCE_NONE,		/* Don't use reference window. Background
				 * region will be defined by user. */
};

typedef struct {
    int x, y, width, height;
} BgRegion;

typedef struct {
    Blt_HashTable styleTable;	/* Hash table of tile structures keyed by 
				 * the name of the image. */

    Tcl_Interp *interp;		/* Interpreter associated with this
				 * set of background styles. */

    int nextId;			/* Serial number of the identifier to
				 * be used for next background style
				 * created.  */
} BgStyleInterpData;


typedef struct {
    char *name;			/* Generated name of background
				 * style. */

    int style;			/* Type of background style: border,
				 * tile, texture, or gradient. */

    Blt_ConfigSpec *configSpecs;

    BgStyleInterpData *dataPtr;

    Tk_Window tkwin;		/* Main window. Used to query background style
				 * options. */

    Display *display;		/* Display of this background style. */

    int flags;			/* See definitions below. */

    Blt_HashEntry *hashPtr;	/*  */

    Tk_3DBorder border;		/* 3D Border.  May be used for all background
				 * types. */
    
    Blt_HashTable pictTable;	/* Table of pictures cached for each
				 * background reference. */

    char *origName;		/* Name of original picture. */

    Blt_Picture original;	/* Original (not resampled) picture for
				 * "picture" and "tile" backgrounds. */

    Blt_ResampleFilter filter;	/* 1-D image filter to use to when resizing
				 * the original picture. */
    Blt_Gradient gradient;

    Blt_Pixel low, high;	/* Texture or gradient colors. */

    int reference;		/* "self", "toplevel", or "window". */

    int xOrigin, yOrigin;

    BgRegion refRegion;

    Tk_Window refWindow;	/* Refer to coordinates in this window when
				 * determining the tile origin. */

    Blt_Chain chain;		/* List of style tokens.  Used to register
				 * callbacks for each client of the background
				 * style. */
} BgStyleObject;

struct Blt_BackgroundStruct {
    BgStyleObject *corePtr;	/* Pointer to master background style
				 * object. */

    Blt_BackgroundChangedProc *notifyProc;

    ClientData clientData;	/* Data to be passed on notifier
				 * callbacks.  */

    Blt_ChainLink link;		/* Link of this entry in notifier list. */

};

typedef struct Blt_BackgroundStruct BgStyle;

#define DEF_BORDER		STD_NORMAL_BACKGROUND
#define DEF_GRADIENT_PATH	"y"
#define DEF_GRADIENT_HIGH	"grey90"
#define DEF_GRADIENT_JITTER	"no"
#define DEF_GRADIENT_LOGSCALE	"yes"
#define DEF_GRADIENT_LOW	"grey50"
#define DEF_GRADIENT_MODE	"xlinear"
#define DEF_GRADIENT_SHAPE	"linear"
#define DEF_REFERENCE		"toplevel"
#define DEF_RESAMPLE_FILTER	"box"

static Blt_OptionParseProc ObjToPictureProc;
static Blt_OptionPrintProc PictureToObjProc;
static Blt_CustomOption pictureOption =
{
    ObjToPictureProc, PictureToObjProc, NULL, (ClientData)0
};

static Blt_OptionParseProc ObjToFilterProc;
static Blt_OptionPrintProc FilterToObjProc;
static Blt_CustomOption filterOption =
{
    ObjToFilterProc, FilterToObjProc, NULL, (ClientData)0
};

static Blt_OptionParseProc ObjToReferenceProc;
static Blt_OptionPrintProc ReferenceToObjProc;
static Blt_CustomOption referenceToOption =
{
    ObjToReferenceProc, ReferenceToObjProc, NULL, (ClientData)0
};

static Blt_OptionParseProc ObjToShapeProc;
static Blt_OptionPrintProc ShapeToObjProc;
static Blt_CustomOption shapeOption =
{
    ObjToShapeProc, ShapeToObjProc, NULL, (ClientData)0
};

static Blt_OptionParseProc ObjToPathProc;
static Blt_OptionPrintProc PathToObjProc;
static Blt_CustomOption pathOption =
{
    ObjToPathProc, PathToObjProc, NULL, (ClientData)0
};

static Blt_ConfigSpec borderConfigSpecs[] =
{
    {BLT_CONFIG_BORDER, "-background", "background", "Background", DEF_BORDER, 
	Blt_Offset(BgStyleObject, border), 0},
    {BLT_CONFIG_SYNONYM, "-bg", "background", (char *)NULL, (char *)NULL, 0, 0},
    {BLT_CONFIG_END, NULL, NULL, NULL, NULL, 0, 0}
};

static Blt_ConfigSpec tileConfigSpecs[] =
{
    {BLT_CONFIG_BORDER, "-background", "background", "Background", DEF_BORDER, 
	Blt_Offset(BgStyleObject, border), 0},
    {BLT_CONFIG_SYNONYM, "-bg", "background", (char *)NULL, (char *)NULL, 0, 0},
    {BLT_CONFIG_CUSTOM, "-picture", "picture", "Picture", (char *)NULL,
        Blt_Offset(BgStyleObject, original), BLT_CONFIG_DONT_SET_DEFAULT, 
	&pictureOption},
    {BLT_CONFIG_CUSTOM, "-relativeto", "relativeTo", "RelativeTo", 
	DEF_REFERENCE, Blt_Offset(BgStyleObject, reference), 
	BLT_CONFIG_DONT_SET_DEFAULT, &referenceToOption},
    {BLT_CONFIG_END, NULL, NULL, NULL, NULL, 0, 0}
};

static Blt_ConfigSpec pictureConfigSpecs[] =
{
    {BLT_CONFIG_BORDER, "-background", "background", "Background", DEF_BORDER, 
	Blt_Offset(BgStyleObject, border), 0},
    {BLT_CONFIG_SYNONYM, "-bg", "background", (char *)NULL, (char *)NULL, 0, 0},
    {BLT_CONFIG_CUSTOM, "-filter", "filter", "Filter", DEF_RESAMPLE_FILTER,
        Blt_Offset(BgStyleObject, filter), 0, &filterOption},
    {BLT_CONFIG_CUSTOM, "-picture", "picture", "Picture", (char *)NULL,
        Blt_Offset(BgStyleObject, original), BLT_CONFIG_DONT_SET_DEFAULT, 
	&pictureOption},
    {BLT_CONFIG_CUSTOM, "-relativeto", "relativeTo", "RelativeTo", 
	DEF_REFERENCE, Blt_Offset(BgStyleObject, reference), 
	BLT_CONFIG_DONT_SET_DEFAULT, &referenceToOption},
    {BLT_CONFIG_END, NULL, NULL, NULL, NULL, 0, 0}
};

static Blt_ConfigSpec gradientConfigSpecs[] =
{
    {BLT_CONFIG_BORDER, "-background", "background", "Background", DEF_BORDER,
	Blt_Offset(BgStyleObject, border), 0},
    {BLT_CONFIG_SYNONYM, "-bg", "background", (char *)NULL, (char *)NULL, 0, 0},
    {BLT_CONFIG_CUSTOM, "-direction", "direction", "Direction", 
	DEF_GRADIENT_PATH, Blt_Offset(BgStyleObject, gradient.path), 
	BLT_CONFIG_DONT_SET_DEFAULT, &pathOption},
    {BLT_CONFIG_PIX32, "-high", "high", "High", DEF_GRADIENT_HIGH,
        Blt_Offset(BgStyleObject, high), 0},
    {BLT_CONFIG_BOOLEAN, "-jitter", "jitter", "Jitter", 
	DEF_GRADIENT_JITTER, Blt_Offset(BgStyleObject, gradient.jitter), 
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_BOOLEAN, "-logscale", "logscale", "Logscale", 
	DEF_GRADIENT_LOGSCALE, Blt_Offset(BgStyleObject, gradient.logScale), 
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_PIX32, "-low", "low", "Low", DEF_GRADIENT_LOW,
        Blt_Offset(BgStyleObject, low), 0},
    {BLT_CONFIG_CUSTOM, "-relativeto", "relativeTo", "RelativeTo", 
	DEF_REFERENCE, Blt_Offset(BgStyleObject, reference), 
	BLT_CONFIG_DONT_SET_DEFAULT, &referenceToOption},
    {BLT_CONFIG_CUSTOM, "-shape", "shape", "Shape", DEF_GRADIENT_SHAPE, 
	Blt_Offset(BgStyleObject, gradient.shape), BLT_CONFIG_DONT_SET_DEFAULT, 
	&shapeOption},
    {BLT_CONFIG_END, NULL, NULL, NULL, NULL, 0, 0}
};

static Blt_ConfigSpec textureConfigSpecs[] =
{
    {BLT_CONFIG_BORDER, "-background", "background", "Background", DEF_BORDER,
	Blt_Offset(BgStyleObject, border), 0},
    {BLT_CONFIG_SYNONYM, "-bg", "background", (char *)NULL, (char *)NULL, 0, 0},
    {BLT_CONFIG_PIX32, "-high", "high", "High", DEF_GRADIENT_HIGH,
        Blt_Offset(BgStyleObject, high), 0},
    {BLT_CONFIG_PIX32, "-low", "low", "Low", DEF_GRADIENT_LOW,
        Blt_Offset(BgStyleObject, low), 0},
    {BLT_CONFIG_CUSTOM, "-relativeto", "relativeTo", "RelativeTo", 
	DEF_REFERENCE, Blt_Offset(BgStyleObject, reference), 
	BLT_CONFIG_DONT_SET_DEFAULT, &referenceToOption},
    {BLT_CONFIG_END, NULL, NULL, NULL, NULL, 0, 0}
};

/*
 *---------------------------------------------------------------------------
 *
 * ObjToFilterProc --
 *
 *	Given a string name, get the resample filter associated with it.
 *
 * Results:
 *	The return value is a standard Tcl result.  
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToFilterProc(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,		/* Interpreter to send results back to */
    Tk_Window tkwin,		/* Not used. */
    Tcl_Obj *objPtr,		/* String representation of value. */
    char *widgRec,		/* Widget record. */
    int offset,			/* Offset to field in structure */
    int flags)	
{
    Blt_ResampleFilter *filterPtr = (Blt_ResampleFilter *)(widgRec + offset);
    char *string;

    string = Tcl_GetString(objPtr);
    if (string[0] == '\0') {
	*filterPtr = NULL;
	return TCL_OK;
    }
    return Blt_GetResampleFilterFromObj(interp, objPtr, filterPtr);
}

/*
 *---------------------------------------------------------------------------
 *
 * FilterToObjProc --
 *
 *	Convert the picture filter into a string Tcl_Obj.
 *
 * Results:
 *	The string representation of the filter is returned.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
FilterToObjProc(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,
    Tk_Window tkwin,		/* Not used. */
    char *widgRec,		/* Widget record */
    int offset,			/* Offset to field in structure */
    int flags)	
{
    Blt_ResampleFilter filter = *(Blt_ResampleFilter *)(widgRec + offset);

    if (filter == NULL) {
	return Tcl_NewStringObj("", -1);
    }
    return Tcl_NewStringObj(Blt_NameOfResampleFilter(filter), -1);
}

/*
 *---------------------------------------------------------------------------
 *
 * ObjToPictureProc --
 *
 *	Given a string name, get the resample filter associated with it.
 *
 * Results:
 *	The return value is a standard Tcl result.  
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToPictureProc(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,		/* Interpreter to send results back to */
    Tk_Window tkwin,		/* Not used. */
    Tcl_Obj *objPtr,		/* String representation of value. */
    char *widgRec,		/* Widget record. */
    int offset,			/* Offset to field in structure */
    int flags)	
{
    BgStyleObject *corePtr = (BgStyleObject *)(widgRec);
    Blt_Picture picture;

    if (Blt_GetPictureFromObj(interp, objPtr, &picture) != TCL_OK) {
	return TCL_ERROR;
    }
    if (corePtr->origName != NULL) {
	Blt_Free(corePtr->origName);
    }
    if (corePtr->original != NULL) {
	Blt_FreePicture(corePtr->original);
    }
    corePtr->origName = Blt_StrdupAssert(Tcl_GetString(objPtr));
    corePtr->original = picture;
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * PictureToObjProc --
 *
 *	Convert the picture filter into a string Tcl_Obj.
 *
 * Results:
 *	The string representation of the filter is returned.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
PictureToObjProc(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,
    Tk_Window tkwin,		/* Not used. */
    char *widgRec,		/* Widget record */
    int offset,			/* Offset to field in structure */
    int flags)	
{
    BgStyleObject *corePtr = (BgStyleObject *)(widgRec);

    if (corePtr->origName == NULL) {
	return Tcl_NewStringObj("", -1);
    }
    return Tcl_NewStringObj(corePtr->origName, -1);
}

/*
 *---------------------------------------------------------------------------
 *
 * ObjToReference --
 *
 *	Given a string name, get the resample filter associated with it.
 *
 * Results:
 *	The return value is a standard Tcl result.  
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToReferenceProc(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,		/* Interpreter to send results back to */
    Tk_Window tkwin,		/* Not used. */
    Tcl_Obj *objPtr,		/* String representation of value. */
    char *widgRec,		/* Widget record. */
    int offset,			/* Offset to field in structure */
    int flags)	
{
    BgStyleObject *corePtr = (BgStyleObject *)(widgRec);
    int *referencePtr = (int *)(widgRec + offset);
    char *string;
    char c;
    int type;

    string = Tcl_GetString(objPtr);
    c = string[0];
    if ((c == 's') && (strcmp(string, "self") == 0)) {
	type = REFERENCE_SELF;
    } else if ((c == 't') && (strcmp(string, "toplevel") == 0)) {
	type = REFERENCE_TOPLEVEL;
    } else if ((c == 'n') && (strcmp(string, "none") == 0)) {
	type = REFERENCE_NONE;
    } else if (c == '.') {
	Tk_Window tkwin, tkMain;

	tkMain = Tk_MainWindow(interp);
	tkwin = Tk_NameToWindow(interp, string, tkMain);
	if (tkwin == NULL) {
	    return TCL_ERROR;
	}
	type = REFERENCE_WINDOW;
	corePtr->refWindow = tkwin;
    } else {
	Tcl_AppendResult(interp, "unknown reference type \"", string, "\"",
			 (char *)NULL);
	return TCL_ERROR;
    }
    *referencePtr = type;
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ReferenceToObj --
 *
 *	Convert the picture filter into a string Tcl_Obj.
 *
 * Results:
 *	The string representation of the filter is returned.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
ReferenceToObjProc(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,
    Tk_Window tkwin,		/* Not used. */
    char *widgRec,		/* Widget record */
    int offset,			/* Offset to field in structure */
    int flags)	
{
    int reference = *(int *)(widgRec + offset);
    const char *string;

    switch (reference) {
    case REFERENCE_SELF:
	string = "self";
	break;

    case REFERENCE_TOPLEVEL:
	string = "toplevel";
	break;

    case REFERENCE_NONE:
	string = "none";
	break;

    case REFERENCE_WINDOW:
	{
	    BgStyleObject *corePtr = (BgStyleObject *)(widgRec);

	    string = Tk_PathName(corePtr->refWindow);
	}
	break;

    default:
	string = "???";
	break;
    }
    return Tcl_NewStringObj(string, -1);
}

/*
 *---------------------------------------------------------------------------
 *
 * ObjToShapeProc --
 *
 *	Translate the given string to the gradient shape is
 *	represents.  Value shapes are "linear", "bilinear", "radial",
 *	and "rectangular".
 *
 * Results:
 *	The return value is a standard Tcl result.  
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToShapeProc(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,		/* Interpreter to send results back to */
    Tk_Window tkwin,		/* Not used. */
    Tcl_Obj *objPtr,		/* String representation of value. */
    char *widgRec,		/* Widget record. */
    int offset,			/* Offset to field in structure */
    int flags)	
{
    Blt_GradientShape *shapePtr = (Blt_GradientShape *)(widgRec + offset);
    char *string;

    string = Tcl_GetString(objPtr);
    if (strcmp(string, "linear") == 0) {
	*shapePtr = BLT_GRADIENT_SHAPE_LINEAR;
    } else if (strcmp(string, "bilinear") == 0) {
	*shapePtr = BLT_GRADIENT_SHAPE_BILINEAR;
    } else if (strcmp(string, "radial") == 0) {
	*shapePtr = BLT_GRADIENT_SHAPE_RADIAL;
    } else if (strcmp(string, "rectangular") == 0) {
	*shapePtr = BLT_GRADIENT_SHAPE_RECTANGULAR;
    } else {
	Tcl_AppendResult(interp, "unknown gradient type \"", string, "\"",
			 (char *) NULL);
	return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ShapeToObjProc --
 *
 *	Returns the string representing the current gradiant shape.
 *
 * Results:
 *	The string representation of the shape is returned.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
ShapeToObjProc(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,
    Tk_Window tkwin,		/* Not used. */
    char *widgRec,		/* Widget record */
    int offset,			/* Offset to field in structure */
    int flags)	
{
    Blt_GradientShape shape = *(Blt_GradientShape *)(widgRec + offset);
    const char *string;
    
    switch (shape) {
    case BLT_GRADIENT_SHAPE_LINEAR:
	string = "linear";
	break;

    case BLT_GRADIENT_SHAPE_BILINEAR:
	string = "bilinear";
	break;

    case BLT_GRADIENT_SHAPE_RADIAL:
	string = "radial";
	break;

    case BLT_GRADIENT_SHAPE_RECTANGULAR:
	string = "rectangular";
	break;

    default:
	string = "???";
    }
    return Tcl_NewStringObj(string, -1);
}

/*
 *---------------------------------------------------------------------------
 *
 * ObjToPathProc --
 *
 *	Translates the given string to the gradient path it
 *	represents.  Valid paths are "x", "y", "xy", and "yx".
 *
 * Results:
 *	The return value is a standard Tcl result.  
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToPathProc(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,		/* Interpreter to send results back to */
    Tk_Window tkwin,		/* Not used. */
    Tcl_Obj *objPtr,		/* String representation of value. */
    char *widgRec,		/* Widget record. */
    int offset,			/* Offset to field in structure */
    int flags)	
{
    Blt_GradientPath *pathPtr = (Blt_GradientPath *)(widgRec + offset);
    char *string;

    string = Tcl_GetString(objPtr);
    if (strcmp(string, "x") == 0) {
	*pathPtr = BLT_GRADIENT_PATH_X;
    } else if (strcmp(string, "y") == 0) {
	*pathPtr = BLT_GRADIENT_PATH_Y;
    } else if (strcmp(string, "xy") == 0) {
	*pathPtr = BLT_GRADIENT_PATH_XY;
    } else if (strcmp(string, "yx") == 0) {
	*pathPtr = BLT_GRADIENT_PATH_YX;
    } else {
	Tcl_AppendResult(interp, "unknown gradient path \"", string, "\"",
			 (char *) NULL);
	return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * PathToObjProc --
 *
 *	Convert the picture filter into a string Tcl_Obj.
 *
 * Results:
 *	The string representation of the filter is returned.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
PathToObjProc(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,
    Tk_Window tkwin,		/* Not used. */
    char *widgRec,		/* Widget record */
    int offset,			/* Offset to field in structure */
    int flags)	
{
    Blt_GradientPath path = *(Blt_GradientPath *)(widgRec + offset);
    const char *string;

    switch (path) {
    case BLT_GRADIENT_PATH_X:
	string = "x";
	break;

    case BLT_GRADIENT_PATH_Y:
	string = "y";
	break;

    case BLT_GRADIENT_PATH_XY:
	string = "xy";
	break;

    case BLT_GRADIENT_PATH_YX:
	string = "yx";
	break;

    default:
	string = "?? unknown path ??";
	break;
    }
    return Tcl_NewStringObj(string, -1);
}

/*
 *---------------------------------------------------------------------------
 *
 * NotifyClients --
 *
 *	Notify each client that's using the background style that it
 *	has changed.
 *
 * Results:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
static void
NotifyClients(BgStyleObject *corePtr)
{
    Blt_ChainLink link;

    for (link = Blt_ChainFirstLink(corePtr->chain); link != NULL;
	link = Blt_ChainNextLink(link)) {
	BgStyle *stylePtr;

	/* Notify each client that the background style has changed. The
	 * client should schedule itself for redrawing.  */
	stylePtr = Blt_ChainGetValue(link);
	if (stylePtr->notifyProc != NULL) {
	    (*stylePtr->notifyProc)(stylePtr->clientData);
	}
    }
}

static void
ClearCache(BgStyleObject *corePtr)
{
    Blt_HashEntry *hPtr;
    Blt_HashSearch cursor;

    for (hPtr = Blt_FirstHashEntry(&corePtr->pictTable, &cursor); hPtr != NULL;
	 hPtr = Blt_NextHashEntry(&cursor)) {
	Blt_Picture picture;

	picture = Blt_GetHashValue(hPtr);
	Blt_FreePicture(picture);
    }
    Blt_DeleteHashTable(&corePtr->pictTable);
    Blt_InitHashTable(&corePtr->pictTable, BLT_ONE_WORD_KEYS);
}

/*
 *---------------------------------------------------------------------------
 *
 * NewBgStyleObject --
 *
 *	Creates a new background style.
 *
 * Results:
 *	Returns pointer to the new background style.
 *
 *---------------------------------------------------------------------------
 */
static BgStyleObject *
NewBgStyleObject(
    BgStyleInterpData *dataPtr,
    Tcl_Interp *interp, 
    Blt_HashEntry *hPtr,
    int style)
{
    BgStyleObject *corePtr;
    char string[200];

    corePtr = Blt_Calloc(1, sizeof(BgStyleObject));
    if (corePtr == NULL) {
	Tcl_AppendResult(interp, "can't allocate background style", 
		(char *)NULL);
	return NULL;
    }
    if (hPtr == NULL) {
	int isNew;

	do {
	    sprintf_s(string, 200, "bg%d", dataPtr->nextId++);
	    hPtr = Blt_CreateHashEntry(&dataPtr->styleTable, string, &isNew);
	} while (!isNew);
    }
    corePtr->hashPtr = hPtr;
    corePtr->dataPtr = dataPtr;
    corePtr->style = style;
    corePtr->name = Blt_GetHashKey(&dataPtr->styleTable, hPtr);
    corePtr->chain = Blt_ChainCreate();
    corePtr->reference = REFERENCE_TOPLEVEL;
    corePtr->gradient.shape = BLT_GRADIENT_SHAPE_LINEAR;
    corePtr->gradient.path = BLT_GRADIENT_PATH_Y;
    corePtr->gradient.jitter = FALSE;
    corePtr->gradient.logScale = TRUE;
    Blt_InitHashTable(&corePtr->pictTable, BLT_ONE_WORD_KEYS);
    Blt_SetHashValue(hPtr, corePtr);
    return corePtr;
}

/*
 *---------------------------------------------------------------------------
 *
 * DestroyBgStyleObject --
 *
 *	Removes the client from the servers's list of clients and memory used
 *	by the client token is released.  When the last client is deleted, the
 *	server is also removed.
 *
 * Results:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
static void
DestroyBgStyleObject(BgStyleObject *corePtr)
{
    Blt_FreeOptions(corePtr->configSpecs, (char *)corePtr, corePtr->display, 0);

    ClearCache(corePtr);
    Blt_DeleteHashTable(&corePtr->pictTable);

    if (corePtr->original != NULL) {
	Blt_FreePicture(corePtr->original);
    }
    if (corePtr->border != NULL) {
	Tk_Free3DBorder(corePtr->border);
    }
    if (corePtr->hashPtr != NULL) {
	Blt_DeleteHashEntry(&corePtr->dataPtr->styleTable, corePtr->hashPtr);
    }
    Blt_ChainDestroy(corePtr->chain);
    Blt_Free(corePtr);
}



static Blt_Picture
GetBackgroundPicture(
    BgStyleObject *corePtr, 
    Tk_Window tkwin, 
    int x, int y,
    int *xPtr, int *yPtr)
{
    Blt_Picture picture;
    Tk_Window refWindow;
    Blt_HashEntry *hPtr;
    int refWidth, refHeight;
    int sx, sy;

    if (corePtr->reference == REFERENCE_SELF) {
	refWindow = tkwin;
    } else if (corePtr->reference == REFERENCE_TOPLEVEL) {
	refWindow = Blt_Toplevel(tkwin);
    } else if (corePtr->reference == REFERENCE_WINDOW) {
	refWindow = corePtr->refWindow;
    } else if (corePtr->reference == REFERENCE_NONE) {
	refWindow = NULL;
    } else {
	return NULL;		/* Unknown reference window. */
    }
    sx = x, sy = y;
    if ((corePtr->reference == REFERENCE_WINDOW) ||
	(corePtr->reference == REFERENCE_TOPLEVEL)) {
	Tk_Window tkwin2;

	sx += corePtr->xOrigin, sy += corePtr->yOrigin;
	tkwin2 = tkwin;
	while ((tkwin2 != refWindow) && (tkwin2 != NULL)) {
	    sx += Tk_X(tkwin2) + Tk_Changes(tkwin2)->border_width;
	    sy += Tk_Y(tkwin2) + Tk_Changes(tkwin2)->border_width;
	    tkwin2 = Tk_Parent(tkwin2);
	}
	if (tkwin2 == NULL) {
	    /* 
	     * The window associated with the background style isn't an
	     * ancestor of the current window. That means we can't use the
	     * reference window as a guide to the size of the picture.  Simply
	     * convert to a self reference.
	     */
	    corePtr->reference = REFERENCE_SELF;
	    refWindow = tkwin;
	    sx = x, sy = y;	
	}
    }
    *xPtr = sx, *yPtr = sy;

    picture = NULL;

    if (corePtr->reference == REFERENCE_NONE) {
	refWidth = corePtr->refRegion.width;
	refHeight = corePtr->refRegion.height;
	hPtr = NULL;
    } else {
	int isNew;

	/* See if a picture has previously been generated. There will be a
	 * picture for each reference window. */
	hPtr = Blt_CreateHashEntry(&corePtr->pictTable, (char *)refWindow, 
				   &isNew);
	if (!isNew) {
	    picture = Blt_GetHashValue(hPtr);
	} 
	refWidth = Tk_Width(refWindow);
	refHeight = Tk_Height(refWindow);
    }
    if ((picture == NULL) || (Blt_PictureWidth(picture) != refWidth) ||
	(Blt_PictureHeight(picture) != refHeight)) {

	/* 
	 * Either the size of the reference window has changed or one of the
	 * background style options has been reset. Resize the picture if
	 * necessary and regenerate the background.
	 */

	if (picture == NULL) {
	    picture = Blt_CreatePicture(refWidth, refHeight);
	    Blt_SetHashValue(hPtr, picture);
	} else {
	    Blt_ResizePicture(picture, refWidth, refHeight);
	}
	if (corePtr->style == STYLE_PICTURE) {
	    Blt_ResamplePicture(picture, corePtr->original, corePtr->filter, 
		corePtr->filter);
	} else if (corePtr->style == STYLE_GRADIENT) {
	    Blt_GradientPicture(picture, &corePtr->high, &corePtr->low, 
		&corePtr->gradient);
	} else if (corePtr->style == STYLE_TILE) {
	    Blt_TilePicture(picture, corePtr->original, 0, 0, 0, 0, refWidth, 
		refHeight);
	} else if (corePtr->style == STYLE_TEXTURE) {
	    Blt_TexturePicture(picture, &corePtr->high, &corePtr->low, 0);
	}
    }
    return picture;
}


/*
 *---------------------------------------------------------------------------
 *
 * GetBackgroundFromObj --
 *
 *	Retrieves the background style named by the given the Tcl_Obj.
 *
 * Results:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
static int
GetBackgroundFromObj(
    Tcl_Interp *interp,
    BgStyleInterpData *dataPtr,
    Tcl_Obj *objPtr,
    BgStyleObject **corePtrPtr)
{
    Blt_HashEntry *hPtr;
    char *string;

    string = Tcl_GetString(objPtr);
    hPtr = Blt_FindHashEntry(&dataPtr->styleTable, string);
    if (hPtr == NULL) {
	Tcl_AppendResult(dataPtr->interp, "can't find background style \"", 
		string, "\"", (char *)NULL);
	return TCL_ERROR;
    }
    *corePtrPtr = Blt_GetHashValue(hPtr);
    return TCL_OK;
}

/*
 * bgstyle create type ?option values?...
 */
static int
CreateOp(    
    ClientData clientData,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    BgStyleObject *corePtr;
    BgStyleInterpData *dataPtr = clientData;
    Blt_ConfigSpec *configSpecs;
    char *string;
    char c;
    int style;

    string = Tcl_GetString(objv[2]);
    c = string[0];
    if ((c == 't') && (strcmp(string, "tile") == 0)) {
	style = STYLE_TILE;
	configSpecs = tileConfigSpecs;
    } else if ((c == 'p') && (strcmp(string, "picture") == 0)) {
	style = STYLE_PICTURE;
	configSpecs = pictureConfigSpecs;
    } else if ((c == 'g') && (strcmp(string, "gradient") == 0)) {
	style = STYLE_GRADIENT;
	configSpecs = gradientConfigSpecs;
    } else if ((c == 'b') && (strcmp(string, "border") == 0)) {
	style = STYLE_BORDER;
	configSpecs = borderConfigSpecs;
    } else if ((c == 't') && (strcmp(string, "texture") == 0)) {
	style = STYLE_TEXTURE;
	configSpecs = textureConfigSpecs;
    } else {
	Tcl_AppendResult(interp, "unknown background style \"", string, "\"",
		(char *)NULL);
	return TCL_ERROR;
    }

    corePtr = NewBgStyleObject(dataPtr, interp, NULL, style);
    corePtr->configSpecs = configSpecs;
    corePtr->tkwin = Tk_MainWindow(interp);
    corePtr->display = Tk_Display(corePtr->tkwin);

    if (Blt_ConfigureWidgetFromObj(interp, corePtr->tkwin, configSpecs, 
		objc - 3, objv + 3, (char *)corePtr, 0) != TCL_OK) {
	DestroyBgStyleObject(corePtr);
	return TCL_ERROR;
    }
    Tcl_SetStringObj(Tcl_GetObjResult(interp), corePtr->name, -1);
    return TCL_OK;
}    

/*
 * bgstyle cget $style ?option?...
 */
static int
CgetOp(    
    ClientData clientData,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    BgStyleInterpData *dataPtr = clientData;
    BgStyleObject *corePtr;

    if (GetBackgroundFromObj(interp, dataPtr, objv[2], &corePtr) != TCL_OK) {
	return TCL_ERROR;
    }
    return Blt_ConfigureValueFromObj(interp, corePtr->tkwin, 
	corePtr->configSpecs, (char *)corePtr, objv[3], 0);
}

/*
 * bgstyle configure $style ?option?...
 */
static int
ConfigureOp(    
    ClientData clientData,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    BgStyleInterpData *dataPtr = clientData;
    BgStyleObject *corePtr;
    int flags;

    if (GetBackgroundFromObj(interp, dataPtr, objv[2], &corePtr) != TCL_OK) {
	return TCL_ERROR;
    }
    flags = BLT_CONFIG_OBJV_ONLY;
    if (objc == 3) {
	return Blt_ConfigureInfoFromObj(interp, corePtr->tkwin, 
		corePtr->configSpecs, (char *)corePtr, (Tcl_Obj *)NULL, flags);
    } else if (objc == 4) {
	return Blt_ConfigureInfoFromObj(interp, corePtr->tkwin, 
		corePtr->configSpecs, (char *)corePtr, objv[3], flags);
    } else {
	if (Blt_ConfigureWidgetFromObj(interp, corePtr->tkwin, 
		corePtr->configSpecs, objc - 3, objv + 3, (char *)corePtr, 
		flags) != TCL_OK) {
	    return TCL_ERROR;
	}
	ClearCache(corePtr);
	NotifyClients(corePtr);
	return TCL_OK;
    }
}

/*
 * bgstyle delete $style... 
 */
static int
DeleteOp(    
    ClientData clientData,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    int i;

    for (i = 2; i < objc; i++) {
	BgStyleInterpData *dataPtr = clientData;
	BgStyleObject *corePtr;

	if (GetBackgroundFromObj(interp, dataPtr, objv[2], &corePtr) != TCL_OK){
	    return TCL_ERROR;
	}
	if (Blt_ChainGetLength(corePtr->chain) <= 0) {
	    /* If no one is using the background style, remove it. */
	    DestroyBgStyleObject(corePtr);
	} else {
	    /* Otherwise just take the entry out of the style hash
	     * table. When the last client is done with the style, it
	     * will be removed. */
	    Blt_DeleteHashEntry(&dataPtr->styleTable, corePtr->hashPtr);
	    corePtr->hashPtr = NULL;
	}
    }
    return TCL_OK;
}

/*
 * bgstyle type $style
 */
static int
TypeOp(    
    ClientData clientData,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    BgStyleInterpData *dataPtr = clientData;
    BgStyleObject *corePtr;

    if (GetBackgroundFromObj(interp, dataPtr, objv[2], &corePtr) != TCL_OK) {
	return TCL_ERROR;
    }
    Tcl_SetStringObj(Tcl_GetObjResult(interp), styleTypes[corePtr->style], -1);
    return TCL_OK;
}

static Blt_OpSpec styleOps[] =
{
    {"cget",      2, CgetOp,      4, 4, "style option",},
    {"configure", 2, ConfigureOp, 3, 0, "style ?option value?...",},
    {"create",    2, CreateOp,    3, 0, "type ?args?",},
    {"delete",    1, DeleteOp,    2, 0, "style...",},
    {"type",      1, TypeOp,      3, 3, "style",},
};
static int nStyleOps = sizeof(styleOps) / sizeof(Blt_OpSpec);

static int
BgStyleCmdProc(
    ClientData clientData,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    Tcl_ObjCmdProc *proc;

    proc = Blt_GetOpFromObj(interp, nStyleOps, styleOps, BLT_OP_ARG1, 
	objc, objv, 0);
    if (proc == NULL) {
	return TCL_ERROR;
    }
    return (*proc) (clientData, interp, objc, objv);
}

/*
 *---------------------------------------------------------------------------
 *
 * GetBgStyleInterpData --
 *
 *---------------------------------------------------------------------------
 */
static BgStyleInterpData *
GetBgStyleInterpData(Tcl_Interp *interp)
{
    BgStyleInterpData *dataPtr;
    Tcl_InterpDeleteProc *proc;

    dataPtr = (BgStyleInterpData *)
	Tcl_GetAssocData(interp, BG_STYLE_THREAD_KEY, &proc);
    if (dataPtr == NULL) {
	dataPtr = Blt_MallocAssert(sizeof(BgStyleInterpData));
	dataPtr->interp = interp;
	dataPtr->nextId = 1;

	/* FIXME: Create interp delete proc to teardown the hash table and
	 * data entry.  Must occur after all the widgets have been destroyed
	 * (clients of the background style). */

	Tcl_SetAssocData(interp, BG_STYLE_THREAD_KEY, 
		(Tcl_InterpDeleteProc *)NULL, dataPtr);
	Blt_InitHashTable(&dataPtr->styleTable, BLT_STRING_KEYS);
    }
    return dataPtr;
}

static void 
ComputeTileStart(Blt_Picture picture, int x, int y, int xOrigin, int yOrigin,
	int *startXPtr, int *startYPtr)
{
    int startX, startY;		/* Starting upper left corner of region. */
    int delta;

    /* Compute the starting x and y offsets of the tile from the coordinates
     * of the origin. */
    startX = x;
    if (x < xOrigin) {
	delta = (xOrigin - x) % Blt_PictureWidth(picture);
	if (delta > 0) {
	    startX -= (Blt_PictureWidth(picture) - delta);
	}
    } else if (x > xOrigin) {
	delta = (x - xOrigin) % Blt_PictureWidth(picture);
	if (delta > 0) {
	    startX -= delta;
	}
    }
    startY = y;
    if (y < yOrigin) {
	delta = (yOrigin - y) % Blt_PictureHeight(picture);
	if (delta > 0) {
	    startY -= (Blt_PictureHeight(picture) - delta);
	}
    } else if (y >= yOrigin) {
	delta = (y - yOrigin) % Blt_PictureHeight(picture);
	if (delta > 0) {
	    startY -= delta;
	}
    }
    *startXPtr = startX;
    *startYPtr = startY;
}

static void
Tile(
    Blt_Painter painter,
    Drawable drawable,
    Blt_Picture picture,	/* Picture used as the tile. */
    int xOrigin, int yOrigin,	/* Tile origin. */
    int x, int y, int w, int h)	/* Region of destination picture to be
				 * tiled. */
{
    int startX, startY;		/* Starting upper left corner of region. */
    struct region {
	int left, top, right, bottom;
    } r;
    ComputeTileStart(picture, x, y, xOrigin, yOrigin, &startX, &startY);

#ifdef notdef
    fprintf(stderr, "tile is (xo=%d,yo=%d,w=%d,h=%d)\n", 
	    xOrigin, yOrigin, Blt_PictureWidth(picture), 
	    Blt_PictureHeight(picture));
    fprintf(stderr, "region is (x=%d,y=%d,w=%d,h=%d)\n", x, y, w, h);
    fprintf(stderr, "starting at sx=%d,sy=%d\n", startX, startY);
#endif
    /* The region to be tiled. */
    r.left = x;
    r.right = x + w;
    r.top = y;
    r.bottom = y + h;
    
    for (y = startY; y < r.bottom; y += Blt_PictureHeight(picture)) {
	int sy, dy;
	int th;		/* Current tile height. */
	
	sy = 0;
	dy = y;
	th = Blt_PictureHeight(picture);
	if (y < r.top) {
	    sy = (r.top - y);
	    th = Blt_PictureHeight(picture) - sy;
	    dy = r.top;
	} 
	if ((dy + th) > r.bottom) {
	    th = (r.bottom - dy);
	}
	for (x = startX; x < r.right; x += Blt_PictureWidth(picture)) {
	    int sx, dx;
	    int tw;		/* Current tile width. */
	    
	    sx = 0;
	    dx = x;
	    tw = Blt_PictureWidth(picture);
	    if (x < r.left) {
		sx = (r.left - x);
		tw = Blt_PictureWidth(picture) - sx;
		dx = r.left;
	    } 
	    if ((dx + tw) > r.right) {
		tw = (r.right - dx);
	    }
#ifdef notdef
	    fprintf(stderr, "drawing pattern (sx=%d,sy=%d,tw=%d,ty=%d) at dx=%d,dy=%d\n",
		    sx, sy, tw, th, dx, dy);
#endif
	    Blt_PaintPicture(painter, drawable, picture, sx, sy, tw, 
				      th, dx, dy, 0);
	}
    }
}

/*LINTLIBRARY*/
int
Blt_BgStyleCmdInitProc(Tcl_Interp *interp)
{
    static Blt_InitCmdSpec cmdSpec = {
	"bgstyle", BgStyleCmdProc 
    };
    cmdSpec.clientData = GetBgStyleInterpData(interp);
    return Blt_InitCmd(interp, "::blt", &cmdSpec);
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_GetBackground
 *
 *	Retrieves a new token of a background style from the named
 *	background style.
 *
 * Results:
 *	Returns a background style token.
 *
 * Side Effects:
 *	Memory is allocated for the new token.
 *
 *---------------------------------------------------------------------------
 */
Blt_Background
Blt_GetBackground(Tcl_Interp *interp, Tk_Window tkwin, const char *styleName)
{
    BgStyleObject *corePtr;
    BgStyleInterpData *dataPtr;
    BgStyle *stylePtr;
    Blt_HashEntry *hPtr;
    int isNew;
    
    dataPtr = GetBgStyleInterpData(interp);
    hPtr = Blt_CreateHashEntry(&dataPtr->styleTable, styleName, &isNew);
    if (isNew) {
	Tk_3DBorder border;

	/* Style doesn't exist, see if it's a color name (i.e. 3D border). */
	border = Tk_Get3DBorder(interp, tkwin, styleName);
	if (border == NULL) {
	    return NULL;
	} 
	corePtr = NewBgStyleObject(dataPtr, interp, hPtr, STYLE_BORDER);
	if (corePtr == NULL) {
	    return NULL;
	}
	corePtr->border = border;
	corePtr->hashPtr = hPtr;
	Blt_SetHashValue(hPtr, corePtr);
    } else {
	corePtr = Blt_GetHashValue(hPtr);
    }
    stylePtr = Blt_Calloc(1, sizeof(BgStyle));
    if (stylePtr == NULL) {
	DestroyBgStyleObject(corePtr);
	return NULL;
    }
    /* Create new token for the background style. */
    stylePtr->link = Blt_ChainAppend(corePtr->chain, stylePtr);
    stylePtr->corePtr = corePtr;
    return stylePtr;
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_GetBackgroundFromObj
 *
 *	Retrieves a new token of a background style from the named background
 *	style.
 *
 * Results:
 *	Returns a background style token.
 *
 * Side Effects:
 *	Memory is allocated for the new token.
 *
 *---------------------------------------------------------------------------
 */
Blt_Background
Blt_GetBackgroundFromObj(Tcl_Interp *interp, Tk_Window tkwin, Tcl_Obj *objPtr)
{
    return Blt_GetBackground(interp, tkwin, Tcl_GetString(objPtr));
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_SetBackgroundChangedProc
 *
 *	Sets the routine to called when an image changes.  
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The designated routine will be called the next time the image
 *	associated with the tile changes.
 *
 *---------------------------------------------------------------------------
 */
/*LINTLIBRARY*/
void
Blt_SetBackgroundChangedProc(
    BgStyle *stylePtr,		/* Background to register callback with. */
    Blt_BackgroundChangedProc *notifyProc, /* Function to call when style has
					    * changed. NULL indicates to unset
					    * the callback.*/
    ClientData clientData)
{
    stylePtr->notifyProc = notifyProc;
    stylePtr->clientData = clientData;
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_FreeBackground
 *
 *	Removes the background style token.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Memory is freed.
 *
 *---------------------------------------------------------------------------
 */
void
Blt_FreeBackground(BgStyle *stylePtr)
{
    BgStyleObject *corePtr = stylePtr->corePtr;

    if (corePtr != NULL) {
	Blt_ChainDeleteLink(corePtr->chain, stylePtr->link);
	if (Blt_ChainGetLength(corePtr->chain) <= 0) {
	    if (corePtr->hashPtr == NULL) { 
		/* Indicates that the background style is ready to be removed
		 * and that no clients are still using it. */
		DestroyBgStyleObject(corePtr);
	    }
	}
    }
    Blt_Free(stylePtr);
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_GetBackgroundOrigin
 *
 *	Returns the coordinates of the origin of the master background style
 *	referenced by the token.
 *
 * Results:
 *	Returns the coordinates of the origin.
 *
 *---------------------------------------------------------------------------
 */
void
Blt_GetBackgroundOrigin(BgStyle *stylePtr, int *xPtr, int *yPtr)
{
    *xPtr = stylePtr->corePtr->xOrigin;
    *yPtr = stylePtr->corePtr->yOrigin;
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_SetBackgroundOrigin
 *
 *	Returns the name of the master background style referenced by the
 *	token.
 *
 * Results:
 *	Return the name of the background style.
 *
 *---------------------------------------------------------------------------
 */
void
Blt_SetBackgroundOrigin(BgStyle *stylePtr, int x, int y)
{
    stylePtr->corePtr->xOrigin = x;
    stylePtr->corePtr->yOrigin = y;
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_NameOfBackground
 *
 *	Returns the name of the master background style referenced
 *	by the token.
 *
 * Results:
 *	Return the name of the background style.
 *
 *---------------------------------------------------------------------------
 */
const char *
Blt_NameOfBackground(BgStyle *stylePtr)
{
    return stylePtr->corePtr->name;
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_BackgroundBorderColor
 *
 *	Returns the name of the master background style referenced
 *	by the token.
 *
 * Results:
 *	Return the name of the background style.
 *
 *---------------------------------------------------------------------------
 */
XColor *
Blt_BackgroundBorderColor(BgStyle *stylePtr)
{
    return Tk_3DBorderColor(stylePtr->corePtr->border);
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_BackgroundBorder
 *
 *	Returns the name of the master background style referenced
 *	by the token.
 *
 * Results:
 *	Return the name of the background style.
 *
 *---------------------------------------------------------------------------
 */
Tk_3DBorder
Blt_BackgroundBorder(BgStyle *stylePtr)
{
    return stylePtr->corePtr->border;
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_DrawBackgroundRectangle
 *
 *	Draws the background style in the designated window.
 *
 * Results:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
void
Blt_DrawBackgroundRectangle(
    Tk_Window tkwin, 
    Drawable drawable, 
    BgStyle *stylePtr, 
    int x, int y, int w, int h, 
    int borderWidth, int relief)
{
    Tk_Draw3DRectangle(tkwin, drawable, stylePtr->corePtr->border, x, y, w, h, 
	borderWidth, relief);
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_FillBackgroundRectangle
 *
 *	Draws the background style in the designated window.
 *
 * Results:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
void
Blt_FillBackgroundRectangle(
    Tk_Window tkwin, 
    Drawable drawable, 
    BgStyle *stylePtr, 
    int x, int y, int w, int h, 
    int borderWidth, int relief)
{
    BgStyleObject *corePtr;

    corePtr = stylePtr->corePtr;
    if (corePtr->style == STYLE_BORDER) {
	Tk_Fill3DRectangle(tkwin, drawable, corePtr->border, x, y, w, h,
		borderWidth, relief);
    } else {
	Blt_Picture picture;
	int sx, sy;

	picture = GetBackgroundPicture(corePtr, tkwin, x, y, &sx, &sy);
	if (picture != NULL) {
	    Blt_Painter painter;
	    
	    painter = Blt_GetPainter(tkwin, 1.0);
	    Tile(painter, drawable, picture, sx, sy, x, y, w, h);
	    
	    /*
	     * We don't free the painter to avoid the expense of generating a
	     * color ramp each time we need to draw a background (this doesn't
	     * pertain to TrueColor visuals). Painters are reference counted,
	     * so there will be only one painter per visual used.  In most
	     * cases, that's one or two.
	     */
	    if ((relief != TK_RELIEF_FLAT) && (borderWidth > 0)) {
		Tk_Draw3DRectangle(tkwin, drawable, corePtr->border, x, y, w, h,
				   borderWidth, relief);
	    }
	}
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_FillPictureBackground
 *
 *	Draws the background style in the designated picture.
 *
 * Results:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
void
Blt_FillPictureBackground(
    Tk_Window tkwin, 
    Blt_Picture dest,
    BgStyle *stylePtr, 
    int x, int y, int w, int h, 
    int borderWidth, int relief)
{
    BgStyleObject *corePtr;
    Blt_Picture picture;
    int sx, sy;

    corePtr = stylePtr->corePtr;
    if (corePtr->style == STYLE_BORDER) {
	return;
    } 
    picture = GetBackgroundPicture(corePtr, tkwin, x, y, &sx, &sy);
    if (picture == NULL) {
	return;
    }
    Blt_CopyPictureBits(dest, picture, sx, sy, w, h, x, y);
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_DrawFocusBackground
 *
 *	Draws the background style in the designated picture.
 *
 * Results:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
void
Blt_DrawFocusBackground(
    Tk_Window tkwin, 
    BgStyle *stylePtr, 
    int highlightWidth, 
    Drawable drawable)
{
    int w, h;
    BgStyleObject *corePtr;
    Blt_Painter painter;
    Blt_Picture picture;
    int sx, sy;

    corePtr = stylePtr->corePtr;
    if (corePtr->style == STYLE_BORDER) {
	GC gc;

	gc = Tk_3DBorderGC(tkwin, corePtr->border, TK_3D_FLAT_GC);
	Tk_DrawFocusHighlight(tkwin, gc, highlightWidth, drawable);
	return;
    } 
    w = Tk_Width(tkwin);
    h = Tk_Height(tkwin);
    picture = GetBackgroundPicture(corePtr, tkwin, 0, 0, &sx, &sy);
    if (picture == NULL) {
	return;
    }
    painter = Blt_GetPainter(tkwin, 1.0);
    Blt_PaintPicture(painter, drawable, picture, sx, sy, w, highlightWidth, 
	0, 0, 0);
    Blt_PaintPicture(painter, drawable, picture, sx, sy, highlightWidth, h,
	0, 0, 0);
    Blt_PaintPicture(painter, drawable, picture, sx, sy + h - highlightWidth, 
	w, highlightWidth, 0, h - highlightWidth, 0);
    Blt_PaintPicture(painter, drawable, picture, sx + w - highlightWidth, sy, 
		     highlightWidth, h, w - highlightWidth, 0, 0);
    /*
     * We don't free the painter to avoid the expense of generating a color
     * ramp each time we need to draw a background (this doesn't pertain to
     * TrueColor visuals). Painters are reference counted, so there will be
     * only one painter per visual used.  In most cases, that's one or two.
     */
}


#ifdef notdef
static void 
Draw3DRectangle(
    Tk_Window tkwin, 
    Drawable drawable, 
    BgStyle *stylePtr, 
    int x, int y, int w, int h,
    int borderWidth, int relief)
{
    int nSegments;
    XSegment *segments, *sp;
    int i;

    nSegments = borderWidth + borderWidth;
    segments = Blt_MallocAssert(sizeof(XSegment) * nSegments);
    sp = segments;
    for (i = 0; i < borderWidth; i++) {
	sp->x1 = x + i;
	sp->y1 = y + i;
	sp->x2 = x + (w - 1) - i;
	sp->y2 = y + i;
	sp++;
	sp->x1 = x + i;
	sp->y1 = y + i;
	sp->x2 = x + i;
	sp->y2 = y + (h - 1) - i;
	sp++;
    }
    gc = Tk_3DBorderGC(tkwin, stylePtr->corePtr->border, TK_3D_LIGHT_GC);
    XDrawSegments(Tk_Display(tkwin), drawable, gc, segments, nSegments);

    sp = segments;
    for (i = 0; i < borderWidth; i++) {
	sp->x1 = x + i;
	sp->y1 = y + (h - 1) - i;
	sp->x2 = x + (w - 1) - i;
	sp->y2 = y + (h - 1) - i;
	sp++;
	sp->x1 = x + (w - 1 ) - i;
	sp->y1 = y + i;
	sp->x2 = x + (w - 1) - i;
	sp->y2 = y + (h - 1) - i;
	sp++;
    }
    gc = Tk_3DBorderGC(tkwin, stylePtr->corePtr->border, TK_3D_DARK_GC);
    XDrawSegments(Tk_Display(tkwin), drawable, gc, segments, nSegments);
}
#endif

void
Blt_SetBackgroundRegion(BgStyle *stylePtr, int x, int y, int w, int h)
{
    if (stylePtr->corePtr->reference == REFERENCE_NONE) {
	stylePtr->corePtr->refRegion.x = x;
	stylePtr->corePtr->refRegion.y = y;
	stylePtr->corePtr->refRegion.width = w;
	stylePtr->corePtr->refRegion.height = h;
    }
}
