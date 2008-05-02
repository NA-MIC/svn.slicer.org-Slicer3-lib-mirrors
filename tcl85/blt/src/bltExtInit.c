
/*
 * BltExInit.c --
 *
 * This module initials the Tk-related commands of BLT toolkit, registering
 * the commands with the Tcl interpreter.
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

#include "bltInt.h"
#include "bltNsUtil.h"

#define EXACT 1

BLT_EXTERN Tcl_AppInitProc Bltx_Init;
BLT_EXTERN Tcl_AppInitProc Bltx_SafeInit;
BLT_EXTERN Tcl_AppInitProc Bltcore_Init;
BLT_EXTERN Tcl_AppInitProc Blt_Init;

static Tcl_AppInitProc *cmdProcs[] =
{
    Blt_BgStyleCmdInitProc,
#ifndef NO_GRAPH
    Blt_GraphCmdInitProc,
#endif
#ifndef NO_PICTURE
    Blt_PictureCmdInitProc,
#endif
#ifndef NO_TABLE
    Blt_TableCmdInitProc,
#endif
#ifndef NO_TABSET
    Blt_TabsetCmdInitProc,
#endif
#ifndef NO_TABNOTEBOOK
    Blt_TabnotebookCmdInitProc,
#endif
#ifndef NO_HTEXT
    Blt_HtextCmdInitProc,
#endif
#ifndef NO_BUSY
    Blt_BusyCmdInitProc,
#endif
#ifndef NO_WINOP
    Blt_WinopCmdInitProc,
#endif
#ifndef NO_BITMAP
    Blt_BitmapCmdInitProc,
#endif
#ifndef NO_DRAGDROP
    Blt_DragDropCmdInitProc,
#endif
#ifndef NO_DND
    Blt_DndCmdInitProc,
#endif
#ifndef NO_CONTAINER
    Blt_ContainerCmdInitProc,
#endif
#ifndef NO_BELL
    Blt_BeepCmdInitProc,
#endif
#ifndef NO_CUTBUFFER
    Blt_CutbufferCmdInitProc,
#endif
#ifndef NO_PRINTER
    Blt_PrinterCmdInitProc,
#endif
#ifndef NO_TILEFRAME
    Blt_FrameCmdInitProc,
#endif
#ifndef NO_TILEBUTTON
    Blt_ButtonCmdInitProc,
#endif
#ifndef NO_TILESCROLLBAR
    Blt_ScrollbarCmdInitProc,
#endif
#ifndef NO_TREEVIEW
    Blt_TreeViewCmdInitProc,
#endif
#if (BLT_MAJOR_VERSION > 3)
#ifndef NO_MOUNTAIN
    Blt_MountainCmdInitProc,
#endif
#endif
#ifndef NO_TED
    Blt_TedCmdInitProc,
#endif
#ifndef NO_TED
    Blt_ComboButtonInitProc,
    Blt_ComboEntryInitProc,
    Blt_ComboMenuInitProc,
    Blt_ComboTreeInitProc,
#endif
    (Tcl_AppInitProc *) NULL
};

/*LINTLIBRARY*/
int
Bltx_Init(Tcl_Interp *interp) /* Interpreter to add extra commands */
{
    Tcl_Namespace *nsPtr;
    Tcl_AppInitProc **p;

#ifdef USE_TCL_STUBS
    if (Tcl_InitStubs(interp, TCL_VERSION, 1) == NULL) {
	return TCL_ERROR;
    };
#endif
    if (Tcl_PkgRequire(interp, "bltcore", BLT_VERSION, EXACT) == NULL) {
	return TCL_ERROR;
    }

#if (_TCL_VERSION >= _VERSION(8,1,0)) 
#  ifdef USE_TK_STUBS
    if (Tk_InitStubs(interp, TK_VERSION, 1) == NULL) {
	return TCL_ERROR;
    };
#  endif
    if (Tcl_PkgPresent(interp, "Tk", TK_VERSION, EXACT) == NULL) {
	return TCL_OK;
    } 
#else
    if (Tcl_PkgRequire(interp, "Tk", TK_VERSION, EXACT) == NULL) {
	Tcl_ResetResult(interp);
	return TCL_OK;
    } 
#endif
    nsPtr = Tcl_CreateNamespace(interp, "::blt::tile", NULL, NULL);
    if (nsPtr == NULL) {
	return TCL_ERROR;
    }
    nsPtr = Tcl_FindNamespace(interp, "::blt", NULL, TCL_LEAVE_ERR_MSG);
    if (nsPtr == NULL) {
	return TCL_ERROR;
    }
    Blt_RegisterPictureImageType();
    Blt_RegisterEpsCanvasItem();
    
    /* Initialize the BLT commands that only use Tk. */
    for (p = cmdProcs; *p != NULL; p++) {
	if ((**p) (interp) != TCL_OK) {
	    Tcl_DeleteNamespace(nsPtr);
	    return TCL_ERROR;
	}
    }
    if (Tcl_PkgProvide(interp, "bltextra", BLT_VERSION) != TCL_OK) {
	return TCL_ERROR;
    }
    XSync (Tk_Display(Tk_MainWindow(interp)), 1);
    return TCL_OK;
}

/*LINTLIBRARY*/
int
Bltx_SafeInit(Tcl_Interp *interp) /* Interpreter to add extra commands */
{
    return Bltx_Init(interp);
}

int
Blt_Init(Tcl_Interp *interp) 
{
    if (Bltcore_Init(interp) != TCL_OK) {
	return TCL_ERROR;
    }
    if (Bltx_Init(interp) != TCL_OK) {
	return TCL_ERROR;
    }
    return TCL_OK;
}

#ifdef USE_DLL
#  include "bltWinDll.c"
#endif
