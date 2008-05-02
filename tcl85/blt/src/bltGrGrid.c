
/*
 * bltGrGrid.c --
 *
 * This module implements grid lines for the BLT graph widget.
 *
 * Graph widget created by Sani Nassif and George Howlett.
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

#ifdef notdef
#include "bltGraph.h"
#include "bltOp.h"

#define DEF_GRID_DASHES		"dot"
#define DEF_GRID_FOREGROUND	RGB_GREY64
#define DEF_GRID_LINE_WIDTH	"0"
#define DEF_GRID_HIDE_BARCHART	"no"
#define DEF_GRID_HIDE_GRAPH	"yes"
#define DEF_GRID_MINOR		"yes"
#define DEF_GRID_MAP_X_GRAPH	"x"
#define DEF_GRID_MAP_X_BARCHART	(char *)NULL
#define DEF_GRID_MAP_Y		"y"


BLT_EXTERN Blt_CustomOption bltXAxisOption;
BLT_EXTERN Blt_CustomOption bltYAxisOption;

static Blt_ConfigSpec configSpecs[] =
{
    {BLT_CONFIG_COLOR, "-color", "color", "Color", DEF_GRID_FOREGROUND, 
	Blt_Offset(Grid, colorPtr), ALL_GRAPHS},
    {BLT_CONFIG_DASHES, "-dashes", "dashes", "Dashes", DEF_GRID_DASHES, 
	Blt_Offset(Grid, dashes), BLT_CONFIG_NULL_OK | ALL_GRAPHS},
    {BLT_CONFIG_BOOLEAN, "-hide", "hide", "Hide",
	DEF_GRID_HIDE_BARCHART, Blt_Offset(Grid, hidden), BARCHART},
    {BLT_CONFIG_BOOLEAN, "-hide", "hide", "Hide",
	DEF_GRID_HIDE_GRAPH, Blt_Offset(Grid, hidden), GRAPH | STRIPCHART},
    {BLT_CONFIG_PIXELS_NNEG, "-linewidth", "lineWidth", "Linewidth",
	DEF_GRID_LINE_WIDTH, Blt_Offset(Grid, lineWidth),
	BLT_CONFIG_DONT_SET_DEFAULT | ALL_GRAPHS},
    {BLT_CONFIG_CUSTOM, "-mapx", "mapX", "MapX", DEF_GRID_MAP_X_GRAPH, 
	Blt_Offset(Grid, axes.x), GRAPH | STRIPCHART | BLT_CONFIG_NULL_OK, 
	&bltXAxisOption},
    {BLT_CONFIG_CUSTOM, "-mapx", "mapX", "MapX", DEF_GRID_MAP_X_BARCHART, 
	Blt_Offset(Grid, axes.x), BARCHART | BLT_CONFIG_NULL_OK, 
	&bltXAxisOption},
    {BLT_CONFIG_CUSTOM, "-mapy", "mapY", "MapY", DEF_GRID_MAP_Y, 
	Blt_Offset(Grid, axes.y), ALL_GRAPHS, &bltYAxisOption},
    {BLT_CONFIG_BOOLEAN, "-minor", "minor", "Minor", DEF_GRID_MINOR, 
	Blt_Offset(Grid, minorGrid), BLT_CONFIG_DONT_SET_DEFAULT | ALL_GRAPHS},
    {BLT_CONFIG_END, NULL, NULL, NULL, NULL, 0, 0}
};

/*
 *---------------------------------------------------------------------------
 *
 * ConfigureGrid --
 *
 *	Configures attributes of the grid such as line width,
 *	dashes, and position.  The grid are first turned off
 *	before any of the attributes changes.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Crosshair GC is allocated.
 *
 *---------------------------------------------------------------------------
 */
static void
ConfigureGrid(Graph *graphPtr, Grid *gridPtr)
{
    XGCValues gcValues;
    unsigned long gcMask;
    GC newGC;

    gcValues.background = gcValues.foreground = gridPtr->colorPtr->pixel;
    gcValues.line_width = LineWidth(gridPtr->lineWidth);
    gcMask = (GCForeground | GCBackground | GCLineWidth);
    if (LineIsDashed(gridPtr->dashes)) {
	gcValues.line_style = LineOnOffDash;
	gcMask |= GCLineStyle;
    }
    newGC = Blt_GetPrivateGC(graphPtr->tkwin, gcMask, &gcValues);
    if (LineIsDashed(gridPtr->dashes)) {
	Blt_SetDashes(graphPtr->display, newGC, &(gridPtr->dashes));
    }
    if (gridPtr->gc != NULL) {
	Blt_FreePrivateGC(graphPtr->display, gridPtr->gc);
    }
    gridPtr->gc = newGC;
}

#ifdef notdef
/*
 *---------------------------------------------------------------------------
 *
 * MapGrid --
 *
 *	Determines the coordinates of the line segments corresponding
 *	to the grid lines for each axis.
 *
 * Results:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
void
Blt_MapGrid(Graph *graphPtr)
{
    Grid *gridPtr = graphPtr->grid;

    if (gridPtr->hidden) {
	return;
    }
    /*
     * Generate line segments to represent the grid.  Line segments
     * are calculated from the major tick intervals of each axis mapped.
     */
    Blt_GetAxisSegments(graphPtr, gridPtr->axes.x, &gridPtr->x);
    Blt_GetAxisSegments(graphPtr, gridPtr->axes.y, &gridPtr->y);
}
#endif
/*
 *---------------------------------------------------------------------------
 *
 * DrawGrid --
 *
 *	Draws the grid lines associated with each axis.
 *
 * Results:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
void
Blt_DrawGrid(
    Graph *graphPtr,
    Drawable drawable)		/* Pixmap or window to draw into */
{
    Grid *gridPtr = graphPtr->grid;

    if (gridPtr->hidden) {
	return;
    }
    if (gridPtr->x.nUsed > 0) {
	Blt_Draw2DSegments(graphPtr->display, drawable, gridPtr->gc, 
		gridPtr->x.segments, gridPtr->x.nUsed);
    }
    if (gridPtr->y.nUsed > 0) {
	Blt_Draw2DSegments(graphPtr->display, drawable, gridPtr->gc,
		gridPtr->y.segments, gridPtr->y.nUsed);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_GridToPostScript --
 *
 *	Prints the grid lines associated with each axis.
 *
 * Results:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
void
Blt_GridToPostScript(Graph *graphPtr, Blt_Ps ps)
{
    Grid *gridPtr = graphPtr->grid;

    if ((gridPtr == NULL) || (gridPtr->hidden)) {
	return;
    }
    Blt_Ps_XSetLineAttributes(ps, gridPtr->colorPtr, gridPtr->lineWidth, 
	&gridPtr->dashes, CapButt, JoinMiter);
    if (gridPtr->x.nUsed > 0) {
	Blt_Ps_Draw2DSegments(ps, gridPtr->x.segments, gridPtr->x.nUsed);
    }
    if (gridPtr->y.nUsed > 0) {
	Blt_Ps_Draw2DSegments(ps, gridPtr->y.segments, gridPtr->y.nUsed);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_DestroyGrid --
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Grid GC is released.
 *
 *---------------------------------------------------------------------------
 */
void
Blt_DestroyGrid(Graph *graphPtr)
{
    if (graphPtr->grid != NULL) {
	Grid *gridPtr = graphPtr->grid;

	Blt_FreeOptions(configSpecs, (char *)gridPtr, graphPtr->display,
			Blt_GraphType(graphPtr));
	if (gridPtr->gc != NULL) {
	    Blt_FreePrivateGC(graphPtr->display, gridPtr->gc);
	}
	if (gridPtr->x.segments != NULL) {
	    Blt_Free(gridPtr->x.segments);
	}
	if (gridPtr->y.segments != NULL) {
	    Blt_Free(gridPtr->y.segments);
	}
	Blt_Free(gridPtr);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_CreateGrid --
 *
 *	Creates and initializes a new grid structure.
 *
 * Results:
 *	Returns TCL_ERROR if the configuration failed, otherwise TCL_OK.
 *
 * Side Effects:
 *	Memory for grid structure is allocated.
 *
 *---------------------------------------------------------------------------
 */
int
Blt_CreateGrid(Graph *graphPtr)
{
    Grid *gridPtr;

    gridPtr = Blt_CallocAssert(1, sizeof(Grid));
    gridPtr->minorGrid = TRUE;
    graphPtr->grid = gridPtr;

    if (Blt_ConfigureComponentFromObj(graphPtr->interp, graphPtr->tkwin, "grid",
	    "Grid", configSpecs, 0, (Tcl_Obj **)NULL, (char *)gridPtr,
	    Blt_GraphType(graphPtr)) != TCL_OK) {
	return TCL_ERROR;
    }
    ConfigureGrid(graphPtr, gridPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * CgetOp --
 *
 *	Queries configuration attributes of the grid such as line
 *	width, dashes, and position.
 *
 * Results:
 *	A standard Tcl result.
 *
 *---------------------------------------------------------------------------
 */
/* ARGSUSED */
static int
CgetOp(
    Graph *graphPtr,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    Grid *gridPtr = graphPtr->grid;

    return Blt_ConfigureValueFromObj(interp, graphPtr->tkwin, configSpecs,
	(char *)gridPtr, objv[3], Blt_GraphType(graphPtr));
}

/*
 *---------------------------------------------------------------------------
 *
 * ConfigureOp --
 *
 *	Queries or resets configuration attributes of the grid
 * 	such as line width, dashes, and position.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side Effects:
 *	Grid attributes are reset.  The graph is redrawn at the
 *	next idle point.
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
    Grid *gridPtr = graphPtr->grid;
    int flags;

    flags = Blt_GraphType(graphPtr) | BLT_CONFIG_OBJV_ONLY;
    if (objc == 3) {
	return Blt_ConfigureInfoFromObj(interp, graphPtr->tkwin, configSpecs,
	    (char *)gridPtr, (Tcl_Obj *)NULL, flags);
    } else if (objc == 4) {
	return Blt_ConfigureInfoFromObj(interp, graphPtr->tkwin, configSpecs,
	    (char *)gridPtr, objv[3], flags);
    }
    if (Blt_ConfigureWidgetFromObj(graphPtr->interp, graphPtr->tkwin, 
	 configSpecs, objc - 3, objv + 3, (char *)gridPtr, flags) != TCL_OK) {
	return TCL_ERROR;
    }
    ConfigureGrid(graphPtr, gridPtr);
    graphPtr->flags |= REDRAW_BACKING_STORE;
    Blt_EventuallyRedrawGraph(graphPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * MapOp --
 *
 *	Maps the grid.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side Effects:
 *	Grid attributes are reset and the graph is redrawn if necessary.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
MapOp(
    Graph *graphPtr,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    Grid *gridPtr = graphPtr->grid;

    if (gridPtr->hidden) {
	gridPtr->hidden = FALSE;/* Changes "-hide" configuration option */
	graphPtr->flags |= REDRAW_BACKING_STORE;
	Blt_EventuallyRedrawGraph(graphPtr);
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * UnmapOp --
 *
 *	Unmaps the grid (off).
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side Effects:
 *	Grid attributes are reset and the graph is redrawn if necessary.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
UnmapOp(
    Graph *graphPtr,
    Tcl_Interp *interp,		/* Not used. */
    int objc,			/* Not used. */
    Tcl_Obj *const *objv)	/* Not used. */
{
    Grid *gridPtr = graphPtr->grid;

    if (!gridPtr->hidden) {
	gridPtr->hidden = TRUE;	/* Changes "-hide" configuration option */
	graphPtr->flags |= REDRAW_BACKING_STORE;
	Blt_EventuallyRedrawGraph(graphPtr);
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ToggleOp --
 *
 *	Toggles the state of the grid shown/hidden.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side Effects:
 *	Grid is hidden/displayed. The graph is redrawn at the next
 *	idle time.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ToggleOp(
    Graph *graphPtr,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    Grid *gridPtr = graphPtr->grid;

    gridPtr->hidden = (!gridPtr->hidden);
    graphPtr->flags |= REDRAW_BACKING_STORE;
    Blt_EventuallyRedrawGraph(graphPtr);
    return TCL_OK;
}

static Blt_OpSpec gridOps[] =
{
    {"cget",      2, CgetOp,      4, 4, "option",},
    {"configure", 2, ConfigureOp, 3, 0, "?options...?",},
    {"off",       2, UnmapOp,     3, 3, "",},
    {"on",        2, MapOp,       3, 3, "",},
    {"toggle",    1, ToggleOp,    3, 3, "",},
};
static int nGridOps = sizeof(gridOps) / sizeof(Blt_OpSpec);

/*
 *---------------------------------------------------------------------------
 *
 * Blt_GridOp --
 *
 *	User routine to configure grid lines.  Grids are drawn
 *	at major tick intervals across the graph.
 *
 * Results:
 *	The return value is a standard Tcl result.
 *
 * Side Effects:
 *	Grid may be drawn in the plotting area.
 *
 *---------------------------------------------------------------------------
 */
int
Blt_GridOp(
    Graph *graphPtr,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    Blt_Op proc;

    proc = Blt_GetOpFromObj(interp, nGridOps, gridOps, BLT_OP_ARG2, objc, objv,
	0);
    if (proc == NULL) {
	return TCL_ERROR;
    }
    return (*proc) (graphPtr, interp, objc, objv);
}

int
Blt_GridHasMinor(Graph *graphPtr)
{
    Grid *gridPtr = graphPtr->grid;

    return gridPtr->minorGrid;
}
#endif
