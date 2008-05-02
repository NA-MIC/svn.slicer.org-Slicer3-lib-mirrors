
/*
 * bltPictPs.c --
 *
 * This module implements Encapsulated PostScript file format conversion
 * routines for the picture image type in the BLT toolkit.
 *
 *	Copyright 2003-2007 George A Howlett.
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
 *
 */

#include "blt.h"
#include "config.h"
#include <tcl.h>
#include <bltSwitch.h>
#include <bltDBuffer.h>
#include "bltTypes.h"
#include "bltPicture.h"
#include "bltPictFmts.h"
#include "bltPs.h"
#include "bltAlloc.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef HAVE_STRING_H
#  include <string.h>
#endif /* HAVE_STRING_H */

#define UCHAR(c)	((unsigned char) (c))
#define ISASCII(c)	(UCHAR(c)<=0177)
#define MIN(a,b)	(((a)<(b))?(a):(b))

typedef struct {
    Tcl_Obj *dataObjPtr;
    Tcl_Obj *fileObjPtr;
    int flags;			/* Flag. */
    Blt_Pixel bg;
    PageSetup setup;
} PsExportSwitches;

typedef struct {
    Tcl_Obj *dataObjPtr;
    Tcl_Obj *fileObjPtr;
} PsImportSwitches;

BLT_EXTERN Blt_SwitchParseProc Blt_ColorSwitchProc;
static Blt_SwitchCustom colorSwitch = {
    Blt_ColorSwitchProc, NULL, (ClientData)0,
};

static Blt_SwitchParseProc PicaSwitchProc;
static Blt_SwitchCustom picaSwitch = {
    PicaSwitchProc, NULL, (ClientData)0,
};

static Blt_SwitchParseProc PadSwitchProc;
static Blt_SwitchCustom padSwitch = {
    PadSwitchProc, NULL, (ClientData)0,
};

static Blt_SwitchSpec psExportSwitches[] = 
{
    {BLT_SWITCH_CUSTOM,  "-bg",		"color",
	Blt_Offset(PsExportSwitches, bg),	   0, 0, &colorSwitch},
    {BLT_SWITCH_OBJ,     "-data",	"data",
	Blt_Offset(PsExportSwitches, dataObjPtr),  0},
    {BLT_SWITCH_OBJ,     "-file",	"fileName",
	Blt_Offset(PsExportSwitches, fileObjPtr),  0},
    {BLT_SWITCH_BITMASK, "-center",	"",
	Blt_Offset(PsExportSwitches, setup.flags), 0, PS_CENTER},
    {BLT_SWITCH_BITMASK, "-greyscale",	"",
	Blt_Offset(PsExportSwitches, setup.flags), 0, PS_GREYSCALE},
    {BLT_SWITCH_BITMASK, "-landscape",	"",
	Blt_Offset(PsExportSwitches, setup.flags), 0, PS_LANDSCAPE},
    {BLT_SWITCH_INT_POS, "-level",	"pslevel",
	Blt_Offset(PsExportSwitches, setup.level), 0},
    {BLT_SWITCH_BITMASK, "-maxpect",	"",
	Blt_Offset(PsExportSwitches, setup.flags), 0, PS_MAXPECT},
    {BLT_SWITCH_CUSTOM,  "-padx",	"pad",
	Blt_Offset(PsExportSwitches, setup.padX),  0, 0, &padSwitch},
    {BLT_SWITCH_CUSTOM,  "-pady",	"pad",
	Blt_Offset(PsExportSwitches, setup.padY),  0, 0, &padSwitch},
    {BLT_SWITCH_CUSTOM,  "-paperheight","pica",
	Blt_Offset(PsExportSwitches, setup.reqPaperHeight), 0, 0, &picaSwitch},
    {BLT_SWITCH_CUSTOM,  "-paperwidth", "pica",
	Blt_Offset(PsExportSwitches, setup.reqPaperWidth), 0, 0, &picaSwitch},
    {BLT_SWITCH_END}
};

static Blt_SwitchSpec psImportSwitches[] =
{
};

DLLEXPORT extern Tcl_AppInitProc Blt_PicturePsInit;

#define TRUE 	1
#define FALSE 	0

typedef struct Blt_PictureStruct Picture;

#include <ctype.h>


/*
 * Parse the lines that define the dimensions of the bitmap, plus the first
 * line that defines the bitmap data (it declares the name of a data variable
 * but doesn't include any actual data).  These lines look something like the
 * following:
 *
 *		#define foo_width 16
 *		#define foo_height 16
 *		#define foo_x_hot 3
 *		#define foo_y_hot 3
 *		static char foo_bits[] = {
 *
 * The x_hot and y_hot lines may or may not be present.  It's important to
 * check for "char" in the last line, in order to reject old X10-style bitmaps
 * that used shorts.
 */

#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif /* HAVE_SYS_TIME_H */
#endif /* TIME_WITH_SYS_TIME */

/*
 *--------------------------------------------------------------------------
 *
 * PicaSwitchProc --
 *
 *	Convert a Tcl_Obj list of 2 or 4 numbers into representing a bounding
 *	box structure.
 *
 * Results:
 *	The return value is a standard Tcl result.
 *
 *--------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
PicaSwitchProc(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,		/* Interpreter to send results back to */
    const char *switchName,	/* Not used. */
    Tcl_Obj *objPtr,		/* String representation */
    char *record,		/* Structure record */
    int offset,			/* Offset to field in structure */
    int flags)	
{
    int *picaPtr = (int *)(record + offset);
    
    return Blt_Ps_GetPicaFromObj(interp, objPtr, picaPtr);
}

/*
 *--------------------------------------------------------------------------
 *
 * PadSwitchProc --
 *
 *	Convert a Tcl_Obj list of 2 or 4 numbers into representing a bounding
 *	box structure.
 *
 * Results:
 *	The return value is a standard Tcl result.
 *
 *--------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
PadSwitchProc(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,		/* Interpreter to send results back to */
    const char *switchName,	/* Not used. */
    Tcl_Obj *objPtr,		/* String representation */
    char *record,		/* Structure record */
    int offset,			/* Offset to field in structure */
    int flags)	
{
    Blt_Pad *padPtr = (Blt_Pad *)(record + offset);
    
    return Blt_Ps_GetPadFromObj(interp, objPtr, padPtr);
}

/*
 * --------------------------------------------------------------------------
 *
 * PostScriptPreamble --
 *
 *    	The PostScript preamble calculates the needed translation and scaling
 *    	to make image coordinates compatible with PostScript.
 *
 * --------------------------------------------------------------------------
 */
static int
PostScriptPreamble(
    Tcl_Interp *interp,
    Picture *srcPtr,
    PsExportSwitches *switchesPtr,
    Blt_Ps ps)
{
    PageSetup *setupPtr = &switchesPtr->setup;
    time_t ticks;
    char date[200];		/* Hold the date string from ctime() */
    const char *version;
    char *newline;

    Blt_Ps_Append(ps, "%!PS-Adobe-3.0 EPSF-3.0\n");

    /* The "BoundingBox" comment is required for EPS files. */
    Blt_Ps_Format(ps, "%%%%BoundingBox: %d %d %d %d\n",
	setupPtr->left, setupPtr->paperHeight - setupPtr->top,
	setupPtr->right, setupPtr->paperHeight - setupPtr->bottom);
    Blt_Ps_Append(ps, "%%Pages: 0\n");

    version = Tcl_GetVar(interp, "blt_version", TCL_GLOBAL_ONLY);
    if (version == NULL) {
	version = "???";
    }
    Blt_Ps_Format(ps, "%%%%Creator: (BLT %s Picture)\n", version);

    ticks = time((time_t *) NULL);
    strcpy(date, ctime(&ticks));
    newline = date + strlen(date) - 1;
    if (*newline == '\n') {
	*newline = '\0';
    }
    Blt_Ps_Format(ps, "%%%%CreationDate: (%s)\n", date);
    Blt_Ps_Append(ps, "%%DocumentData: Clean7Bit\n");
    if (setupPtr->flags & PS_LANDSCAPE) {
	Blt_Ps_Append(ps, "%%Orientation: Landscape\n");
    } else {
	Blt_Ps_Append(ps, "%%Orientation: Portrait\n");
    }
    Blt_Ps_Append(ps, "%%EndComments\n\n");
    Blt_Ps_Append(ps, "%%BeginProlog\n");
    Blt_Ps_Append(ps, "%%EndProlog\n");
    Blt_Ps_Append(ps, "%%BeginSetup\n");
    Blt_Ps_Append(ps, "gsave\n");
    /*
     * Set the conversion from PostScript to X11 coordinates.  Scale pica to
     * pixels and flip the y-axis (the origin is the upperleft corner).
     */
    Blt_Ps_VarAppend(ps,
	"% Transform coordinate system to use X11 coordinates\n"
	"% 1. Flip y-axis over by reversing the scale,\n", (char *)NULL);
    Blt_Ps_Append(ps, "1 -1 scale\n");
    Blt_Ps_VarAppend(ps, 
	"% 2. Translate the origin to the other side of the page,\n"
	"%    making the origin the upper left corner\n", (char *)NULL);
    Blt_Ps_Format(ps, "0 %d translate\n\n", -setupPtr->paperHeight);
    Blt_Ps_VarAppend(ps, "% User defined page layout\n\n",
	"% Set color level\n", (char *)NULL);
    Blt_Ps_Format(ps, "%% Set origin\n%d %d translate\n\n",
		  setupPtr->left, setupPtr->bottom);
    if (setupPtr->flags & PS_LANDSCAPE) {
	Blt_Ps_Format(ps,
	    "%% Landscape orientation\n0 %g translate\n-90 rotate\n",
	    ((double)srcPtr->width * setupPtr->scale));
    }
    if (setupPtr->scale != 1.0f) {
	Blt_Ps_Append(ps, "\n% Setting picture scale factor\n");
	Blt_Ps_Format(ps, " %g %g scale\n", setupPtr->scale, setupPtr->scale);
    }
    Blt_Ps_Append(ps, "\n%%EndSetup\n\n");
    return TCL_OK;
}

static Blt_Picture
PsToPicture(
    Tcl_Interp *interp, 
    const char *fileName,
    Blt_DBuffer buffer,
    PsImportSwitches *switchesPtr)
{
    return NULL;
}

static int
PictureToPs(
    Tcl_Interp *interp,
    Blt_Picture original,
    Blt_Ps ps,
    PsExportSwitches *switchesPtr)
{
    Picture *srcPtr;
    int w, h;

    srcPtr = original;
    w = srcPtr->width, h = srcPtr->height;
    Blt_Ps_ComputeBoundingBox(&switchesPtr->setup, &w, &h);
    if (PostScriptPreamble(interp, srcPtr, switchesPtr, ps) != TCL_OK) {
	return TCL_ERROR;
    }
    Blt_ClassifyPicture(srcPtr); 
    if (!Blt_PictureIsOpaque(srcPtr)) {
	Blt_Picture background;

	background = Blt_CreatePicture(srcPtr->width, srcPtr->height);
	Blt_BlankPicture(background, &switchesPtr->bg);
	Blt_BlendPictures(background, srcPtr, 0, 0, srcPtr->width, 
		srcPtr->height, 0, 0);
	srcPtr = background;
    }
    if (srcPtr->flags & BLT_PIC_ASSOCIATED_COLORS) {
	Blt_UnassociateColors(srcPtr);
    }
    Blt_Ps_Rectangle(ps, 0, 0, srcPtr->width, srcPtr->height);
    Blt_Ps_Append(ps, "gsave clip\n\n");
    Blt_Ps_DrawPicture(ps, srcPtr, 0, 0);
    Blt_Ps_VarAppend(ps, "\n",
	"% Unset clipping\n",
	"grestore\n\n", (char *)NULL);
    Blt_Ps_VarAppend(ps,
	"showpage\n",
	"%Trailer\n",
	"grestore\n",
	"end\n",
	"%EOF\n", (char *)NULL);
    if (srcPtr != original) {
	Blt_Free(srcPtr);
    }
    return TCL_OK;
}

static int 
IsPs(Blt_DBuffer dbuffer)
{
    unsigned char *bp;

    Blt_DBuffer_ResetCursor(dbuffer);
    if (Blt_DBuffer_BytesLeft(dbuffer) < 4) {
	return FALSE;
    }
    bp = Blt_DBuffer_Pointer(dbuffer);
    return (strncmp("%!PS", (char *)bp, 4) == 0);
}

static Blt_Picture
ReadPs(Tcl_Interp *interp, const char *fileName, Blt_DBuffer dbuffer)
{
    PsImportSwitches switches;

    memset(&switches, 0, sizeof(switches));
    return PsToPicture(interp, fileName, dbuffer, &switches);
}

static Tcl_Obj *
WritePs(Tcl_Interp *interp, Blt_Picture picture)
{
    Blt_Ps ps;
    PsExportSwitches switches;
    Tcl_Obj *objPtr;
    int result;

    /* Default export switch settings. */
    memset(&switches, 0, sizeof(switches));
    switches.bg.u32 = 0xFFFFFFFF; /* Default bgcolor is white. */
    switches.setup.reqPaperHeight = 792; /* 11 inches */
    switches.setup.reqPaperWidth = 612; /* 8.5 inches */
    switches.setup.level = 1;
    switches.setup.padX.side1 = 72;
    switches.setup.padX.side2 = 72;
    switches.setup.padY.side1 = 72;
    switches.setup.padY.side2 = 72;
    switches.setup.flags = 0;

    ps = Blt_Ps_Create(interp, &switches.setup);
    result = PictureToPs(interp, picture, ps, &switches);
    objPtr = NULL;
    if (result == TCL_OK) {
	objPtr = Tcl_NewStringObj((char *)Blt_Ps_GetValue(ps), -1);
    }
    Blt_Ps_Free(ps);
    return objPtr;
}

static Blt_Picture
ImportPs(
    Tcl_Interp *interp, 
    int objc,
    Tcl_Obj *const *objv,
    const char **fileNamePtr)
{
    Blt_DBuffer dbuffer;
    Blt_Picture picture;
    const char *string;
    PsImportSwitches switches;

    memset(&switches, 0, sizeof(switches));
    if (Blt_ParseSwitches(interp, psImportSwitches, objc - 3, objv + 3, 
	&switches, BLT_SWITCH_DEFAULTS) < 0) {
	return NULL;
    }
    if ((switches.dataObjPtr != NULL) && (switches.fileObjPtr != NULL)) {
	Tcl_AppendResult(interp, "more than one import source: ",
		"use only one -file or -data flag.", (char *)NULL);
	return NULL;
    }
    picture = NULL;
    dbuffer = Blt_DBuffer_Create();
    if (switches.dataObjPtr != NULL) {
	int length;

	string = Tcl_GetStringFromObj(switches.fileObjPtr, &length);
	Blt_DBuffer_AppendData(dbuffer, (unsigned char *)string, 
		(size_t)length);
	string = "data buffer";
	*fileNamePtr = NULL;
    } else {
	string = Tcl_GetString(switches.fileObjPtr);
	if (Blt_DBuffer_LoadFile(interp, string, dbuffer) != TCL_OK) {
	    Blt_DBuffer_Destroy(dbuffer);
	    return NULL;
	}
	*fileNamePtr = string;
    }
    picture = PsToPicture(interp, string, dbuffer, &switches);
    Blt_DBuffer_Destroy(dbuffer);
    return picture;
}

static int
ExportPs(
    Tcl_Interp *interp, 
    Blt_Picture picture,
    int objc,
    Tcl_Obj *const *objv)
{
    PsExportSwitches switches;
    Blt_Ps ps;
    int result;

    memset(&switches, 0, sizeof(switches));
    switches.bg.u32 = 0xFFFFFFFF; /* Default bgcolor is white. */
    switches.setup.reqPaperHeight = 792; /* 11 inches */
    switches.setup.reqPaperWidth = 612; /* 8.5 inches */
    switches.setup.level = 1;
    switches.setup.padX.side1 = 72;
    switches.setup.padX.side2 = 72;
    switches.setup.padY.side1 = 72;
    switches.setup.padY.side2 = 72;
    switches.setup.flags = 0;
    if (Blt_ParseSwitches(interp, psExportSwitches, objc - 3, objv + 3, 
	&switches, BLT_SWITCH_DEFAULTS) < 0) {
	return TCL_ERROR;
    }
    if ((switches.dataObjPtr != NULL) && (switches.fileObjPtr != NULL)) {
	Tcl_AppendResult(interp, "more than one export destination: ",
		"use only one -file or -data switch.", (char *)NULL);
	return TCL_ERROR;
    }
    ps = Blt_Ps_Create(interp, &switches.setup);
    result = PictureToPs(interp, picture, ps, &switches);
    if (result != TCL_OK) {
	Tcl_AppendResult(interp, "can't convert \"", 
		Tcl_GetString(objv[2]), "\"", (char *)NULL);
	goto error;
    }
    if (switches.fileObjPtr != NULL) {
	char *fileName;

	fileName = Tcl_GetString(switches.fileObjPtr);
	result = Blt_Ps_SaveFile(interp, ps, fileName);
    } else if (switches.dataObjPtr != NULL) {
	Tcl_Obj *objPtr;

	objPtr = Tcl_NewStringObj(Blt_Ps_GetValue(ps), -1);
	objPtr = Tcl_ObjSetVar2(interp, switches.dataObjPtr, NULL, objPtr, 0);
	result = (objPtr == NULL) ? TCL_ERROR : TCL_OK;
    } else {
	Tcl_SetObjResult(interp, Tcl_NewStringObj(Blt_Ps_GetValue(ps), -1));
    }  
 error:
    Blt_FreeSwitches(psExportSwitches, (char *)&switches, 0);
    Blt_Ps_Free(ps);
    return result;
}

int 
Blt_PicturePsInit(Tcl_Interp *interp)
{
#ifdef USE_TCL_STUBS
    if (Tcl_InitStubs(interp, TCL_VERSION, 1) == NULL) {
	return TCL_ERROR;
    };
#endif
    if (Tcl_PkgRequire(interp, "bltextra", BLT_VERSION, /*Exact*/1) == NULL) {
	return TCL_ERROR;
    }
    if (Tcl_PkgProvide(interp, "bltpictureps", BLT_VERSION) != TCL_OK) {
	return TCL_ERROR;
    }
    return Blt_PictureRegisterFormat(interp,
	"ps",			/* Name of format. */
	IsPs,			/* Discovery routine. */
	ReadPs,			/* Import format procedure. */
	WritePs,		/* Export format procedure. */
	ImportPs,		/* Import format procedure. */
	ExportPs);		/* Export format procedure. */
}
