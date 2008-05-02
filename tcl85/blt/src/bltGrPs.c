
/*
 * bltGrPs.c --
 *
 * This module implements the "postscript" operation for BLT graph widget.
 *
 *	Copyright 1991-2004 George A Howlett.
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

/*
 *---------------------------------------------------------------------------
 *
 * PostScript routines to print a graph
 *
 *---------------------------------------------------------------------------
 */
#include "bltGraph.h"
#include "bltOp.h"
#include "bltPsInt.h"
#include "bltPicture.h"
#include <X11/Xutil.h>
#include "tkDisplay.h"
#include <stdarg.h>

#define MM_INCH		25.4
#define PICA_INCH	72.0

typedef int (GraphPsProc)(Graph *graphPtr, Tcl_Interp *interp, int objc, 
	Tcl_Obj *const *objv);

static Blt_OptionParseProc ObjToFormat;
static Blt_OptionPrintProc FormatToObj;
static Blt_CustomOption formatOption =
{
    ObjToFormat, FormatToObj, NULL, (ClientData)0,
};

static Blt_OptionParseProc ObjToPica;
static Blt_OptionPrintProc PicaToObj;
static Blt_CustomOption picaOption =
{
    ObjToPica, PicaToObj, NULL, (ClientData)0,
};

static Blt_OptionParseProc ObjToPad;
static Blt_OptionPrintProc PadToObj;
static Blt_CustomOption padOption =
{
    ObjToPad, PadToObj, NULL, (ClientData)0,
};

#define DEF_PS_CENTER		"yes"
#define DEF_PS_COLOR_MAP	(char *)NULL
#define DEF_PS_GREYSCALE	"no"
#define DEF_PS_DECORATIONS	"no"
#define DEF_PS_FONT_MAP		(char *)NULL
#define DEF_PS_FOOTER		"no"
#define DEF_PS_LEVEL		"2"
#define DEF_PS_HEIGHT		"0"
#define DEF_PS_LANDSCAPE	"no"
#define DEF_PS_MAXPECT		"no"
#define DEF_PS_PADX		"1.0i"
#define DEF_PS_PADY		"1.0i"
#define DEF_PS_PAPERHEIGHT	"11.0i"
#define DEF_PS_PAPERWIDTH	"8.5i"
#define DEF_PS_PREVIEW		""
#define DEF_PS_WIDTH		"0"

static Blt_ConfigSpec configSpecs[] =
{
    {BLT_CONFIG_BITMASK, "-center", "center", "Center", DEF_PS_CENTER, 
	Blt_Offset(PageSetup, flags), BLT_CONFIG_DONT_SET_DEFAULT, 
        (Blt_CustomOption *)PS_CENTER},
    {BLT_CONFIG_STRING, "-colormap", "colorMap", "ColorMap",
	DEF_PS_COLOR_MAP, Blt_Offset(PageSetup, colorVarName),
	BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_BITMASK, "-decorations", "decorations", "Decorations",
	DEF_PS_DECORATIONS, Blt_Offset(PageSetup, flags),
	BLT_CONFIG_DONT_SET_DEFAULT, (Blt_CustomOption *)PS_DECORATIONS},
    {BLT_CONFIG_STRING, "-fontmap", "fontMap", "FontMap",
	DEF_PS_FONT_MAP, Blt_Offset(PageSetup, fontVarName),
	BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_BITMASK, "-footer", "footer", "Footer", DEF_PS_FOOTER, 
        Blt_Offset(PageSetup, flags), BLT_CONFIG_DONT_SET_DEFAULT,
        (Blt_CustomOption *)PS_FOOTER},
    {BLT_CONFIG_BITMASK, "-greyscale", "greyscale", "Greyscale",
	DEF_PS_GREYSCALE, Blt_Offset(PageSetup, flags),
	BLT_CONFIG_DONT_SET_DEFAULT, (Blt_CustomOption *)PS_GREYSCALE},
    {BLT_CONFIG_PIXELS_NNEG, "-height", "height", "Height", DEF_PS_HEIGHT, 
	Blt_Offset(PageSetup, reqHeight), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_BITMASK, "-landscape", "landscape", "Landscape",
	DEF_PS_LANDSCAPE, Blt_Offset(PageSetup, flags),
	BLT_CONFIG_DONT_SET_DEFAULT, (Blt_CustomOption *)PS_LANDSCAPE},
    {BLT_CONFIG_INT_POS, "-level", "level", "Level", DEF_PS_LEVEL, 
	Blt_Offset(PageSetup, level), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_BITMASK, "-maxpect", "maxpect", "Maxpect", DEF_PS_MAXPECT, 
	Blt_Offset(PageSetup, flags), BLT_CONFIG_DONT_SET_DEFAULT, 
	(Blt_CustomOption *)PS_MAXPECT},
    {BLT_CONFIG_CUSTOM, "-padx", "padX", "PadX", DEF_PS_PADX, 
	Blt_Offset(PageSetup, padX), 0, &padOption},
    {BLT_CONFIG_CUSTOM, "-pady", "padY", "PadY", DEF_PS_PADY, 
	Blt_Offset(PageSetup, padY), 0, &padOption},
    {BLT_CONFIG_CUSTOM, "-paperheight", "paperHeight", "PaperHeight",
	DEF_PS_PAPERHEIGHT, Blt_Offset(PageSetup, reqPaperHeight), 0,
	&picaOption},
    {BLT_CONFIG_CUSTOM, "-paperwidth", "paperWidth", "PaperWidth",
	DEF_PS_PAPERWIDTH, Blt_Offset(PageSetup, reqPaperWidth), 0,
	&picaOption},
    {BLT_CONFIG_CUSTOM, "-preview", "preview", "Preview", DEF_PS_PREVIEW, 
	Blt_Offset(PageSetup, flags), BLT_CONFIG_DONT_SET_DEFAULT, 
	&formatOption},
    {BLT_CONFIG_PIXELS_NNEG, "-width", "width", "Width", DEF_PS_WIDTH, 
	Blt_Offset(PageSetup, reqWidth), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_END, NULL, NULL, NULL, NULL, 0, 0}
};

/*
 *---------------------------------------------------------------------------
 *
 * ObjToFormat --
 *
 *	Convert the string of the PostScript preview format into 
 *	an enumerated type representing the desired format.  The
 *	available formats are:
 *
 *	    PS_FMT_WMF		- Windows Metafile.
 *	    PS_FMT_TIFF  	- TIFF bitmap image.
 *	    PS_FMT_EPSI 	- Device independent ASCII preview
 *
 * Results:
 *	A standard Tcl result.  The format is written into the
 *	page layout information structure.
 *
 * Side Effects:
 *	Future invocations of the "postscript" option will use this
 *	variable to determine how to format a preview image (if one
 *	is selected) when the PostScript output is produced.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToFormat(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,		/* Interpreter to send results back to */
    Tk_Window tkwin,		/* Not used. */
    Tcl_Obj *objPtr,		/* New value. */
    char *widgRec,		/* Widget record */
    int offset,			/* Offset to field in structure */
    int flags)			/* Not used. */
{
    int *formatPtr = (int *) (widgRec + offset);
    size_t length;
    char c;
    char *string;

    string = Tcl_GetString(objPtr);
    c = string[0];
    length = strlen(string);
    if (c == '\0') {
	*formatPtr &= ~PS_FMT_MASK;
    } else if ((c == 'e') && (strncmp(string, "epsi", length) == 0)) {
	*formatPtr &= ~PS_FMT_MASK;
	*formatPtr |= PS_FMT_EPSI;
#ifdef HAVE_TIFF_H
    } else if ((c == 't') && (strncmp(string, "tiff", length) == 0)) {
	*formatPtr &= ~PS_FMT_MASK;
	*formatPtr |= PS_FMT_TIFF;
#endif /* HAVE_TIFF_H */
#ifdef WIN32
    } else if ((c == 'w') && (strncmp(string, "wmf", length) == 0)) {
	*formatPtr &= ~PS_FMT_MASK;
	*formatPtr |= PS_FMT_WMF;
#endif /* WIN32 */
    } else {
	Tcl_AppendResult(interp, "bad format \"", string, "\": should be ",
#ifdef HAVE_TIFF_H
			 "\"tiff\" or ",
#endif /* HAVE_TIFF_H */
#ifdef WIN32
			 "\"wmf\" or ",
#endif /* WIN32 */
			 "\"epsi\"", (char *)NULL);
	return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * FormatToObj --
 *
 *	Convert the preview format into the string representing its
 *	type.
 *
 * Results:
 *	The string representing the preview format is returned.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
FormatToObj(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,		/* Not used. */
    Tk_Window tkwin,		/* Not used. */
    char *widgRec,		/* PostScript structure record */
    int offset,			/* Offset to field in structure */
    int flags)			/* Not used. */
{
    int format = *(int *)(widgRec + offset);
    Tcl_Obj *objPtr;

    switch (format & PS_FMT_MASK) {
    case PS_FMT_EPSI:
	objPtr = Tcl_NewStringObj("epsi", -1);
	break;

    case PS_FMT_WMF:
	objPtr = Tcl_NewStringObj("wmf", -1);
	break;

    case PS_FMT_TIFF:
	objPtr = Tcl_NewStringObj("tiff", -1);
	break;

    default:
	objPtr = Tcl_NewStringObj("", -1);
	break;
    }
    return objPtr;
}

/*
 *---------------------------------------------------------------------------
 *
 * ObjToPica --
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToPica(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,		/* Interpreter to send results back to */
    Tk_Window tkwin,		/* Not used. */
    Tcl_Obj *objPtr,		/* New value. */
    char *widgRec,		/* Widget record */
    int offset,			/* Offset to field in structure */
    int flags)			/* Not used. */
{
    int *picaPtr = (int *)(widgRec + offset);

    return Blt_Ps_GetPicaFromObj(interp, objPtr, picaPtr);
}

/*
 *---------------------------------------------------------------------------
 *
 * PicaToObj --
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
PicaToObj(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,		/* Not used. */
    Tk_Window tkwin,		/* Not used. */
    char *widgRec,		/* PostScript structure record */
    int offset,			/* Offset to field in structure */
    int flags)			/* Not used. */
{
    int pica = *(int *)(widgRec + offset);

    return Tcl_NewIntObj(pica);
}

/*
 *---------------------------------------------------------------------------
 *
 * ObjToPad --
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToPad(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,		/* Interpreter to send results back to */
    Tk_Window tkwin,		/* Not used. */
    Tcl_Obj *objPtr,		/* New value. */
    char *widgRec,		/* Widget record */
    int offset,			/* Offset to field in structure */
    int flags)			/* Not used. */
{
    Blt_Pad *padPtr = (Blt_Pad *) (widgRec + offset);

    return Blt_Ps_GetPadFromObj(interp, objPtr, padPtr);
}

/*
 *---------------------------------------------------------------------------
 *
 * PadToObj --
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
PadToObj(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,
    Tk_Window tkwin,		/* Not used. */
    char *widgRec,		/* PostScript structure record */
    int offset,			/* Offset to field in structure */
    int flags)			/* Not used. */
{
    Blt_Pad *padPtr = (Blt_Pad *)(widgRec + offset);
    Tcl_Obj *objPtr, *listObjPtr;
	    
    listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
    objPtr = Tcl_NewIntObj(padPtr->side1);
    Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
    objPtr = Tcl_NewIntObj(padPtr->side2);
    Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
    return listObjPtr;
}


/*
 *---------------------------------------------------------------------------
 *
 * PreviewImage --
 *
 * 	Generates a EPSI thumbnail of the graph.  The thumbnail is restricted
 * 	to a certain size.  This is to keep the size of the PostScript file
 * 	small and the processing time low.
 *
 *	The graph is drawn into a pixmap.  We then take a snapshot of that
 *	pixmap, and rescale it to a smaller image.  Finally, the image is
 *	dumped to PostScript.
 *
 * Results:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
static void
PreviewImage(Graph *graphPtr, Blt_Ps ps)
{
    PageSetup *setupPtr = graphPtr->pageSetup;
    int noBackingStore = 0;
    Pixmap drawable;
    Blt_Picture picture, greyscale;
    int nLines;
    Tcl_DString dString;

    /* Create a pixmap and draw the graph into it. */

    drawable = Tk_GetPixmap(graphPtr->display, Tk_WindowId(graphPtr->tkwin),
	graphPtr->width, graphPtr->height, Tk_Depth(graphPtr->tkwin));
    Blt_DrawGraph(graphPtr, drawable, noBackingStore);
    
    /* Get a color picture from the pixmap. */
    picture = Blt_DrawableToPicture(graphPtr->tkwin, drawable, 0, 0, 
	graphPtr->width, graphPtr->height, 1.0f);
    Tk_FreePixmap(graphPtr->display, drawable);
    if (picture == NULL) {
	return;			/* Can't grab pixmap? */
    }
#ifdef THUMBNAIL_PREVIEW
    {
	float scale, xScale, yScale;
	int width, height;
	Blt_Picture destPict;

	/* Scale the source picture into a size appropriate for a thumbnail. */
#define PS_MAX_PREVIEW_WIDTH	300.0
#define PS_MAX_PREVIEW_HEIGHT	300.0
	xScale = PS_MAX_PREVIEW_WIDTH / (float)graphPtr->width;
	yScale = PS_MAX_PREVIEW_HEIGHT / (float)graphPtr->height;
	scale = MIN(xScale, yScale);

	width = (int)(scale * graphPtr->width + 0.5f);
	height = (int)(scale * graphPtr->height + 0.5f);
	destPict = Blt_CreatePicture(width, height);
	destPict = Blt_ResamplePicture(destPict, picture, bltBoxFilter, 
		bltBoxFilter);
	Blt_FreePicture(picture);
	picture = destPict;
    }
#endif /* THUMBNAIL_PREVIEW */
    greyscale = Blt_GreyscalePicture(picture);
    Blt_FreePicture(picture);
    picture = greyscale;

    if (setupPtr->flags & PS_LANDSCAPE) {
	Blt_Picture landscape;

	landscape = Blt_RotatePicture(picture, 90.0);
	Blt_FreePicture(picture);
	picture = landscape;
    }
    Tcl_DStringInit(&dString);
    /* Finally, we can generate PostScript for the picture */
    nLines = Blt_PictureToPsData(picture, 1, &dString, "%");

    Blt_Ps_Append(ps, "%%BeginPreview: ");
    Blt_Ps_Format(ps, "%d %d 8 %d\n", Blt_PictureWidth(picture),
	Blt_PictureHeight(picture), nLines);
    Blt_Ps_Append(ps, Tcl_DStringValue(&dString));
    Blt_Ps_Append(ps, "%%EndPreview\n\n");
    Tcl_DStringFree(&dString);
    Blt_FreePicture(picture);
}

/*
 *---------------------------------------------------------------------------
 *
 * PostScriptPreamble
 *
 *    	The PostScript preamble calculates the needed translation and scaling
 *    	to make X11 coordinates compatible with PostScript.
 *
 *---------------------------------------------------------------------------
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

static int
PostScriptPreamble(
    Graph *graphPtr, 
    char *fileName, 
    Blt_Ps ps)
{
    PageSetup *setupPtr = graphPtr->pageSetup;
    time_t ticks;
    char date[200];		/* Hold the date string from ctime() */
    const char *version;
    char *newline;

    if (fileName == NULL) {
	fileName = Tk_PathName(graphPtr->tkwin);
    }
    Blt_Ps_Append(ps, "%!PS-Adobe-3.0 EPSF-3.0\n");

    /*
     * The "BoundingBox" comment is required for EPS files. The box
     * coordinates are integers, so we need round away from the center of the
     * box.
     */
    Blt_Ps_Format(ps, "%%%%BoundingBox: %d %d %d %d\n",
	setupPtr->left, setupPtr->paperHeight - setupPtr->top,
	setupPtr->right, setupPtr->paperHeight - setupPtr->bottom);
	
    Blt_Ps_Append(ps, "%%Pages: 0\n");

    version = Tcl_GetVar(graphPtr->interp, "blt_version", TCL_GLOBAL_ONLY);
    if (version == NULL) {
	version = "???";
    }
    Blt_Ps_Format(ps, "%%%%Creator: (BLT %s %s)\n", version,
	Tk_Class(graphPtr->tkwin));

    ticks = time((time_t *) NULL);
    strcpy(date, ctime(&ticks));
    newline = date + strlen(date) - 1;
    if (*newline == '\n') {
	*newline = '\0';
    }
    Blt_Ps_Format(ps, "%%%%CreationDate: (%s)\n", date);
    Blt_Ps_Format(ps, "%%%%Title: (%s)\n", fileName);
    Blt_Ps_Append(ps, "%%DocumentData: Clean7Bit\n");
    if (setupPtr->flags & PS_LANDSCAPE) {
	Blt_Ps_Append(ps, "%%Orientation: Landscape\n");
    } else {
	Blt_Ps_Append(ps, "%%Orientation: Portrait\n");
    }
    Blt_Ps_Append(ps, "%%DocumentNeededResources: font Helvetica Courier\n");
    Blt_Ps_Append(ps, "%%EndComments\n\n");
    if (setupPtr->flags & PS_FMT_EPSI) {
	PreviewImage(graphPtr, ps);
    }
    if (Blt_Ps_IncludeFile(graphPtr->interp, ps, "bltGraph.pro") != TCL_OK) {
	return TCL_ERROR;
    }
    if (setupPtr->flags & PS_FOOTER) {
	const char *who;

	who = getenv("LOGNAME");
	if (who == NULL) {
	    who = "???";
	}
	Blt_Ps_VarAppend(ps,
	    "8 /Helvetica SetFont\n",
	    "10 30 moveto\n",
	    "(Date: ", date, ") show\n",
	    "10 20 moveto\n",
	    "(File: ", fileName, ") show\n",
	    "10 10 moveto\n",
	    "(Created by: ", who, "@", Tcl_GetHostName(), ") show\n",
	    "0 0 moveto\n",
	    (char *)NULL);
    }
    /*
     * Set the conversion from PostScript to X11 coordinates.  Scale
     * pica to pixels and flip the y-axis (the origin is the upperleft
     * corner).
     */
    Blt_Ps_VarAppend(ps,
	"% Transform coordinate system to use X11 coordinates\n\n",
	"% 1. Flip y-axis over by reversing the scale,\n",
	"% 2. Translate the origin to the other side of the page,\n",
	"%    making the origin the upper left corner\n", (char *)NULL);
    Blt_Ps_Format(ps, "1 -1 scale\n");
    /* Papersize is in pixels.  Translate the new origin *after*
     * changing the scale. */
    Blt_Ps_Format(ps, "0 %d translate\n\n", -setupPtr->paperHeight);
    Blt_Ps_VarAppend(ps, "% User defined page layout\n\n",
		     "% Set color level\n", (char *)NULL);
    Blt_Ps_Format(ps, "%% Set origin\n%d %d translate\n\n",
	setupPtr->left, setupPtr->bottom);
    if (setupPtr->flags & PS_LANDSCAPE) {
	Blt_Ps_Format(ps,
	    "%% Landscape orientation\n0 %g translate\n-90 rotate\n",
	    ((double)graphPtr->width * setupPtr->scale));
    }
    if (setupPtr->scale != 1.0f) {
	Blt_Ps_Append(ps, "\n% Setting graph scale factor\n");
	Blt_Ps_Format(ps, " %g %g scale\n", setupPtr->scale, setupPtr->scale);
    }
    Blt_Ps_Append(ps, "\n%%EndSetup\n\n");
    return TCL_OK;
}


static void
MarginsToPostScript(
    Graph *graphPtr, 
    Blt_Ps ps)
{
    PageSetup *setupPtr = graphPtr->pageSetup;
    XRectangle margin[4];

    margin[0].x = margin[0].y = margin[3].x = margin[1].x = 0;
    margin[0].width = margin[3].width = graphPtr->width;
    margin[0].height = graphPtr->top;
    margin[3].y = graphPtr->bottom;
    margin[3].height = graphPtr->height - graphPtr->bottom;
    margin[2].y = margin[1].y = graphPtr->top;
    margin[1].width = graphPtr->left;
    margin[2].height = margin[1].height = graphPtr->bottom - graphPtr->top;
    margin[2].x = graphPtr->right;
    margin[2].width = graphPtr->width - graphPtr->right;

    /* Clear the surrounding margins and clip the plotting surface */
    if (setupPtr->flags & PS_DECORATIONS) {
	Blt_Ps_XSetBackground(ps,Blt_BackgroundBorderColor(graphPtr->normalBg));
    } else {
	Blt_Ps_SetClearBackground(ps);
    }
    Blt_Ps_Append(ps, "% Margins\n");
    Blt_Ps_XFillRectangles(ps, margin, 4);
    
    Blt_Ps_Append(ps, "% Interior 3D border\n");
    /* Interior 3D border */
    if ((graphPtr->plotBorderWidth > 0) && (setupPtr->flags & PS_DECORATIONS)) {
	Tk_3DBorder border;
	int x, y, width, height;

	x = graphPtr->left - graphPtr->plotBorderWidth;
	y = graphPtr->top - graphPtr->plotBorderWidth;
	width = (graphPtr->right - graphPtr->left) + 
		(2 * graphPtr->plotBorderWidth);
	height = (graphPtr->bottom - graphPtr->top) + 
		(2 * graphPtr->plotBorderWidth);
	border = Blt_BackgroundBorder(graphPtr->normalBg);
	Blt_Ps_Draw3DRectangle(ps, border, (double)x, (double)y, width, height,
		graphPtr->plotBorderWidth, graphPtr->plotRelief);
    }
    if (Blt_LegendSite(graphPtr->legend) & LEGEND_IN_MARGIN) {
	/*
	 * Print the legend if we're using a site which lies in one
	 * of the margins (left, right, top, or bottom) of the graph.
	 */
	Blt_LegendToPostScript(graphPtr->legend, ps);
    }
    if (graphPtr->title != NULL) {
	Blt_Ps_Append(ps, "% Graph title\n");
	Blt_Ps_DrawText(ps, graphPtr->title, &graphPtr->titleTextStyle, 
		(double)graphPtr->titleX, (double)graphPtr->titleY);
    }
    Blt_AxesToPostScript(graphPtr, ps);
}


static int
GraphToPostScript(
    Graph *graphPtr,
    char *ident,		/* Identifier string (usually the filename) */
    Blt_Ps ps)
{
    int x, y, w, h;
    int result;

    /*   
     * We need to know how big a graph to print.  If the graph hasn't been
     * drawn yet, the width and height will be 1.  Instead use the requested
     * size of the widget.  The user can still override this with the -width
     * and -height postscript options.
     */
    h = (graphPtr->height>0) ? graphPtr->height : Tk_ReqHeight(graphPtr->tkwin);
    w = (graphPtr->width>0) ? graphPtr->width : Tk_ReqWidth(graphPtr->tkwin);

    Blt_Ps_ComputeBoundingBox(graphPtr->pageSetup, &w, &h);
    graphPtr->width = w;
    graphPtr->height = h;
    graphPtr->flags |= LAYOUT_NEEDED | MAP_WORLD;
    Blt_LayoutGraph(graphPtr);

    result = PostScriptPreamble(graphPtr, ident, ps);
    if (result != TCL_OK) {
	goto error;
    }
    /* Determine rectangle of the plotting area for the graph window */
    x = graphPtr->left - graphPtr->plotBorderWidth;
    y = graphPtr->top - graphPtr->plotBorderWidth;

    w = (graphPtr->right - graphPtr->left + 1) + (2*graphPtr->plotBorderWidth);
    h = (graphPtr->bottom - graphPtr->top + 1) + (2*graphPtr->plotBorderWidth);

    Blt_Ps_XSetFont(ps, Blt_Ts_GetFont(graphPtr->titleTextStyle));
    if (graphPtr->pageSetup->flags & PS_DECORATIONS) {
	Blt_Ps_XSetBackground(ps, Blt_BackgroundBorderColor(graphPtr->plotBg));
    } else {
	Blt_Ps_SetClearBackground(ps);
    }
    Blt_Ps_XFillRectangle(ps, x, y, w, h);
    Blt_Ps_Rectangle(ps, x, y, w, h);
    Blt_Ps_Append(ps, "gsave clip\n\n");
    /* Draw the grid, elements, and markers in the plotting area. */
    Blt_GridsToPostScript(graphPtr, ps);
    Blt_MarkersToPostScript(graphPtr, ps, TRUE);
    if ((Blt_LegendSite(graphPtr->legend) & LEGEND_IN_PLOT) && 
	(!Blt_LegendIsRaised(graphPtr->legend))) {
	/* Print legend underneath elements and markers */
	Blt_LegendToPostScript(graphPtr->legend, ps);
    }
    Blt_AxisLimitsToPostScript(graphPtr, ps);
    Blt_ElementsToPostScript(graphPtr, ps);
    if ((Blt_LegendSite(graphPtr->legend) & LEGEND_IN_PLOT) && 
	(Blt_LegendIsRaised(graphPtr->legend))) {
	/* Print legend above elements (but not markers) */
	Blt_LegendToPostScript(graphPtr->legend, ps);
    }
    Blt_MarkersToPostScript(graphPtr, ps, FALSE);
    Blt_ActiveElementsToPostScript(graphPtr, ps);
    Blt_Ps_VarAppend(ps, "\n",
	"% Unset clipping\n",
	"grestore\n\n", (char *)NULL);
    MarginsToPostScript(graphPtr, ps);
    Blt_Ps_VarAppend(ps,
	"showpage\n",
	"%Trailer\n",
	"grestore\n",
	"end\n",
	"%EOF\n", (char *)NULL);
  error:
    /* Reset height and width of graph window */
    graphPtr->width = Tk_Width(graphPtr->tkwin);
    graphPtr->height = Tk_Height(graphPtr->tkwin);
    graphPtr->flags = MAP_WORLD;

    /*
     * Redraw the graph in order to re-calculate the layout as soon as
     * possible. This is in the case the crosshairs are active.
     */
    Blt_EventuallyRedrawGraph(graphPtr);
    return result;
}

#ifdef WIN32

static void
InitAPMHeader(
    Tk_Window tkwin,
    int width, int height,
    APMHEADER *headerPtr)
{
    unsigned int *p;
    unsigned int sum;
    Screen *screen;
#define MM_INCH		25.4
    double dpiX, dpiY;

    headerPtr->key = 0x9ac6cdd7L;
    headerPtr->hmf = 0;
    headerPtr->inch = 1440;

    screen = Tk_Screen(tkwin);
    dpiX = (WidthOfScreen(screen) * MM_INCH) / WidthMMOfScreen(screen);
    dpiY = (HeightOfScreen(screen) * MM_INCH) / HeightMMOfScreen(screen);

    headerPtr->bbox.Left = headerPtr->bbox.Top = 0;
    headerPtr->bbox.Bottom = (SHORT)((width * 1440) / dpiX);
    headerPtr->bbox.Right = (SHORT)((height * 1440) / dpiY);
    headerPtr->reserved = 0;
    sum = 0;
    for (p = (unsigned int *)headerPtr; 
	 p < (unsigned int *)&(headerPtr->checksum); p++) {
	sum ^= *p;
    }
    headerPtr->checksum = sum;
}

/*
 *---------------------------------------------------------------------------
 *
 * CreateWindowEPS --
 *
 * 	Generates an EPS file with a Window metafile preview.  
 *
 *	Windows metafiles aren't very robust.  Including exactly the
 *	same metafile (one embedded in a DOS EPS, the other as .wmf
 *	file) will play back differently.  
 *	
 * Results:
 *	None.
 *
 * -------------------------------------------------------------------------- 
 */
static int
CreateWindowsEPS(
    Tcl_Interp *interp,
    Graph *graphPtr,
    Blt_Ps ps,
    Tcl_Channel channel)
{
    DWORD size;
    DOSEPSHEADER epsHeader;
    HANDLE hMem;
    HDC hRefDC, hDC;
    HENHMETAFILE hMetaFile;
    Tcl_DString dString;
    TkWinDC drawableDC;
    TkWinDCState state;
    int result;
    unsigned char *buffer, *psBuffer;
    
    Blt_Ps_Append(ps, "\n");
    psBuffer = Blt_Ps_GetValue(ps);
    /*
     * Fill out as much information as we can into the DOS EPS header.
     * We won't know the start of the length of the WMF segment until 
     * we create the metafile.
     */
    epsHeader.magic[0] = 0xC5;
    epsHeader.magic[1] = 0xD0;
    epsHeader.magic[2] = 0xD3;
    epsHeader.magic[3] = 0xC6;
    epsHeader.psStart = sizeof(epsHeader); 
    epsHeader.psLength = strlen(psBuffer) + 1;
    epsHeader.wmfStart = epsHeader.psStart + epsHeader.psLength;
    epsHeader.wmfLength = 0L;	/* Fill in later. */
    epsHeader.tiffStart = 0L;	
    epsHeader.tiffLength = 0L;
    epsHeader.checksum = 0xFFFF;

    result = TCL_ERROR;
    hRefDC = TkWinGetDrawableDC(graphPtr->display, Tk_WindowId(graphPtr->tkwin),
	&state);

    /* Build a description string. */
    Tcl_DStringInit(&dString);
    Tcl_DStringAppend(&dString, "BLT Graph ", -1);
    Tcl_DStringAppend(&dString, BLT_VERSION, -1);
    Tcl_DStringAppend(&dString, "\0", -1);
    Tcl_DStringAppend(&dString, Tk_PathName(graphPtr->tkwin), -1);
    Tcl_DStringAppend(&dString, "\0", -1);

    hDC = CreateEnhMetaFile(hRefDC, NULL, NULL, Tcl_DStringValue(&dString));
    Tcl_DStringFree(&dString);
    
    if (hDC == NULL) {
	Tcl_AppendResult(interp, "can't create metafile: ", Blt_LastError(), 
		(char *)NULL);
	return TCL_ERROR;
    }
    /* Assemble a Tk drawable that points to the metafile and let the
     * graph's drawing routine draw into it. */
    drawableDC.hdc = hDC;
    drawableDC.type = TWD_WINDC;
    
    graphPtr->width = Tk_Width(graphPtr->tkwin);
    graphPtr->height = Tk_Height(graphPtr->tkwin);
    graphPtr->flags |= RESET_WORLD;
    Blt_LayoutGraph(graphPtr);
    Blt_DrawGraph(graphPtr, (Drawable)&drawableDC, FALSE);
    GdiFlush();
    hMetaFile = CloseEnhMetaFile(hDC);

    size = GetWinMetaFileBits(hMetaFile, 0, NULL, MM_ANISOTROPIC, hRefDC);
    hMem = GlobalAlloc(GHND, size);
    if (hMem == NULL) {
	Tcl_AppendResult(interp, "can't allocate global memory:", 
		Blt_LastError(), (char *)NULL);
	goto error;
    }
    buffer = (LPVOID)GlobalLock(hMem);
    if (!GetWinMetaFileBits(hMetaFile, size, buffer, MM_ANISOTROPIC, hRefDC)) {
	Tcl_AppendResult(interp, "can't get metafile data:", Blt_LastError(), 
		(char *)NULL);
	goto error;
    }

    /*  
     * Fix up the EPS header with the correct metafile length and PS
     * offset (now that we what they are).
     */
    epsHeader.wmfLength = size;
    epsHeader.wmfStart = epsHeader.psStart + epsHeader.psLength;

    /* Write out the eps header, */
    if (Tcl_Write(channel, (char *)&epsHeader, sizeof(epsHeader))
	!= sizeof(epsHeader)) {
	Tcl_AppendResult(interp, "error writing eps header:", Blt_LastError(), 
			 (char *)NULL);
	goto error;
    }
    /* the PostScript, */
    if (Tcl_Write(channel, psBuffer, epsHeader.psLength)!= epsHeader.psLength) {
	Tcl_AppendResult(interp, "error writing PostScript data:", 
			 Blt_LastError(), (char *)NULL);
	goto error;
    }
    /* and finally the metadata itself. */
    if (Tcl_Write(channel, buffer, size) != (int)size) {
	Tcl_AppendResult(interp, "error writing metafile data:", 
		Blt_LastError(), (char *)NULL);
	goto error;
    }
    result = TCL_OK;

 error:
    DeleteEnhMetaFile(hMetaFile); 
    TkWinReleaseDrawableDC(Tk_WindowId(graphPtr->tkwin), hRefDC, &state);
    Tcl_Close(interp, channel);
    if (hMem != NULL) {
	GlobalUnlock(hMem);
	GlobalFree(hMem);
    }
    graphPtr->flags = MAP_WORLD;
    Blt_EventuallyRedrawGraph(graphPtr);
    return result;
}

#endif /*WIN32*/

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
    Graph *graphPtr,
    Tcl_Interp *interp,
    int objc,			/* Not used. */
    Tcl_Obj *const *objv)
{
    PageSetup *setupPtr = graphPtr->pageSetup;

    if (Blt_ConfigureValueFromObj(interp, graphPtr->tkwin, configSpecs, 
	(char *)setupPtr, objv[3], 0) != TCL_OK) {
	return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ConfigureOp --
 *
 *      This procedure is invoked to print the graph in a file.
 *
 * Results:
 *      A standard TCL result.
 *
 * Side effects:
 *      A new PostScript file is created.
 *
 *---------------------------------------------------------------------------
 */
static int
ConfigureOp(
    Graph *graphPtr,
    Tcl_Interp *interp,
    int objc,			/* Number of options in objv vector */
    Tcl_Obj *const *objv)		/* Option vector */
{
    int flags = BLT_CONFIG_OBJV_ONLY;
    PageSetup *setupPtr = graphPtr->pageSetup;

    if (objc == 3) {
	return Blt_ConfigureInfoFromObj(interp, graphPtr->tkwin, configSpecs,
		(char *)setupPtr, (Tcl_Obj *)NULL, flags);
    } else if (objc == 4) {
	return Blt_ConfigureInfoFromObj(interp, graphPtr->tkwin, configSpecs,
		(char *)setupPtr, objv[3], flags);
    }
    if (Blt_ConfigureWidgetFromObj(interp, graphPtr->tkwin, configSpecs, 
	    objc - 3, objv + 3, (char *)setupPtr, flags) != TCL_OK) {
	return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * OutputOp --
 *
 *      This procedure is invoked to print the graph in a file.
 *
 * Results:
 *      Standard TCL result.  TCL_OK if plot was successfully printed,
 *	TCL_ERROR otherwise.
 *
 * Side effects:
 *      A new PostScript file is created.
 *
 *---------------------------------------------------------------------------
 */
static int
OutputOp(
    Graph *graphPtr,		/* Graph widget record */
    Tcl_Interp *interp,
    int objc,			/* Number of options in objv vector */
    Tcl_Obj *const *objv)	/* Option vector */
{
    const char *buffer;
    PostScript *psPtr;
    Tcl_Channel channel;
    char *fileName;		/* Name of file to write PostScript output
                                 * If NULL, output is returned via
                                 * interp->result. */
    size_t length;

    fileName = NULL;		/* Used to identify the output sink. */
    channel = NULL;
    if (objc > 3) {
	fileName = Tcl_GetString(objv[3]);
	if (fileName[0] != '-') {
	    objv++, objc--;	/* First argument is the file name. */
	    channel = Tcl_OpenFileChannel(interp, fileName, "w", 0666);
	    if (channel == NULL) {
		return TCL_ERROR;
	    }
	    if (Tcl_SetChannelOption(interp, channel, "-translation", "binary") 
		!= TCL_OK) {
		return TCL_ERROR;
	    }
	}
    }

    psPtr = Blt_Ps_Create(graphPtr->interp, graphPtr->pageSetup);
    if (objc > 3) {
	if (Blt_ConfigureWidgetFromObj(interp, graphPtr->tkwin, configSpecs, 
		objc - 3, objv + 3, (char *)graphPtr->pageSetup, 
		BLT_CONFIG_OBJV_ONLY) != TCL_OK) {
	    return TCL_ERROR;
	}
    }
    if (GraphToPostScript(graphPtr, fileName, psPtr) != TCL_OK) {
	goto error;
    }
    buffer = Blt_Ps_GetValue(psPtr);
    length = strlen(buffer);
    if (channel != NULL) {
	int nBytes;
	/*
	 * If a file name was given, write the results to that file
	 */
#ifdef WIN32
	if (graphPtr->pageSetup->flags & PS_FMT_EPSI) {
	    if (CreateWindowsEPS(interp, graphPtr, psPtr, channel)) {
		return TCL_ERROR;
	    }
	} else {	    
	    nBytes = Tcl_Write(channel, buffer, length);
	    if (nBytes < 0) {
		Tcl_AppendResult(interp, "error writing file \"", fileName, 
			"\": ", Tcl_PosixError(interp), (char *)NULL);
		goto error;
	    }
	}
#else 
	nBytes = Tcl_Write(channel, buffer, length);
	if (nBytes < 0) {
	    Tcl_AppendResult(interp, "error writing file \"", fileName, "\": ",
		Tcl_PosixError(interp), (char *)NULL);
	    goto error;
	}
#endif /* WIN32 */
        Tcl_Close(interp, channel);
    } else {
	Tcl_SetStringObj(Tcl_GetObjResult(interp), buffer, length);
    }
    Blt_Ps_Free(psPtr);
    return TCL_OK;

  error:
    if (channel != NULL) {
        Tcl_Close(interp, channel);
    }
    Blt_Ps_Free(psPtr);
    return TCL_ERROR;
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_CreatePostScript --
 *
 *      Creates a postscript structure.
 *
 * Results:
 *      Always TCL_OK.
 *
 * Side effects:
 *      A new PostScript structure is created.
 *
 *---------------------------------------------------------------------------
 */
int
Blt_CreatePageSetup(Graph *graphPtr)
{
    PageSetup *setupPtr;

    setupPtr = Blt_CallocAssert(1, sizeof(PostScript));
    setupPtr->flags = PS_CENTER;
    setupPtr->level = 2;
    graphPtr->pageSetup = setupPtr;

    if (Blt_ConfigureComponentFromObj(graphPtr->interp, graphPtr->tkwin,
	    "postscript", "Postscript", configSpecs, 0, (Tcl_Obj **)NULL,
	    (char *)setupPtr, 0) != TCL_OK) {
	return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_PostScriptOp --
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
static Blt_OpSpec psOps[] =
{
    {"cget", 2, CgetOp, 4, 4, "option",},
    {"configure", 2, ConfigureOp, 3, 0, "?option value?...",},
    {"output", 1, OutputOp, 3, 0, "?fileName? ?option value?...",},
};

static int nPsOps = sizeof(psOps) / sizeof(Blt_OpSpec);

int
Blt_PostScriptOp(
    Graph *graphPtr,		/* Graph widget record */
    Tcl_Interp *interp,
    int objc,			/* # arguments */
    Tcl_Obj *const *objv)	/* Argument list */
{
    GraphPsProc *proc;
    int result;

    proc = Blt_GetOpFromObj(interp, nPsOps, psOps, BLT_OP_ARG2, objc, objv, 0);
    if (proc == NULL) {
	return TCL_ERROR;
    }
    result = (*proc) (graphPtr, interp, objc, objv);
    return result;
}


void
Blt_DestroyPageSetup(Graph *graphPtr)
{
    if (graphPtr->pageSetup != NULL) {
	Blt_FreeOptions(configSpecs, (char *)graphPtr->pageSetup, 
			graphPtr->display, 0);
	Blt_Free(graphPtr->pageSetup);
    }
}

