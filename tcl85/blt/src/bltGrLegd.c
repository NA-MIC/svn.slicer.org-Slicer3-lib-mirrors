
/*
 * bltGrLegd.c --
 *
 * This module implements the legend for the BLT graph widget.
 *
 *	Copyright 1993-2004 George A Howlett.
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

#include "bltGraph.h"
#include "bltOp.h"
#include "bltGrElem.h"

typedef int (GraphLegendProc)(Graph *graphPtr, Tcl_Interp *interp, int objc, 
	Tcl_Obj *const *objv);

/*
 *---------------------------------------------------------------------------
 *
 * Legend --
 *
 * 	Contains information specific to how the legend will be
 *	displayed.
 *
 *
 *---------------------------------------------------------------------------
 */
struct LegendStruct {
    unsigned int flags;
    ClassId classId;		/* Type: Element or Marker. */

    int hidden;			/* If non-zero, don't display the legend. */

    int raised;			/* If non-zero, draw the legend last, above
				 * everything else. */

    int nEntries;		/* Number of element entries in table. */
    short int nColumns, nRows;	/* Number of columns and rows in legend */
    short int width, height;	/* Dimensions of the legend */
    short int entryWidth, entryHeight;

    int site;
    Point2d screenPt;		/* Says how to position the legend. Indicates 
				 * the site and/or x-y screen coordinates of 
				 * the legend.  Used in conjunction with the 
				 * anchor to determine its location. */

    Tk_Anchor anchor;		/* Anchor of legend. Used to interpret the
				 * positioning point of the legend in the
				 * graph*/

    int x, y;			/* Computed origin of legend. */

    Graph *graphPtr;
    Tcl_Command cmdToken;	/* Token for graph's widget command. */
    int reqColumns, reqRows;

    Blt_Pad ipadX, ipadY;	/* # of pixels padding around legend entries */
    Blt_Pad padX, padY;		/* # of pixels padding to exterior of legend */

    Tk_Window tkwin;		/* Optional external window to draw legend. */

    TextStyle style;

    int maxSymSize;		/* Size of largest symbol to be displayed.
				 * Used to calculate size of legend */

    XColor *fgColor;
    Blt_Background activeBg; /* Active legend entry background color. */
    XColor *activeFgColor;
    int activeRelief;		/* 3-D effect on active entry. */
    int entryBorderWidth;	/* Border width around each entry in legend. */

    Blt_Background normalBg; /* 3-D effect of legend. */
    int borderWidth;		/* Width of legend 3-D border */
    int relief;			/* 3-d effect of border around the legend:
				 * TK_RELIEF_RAISED etc. */

    Blt_BindTable bindTable;
};

#define padLeft  	padX.side1
#define padRight  	padX.side2
#define padTop		padY.side1
#define padBottom	padY.side2
#define PADDING(x)	((x).side1 + (x).side2)

#define DEF_LEGEND_ACTIVE_BACKGROUND 	STD_ACTIVE_BACKGROUND
#define DEF_LEGEND_ACTIVE_BORDERWIDTH  "2"
#define DEF_LEGEND_ACTIVE_FOREGROUND	STD_ACTIVE_FOREGROUND
#define DEF_LEGEND_ACTIVE_RELIEF	"flat"
#define DEF_LEGEND_ANCHOR	   	"n"
#define DEF_LEGEND_BACKGROUND	   	(char *)NULL
#define DEF_LEGEND_BORDERWIDTH		STD_BORDERWIDTH
#define DEF_LEGEND_FOREGROUND		STD_NORMAL_FOREGROUND
#define DEF_LEGEND_FONT			"{Sans Serif} 8"
#define DEF_LEGEND_HIDE			"no"
#define DEF_LEGEND_IPAD_X		"1"
#define DEF_LEGEND_IPAD_Y		"1"
#define DEF_LEGEND_PAD_X		"1"
#define DEF_LEGEND_PAD_Y		"1"
#define DEF_LEGEND_POSITION		"rightmargin"
#define DEF_LEGEND_RAISED       	"no"
#define DEF_LEGEND_RELIEF		"sunken"
#define DEF_LEGEND_ROWS			"0"
#define DEF_LEGEND_COLUMNS		"0"

static Blt_OptionParseProc ObjToPosition;
static Blt_OptionPrintProc PositionToObj;
static Blt_CustomOption legendPositionOption =
{
    ObjToPosition, PositionToObj, NULL, (ClientData)0
};

static Blt_ConfigSpec configSpecs[] =
{
    {BLT_CONFIG_BACKGROUND, "-activebackground", "activeBackground",
	"ActiveBackground", DEF_LEGEND_ACTIVE_BACKGROUND,
	Blt_Offset(Legend, activeBg), 0},
    {BLT_CONFIG_PIXELS_NNEG, "-activeborderwidth", "activeBorderWidth",
	"BorderWidth", DEF_LEGEND_BORDERWIDTH, 
	Blt_Offset(Legend, entryBorderWidth),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_COLOR, "-activeforeground", "activeForeground",
	"ActiveForeground", DEF_LEGEND_ACTIVE_FOREGROUND,
	Blt_Offset(Legend, activeFgColor), 0},
    {BLT_CONFIG_RELIEF, "-activerelief", "activeRelief", "Relief",
	DEF_LEGEND_ACTIVE_RELIEF, Blt_Offset(Legend, activeRelief),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_ANCHOR, "-anchor", "anchor", "Anchor", DEF_LEGEND_ANCHOR, 
	Blt_Offset(Legend, anchor), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_SYNONYM, "-bg", "background", (char *)NULL, (char *)NULL, 0, 0},
    {BLT_CONFIG_BACKGROUND, "-background", "background", "Background",
	DEF_LEGEND_BACKGROUND, Blt_Offset(Legend, normalBg),BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_PIXELS_NNEG, "-borderwidth", "borderWidth", "BorderWidth",
	DEF_LEGEND_BORDERWIDTH, Blt_Offset(Legend, borderWidth),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_SYNONYM, "-bd", "borderWidth", (char *)NULL, (char *)NULL, 0,0},
    {BLT_CONFIG_INT_NNEG, "-columns", "columns", "columns",
	DEF_LEGEND_COLUMNS, Blt_Offset(Legend, reqColumns),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_FONT, "-font", "font", "Font", DEF_LEGEND_FONT, 
	Blt_Offset(Legend, style.font), 0},
    {BLT_CONFIG_SYNONYM, "-fg", "foreground", (char *)NULL, (char *)NULL, 0, 0},
    {BLT_CONFIG_COLOR, "-foreground", "foreground", "Foreground",
	DEF_LEGEND_FOREGROUND, Blt_Offset(Legend, fgColor), 0},
    {BLT_CONFIG_BOOLEAN, "-hide", "hide", "Hide", DEF_LEGEND_HIDE, 
	Blt_Offset(Legend, hidden), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_PAD, "-ipadx", "iPadX", "Pad", DEF_LEGEND_IPAD_X, 
	Blt_Offset(Legend, ipadX), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_PAD, "-ipady", "iPadY", "Pad", DEF_LEGEND_IPAD_Y, 
	Blt_Offset(Legend, ipadY), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_PAD, "-padx", "padX", "Pad", DEF_LEGEND_PAD_X, 
	Blt_Offset(Legend, padX), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_PAD, "-pady", "padY", "Pad", DEF_LEGEND_PAD_Y, 
	Blt_Offset(Legend, padY), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_CUSTOM, "-position", "position", "Position", 
	DEF_LEGEND_POSITION, 0, BLT_CONFIG_DONT_SET_DEFAULT, 
        &legendPositionOption},
    {BLT_CONFIG_BOOLEAN, "-raised", "raised", "Raised", DEF_LEGEND_RAISED, 
	Blt_Offset(Legend, raised), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_RELIEF, "-relief", "relief", "Relief", DEF_LEGEND_RELIEF, 
	Blt_Offset(Legend, relief), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_INT_NNEG, "-rows", "rows", "rows", DEF_LEGEND_ROWS, 
	Blt_Offset(Legend, reqRows),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_END, NULL, NULL, NULL, NULL, 0, 0}
};

static Tcl_IdleProc DisplayLegend;
static Blt_BindPickProc PickLegendEntry;
static Tk_EventProc LegendEventProc;

BLT_EXTERN Tcl_ObjCmdProc Blt_GraphInstCmdProc;

/*
 *---------------------------------------------------------------------------
 *
 * EventuallyRedrawLegend --
 *
 *	Tells the Tk dispatcher to call the graph display routine at
 *	the next idle point.  This request is made only if the window
 *	is displayed and no other redraw request is pending.
 *
 * Results: None.
 *
 * Side effects:
 *	The window is eventually redisplayed.
 *
 *---------------------------------------------------------------------------
 */
static void
EventuallyRedrawLegend(Legend *legendPtr) 
{
    if ((legendPtr->tkwin != NULL) && !(legendPtr->flags & REDRAW_PENDING)) {
	Tcl_DoWhenIdle(DisplayLegend, legendPtr);
	legendPtr->flags |= REDRAW_PENDING;
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * LegendEventProc --
 *
 *	This procedure is invoked by the Tk dispatcher for various
 *	events on graphs.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	When the window gets deleted, internal structures get
 *	cleaned up.  When it gets exposed, the graph is eventually
 *	redisplayed.
 *
 *---------------------------------------------------------------------------
 */
static void
LegendEventProc(
    ClientData clientData,	/* Legend record */
    XEvent *eventPtr)		/* Event which triggered call to routine */
{
    Legend *legendPtr = clientData;

    if (eventPtr->type == Expose) {
	if (eventPtr->xexpose.count == 0) {
	    EventuallyRedrawLegend(legendPtr);
	}
    } else if (eventPtr->type == DestroyNotify) {
	Graph *graphPtr = legendPtr->graphPtr;

	if (legendPtr->tkwin != graphPtr->tkwin) {
	    Blt_DeleteWindowInstanceData(legendPtr->tkwin);
	    if (legendPtr->cmdToken != NULL) {
		Tcl_DeleteCommandFromToken(graphPtr->interp, 
					   legendPtr->cmdToken);
		legendPtr->cmdToken = NULL;
	    }
	    legendPtr->tkwin = graphPtr->tkwin;
	}
	if (legendPtr->flags & REDRAW_PENDING) {
	    Tcl_CancelIdleCall(DisplayLegend, legendPtr);
	    legendPtr->flags &= ~REDRAW_PENDING;
	}
	legendPtr->site = LEGEND_RIGHT;
	graphPtr->flags |= (MAP_WORLD | REDRAW_WORLD);
	Blt_MoveBindingTable(legendPtr->bindTable, graphPtr->tkwin);
	Blt_EventuallyRedrawGraph(graphPtr);
    } else if (eventPtr->type == ConfigureNotify) {
	EventuallyRedrawLegend(legendPtr);
    }
}

static int
CreateLegendWindow(
    Tcl_Interp *interp,
    Legend *legendPtr,
    char *pathName)
{
    Tk_Window tkwin;

    tkwin = Tk_CreateWindowFromPath(interp, legendPtr->graphPtr->tkwin, 
	pathName, NULL);
    if (tkwin == NULL) {
	return TCL_ERROR;
    }
    Blt_SetWindowInstanceData(tkwin, legendPtr);
    Tk_CreateEventHandler(tkwin, ExposureMask | StructureNotifyMask,
	  LegendEventProc, legendPtr);
    /* Move the legend's binding table to the new window. */
    Blt_MoveBindingTable(legendPtr->bindTable, tkwin);
    if (legendPtr->tkwin != legendPtr->graphPtr->tkwin) {
	Tk_DestroyWindow(legendPtr->tkwin);
    }
    legendPtr->cmdToken = Tcl_CreateObjCommand(interp, pathName, 
	Blt_GraphInstCmdProc, legendPtr->graphPtr, NULL);
    legendPtr->tkwin = tkwin;
    legendPtr->site = LEGEND_WINDOW;
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ObjToPosition --
 *
 *	Convert the string representation of a legend XY position into
 *	window coordinates.  The form of the string must be "@x,y" or
 *	none.
 *
 * Results:
 *	The return value is a standard Tcl result.  The symbol type is
 *	written into the widget record.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToPosition(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,		/* Interpreter to send results back to */
    Tk_Window tkwin,		/* Not used. */
    Tcl_Obj *objPtr,		/* New legend position string */
    char *widgRec,		/* Widget record */
    int offset,			/* Not used. */
    int flags)			/* Not used. */
{
    Legend *legendPtr = (Legend *)widgRec;
    char c;
    size_t length;
    char *string;

    string = Tcl_GetString(objPtr);
    c = string[0];
    length = strlen(string);
    if (c == '\0') {
	legendPtr->site = LEGEND_RIGHT;
    } else if ((c == 'l') && (strncmp(string, "leftmargin", length) == 0)) {
	legendPtr->site = LEGEND_LEFT;
    } else if ((c == 'r') && (strncmp(string, "rightmargin", length) == 0)) {
	legendPtr->site = LEGEND_RIGHT;
    } else if ((c == 't') && (strncmp(string, "topmargin", length) == 0)) {
	legendPtr->site = LEGEND_TOP;
    } else if ((c == 'b') && (strncmp(string, "bottommargin", length) == 0)) {
	legendPtr->site = LEGEND_BOTTOM;
    } else if ((c == 'p') && (strncmp(string, "plotarea", length) == 0)) {
	legendPtr->site = LEGEND_PLOT;
    } else if (c == '@') {
	char *comma;
	long x, y;
	int result;
	
	comma = strchr(string + 1, ',');
	if (comma == NULL) {
	    Tcl_AppendResult(interp, "bad screen position \"", string,
			     "\": should be @x,y", (char *)NULL);
	    return TCL_ERROR;
	}
	x = y = 0;
	*comma = '\0';
	result = ((Tcl_ExprLong(interp, string + 1, &x) == TCL_OK) &&
		  (Tcl_ExprLong(interp, comma + 1, &y) == TCL_OK));
	*comma = ',';
	if (!result) {
	    return TCL_ERROR;
	}
	legendPtr->screenPt.x = (int)x;
	legendPtr->screenPt.y = (int)y;
	legendPtr->site = LEGEND_XY;
    } else if (c == '.') {
	if (legendPtr->tkwin != legendPtr->graphPtr->tkwin) {
	    Tk_DestroyWindow(legendPtr->tkwin);
	    legendPtr->tkwin = legendPtr->graphPtr->tkwin;
	}
	if (CreateLegendWindow(interp, legendPtr, string) != TCL_OK) {
	    return TCL_ERROR;
	}
	legendPtr->site = LEGEND_WINDOW;
    } else {
	Tcl_AppendResult(interp, "bad position \"", string, "\": should be  \
\"leftmargin\", \"rightmargin\", \"topmargin\", \"bottommargin\", \
\"plotarea\", windowName or @x,y", (char *)NULL);
	return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * PositionToObj --
 *
 *	Convert the window coordinates into a string.
 *
 * Results:
 *	The string representing the coordinate position is returned.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
PositionToObj(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,		/* Not used. */
    Tk_Window tkwin,		/* Not used. */
    char *widgRec,		/* Widget record */
    int offset,			/* Not used. */
    int flags)			/* Not used. */
{
    Legend *legendPtr = (Legend *)widgRec;
    Tcl_Obj *objPtr;

    switch (legendPtr->site) {
    case LEGEND_LEFT:
	objPtr = Tcl_NewStringObj("leftmargin", -1);
	break;

    case LEGEND_RIGHT:
	objPtr = Tcl_NewStringObj("rightmargin", -1);
	break;

    case LEGEND_TOP:
	objPtr = Tcl_NewStringObj("topmargin", -1);
	break;

    case LEGEND_BOTTOM:
	objPtr = Tcl_NewStringObj("bottommargin", -1);
	break;

    case LEGEND_PLOT:
	objPtr = Tcl_NewStringObj("plotarea", -1);
	break;

    case LEGEND_WINDOW:
	objPtr = Tcl_NewStringObj(Tk_PathName(legendPtr->tkwin), -1);
	break;

    case LEGEND_XY:
	{
	    char string[200];

	    sprintf_s(string, 200, "@%d,%d", (int)legendPtr->screenPt.x, 
		    (int)legendPtr->screenPt.y);
	    objPtr = Tcl_NewStringObj(string, -1);
	}
    default:
	objPtr = Tcl_NewStringObj("unknown legend position", -1);
    }
    return objPtr;
}

static void
SetLegendOrigin(Legend *legendPtr)
{
    Graph *graphPtr;
    int x, y, width, height;

    graphPtr = legendPtr->graphPtr;
    x = y = width = height = 0;		/* Suppress compiler warning. */
    switch (legendPtr->site) {
    case LEGEND_RIGHT:
	width = graphPtr->rightMargin.width - graphPtr->rightMargin.axesOffset;
	height = graphPtr->bottom - graphPtr->top;
	x = graphPtr->width - (width + graphPtr->inset);
	y = graphPtr->top;
	break;
    case LEGEND_LEFT:
	width = graphPtr->leftMargin.width - graphPtr->leftMargin.axesOffset;
	height = graphPtr->bottom - graphPtr->top;
	x = graphPtr->inset;
	y = graphPtr->top;
	break;
    case LEGEND_TOP:
	width = graphPtr->right - graphPtr->left;
	height = graphPtr->topMargin.height - graphPtr->topMargin.axesOffset;
	if (graphPtr->title != NULL) {
	    height -= graphPtr->titleHeight;
	}
	x = graphPtr->left;
	y = graphPtr->inset;
	if (graphPtr->title != NULL) {
	    y += graphPtr->titleHeight;
	}
	break;
    case LEGEND_BOTTOM:
	width = graphPtr->right - graphPtr->left;
	height = graphPtr->bottomMargin.height - 
	    graphPtr->bottomMargin.axesOffset;
	x = graphPtr->left;
	y = graphPtr->height - (height + graphPtr->inset);
	break;
    case LEGEND_PLOT:
	width = graphPtr->right - graphPtr->left;
	height = graphPtr->bottom - graphPtr->top;
	x = graphPtr->left;
	y = graphPtr->top;
	break;
    case LEGEND_XY:
	width = legendPtr->width;
	height = legendPtr->height;
	x = (int)legendPtr->screenPt.x;
	y = (int)legendPtr->screenPt.y;
	if (x < 0) {
	    x += graphPtr->width;
	}
	if (y < 0) {
	    y += graphPtr->height;
	}
	break;
    case LEGEND_WINDOW:
	legendPtr->anchor = TK_ANCHOR_NW;
	legendPtr->x = legendPtr->y = 0;
	return;
    }
    width = legendPtr->width - width;
    height = legendPtr->height - height;
    Blt_TranslateAnchor(x, y, width, height, legendPtr->anchor, &x, &y);

    legendPtr->x = x + legendPtr->padLeft;
    legendPtr->y = y + legendPtr->padTop;
}


/*ARGSUSED*/
static ClientData
PickLegendEntry(
    ClientData clientData,
    int x, int y,		/* Point to be tested */
    ClientData *contextPtr)	/* Not used. */
{
    Graph *graphPtr = clientData;
    Legend *legendPtr;
    int width, height;

    legendPtr = graphPtr->legend;
    width = legendPtr->width;
    height = legendPtr->height;

    x -= legendPtr->x + legendPtr->borderWidth;
    y -= legendPtr->y + legendPtr->borderWidth;
    width -= 2 * legendPtr->borderWidth + PADDING(legendPtr->padX);
    height -= 2 * legendPtr->borderWidth + PADDING(legendPtr->padY);

    if ((x >= 0) && (x < width) && (y >= 0) && (y < height)) {
	int row, column;
	int n;

	/*
	 * It's in the bounding box, so compute the index.
	 */
	row = y / legendPtr->entryHeight;
	column = x / legendPtr->entryWidth;
	n = (column * legendPtr->nRows) + row;
	if (n < legendPtr->nEntries) {
	    Blt_ChainLink link;
	    Element *elemPtr;
	    int count;

	    /* Legend entries are stored in reverse. */
	    count = 0;
	    for (link = Blt_ChainLastLink(graphPtr->elements.displayList);
		 link != NULL; link = Blt_ChainPrevLink(link)) {
		elemPtr = Blt_ChainGetValue(link);
		if (elemPtr->label != NULL) {
		    if (count == n) {
			return elemPtr;
		    }
		    count++;
		}
	    }	      
	    if (link != NULL) {
		return Blt_ChainGetValue(link);
	    }	
	}
    }
    return NULL;
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_MapLegend --
 *
 * 	Calculates the dimensions (width and height) needed for
 *	the legend.  Also determines the number of rows and columns
 *	necessary to list all the valid element labels.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *   	The following fields of the legend are calculated and set.
 *
 * 	nEntries   - number of valid labels of elements in the
 *		      display list.
 * 	nRows	    - number of rows of entries
 * 	nColumns    - number of columns of entries
 * 	entryHeight - height of each entry
 * 	entryWidth  - width of each entry
 * 	height	    - width of legend (includes borders and padding)
 * 	width	    - height of legend (includes borders and padding)
 *
 *---------------------------------------------------------------------------
 */
void
Blt_MapLegend(
    Legend *legendPtr,
    int plotWidth,		/* Maximum width available in window
				 * to draw the legend. Will calculate number
				 * of columns from this. */
    int plotHeight)		/* Maximum height available in window
				 * to draw the legend. Will calculate number
				 * of rows from this. */
{
    Blt_ChainLink link;
    Element *elemPtr;
    int nRows, nColumns, nEntries;
    int legendWidth, legendHeight;
    int entryWidth, entryHeight;
    int symbolWidth;
    Blt_FontMetrics fontMetrics;

    /* Initialize legend values to default (no legend displayed) */

    legendPtr->entryWidth = legendPtr->entryHeight = 0;
    legendPtr->nRows = legendPtr->nColumns = 
    legendPtr->nEntries = 0;
    legendPtr->height = legendPtr->width = 0;

    if (legendPtr->site == LEGEND_WINDOW) {
	if (Tk_Width(legendPtr->tkwin) > 1) {
	    plotWidth = Tk_Width(legendPtr->tkwin);
	}
	if (Tk_Height(legendPtr->tkwin) > 1) {
	    plotHeight = Tk_Height(legendPtr->tkwin);
	}
    }
    if ((legendPtr->hidden) || (plotWidth < 1) || (plotHeight < 1)) {
	return;			/* Legend is not being displayed */
    }

    /*   
     * Count the number of legend entries and determine the widest and
     * tallest label.  The number of entries would normally be the
     * number of elements, but 1) elements can be hidden and 2)
     * elements can have no legend entry (-label "").  
     */
    nEntries = 0;
    entryWidth = entryHeight = 0;
    for (link = Blt_ChainLastLink(legendPtr->graphPtr->elements.displayList);
	link != NULL; link = Blt_ChainPrevLink(link)) {
	int width, height;
	elemPtr = Blt_ChainGetValue(link);
	if (elemPtr->label == NULL) {
	    continue;		/* Element has no legend entry. */
	}
	Blt_Ts_GetExtents(&legendPtr->style, elemPtr->label, &width, &height);
	if (entryWidth < width) {
	    entryWidth = width;
	}
	if (entryHeight < height) {
	    entryHeight = height;
	}
	nEntries++;
    }

    if (nEntries == 0) {
	return;			/* No legend entries. */
    }


    Blt_GetFontMetrics(legendPtr->style.font, &fontMetrics);
    symbolWidth = 2 * fontMetrics.ascent;

    entryWidth += 2 * legendPtr->entryBorderWidth + PADDING(legendPtr->ipadX) +
	5 + symbolWidth;
    entryHeight += 2 * legendPtr->entryBorderWidth + PADDING(legendPtr->ipadY);

    legendWidth = plotWidth - 2 * legendPtr->borderWidth - 
	PADDING(legendPtr->padX);
    legendHeight = plotHeight - 2 * legendPtr->borderWidth - 
	PADDING(legendPtr->padY);

    /*
     * The number of rows and columns is computed as one of the following:
     *
     *	both options set		User defined. 
     *  -rows				Compute columns from rows.
     *  -columns			Compute rows from columns.
     *	neither set			Compute rows and columns from
     *					size of plot.  
     */
    if (legendPtr->reqRows > 0) {
	nRows = legendPtr->reqRows; 
	if (nRows > nEntries) {
	    nRows = nEntries;	
	}
	if (legendPtr->reqColumns > 0) {
	    nColumns = legendPtr->reqColumns;
	    if (nColumns > nEntries) {
		nColumns = nEntries; /* Both -rows, -columns set. */
	    }
	} else {
	    nColumns = ((nEntries - 1) / nRows) + 1; /* Only -rows. */
	}
    } else if (legendPtr->reqColumns > 0) { /* Only -columns. */
	nColumns = legendPtr->reqColumns;
	if (nColumns > nEntries) {
	    nColumns = nEntries;
	}
	nRows = ((nEntries - 1) / nColumns) + 1;
    } else {			
	/* Compute # of rows and columns from the legend size. */
	nRows = legendHeight / entryHeight;
	nColumns = legendWidth / entryWidth;
	
	if (nRows > nEntries) {
	    nRows = nEntries;
	} else if (nRows < 1) {
	    nRows = 1;
	} 
	if (nColumns > nEntries) {
	    nColumns = nEntries;
	} else if (nColumns < 1) {
	    nColumns = 1;
	}
	if ((legendPtr->site == LEGEND_TOP) || 
	    (legendPtr->site == LEGEND_BOTTOM)) {
	    nRows = ((nEntries - 1) / nColumns) + 1;
	} else {
	    nColumns = ((nEntries - 1) / nRows) + 1;
	}
    }
    if (nRows < 1) {
	nRows = 1;
    }
    if (nColumns < 1) {
	nColumns = 1;
    }
    legendWidth = 2 * legendPtr->borderWidth + PADDING(legendPtr->padX);
    legendHeight = 2 * legendPtr->borderWidth + PADDING(legendPtr->padY);
    legendHeight += nRows * entryHeight;
    legendWidth += nColumns * entryWidth;

    legendPtr->height = legendHeight;
    legendPtr->width = legendWidth;
    legendPtr->nRows = nRows;
    legendPtr->nColumns = nColumns;
    legendPtr->nEntries = nEntries;
    legendPtr->entryHeight = entryHeight;
    legendPtr->entryWidth = entryWidth;

    if ((legendPtr->tkwin != legendPtr->graphPtr->tkwin) &&
	((Tk_ReqWidth(legendPtr->tkwin) != legendWidth) ||
	 (Tk_ReqHeight(legendPtr->tkwin) != legendHeight))) {
	Tk_GeometryRequest(legendPtr->tkwin, legendWidth, legendHeight);
    }
}

void
Blt_DrawLegend(
    Legend *legendPtr,
    Drawable drawable)		/* Pixmap or window to draw into */
{
    Graph *graphPtr;
    Blt_ChainLink link;
    Pixmap pixmap;
    Blt_Background bg;
    Blt_FontMetrics fontMetrics;
    Tk_Window tkwin;
    int count;
    int labelX, startY, symbolX, symbolY;
    int symbolSize, midX, midY;
    int width, height;
    int x, y;

    graphPtr = legendPtr->graphPtr;
    graphPtr->flags &= ~DRAW_LEGEND;
    if ((legendPtr->hidden) || (legendPtr->nEntries == 0)) {
	return;
    }
    SetLegendOrigin(legendPtr);

    if (legendPtr->tkwin != graphPtr->tkwin) {
	tkwin = legendPtr->tkwin;
	width = Tk_Width(tkwin);
	if (width < 1) {
	    width = legendPtr->width;
	}
	height = Tk_Height(tkwin);
	if (height < 1) {
	    height = legendPtr->height;
	}
    } else {
	width = legendPtr->width;
	height = legendPtr->height;
    }
    Blt_GetFontMetrics(legendPtr->style.font, &fontMetrics);

    symbolSize = fontMetrics.ascent;
    midX = symbolSize + 1 + legendPtr->entryBorderWidth;
    midY = (symbolSize / 2) + 1 + legendPtr->entryBorderWidth;
    labelX = 2 * symbolSize + legendPtr->entryBorderWidth + 
	legendPtr->ipadX.side1 + 5;
    symbolY = midY + legendPtr->ipadY.side1;
    symbolX = midX + legendPtr->ipadX.side1;

    pixmap = Tk_GetPixmap(graphPtr->display, Tk_WindowId(legendPtr->tkwin), 
	width, height, Tk_Depth(legendPtr->tkwin));
    if (legendPtr->normalBg != NULL) {
	Blt_FillBackgroundRectangle(legendPtr->tkwin, pixmap, 
		legendPtr->normalBg, 0, 0, width, height, 0, TK_RELIEF_FLAT);
    } else if (legendPtr->site & LEGEND_IN_PLOT) {
	/* 
	 * Legend background is transparent and is positioned over the
	 * the plot area.  Either copy the part of the background from
	 * the backing store pixmap or (if no backing store exists)
	 * just fill it with the background color of the plot.
	 */
	if (graphPtr->backPixmap != None) {
	    XCopyArea(graphPtr->display, graphPtr->backPixmap, pixmap, 
		graphPtr->drawGC, legendPtr->x, legendPtr->y, width, height, 
		0, 0);
        } else {
	    Blt_FillBackgroundRectangle(graphPtr->tkwin, pixmap, 
		graphPtr->plotBg, 0, 0, width, height, TK_RELIEF_FLAT, 0);
 	}
    } else {
	/* 
	 * The legend is located in one of the margins or the external
	 * window.
	 */
	Blt_SetBackgroundOrigin(graphPtr->normalBg, legendPtr->x, legendPtr->y);
	Blt_FillBackgroundRectangle(legendPtr->tkwin, pixmap, 
		graphPtr->normalBg, 0, 0, width, height, 0, TK_RELIEF_FLAT);
	Blt_SetBackgroundOrigin(graphPtr->normalBg, 0, 0);
    }
    x = legendPtr->padLeft + legendPtr->borderWidth;
    y = legendPtr->padTop + legendPtr->borderWidth;
    count = 0;
    startY = y;
    for (link = Blt_ChainLastLink(graphPtr->elements.displayList);
	link != NULL; link = Blt_ChainPrevLink(link)) {
	Element *elemPtr;

	elemPtr = Blt_ChainGetValue(link);
	if (elemPtr->label == NULL) {
	    continue;		/* Skip this entry */
	}
	if (elemPtr->flags & LABEL_ACTIVE) {
	    Blt_SetBackgroundOrigin(legendPtr->activeBg, legendPtr->x, 
		legendPtr->y);
	    Blt_Ts_SetForeground(legendPtr->style, legendPtr->activeFgColor);
	    Blt_FillBackgroundRectangle(legendPtr->tkwin, pixmap, 
		legendPtr->activeBg, x, y, 
		legendPtr->entryWidth, legendPtr->entryHeight, 
		legendPtr->entryBorderWidth, legendPtr->activeRelief);
	    Blt_SetBackgroundOrigin(legendPtr->activeBg, 0, 0);
	} else {
	    Blt_Ts_SetForeground(legendPtr->style, legendPtr->fgColor);
	    if (elemPtr->labelRelief != TK_RELIEF_FLAT) {
		Blt_FillBackgroundRectangle(legendPtr->tkwin, pixmap, 
			graphPtr->normalBg, x, y, legendPtr->entryWidth, 
			legendPtr->entryHeight, legendPtr->entryBorderWidth, 
			elemPtr->labelRelief);
	    }
	}
	(*elemPtr->procsPtr->drawSymbolProc) (graphPtr, pixmap, elemPtr,
		x + symbolX, y + symbolY, symbolSize);
	Blt_DrawText(legendPtr->tkwin, pixmap, elemPtr->label, 
		&legendPtr->style, x + labelX, 
		y + legendPtr->entryBorderWidth + legendPtr->ipadY.side1);
	count++;

	/* Check when to move to the next column */
	if ((count % legendPtr->nRows) > 0) {
	    y += legendPtr->entryHeight;
	} else {
	    x += legendPtr->entryWidth;
	    y = startY;
	}
    }
    /*
     * Draw the border and/or background of the legend.
     */
    bg = legendPtr->normalBg;
    if (bg == NULL) {
	bg = graphPtr->normalBg;
    }
    Blt_DrawBackgroundRectangle(legendPtr->tkwin, pixmap, bg, 0, 0, width, 
	height, legendPtr->borderWidth, legendPtr->relief);

    XCopyArea(graphPtr->display, pixmap, drawable, graphPtr->drawGC, 0, 0, 
	width, height, legendPtr->x, legendPtr->y);
    Tk_FreePixmap(graphPtr->display, pixmap);
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_LegendToPostScript --
 *
 *---------------------------------------------------------------------------
 */
void
Blt_LegendToPostScript(Legend *legendPtr, Blt_Ps ps)
{
    Graph *graphPtr;
    double x, y, startY;
    Element *elemPtr;
    int labelX, symbolX, symbolY;
    int count;
    Blt_ChainLink link;
    int symbolSize, midX, midY;
    int width, height;
    Blt_FontMetrics fontMetrics;

    if ((legendPtr->hidden) || (legendPtr->nEntries == 0)) {
	return;
    }
    SetLegendOrigin(legendPtr);

    x = legendPtr->x, y = legendPtr->y;
    width = legendPtr->width - PADDING(legendPtr->padX);
    height = legendPtr->height - PADDING(legendPtr->padY);

    Blt_Ps_Append(ps, "% Legend\n");
    graphPtr = legendPtr->graphPtr;
    if (graphPtr->pageSetup->flags & PS_DECORATIONS) {
	if (legendPtr->normalBg != NULL) {
	    Tk_3DBorder border;

	    border = Blt_BackgroundBorder(legendPtr->normalBg);
	    Blt_Ps_Fill3DRectangle(ps, border, x, y, width, height, 
		legendPtr->borderWidth, legendPtr->relief);
	} else {
	    Tk_3DBorder border;

	    border = Blt_BackgroundBorder(graphPtr->normalBg);
	    Blt_Ps_Draw3DRectangle(ps, border, x, y, width, height, 
		legendPtr->borderWidth, legendPtr->relief);
	}
    } else {
	Blt_Ps_SetClearBackground(ps);
	Blt_Ps_XFillRectangle(ps, x, y, width, height);
    }
    x += legendPtr->borderWidth;
    y += legendPtr->borderWidth;

    Blt_GetFontMetrics(legendPtr->style.font, &fontMetrics);
    symbolSize = fontMetrics.ascent;
    midX = symbolSize + 1 + legendPtr->entryBorderWidth;
    midY = (symbolSize / 2) + 1 + legendPtr->entryBorderWidth;
    labelX = 2 * symbolSize + legendPtr->entryBorderWidth + 
	legendPtr->ipadX.side1 + 5;
    symbolY = midY + legendPtr->ipadY.side1;
    symbolX = midX + legendPtr->ipadX.side1;

    count = 0;
    startY = y;
    for (link = Blt_ChainLastLink(graphPtr->elements.displayList);
	link != NULL; link = Blt_ChainPrevLink(link)) {
	elemPtr = Blt_ChainGetValue(link);
	if (elemPtr->label == NULL) {
	    continue;		/* Skip this label */
	}
	if (elemPtr->flags & LABEL_ACTIVE) {
	    Tk_3DBorder border;
	    
	    border = Blt_BackgroundBorder(legendPtr->activeBg);
	    Blt_Ts_SetForeground(legendPtr->style, legendPtr->activeFgColor);
	    Blt_Ps_Fill3DRectangle(ps, border, x, y, legendPtr->entryWidth, 
		legendPtr->entryHeight, legendPtr->entryBorderWidth, 
		legendPtr->activeRelief);
	} else {
	    Blt_Ts_SetForeground(legendPtr->style, legendPtr->fgColor);
	    if (elemPtr->labelRelief != TK_RELIEF_FLAT) {
		Tk_3DBorder border;

		border = Blt_BackgroundBorder(graphPtr->normalBg);
		Blt_Ps_Draw3DRectangle(ps, border, x, y, legendPtr->entryWidth,
			legendPtr->entryHeight, legendPtr->entryBorderWidth, 
			elemPtr->labelRelief);
	    }
	}
	(*elemPtr->procsPtr->printSymbolProc) (graphPtr, ps, elemPtr, 
		x + symbolX, y + symbolY, symbolSize);
	Blt_Ps_DrawText(ps, elemPtr->label, &(legendPtr->style), 
		x + labelX, 
		y + legendPtr->entryBorderWidth + legendPtr->ipadY.side1);
	count++;
	if ((count % legendPtr->nRows) > 0) {
	    y += legendPtr->entryHeight;
	} else {
	    x += legendPtr->entryWidth;
	    y = startY;
	}
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * DisplayLegend --
 *
 *---------------------------------------------------------------------------
 */
static void
DisplayLegend(ClientData clientData)
{
    Legend *legendPtr = clientData;
    int width, height;

    legendPtr->flags &= ~REDRAW_PENDING;

    if (legendPtr->tkwin == NULL) {
	return;			/* Window has been destroyed. */
    }
    if (legendPtr->site == LEGEND_WINDOW) {
	width = Tk_Width(legendPtr->tkwin);
	height = Tk_Height(legendPtr->tkwin);
	if ((width <= 1) || (height <= 1)) {
	    return;
	}
	if ((width != legendPtr->width) || (height != legendPtr->height)) {
	    Blt_MapLegend(legendPtr, width, height);
	}
    }
    if (!Tk_IsMapped(legendPtr->tkwin)) {
	return;
    }
    Blt_DrawLegend(legendPtr, Tk_WindowId(legendPtr->tkwin));
}

/*
 *---------------------------------------------------------------------------
 *
 * ConfigureLegend --
 *
 * 	Routine to configure the legend.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side Effects:
 *	Graph will be redrawn to reflect the new legend attributes.
 *
 *---------------------------------------------------------------------------
 */
static void
ConfigureLegend(Graph *graphPtr, Legend *legendPtr)
{
    if (legendPtr->site == LEGEND_WINDOW) {
	EventuallyRedrawLegend(legendPtr);
    } else {
	/*
	 *  Update the layout of the graph (and redraw the elements) if
	 *  any of the following legend options (all of which affect the
	 *	size of the legend) have changed.
	 *
	 *		-activeborderwidth, -borderwidth
	 *		-border
	 *		-font
	 *		-hide
	 *		-ipadx, -ipady, -padx, -pady
	 *		-rows
	 *
	 *  If the position of the legend changed to/from the default
	 *  position, also indicate that a new layout is needed.
	 *
	 */
	if (Blt_ConfigModified(configSpecs, "-*border*", "-*pad?",
		"-position", "-hide", "-font", "-rows", (char *)NULL)) {
	    graphPtr->flags |= MAP_WORLD;
	}
	graphPtr->flags |= (REDRAW_WORLD | REDRAW_BACKING_STORE);
	Blt_EventuallyRedrawGraph(graphPtr);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_DestroyLegend --
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Resources associated with the legend are freed.
 *
 *---------------------------------------------------------------------------
 */
void
Blt_DestroyLegend(Graph *graphPtr)
{
    if (graphPtr->legend != NULL) {
	Legend *legendPtr = graphPtr->legend;

	Blt_FreeOptions(configSpecs, (char *)legendPtr, graphPtr->display, 0);
	Blt_Ts_FreeStyle(graphPtr->display, &legendPtr->style);
	Blt_DestroyBindingTable(legendPtr->bindTable);
	if (legendPtr->tkwin != graphPtr->tkwin) {
	    Tk_Window tkwin;
	    
	    /* The graph may be in the process of being torn down */
	    if (legendPtr->cmdToken != NULL) {
		Tcl_DeleteCommandFromToken(graphPtr->interp, 
					   legendPtr->cmdToken);
	    }
	    if (legendPtr->flags & REDRAW_PENDING) {
		Tcl_CancelIdleCall(DisplayLegend, legendPtr);
		legendPtr->flags &= ~REDRAW_PENDING;
	    }
	    tkwin = legendPtr->tkwin;
	    legendPtr->tkwin = NULL;
	    if (tkwin != NULL) {
		Tk_DeleteEventHandler(tkwin, ExposureMask | StructureNotifyMask,
			LegendEventProc, legendPtr);
		Blt_DeleteWindowInstanceData(tkwin);
		Tk_DestroyWindow(tkwin);
	    }
	}
	Blt_Free(legendPtr);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_CreateLegend --
 *
 * 	Creates and initializes a legend structure with default settings
 *
 * Results:
 *	A standard Tcl result.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
int
Blt_CreateLegend(Graph *graphPtr)
{
    Legend *legendPtr;

    legendPtr = Blt_CallocAssert(1, sizeof(Legend));
    graphPtr->legend = legendPtr;
    legendPtr->graphPtr = graphPtr;
    legendPtr->tkwin = graphPtr->tkwin;
    legendPtr->hidden = FALSE;
    legendPtr->screenPt.x = legendPtr->screenPt.y = -SHRT_MAX;
    legendPtr->relief = TK_RELIEF_SUNKEN;
    legendPtr->activeRelief = TK_RELIEF_FLAT;
    legendPtr->entryBorderWidth = legendPtr->borderWidth = 2;
    legendPtr->ipadX.side1 = legendPtr->ipadX.side2 = 1;
    legendPtr->ipadY.side1 = legendPtr->ipadY.side2 = 1;
    legendPtr->padX.side1 = legendPtr->padX.side2 = 1;
    legendPtr->padY.side1 = legendPtr->padY.side2 = 1;
    legendPtr->anchor = TK_ANCHOR_N;
    legendPtr->site = LEGEND_RIGHT;
    Blt_Ts_InitStyle(legendPtr->style);
    legendPtr->style.justify = TK_JUSTIFY_LEFT;
    legendPtr->style.anchor = TK_ANCHOR_NW;
    legendPtr->bindTable = Blt_CreateBindingTable(graphPtr->interp,
	graphPtr->tkwin, graphPtr, PickLegendEntry, Blt_GraphTags);

    if (Blt_ConfigureComponentFromObj(graphPtr->interp, graphPtr->tkwin,
	    "legend", "Legend", configSpecs, 0, (Tcl_Obj **)NULL,
	    (char *)legendPtr, 0) != TCL_OK) {
	return TCL_ERROR;
    }
    ConfigureLegend(graphPtr, legendPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * GetOp --
 *
 * 	Find the legend entry from the given argument.  The argument
 *	can be either a screen position "@x,y" or the name of an
 *	element.
 *
 *	I don't know how useful it is to test with the name of an
 *	element.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side Effects:
 *	Graph will be redrawn to reflect the new legend attributes.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
GetOp(
    Graph *graphPtr,
    Tcl_Interp *interp,
    int objc,			/* Not used. */
    Tcl_Obj *const *objv)
{
    Element *elemPtr;
    Legend *legendPtr = graphPtr->legend;
    int x, y;
    char *string;
    char c;

    if ((legendPtr->hidden) || (legendPtr->nEntries == 0)) {
	return TCL_OK;
    }
    elemPtr = NULL;
    string = Tcl_GetString(objv[3]);
    c = string[0];
    if ((c == 'c') && (strcmp(string, "current") == 0)) {
	elemPtr = (Element *)Blt_GetCurrentItem(legendPtr->bindTable);
    } else if ((c == '@') &&
       (Blt_GetXY(interp, graphPtr->tkwin, string, &x, &y) == TCL_OK)) { 
	elemPtr = (Element *)PickLegendEntry(graphPtr, x, y, NULL);
    }
    if (elemPtr != NULL) {
	Tcl_SetStringObj(Tcl_GetObjResult(interp), elemPtr->object.name, -1);
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ActivateOp --
 *
 * 	Activates a particular label in the legend.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side Effects:
 *	Graph will be redrawn to reflect the new legend attributes.
 *
 *---------------------------------------------------------------------------
 */
static int
ActivateOp(
    Graph *graphPtr,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    Blt_HashEntry *hp;
    Blt_HashSearch cursor;
    Legend *legendPtr = graphPtr->legend;
    unsigned int active, redraw;
    char *string;

    string = Tcl_GetString(objv[2]);
    active = (string[0] == 'a') ? LABEL_ACTIVE : 0;
    redraw = 0;
    for (hp = Blt_FirstHashEntry(&graphPtr->elements.table, &cursor); 
	 hp != NULL; hp = Blt_NextHashEntry(&cursor)) {
	Element *elemPtr;
	int i;

	elemPtr = Blt_GetHashValue(hp);
	for (i = 3; i < objc; i++) {
	    if (Tcl_StringMatch(elemPtr->object.name, Tcl_GetString(objv[i]))) {
		break;
	    }
	}
	if ((i < objc) && (active != (elemPtr->flags & LABEL_ACTIVE))) {
	    elemPtr->flags ^= LABEL_ACTIVE;
	    if (elemPtr->label != NULL) {
		redraw++;
	    }
	}
    }
    if ((redraw) && (!legendPtr->hidden)) {
	/*
	 * See if how much we need to draw. If the graph is already
	 * scheduled for a redraw, just make sure the right flags are
	 * set.  Otherwise redraw only the legend: it's either in an
	 * external window or it's the only thing that need updating.
	 */
	if (graphPtr->flags & REDRAW_PENDING) {
	    if (legendPtr->site & LEGEND_IN_PLOT) {
		graphPtr->flags |= REDRAW_BACKING_STORE;
	    }
	    graphPtr->flags |= REDRAW_WORLD; /* Redraw entire graph. */
	} else {
	    EventuallyRedrawLegend(legendPtr);
	}
    }
    {
	Tcl_Obj *listObjPtr;
	
	/* Return the names of all the active legend entries */
	listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
	for (hp = Blt_FirstHashEntry(&(graphPtr->elements.table), &cursor);
	     hp != NULL; hp = Blt_NextHashEntry(&cursor)) {
	    Element *elemPtr;
	    
	    elemPtr = Blt_GetHashValue(hp);
	    if (elemPtr->flags & LABEL_ACTIVE) {
		Tcl_ListObjAppendElement(interp, listObjPtr, 
			Tcl_NewStringObj(elemPtr->object.name, -1));
	    }
	}
	Tcl_SetObjResult(interp, listObjPtr);
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
    Graph *graphPtr,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    if (objc == 3) {
	Blt_HashEntry *hp;
	Blt_HashSearch cursor;
	Tcl_Obj *listObjPtr;

	listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
	for (hp = Blt_FirstHashEntry(&(graphPtr->elements.tagTable), &cursor);
	    hp != NULL; hp = Blt_NextHashEntry(&cursor)) {
	    char *tagName;
	    Tcl_Obj *objPtr;

	    tagName = Blt_GetHashKey(&(graphPtr->elements.tagTable), hp);
	    objPtr = Tcl_NewStringObj(tagName, -1);
	    Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
	}
	Tcl_SetObjResult(interp, listObjPtr);
	return TCL_OK;
    }
    return Blt_ConfigureBindingsFromObj(interp, graphPtr->legend->bindTable,
	Blt_MakeElementTag(graphPtr, Tcl_GetString(objv[3])), objc - 4, 
	objv + 4);
}

/*
 *---------------------------------------------------------------------------
 *
 * CgetOp --
 *
 * 	Queries or resets options for the legend.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side Effects:
 *	Graph will be redrawn to reflect the new legend attributes.
 *
 *---------------------------------------------------------------------------
 */
/* ARGSUSED */
static int
CgetOp(
    Graph *graphPtr,
    Tcl_Interp *interp,
    int objc,			/* Not used. */
    Tcl_Obj *const *objv)
{
    return Blt_ConfigureValueFromObj(interp, graphPtr->tkwin, configSpecs,
	    (char *)graphPtr->legend, objv[3], 0);
}

/*
 *---------------------------------------------------------------------------
 *
 * ConfigureOp --
 *
 * 	Queries or resets options for the legend.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side Effects:
 *	Graph will be redrawn to reflect the new legend attributes.
 *
 *---------------------------------------------------------------------------
 */
static int
ConfigureOp(
    Graph *graphPtr,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    int flags = BLT_CONFIG_OBJV_ONLY;
    Legend *legendPtr;

    legendPtr = graphPtr->legend;
    if (objc == 3) {
	return Blt_ConfigureInfoFromObj(interp, graphPtr->tkwin, configSpecs,
		(char *)legendPtr, (Tcl_Obj *)NULL, flags);
    } else if (objc == 4) {
	return Blt_ConfigureInfoFromObj(interp, graphPtr->tkwin, configSpecs,
		(char *)legendPtr, objv[3], flags);
    }
    if (Blt_ConfigureWidgetFromObj(interp, graphPtr->tkwin, configSpecs, 
		objc - 3, objv + 3, (char *)legendPtr, flags) != TCL_OK) {
	return TCL_ERROR;
    }
    ConfigureLegend(graphPtr, legendPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_LegendOp --
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side Effects:
 *	Legend is possibly redrawn.
 *
 *---------------------------------------------------------------------------
 */

static Blt_OpSpec legendOps[] =
{
    {"activate", 1, ActivateOp, 3, 0, "?pattern?...",},
    {"bind", 1, BindOp, 3, 6, "elemName sequence command",},
    {"cget", 2, CgetOp, 4, 4, "option",},
    {"configure", 2, ConfigureOp, 3, 0, "?option value?...",},
    {"deactivate", 1, ActivateOp, 3, 0, "?pattern?...",},
    {"get", 1, GetOp, 4, 4, "index",},
};
static int nLegendOps = sizeof(legendOps) / sizeof(Blt_OpSpec);

int
Blt_LegendOp(
    Graph *graphPtr,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    GraphLegendProc *proc;
    int result;

    proc = Blt_GetOpFromObj(interp, nLegendOps, legendOps, BLT_OP_ARG2, 
	objc, objv,0);
    if (proc == NULL) {
	return TCL_ERROR;
    }
    result = (*proc) (graphPtr, interp, objc, objv);
    return result;
}

int 
Blt_LegendSite(Legend *legendPtr)
{
    return legendPtr->site;
}

int 
Blt_LegendWidth(Legend *legendPtr)
{
    return legendPtr->width;
}

int 
Blt_LegendHeight(Legend *legendPtr)
{
    return legendPtr->height;
}

int 
Blt_LegendIsHidden(Legend *legendPtr)
{
    return legendPtr->hidden;
}

int 
Blt_LegendIsRaised(Legend *legendPtr)
{
    return legendPtr->raised;
}

int 
Blt_LegendX(Legend *legendPtr)
{
    return legendPtr->x;
}

int 
Blt_LegendY(Legend *legendPtr)
{
    return legendPtr->y;
}

void
Blt_LegendRemoveElement(Legend *legendPtr, Element *elemPtr)
{
    Blt_DeleteBindings(legendPtr->bindTable, elemPtr);
}
