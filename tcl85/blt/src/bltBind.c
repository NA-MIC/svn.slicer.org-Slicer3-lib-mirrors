
/*
 * bltBind.c --
 *
 *	This module implements object binding procedures for the BLT toolkit.
 *
 *	Copyright 1998 George A Howlett.
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
#include "bltBind.h"
#include <bltList.h>

static Tk_EventProc BindProc;

typedef struct Blt_BindTableStruct BindTable;

/*
 * Binding table procedures.
 */
#define REPICK_IN_PROGRESS (1<<0)
#define LEFT_GRABBED_ITEM  (1<<1)

#define ALL_BUTTONS_MASK \
	(Button1Mask | Button2Mask | Button3Mask | Button4Mask | Button5Mask)

#ifndef VirtualEventMask
#define VirtualEventMask    (1L << 30)
#endif

#define ALL_VALID_EVENTS_MASK \
	(ButtonMotionMask | Button1MotionMask | Button2MotionMask | \
	 Button3MotionMask | Button4MotionMask | Button5MotionMask | \
	 ButtonPressMask | ButtonReleaseMask | EnterWindowMask | \
	 LeaveWindowMask | KeyPressMask | KeyReleaseMask | \
	 PointerMotionMask | VirtualEventMask)

static int buttonMasks[] =
{
    0,				/* No buttons pressed */
    Button1Mask, Button2Mask, Button3Mask, Button4Mask, Button5Mask,
};


/*
 * How to make drag&drop work?
 *
 *	Right now we generate pseudo <Enter> <Leave> events within button grab
 *	on an object.  They're marked NotifyVirtual instead of NotifyAncestor.
 *	A better solution: generate new-style virtual <<DragEnter>>
 *	<<DragMotion>> <<DragLeave>> events.  These virtual events don't have
 *	to exist as "real" event sequences, like virtual events do now.
 */

/*
 *---------------------------------------------------------------------------
 *
 * DoEvent --
 *
 *	This procedure is called to invoke binding processing for a new event
 *	that is associated with the current item for a legend.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Depends on the bindings for the legend.  A binding script could delete
 *	an entry, so callers should protect themselves with Tcl_Preserve and
 *	Tcl_Release.
 *
 *---------------------------------------------------------------------------
 */
static void
DoEvent(
    BindTable *bindPtr,		/* Binding information for widget in
				 * which event occurred. */
    XEvent *eventPtr,		/* Real or simulated X event that
				 * is to be processed. */
    ClientData item,		/* Item picked. */
    ClientData context)		/* Context of item.  */
{
    Blt_List bindIds;
    int nIds;

    if ((bindPtr->tkwin == NULL) || (bindPtr->bindingTable == NULL)) {
	return;
    }
    if ((eventPtr->type == KeyPress) || (eventPtr->type == KeyRelease)) {
	item = bindPtr->focusItem;
	context = bindPtr->focusContext;
    }
    if (item == NULL) {
	return;
    }
    /*
     * Invoke the binding system.
     */

    bindIds = Blt_ListCreate(BLT_ONE_WORD_KEYS);
    if (bindPtr->tagProc == NULL) {
	Blt_ListAppend(bindIds, (char *)Tk_GetUid("all"), 0);
	Blt_ListAppend(bindIds, (char *)item, 0);
    } else {
	(*bindPtr->tagProc) (bindPtr, item, context, bindIds);
    }
    nIds = Blt_ListGetLength(bindIds);
    if (nIds > 0) {
	ClientData *idArray;
	ClientData tags[32];
	Blt_ListNode node;
	
	idArray = tags;
	if (nIds >= 32) {
	    idArray = Blt_MallocAssert(sizeof(ClientData) * nIds);
	    
	} 
	nIds = 0;
	for (node = Blt_ListFirstNode(bindIds); node != NULL;
	     node = Blt_ListNextNode(node)) {
	    idArray[nIds++] = (ClientData)Blt_ListGetKey(node);
	}
	Tk_BindEvent(bindPtr->bindingTable, eventPtr, bindPtr->tkwin, nIds, 
		idArray);
	if (nIds >= 32) {
	    Blt_Free(idArray);
	}
    }
    Blt_ListDestroy(bindIds);
}

/*
 *---------------------------------------------------------------------------
 *
 * PickCurrentItem --
 *
 *	Find the topmost item in a legend that contains a given
 *	location and mark the the current item.  If the current
 *	item has changed, generate a fake exit event on the old
 *	current item and a fake enter event on the new current
 *	item.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The current item may change.  If it does, then the commands
 *	associated with item entry and exit could do just about 
 *	anything.  A binding script could delete the legend, so 
 *	callers should protect themselves with Tcl_Preserve and 
 *	Tcl_Release.
 *
 *---------------------------------------------------------------------------
 */
static void
PickCurrentItem(
    BindTable *bindPtr,		/* Binding table information. */
    XEvent *eventPtr)		/* Event describing location of
				 * mouse cursor.  Must be EnterWindow,
				 * LeaveWindow, ButtonRelease, or
				 * MotionNotify. */
{
    int buttonDown;
    ClientData newItem, oldItem;
    ClientData newContext;

    /*
     * Check whether or not a button is down.  If so, we'll log entry
     * and exit into and out of the current item, but not entry into
     * any other item.  This implements a form of grabbing equivalent
     * to what the X server does for windows.
     */
    buttonDown = (bindPtr->state & ALL_BUTTONS_MASK);
    if (!buttonDown) {
	bindPtr->flags &= ~LEFT_GRABBED_ITEM;
    }

    /*
     * Save information about this event in the widget.  The event in
     * the widget is used for two purposes:
     *
     * 1. Event bindings: if the current item changes, fake events are
     *    generated to allow item-enter and item-leave bindings to trigger.
     * 2. Reselection: if the current item gets deleted, can use the
     *    saved event to find a new current item.
     * Translate MotionNotify events into EnterNotify events, since that's
     * what gets reported to item handlers.
     */

    if (eventPtr != &bindPtr->pickEvent) {
	if ((eventPtr->type == MotionNotify) ||
	    (eventPtr->type == ButtonRelease)) {
	    bindPtr->pickEvent.xcrossing.type = EnterNotify;
	    bindPtr->pickEvent.xcrossing.serial = eventPtr->xmotion.serial;
	    bindPtr->pickEvent.xcrossing.send_event =
		eventPtr->xmotion.send_event;
	    bindPtr->pickEvent.xcrossing.display = eventPtr->xmotion.display;
	    bindPtr->pickEvent.xcrossing.window = eventPtr->xmotion.window;
	    bindPtr->pickEvent.xcrossing.root = eventPtr->xmotion.root;
	    bindPtr->pickEvent.xcrossing.subwindow = None;
	    bindPtr->pickEvent.xcrossing.time = eventPtr->xmotion.time;
	    bindPtr->pickEvent.xcrossing.x = eventPtr->xmotion.x;
	    bindPtr->pickEvent.xcrossing.y = eventPtr->xmotion.y;
	    bindPtr->pickEvent.xcrossing.x_root = eventPtr->xmotion.x_root;
	    bindPtr->pickEvent.xcrossing.y_root = eventPtr->xmotion.y_root;
	    bindPtr->pickEvent.xcrossing.mode = NotifyNormal;
	    bindPtr->pickEvent.xcrossing.detail = NotifyNonlinear;
	    bindPtr->pickEvent.xcrossing.same_screen
		= eventPtr->xmotion.same_screen;
	    bindPtr->pickEvent.xcrossing.focus = False;
	    bindPtr->pickEvent.xcrossing.state = eventPtr->xmotion.state;
	} else {
	    bindPtr->pickEvent = *eventPtr;
	}
    }
    bindPtr->activePick = TRUE;

    /*
     * If this is a recursive call (there's already a partially completed
     * call pending on the stack;  it's in the middle of processing a
     * Leave event handler for the old current item) then just return;
     * the pending call will do everything that's needed.
     */
    if (bindPtr->flags & REPICK_IN_PROGRESS) {
	return;
    }
    /*
     * A LeaveNotify event automatically means that there's no current
     * item, so the check for closest item can be skipped.
     */
    newContext = NULL;
    if (bindPtr->pickEvent.type != LeaveNotify) {
	int x, y;

	x = bindPtr->pickEvent.xcrossing.x;
	y = bindPtr->pickEvent.xcrossing.y;
	newItem = (*bindPtr->pickProc) (bindPtr->clientData, x, y, &newContext);
    } else {
	newItem = NULL;
    }
    if (((newItem == bindPtr->currentItem) && 
	 (newContext == bindPtr->currentContext)) && 
	((bindPtr->flags & LEFT_GRABBED_ITEM) == 0)) {
	/*
	 * Nothing to do:  the current item hasn't changed.
	 */
	return;
    }
#ifdef FULLY_SIMULATE_GRAB
    if (((newItem != bindPtr->currentItem) || 
	 (newContext != bindPtr->currentContext)) && 
	(buttonDown)) {
	bindPtr->flags |= LEFT_GRABBED_ITEM;
	fprintf(stderr, "pickcurrentitem: simulate grab.\n");
	return;
    }
#endif
    /*
     * Simulate a LeaveNotify event on the previous current item and
     * an EnterNotify event on the new current item.  Remove the "current"
     * tag from the previous current item and place it on the new current
     * item.
     */
    oldItem = bindPtr->currentItem;
    Tcl_Preserve(oldItem);
    Tcl_Preserve(newItem);

    if ((bindPtr->currentItem != NULL) &&
	((newItem != bindPtr->currentItem) || 
	 (newContext != bindPtr->currentContext)) && 
	((bindPtr->flags & LEFT_GRABBED_ITEM) == 0)) {
	XEvent event;

	event = bindPtr->pickEvent;
	event.type = LeaveNotify;
	/*
	 * If the event's detail happens to be NotifyInferior the
	 * binding mechanism will discard the event.  To be consistent,
	 * always use NotifyAncestor.
	 */
	event.xcrossing.detail = NotifyAncestor;

	bindPtr->flags |= REPICK_IN_PROGRESS;
	DoEvent(bindPtr, &event, bindPtr->currentItem, bindPtr->currentContext);
	bindPtr->flags &= ~REPICK_IN_PROGRESS;

	/*
	 * Note:  during DoEvent above, it's possible that
	 * bindPtr->newItem got reset to NULL because the
	 * item was deleted.
	 */
    }
    if (((newItem != bindPtr->currentItem) || 
	 (newContext != bindPtr->currentContext)) && 
	(buttonDown)) {
	XEvent event;

	bindPtr->flags |= LEFT_GRABBED_ITEM;
	event = bindPtr->pickEvent;
	if ((newItem != bindPtr->newItem) || 
	    (newContext != bindPtr->newContext)) {
	    ClientData savedItem;
	    ClientData savedContext;

	    /*
	     * Generate <Enter> and <Leave> events for objects during
	     * button grabs.  This isn't standard. But for example, it
	     * allows one to provide balloon help on the individual
	     * entries of the Hierbox widget.
	     */
	    savedItem = bindPtr->currentItem;
	    savedContext = bindPtr->currentContext;
	    if (bindPtr->newItem != NULL) {
		event.type = LeaveNotify;
		event.xcrossing.detail = NotifyVirtual /* Ancestor */ ;
		bindPtr->currentItem = bindPtr->newItem;
		DoEvent(bindPtr, &event, bindPtr->newItem, bindPtr->newContext);
	    }
	    bindPtr->newItem = newItem;
	    bindPtr->newContext = newContext;
	    if (newItem != NULL) {
		event.type = EnterNotify;
		event.xcrossing.detail = NotifyVirtual /* Ancestor */ ;
		bindPtr->currentItem = newItem;
		DoEvent(bindPtr, &event, newItem, newContext);
	    }
	    bindPtr->currentItem = savedItem;
	    bindPtr->currentContext = savedContext;
	}
	goto done;
    }
    /*
     * Special note:  it's possible that
     *		bindPtr->newItem == bindPtr->currentItem
     * here.  This can happen, for example, if LEFT_GRABBED_ITEM was set.
     */

    bindPtr->flags &= ~LEFT_GRABBED_ITEM;
    bindPtr->currentItem = bindPtr->newItem = newItem;
    bindPtr->currentContext = bindPtr->newContext = newContext;
    if (bindPtr->currentItem != NULL) {
	XEvent event;

	event = bindPtr->pickEvent;
	event.type = EnterNotify;
	event.xcrossing.detail = NotifyAncestor;
	DoEvent(bindPtr, &event, newItem, newContext);
#ifdef notdef
	if ((eventPtr->type == MotionNotify) ||
	    (eventPtr->type == ButtonRelease)) {
	    fprintf(stderr, "pickcurrentitem: DoEvent Button buttondown=%d.\n",
		    buttonDown);
	    event.type = eventPtr->type;
	    event.xbutton.button = eventPtr->xbutton.button;
	    DoEvent(bindPtr, &event, newItem, newContext);
	    fprintf(stderr, "pickcurrentitem: done.\n");
	}
#endif
    }
 done:
    Tcl_Release(newItem);
    Tcl_Release(oldItem);
}

/*
 *---------------------------------------------------------------------------
 *
 * BindProc --
 *
 *	This procedure is invoked by the Tk dispatcher to handle
 *	events associated with bindings on items.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Depends on the command invoked as part of the binding
 *	(if there was any).
 *
 *---------------------------------------------------------------------------
 */
static void
BindProc(
    ClientData clientData,	/* Pointer to widget structure. */
    XEvent *eventPtr)		/* Pointer to X event that just
				 * happened. */
{
    BindTable *bindPtr = clientData;
    int mask;

    Tcl_Preserve(bindPtr->clientData);
    /*
     * This code below keeps track of the current modifier state in
     * bindPtr->state.  This information is used to defer repicks of
     * the current item while buttons are down.
     */
    switch (eventPtr->type) {
    case ButtonPress:
    case ButtonRelease:
	mask = 0;
	if ((eventPtr->xbutton.button >= Button1) &&
	    (eventPtr->xbutton.button <= Button5)) {
	    mask = buttonMasks[eventPtr->xbutton.button];
	}
	/*
	 * For button press events, repick the current item using the
	 * button state before the event, then process the event.  For
	 * button release events, first process the event, then repick
	 * the current item using the button state *after* the event
	 * (the button has logically gone up before we change the
	 * current item).
	 */

	if (eventPtr->type == ButtonPress) {

	    /*
	     * On a button press, first repick the current item using
	     * the button state before the event, the process the event.
	     */

	    bindPtr->state = eventPtr->xbutton.state;
	    PickCurrentItem(bindPtr, eventPtr);
	    bindPtr->state ^= mask;
	    DoEvent(bindPtr, eventPtr, bindPtr->currentItem, 
		bindPtr->currentContext);

	} else {

	    /*
	     * Button release: first process the event, with the button
	     * still considered to be down.  Then repick the current
	     * item under the assumption that the button is no longer down.
	     */
	    bindPtr->state = eventPtr->xbutton.state;
	    DoEvent(bindPtr, eventPtr, bindPtr->currentItem, 
		bindPtr->currentContext);
	    eventPtr->xbutton.state ^= mask;
	    bindPtr->state = eventPtr->xbutton.state;
	    PickCurrentItem(bindPtr, eventPtr);
	    eventPtr->xbutton.state ^= mask;
	}
	break;

    case EnterNotify:
    case LeaveNotify:
	bindPtr->state = eventPtr->xcrossing.state;
	PickCurrentItem(bindPtr, eventPtr);
	break;

    case MotionNotify:
	bindPtr->state = eventPtr->xmotion.state;
	PickCurrentItem(bindPtr, eventPtr);
	DoEvent(bindPtr, eventPtr, bindPtr->currentItem, 
		bindPtr->currentContext);
	break;

    case KeyPress:
    case KeyRelease:
	bindPtr->state = eventPtr->xkey.state;
	PickCurrentItem(bindPtr, eventPtr);
	DoEvent(bindPtr, eventPtr, bindPtr->currentItem, 
		bindPtr->currentContext);
	break;
    }
    Tcl_Release(bindPtr->clientData);
}

int
Blt_ConfigureBindings(
    Tcl_Interp *interp,
    BindTable *bindPtr,
    ClientData item,
    int argc,
    const char **argv)
{
    const char *command;
    unsigned long mask;
    const char *seq;

    if (argc == 0) {
	Tk_GetAllBindings(interp, bindPtr->bindingTable, item);
	return TCL_OK;
    }
    if (argc == 1) {
	command = Tk_GetBinding(interp, bindPtr->bindingTable, item, argv[0]);
	if (command == NULL) {
	    Tcl_AppendResult(interp, "can't find event \"", argv[0], "\"",
			     (char *)NULL);
	    return TCL_ERROR;
	}
	Tcl_SetStringObj(Tcl_GetObjResult(interp), command, -1);
	return TCL_OK;
    }

    seq = argv[0];
    command = argv[1];

    if (command[0] == '\0') {
	return Tk_DeleteBinding(interp, bindPtr->bindingTable, item, seq);
    }

    if (command[0] == '+') {
	mask = Tk_CreateBinding(interp, bindPtr->bindingTable, item, seq,
		command + 1, TRUE);
    } else {
	mask = Tk_CreateBinding(interp, bindPtr->bindingTable, item, seq,
		command, FALSE);
    }
    if (mask == 0) {
	Tcl_AppendResult(interp, "event mask can't be zero for \"", item, "\"",
			     (char *)NULL);
	return TCL_ERROR;
    }
    if (mask & (unsigned)~ALL_VALID_EVENTS_MASK) {
	Tk_DeleteBinding(interp, bindPtr->bindingTable, item, seq);
	Tcl_ResetResult(interp);
	Tcl_AppendResult(interp, "requested illegal events; ",
		 "only key, button, motion, enter, leave, and virtual ",
		 "events may be used", (char *)NULL);
	return TCL_ERROR;
    }
    return TCL_OK;
}


int
Blt_ConfigureBindingsFromObj(
    Tcl_Interp *interp,
    BindTable *bindPtr,
    ClientData item,
    int objc,
    Tcl_Obj *const *objv)
{
    const char *command;
    unsigned long mask;
    const char *seq;
    char *string;

    if (objc == 0) {
	Tk_GetAllBindings(interp, bindPtr->bindingTable, item);
	return TCL_OK;
    }
    string = Tcl_GetString(objv[0]);
    if (objc == 1) {
	command = Tk_GetBinding(interp, bindPtr->bindingTable, item, string);
	if (command == NULL) {
	    Tcl_ResetResult(interp);
	    Tcl_AppendResult(interp, "invalid binding event \"", string, "\"", 
		(char *)NULL);
	    return TCL_ERROR;
	}
	Tcl_SetStringObj(Tcl_GetObjResult(interp), command, -1);
	return TCL_OK;
    }

    seq = string;
    command = Tcl_GetString(objv[1]);

    if (command[0] == '\0') {
	return Tk_DeleteBinding(interp, bindPtr->bindingTable, item, seq);
    }

    if (command[0] == '+') {
	mask = Tk_CreateBinding(interp, bindPtr->bindingTable, item, seq,
		command + 1, TRUE);
    } else {
	mask = Tk_CreateBinding(interp, bindPtr->bindingTable, item, seq,
		command, FALSE);
    }
    if (mask == 0) {
	return TCL_ERROR;
    }
    if (mask & (unsigned)~ALL_VALID_EVENTS_MASK) {
	Tk_DeleteBinding(interp, bindPtr->bindingTable, item, seq);
	Tcl_ResetResult(interp);
	Tcl_AppendResult(interp, "requested illegal events; ",
		 "only key, button, motion, enter, leave, and virtual ",
		 "events may be used", (char *)NULL);
	return TCL_ERROR;
    }
    return TCL_OK;
}

Blt_BindTable
Blt_CreateBindingTable(
    Tcl_Interp *interp,
    Tk_Window tkwin,
    ClientData clientData,
    Blt_BindPickProc *pickProc,
    Blt_BindTagProc *tagProc)
{
    unsigned int mask;
    BindTable *bindPtr;

    bindPtr = Blt_CallocAssert(1, sizeof(BindTable));
    bindPtr->bindingTable = Tk_CreateBindingTable(interp);
    bindPtr->clientData = clientData;
    bindPtr->tkwin = tkwin;
    bindPtr->pickProc = pickProc;
    bindPtr->tagProc = tagProc;
    mask = (KeyPressMask | KeyReleaseMask | ButtonPressMask |
	ButtonReleaseMask | EnterWindowMask | LeaveWindowMask |
	PointerMotionMask);
    Tk_CreateEventHandler(tkwin, mask, BindProc, bindPtr);
    return bindPtr;
}

void
Blt_DestroyBindingTable(BindTable *bindPtr)
{
    unsigned int mask;

    Tk_DeleteBindingTable(bindPtr->bindingTable);
    mask = (KeyPressMask | KeyReleaseMask | ButtonPressMask |
	ButtonReleaseMask | EnterWindowMask | LeaveWindowMask |
	PointerMotionMask);
    Tk_DeleteEventHandler(bindPtr->tkwin, mask, BindProc, bindPtr);
    Blt_Free(bindPtr);
}

void
Blt_PickCurrentItem(BindTable *bindPtr)
{
    if (bindPtr->activePick) {
	PickCurrentItem(bindPtr, &(bindPtr->pickEvent));
    }
}

void
Blt_DeleteBindings(
    BindTable *bindPtr,
    ClientData object)
{
    Tk_DeleteAllBindings(bindPtr->bindingTable, object);

    /*
     * If this is the object currently picked, we need to repick one.
     */
    if (bindPtr->currentItem == object) {
	bindPtr->currentItem = NULL;
	bindPtr->currentContext = NULL;
    }
    if (bindPtr->newItem == object) {
	bindPtr->newItem = NULL;
	bindPtr->newContext = NULL;
    }
    if (bindPtr->focusItem == object) {
	bindPtr->focusItem = NULL;
	bindPtr->focusContext = NULL;
    }
}

void
Blt_MoveBindingTable(
    BindTable *bindPtr,
    Tk_Window tkwin)
{
    unsigned int mask;

    mask = (KeyPressMask | KeyReleaseMask | ButtonPressMask |
	ButtonReleaseMask | EnterWindowMask | LeaveWindowMask |
	PointerMotionMask);
    if (bindPtr->tkwin != NULL) {
	Tk_DeleteEventHandler(bindPtr->tkwin, mask, BindProc, bindPtr);
    }
    Tk_CreateEventHandler(tkwin, mask, BindProc, bindPtr);
    bindPtr->tkwin = tkwin;
}
