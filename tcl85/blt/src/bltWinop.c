
/*
 * bltWinop.c --
 *
 * This module implements simple window commands for the BLT toolkit.
 *
 *	Copyright 1991-2004 George A Howlett.
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

#ifndef NO_WINOP
#include "bltOp.h"
#include "bltPicture.h"
#include "bltImage.h"
#include <X11/Xutil.h>
#include "tkDisplay.h"

#define CLAMP(c)	((((c) < 0.0) ? 0.0 : ((c) > 255.0) ? 255.0 : (c)))
static Tcl_ObjCmdProc WinopCmd;

static int
GetRealizedWindowFromObj(Tcl_Interp *interp, Tcl_Obj *objPtr, 
			 Tk_Window *tkwinPtr)
{
    char *string;
    Tk_Window tkwin;

    string = Tcl_GetString(objPtr);
    assert(interp != NULL);
    tkwin = Tk_NameToWindow(interp, string, Tk_MainWindow(interp));
    if (tkwin == NULL) {
	return TCL_ERROR;
    }
    if (Tk_WindowId(tkwin) == None) {
	Tk_MakeWindowExist(tkwin);
    }
    *tkwinPtr = tkwin;
    return TCL_OK;
}

static int
GetWindowFromObj(Tcl_Interp *interp, Tcl_Obj *objPtr, Window *windowPtr)
{
    int xid;
    char *string;

    string = Tcl_GetString(objPtr);
    if (string[0] == '.') {
	Tk_Window tkwin;

	if (GetRealizedWindowFromObj(interp, objPtr, &tkwin) != TCL_OK) {
	    return TCL_ERROR;
	}
	if (Tk_IsTopLevel(tkwin)) {
	    *windowPtr = Blt_GetWindowId(tkwin);
	} else {
	    *windowPtr = Tk_WindowId(tkwin);
	}
    } else {
	if (Tcl_GetIntFromObj(interp, objPtr, &xid) != TCL_OK) {
	    return TCL_ERROR;
	}
#ifdef WIN32
	{ 
	    static TkWinWindow tkWinWindow;
	    
	    tkWinWindow.handle = (HWND)xid;
	    tkWinWindow.winPtr = NULL;
	    tkWinWindow.type = TWD_WINDOW;
	    *windowPtr = (Window)&tkWinWindow;
	}
#else
	*windowPtr = (Window)xid;
#endif
    }
    return TCL_OK;
}

/*ARGSUSED*/
static int
LowerOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	Tcl_Obj *const *objv)
{
    int i;
    Window window;
    Display *display;

    display = Tk_Display(Tk_MainWindow(interp));
    for (i = 2; i < objc; i++) {
	if (GetWindowFromObj(interp, objv[i], &window) != TCL_OK) {
	    return TCL_ERROR;
	}
	XLowerWindow(display, window);
    }
    return TCL_OK;
}

/*ARGSUSED*/
static int
RaiseOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	Tcl_Obj *const *objv)
{
    int i;
    Window window;
    Display *display;

    display = Tk_Display(Tk_MainWindow(interp));
    for (i = 2; i < objc; i++) {
	if (GetWindowFromObj(interp, objv[i], &window) != TCL_OK) {
	    return TCL_ERROR;
	}
	XRaiseWindow(display, window);
    }
    return TCL_OK;
}

/*ARGSUSED*/
static int
MapOp(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const *objv)
{
    int i;
    Window window;
    Display *display;
    char *string;

    display = Tk_Display(Tk_MainWindow(interp));
    for (i = 2; i < objc; i++) {
	string = Tcl_GetString(objv[i]);
	if (string[0] == '.') {
	    Tk_Window tkwin;
	    Tk_FakeWin *fakePtr;

	    if (GetRealizedWindowFromObj(interp, objv[i], &tkwin) != TCL_OK) {
		return TCL_ERROR;
	    }
#ifdef  WIN32
	Tk_MapWindow(tkwin);		
#endif
	    fakePtr = (Tk_FakeWin *) tkwin;
	    fakePtr->flags |= TK_MAPPED;
	    window = Tk_WindowId(tkwin);
	} else {
	    int xid;

	    if (Tcl_GetIntFromObj(interp, objv[i], &xid) != TCL_OK) {
		return TCL_ERROR;
	    }
	    window = (Window)xid;
	}
	XMapWindow(display, window);
    }
    return TCL_OK;
}

/*ARGSUSED*/
static int
MoveOp(ClientData clientData, Tcl_Interp *interp, int objc, 
       Tcl_Obj *const *objv)
{
    int x, y;
    Tk_Window tkwin;
    Window window;
    Display *display;

    tkwin = Tk_MainWindow(interp);
    display = Tk_Display(tkwin);
    if (GetWindowFromObj(interp, objv[2], &window) != TCL_OK) {
	return TCL_ERROR;
    }
    if (Tk_GetPixelsFromObj(interp, tkwin, objv[3], &x) != TCL_OK) {
	return TCL_ERROR;
    }
    if (Tk_GetPixelsFromObj(interp, tkwin, objv[4], &y) != TCL_OK) {
	return TCL_ERROR;
    }
    XMoveWindow(display, window, x, y);
    return TCL_OK;
}

/*ARGSUSED*/
static int
UnmapOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	Tcl_Obj *const *objv)
{
    int i;
    Window window;
    Display *display;
    char *string;

    display = Tk_Display(Tk_MainWindow(interp));
    for (i = 2; i < objc; i++) {
	string = Tcl_GetString(objv[i]);
	if (string[0] == '.') {
	    Tk_Window tkwin;
	    Tk_FakeWin *fakePtr;

	    if (GetRealizedWindowFromObj(interp, objv[i], &tkwin) != TCL_OK) {
		return TCL_ERROR;
	    }
	    fakePtr = (Tk_FakeWin *) tkwin;
	    fakePtr->flags &= ~TK_MAPPED;
	    window = Tk_WindowId(tkwin);
	} else {
	    int xid;

	    if (Tcl_GetIntFromObj(interp, objv[i], &xid) != TCL_OK) {
		return TCL_ERROR;
	    }
	    window = (Window)xid;
	}
	XMapWindow(display, window);
    }
    return TCL_OK;
}

/* ARGSUSED */
static int
ChangesOp(ClientData clientData, Tcl_Interp *interp, int objc,
	  Tcl_Obj *const *objv)
{
    Tk_Window tkwin;

    if (GetRealizedWindowFromObj(interp, objv[2], &tkwin) != TCL_OK) {
	return TCL_ERROR;
    }
    if (Tk_IsTopLevel(tkwin)) {
	XSetWindowAttributes attrs;
	Window window;
	unsigned int mask;

	window = Blt_GetWindowId(tkwin);
	attrs.backing_store = WhenMapped;
	attrs.save_under = True;
	mask = CWBackingStore | CWSaveUnder;
	XChangeWindowAttributes(Tk_Display(tkwin), window, mask, &attrs);
    }
    return TCL_OK;
}

/* ARGSUSED */
static int
QueryOp(
    ClientData clientData,
    Tcl_Interp *interp,
    int objc,			/* Not used. */
    Tcl_Obj *const *objv)	/* Not used. */
{
    int rootX, rootY, childX, childY;
    Window root, child;
    unsigned int mask;
    Tk_Window tkwin = (Tk_Window)clientData;

    /* GetCursorPos */
    if (XQueryPointer(Tk_Display(tkwin), Tk_WindowId(tkwin), &root,
	    &child, &rootX, &rootY, &childX, &childY, &mask)) {
	Tcl_Obj *listObjPtr;

	listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
	Tcl_ListObjAppendElement(interp, listObjPtr, Tcl_NewIntObj(rootX));
	Tcl_ListObjAppendElement(interp, listObjPtr, Tcl_NewIntObj(rootY));
	Tcl_SetObjResult(interp, listObjPtr);
    }
    return TCL_OK;
}

/*ARGSUSED*/
static int
WarpToOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	 Tcl_Obj *const *objv)
{
    Tk_Window tkMain;

    tkMain = (Tk_Window)clientData;
    if (objc == 3) {
	Tk_Window tkwin;

	if (GetRealizedWindowFromObj(interp, objv[2], &tkwin) != TCL_OK) {
	    return TCL_ERROR;
	}
	if (!Tk_IsMapped(tkwin)) {
	    Tcl_AppendResult(interp, "can't warp to unmapped window \"",
		     Tk_PathName(tkwin), "\"", (char *)NULL);
	    return TCL_ERROR;
	}
	XWarpPointer(Tk_Display(tkwin), None, Tk_WindowId(tkwin),
	     0, 0, 0, 0, Tk_Width(tkwin) / 2, Tk_Height(tkwin) / 2);
    } else if (objc == 4) {
	int x, y;
	Window root;
	
	if ((Tk_GetPixelsFromObj(interp, tkMain, objv[2], &x) != TCL_OK) ||
	    (Tk_GetPixelsFromObj(interp, tkMain, objv[3], &y) != TCL_OK)) {
	    return TCL_ERROR;
	}
	root = Tk_RootWindow(tkMain);
	XWarpPointer(Tk_Display(tkMain), None, root, 0, 0, 0, 0, x, y);
    }
    return QueryOp(clientData, interp, 0, (Tcl_Obj **)NULL);
}

static Blt_OpSpec winOps[] =
{
    {"changes", 3, ChangesOp, 3, 3, "window",},
    {"lower", 1, LowerOp, 2, 0, "window ?window?...",},
    {"map", 2, MapOp, 2, 0, "window ?window?...",},
    {"move", 2, MoveOp, 5, 5, "window x y",},
    {"query", 3, QueryOp, 2, 2, "",},
    {"raise", 2, RaiseOp, 2, 0, "window ?window?...",},
    {"unmap", 1, UnmapOp, 2, 0, "window ?window?...",},
    {"warpto", 1, WarpToOp, 2, 5, "?window?",},
};

static int nWinOps = sizeof(winOps) / sizeof(Blt_OpSpec);

/* ARGSUSED */
static int
WinopCmd(ClientData clientData, Tcl_Interp *interp, int objc, 
	 Tcl_Obj *const *objv)
{
    Tcl_ObjCmdProc *proc;
    int result;

    proc = Blt_GetOpFromObj(interp, nWinOps, winOps, BLT_OP_ARG1,  objc, objv, 
	0);
    if (proc == NULL) {
	return TCL_ERROR;
    }
    clientData = (ClientData)Tk_MainWindow(interp);
    result = (*proc) (clientData, interp, objc, objv);
    return result;
}

int
Blt_WinopCmdInitProc(Tcl_Interp *interp)
{
    static Blt_InitCmdSpec cmdSpec = {"winop", WinopCmd,};

    return Blt_InitCmd(interp, "::blt", &cmdSpec);
}

#endif /* NO_WINOP */
