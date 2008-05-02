/*
 * bltBind.h --
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

#ifndef _BLT_BIND_H
#define _BLT_BIND_H

#include <bltList.h>

typedef struct Blt_BindTableStruct *Blt_BindTable;

typedef ClientData (Blt_BindPickProc) _ANSI_ARGS_((ClientData clientData,
	int x, int y, ClientData *contextPtr));

typedef void (Blt_BindTagProc) _ANSI_ARGS_((Blt_BindTable bindTable, 
	ClientData object, ClientData context, Blt_List list));


/*
 *  Binding structure information:
 */

struct Blt_BindTableStruct {
    unsigned int flags;
    Tk_BindingTable bindingTable;
				/* Table of all bindings currently defined.
				 * NULL means that no bindings exist, so the
				 * table hasn't been created.  Each "object"
				 * used for this table is either a Tk_Uid for
				 * a tag or the address of an item named by
				 * id. */

    ClientData currentItem;	/* The item currently containing the mouse
				 * pointer, or NULL if none. */
    ClientData currentContext;	/* One word indicating what kind of object
				 * was picked. */

    ClientData newItem;		/* The item that is about to become the
				 * current one, or NULL.  This field is
				 * used to detect deletions of the new
				 * current item pointer that occur during
				 * Leave processing of the previous current
				 * tab.  */
    ClientData newContext;	/* One-word indicating what kind of object 
				 * was just picked. */

    ClientData focusItem;
    ClientData focusContext;

    XEvent pickEvent;		/* The event upon which the current choice
				 * of the current tab is based.  Must be saved
				 * so that if the current item is deleted,
				 * we can pick another. */
    int activePick;		/* The pick event has been initialized so
				 * that we can repick it */

    int state;			/* Last known modifier state.  Used to
				 * defer picking a new current object
				 * while buttons are down. */

    ClientData clientData;
    Tk_Window tkwin;
    Blt_BindPickProc *pickProc;	/* Routine to report the item the mouse is
				 * currently over. */
    Blt_BindTagProc *tagProc;	/* Routine to report tags picked items. */
};

BLT_EXTERN void Blt_DestroyBindingTable _ANSI_ARGS_((Blt_BindTable table));

BLT_EXTERN Blt_BindTable Blt_CreateBindingTable _ANSI_ARGS_((Tcl_Interp *interp,
	Tk_Window tkwin, ClientData clientData, Blt_BindPickProc *pickProc,
	Blt_BindTagProc *tagProc));

BLT_EXTERN int Blt_ConfigureBindings _ANSI_ARGS_((Tcl_Interp *interp,
	Blt_BindTable table, ClientData item, int argc, CONST char **argv));

BLT_EXTERN int Blt_ConfigureBindingsFromObj _ANSI_ARGS_((Tcl_Interp *interp,
	Blt_BindTable table, ClientData item, int objc, Tcl_Obj *CONST *objv));

BLT_EXTERN void Blt_PickCurrentItem _ANSI_ARGS_((Blt_BindTable table));

BLT_EXTERN void Blt_DeleteBindings _ANSI_ARGS_((Blt_BindTable table,
	ClientData object));

BLT_EXTERN void Blt_MoveBindingTable _ANSI_ARGS_((Blt_BindTable table, 
	Tk_Window tkwin));

#define Blt_SetFocusItem(bindPtr, object, context) \
	((bindPtr)->focusItem = (ClientData)(object),\
	 (bindPtr)->focusContext = (ClientData)(context))

#define Blt_SetCurrentItem(bindPtr, object, context) \
	((bindPtr)->currentItem = (ClientData)(object),\
	 (bindPtr)->currentContext = (ClientData)(context))

#define Blt_GetCurrentItem(bindPtr)  ((bindPtr)->currentItem)
#define Blt_GetCurrentContext(bindPtr)  ((bindPtr)->currentContext)
#define Blt_GetLatestItem(bindPtr)  ((bindPtr)->newItem)

#define Blt_GetBindingData(bindPtr)  ((bindPtr)->clientData)

#endif /*_BLT_BIND_H*/
