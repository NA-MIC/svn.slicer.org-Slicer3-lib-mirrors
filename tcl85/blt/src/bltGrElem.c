\
/*
 * bltGrElem.c --
 *
 * This module implements generic elements for the BLT graph widget.
 *
 *	Copyright 1993-2004 George A Howlett.
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

#include "bltGraph.h"
#include "bltOp.h"
#include "bltChain.h"
#include <X11/Xutil.h>
#include <bltDataTable.h>

#define GRAPH_KEY		"BLT Graph Data"

typedef struct {
    Blt_HashTable tableClients;	/* Table of trees. */
    Tcl_Interp *interp;
} GraphInterpData;

typedef struct {
    Blt_Dt table;
    int refCount;
} TableClient;

static Blt_OptionParseProc ObjToAlong;
static Blt_OptionPrintProc AlongToObj;
static Blt_CustomOption alongOption =
{
    ObjToAlong, AlongToObj, NULL, (ClientData)0
};
static Blt_OptionFreeProc FreeValues;
static Blt_OptionParseProc ObjToValues;
static Blt_OptionPrintProc ValuesToObj;
Blt_CustomOption bltValuesOption =
{
    ObjToValues, ValuesToObj, FreeValues, (ClientData)0
};
static Blt_OptionFreeProc FreeValuePairs;
static Blt_OptionParseProc ObjToValuePairs;
static Blt_OptionPrintProc ValuePairsToObj;
Blt_CustomOption bltValuePairsOption =
{
    ObjToValuePairs, ValuePairsToObj, FreeValuePairs, (ClientData)0
};

static Blt_OptionFreeProc  FreeStyles;
static Blt_OptionParseProc ObjToStyles;
static Blt_OptionPrintProc StylesToObj;
Blt_CustomOption bltLineStylesOption =
{
    ObjToStyles, StylesToObj, FreeStyles, (ClientData)0,
};

Blt_CustomOption bltBarStylesOption =
{
    ObjToStyles, StylesToObj, FreeStyles, (ClientData)0,
};

#include "bltGrElem.h"

static Blt_VectorChangedProc VectorChangedProc;

static void FindRange(ElemVector *evPtr);
static void FreeDataValues(ElemVector *evPtr);
static Tcl_InterpDeleteProc GraphInterpDeleteProc;
static Tcl_FreeProc FreeElement;

typedef int (GraphElementProc)(Graph *graphPtr, Tcl_Interp *interp, int objc, 
	Tcl_Obj *const *objv);

/*
 *---------------------------------------------------------------------------
 *
 * GraphInterpDeleteProc --
 *
 *	This is called when the interpreter hosting the tree object
 *	is deleted from the interpreter.  
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Destroys all remaining trees and removes the hash table
 *	used to register tree names.
 *
 *---------------------------------------------------------------------------
 */
/* ARGSUSED */
static void
GraphInterpDeleteProc(
    ClientData clientData,	/* Interpreter-specific data. */
    Tcl_Interp *interp)
{
    GraphInterpData *dataPtr = clientData;
    Blt_HashEntry *hPtr;
    Blt_HashSearch cursor;
    
    for (hPtr = Blt_FirstHashEntry(&dataPtr->tableClients, &cursor);
	 hPtr != NULL; hPtr = Blt_NextHashEntry(&cursor)) {
	TableClient *clientPtr;

	clientPtr = Blt_GetHashValue(hPtr);
	if (clientPtr->table != NULL) {
	    Blt_Dt_Close(clientPtr->table);
	}
	Blt_Free(clientPtr);
    }
    Blt_DeleteHashTable(&dataPtr->tableClients);
    Tcl_DeleteAssocData(interp, GRAPH_KEY);
    Blt_Free(dataPtr);
}

/*
 *---------------------------------------------------------------------------
 *
 * GetGraphInterpData --
 *
 *	Creates or retrieves data associated with tree data objects
 *	for a particular thread.  We're using Tcl_GetAssocData rather
 *	than the Tcl thread routines so BLT can work with pre-8.0 
 *	Tcl versions that don't have thread support.
 *
 * Results:
 *	Returns a pointer to the tree interpreter data.
 *
 * -------------------------------------------------------------------------- 
 */
static GraphInterpData *
GetGraphInterpData(Tcl_Interp *interp)
{
    Tcl_InterpDeleteProc *proc;
    GraphInterpData *dataPtr;

    dataPtr = Tcl_GetAssocData(interp, GRAPH_KEY, &proc);
    if (dataPtr == NULL) {
	dataPtr = Blt_MallocAssert(sizeof(GraphInterpData));
	dataPtr->interp = interp;
	Tcl_SetAssocData(interp, GRAPH_KEY, GraphInterpDeleteProc, dataPtr);
	Blt_InitHashTable(&dataPtr->tableClients, BLT_STRING_KEYS);
    }
    return dataPtr;
}

/*
 *---------------------------------------------------------------------------
 * Custom option parse and print procedures
 *---------------------------------------------------------------------------
 */
static int
GetPenStyleFromObj(
    Tcl_Interp *interp,
    Graph *graphPtr,
    Tcl_Obj *objPtr,
    ClassId classId,
    PenStyle *stylePtr)
{
    Pen *penPtr;
    Tcl_Obj **objv;
    int objc;

    if (Tcl_ListObjGetElements(interp, objPtr, &objc, &objv) != TCL_OK) {
	return TCL_ERROR;
    }
    if ((objc != 1) && (objc != 3)) {
	if (interp != NULL) {
	    Tcl_AppendResult(interp, "bad style entry \"", 
			Tcl_GetString(objPtr), 
			"\": should be \"penName\" or \"penName min max\"", 
			(char *)NULL);
	}
	return TCL_ERROR;
    }
    if (Blt_GetPenFromObj(interp, graphPtr, objv[0], classId, &penPtr) 
	!= TCL_OK) {
	return TCL_ERROR;
    }
    if (objc == 3) {
	double min, max;

	if ((Tcl_GetDoubleFromObj(interp, objv[1], &min) != TCL_OK) ||
	    (Tcl_GetDoubleFromObj(interp, objv[2], &max) != TCL_OK)) {
	    return TCL_ERROR;
	}
	SetWeight(stylePtr->weight, min, max);
    }
    stylePtr->penPtr = penPtr;
    return TCL_OK;
}

static void
FreeVector(ElemVector *evPtr)
{
    Blt_SetVectorChangedProc(evPtr->vectorInfo.id, NULL, NULL);
    if (evPtr->vectorInfo.id != NULL) { 
	Blt_FreeVectorId(evPtr->vectorInfo.id); 
    }
}

static int
FetchVectorValues(Tcl_Interp *interp, ElemVector *evPtr, Blt_Vector *vector)
{
    double *array;
    
    if (evPtr->values == NULL) {
	array = Blt_Malloc(Blt_VecLength(vector) * sizeof(double));
    } else {
	array = Blt_Realloc(evPtr->values, 
			    Blt_VecLength(vector) * sizeof(double));
    }
    if (array == NULL) {
	if (interp != NULL) {
	    Tcl_AppendResult(interp, "can't allocate new vector", (char *)NULL);
	}
	return TCL_ERROR;
    }
    memcpy(array, Blt_VecData(vector), sizeof(double) * Blt_VecLength(vector));
    evPtr->min = Blt_VecMin(vector);
    evPtr->max = Blt_VecMax(vector);
    evPtr->values = array;
    evPtr->nValues = Blt_VecLength(vector);
    /* FindRange(evPtr); */
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * VectorChangedProc --
 *
 * Results:
 *     	None.
 *
 * Side Effects:
 *	Graph is redrawn.
 *
 *---------------------------------------------------------------------------
 */
static void
VectorChangedProc(
    Tcl_Interp *interp, 
    ClientData clientData, 
    Blt_VectorNotify notify)
{
    ElemVector *evPtr = clientData;

    if (notify == BLT_VECTOR_NOTIFY_DESTROY) {
	FreeDataValues(evPtr);
    } else {
	Blt_Vector *vector;
	
	Blt_GetVectorById(interp, evPtr->vectorInfo.id, &vector);
	if (FetchVectorValues(NULL, evPtr, vector) != TCL_OK) {
	    return;
	}
    }
    {
	Element *elemPtr = evPtr->elemPtr;
	Graph *graphPtr = elemPtr->object.graphPtr;
	
	graphPtr->flags |= RESET_AXES;
	elemPtr->flags |= MAP_ITEM;
	if ((elemPtr->flags & (HIDDEN|DELETE_PENDING)) == 0) {
	    graphPtr->flags |= REDRAW_BACKING_STORE;
	    Blt_EventuallyRedrawGraph(graphPtr);
	}
    }
}

static int 
GetVector(Tcl_Interp *interp, ElemVector *evPtr, Tcl_Obj *vectorObjPtr)
{
    Blt_Vector *vector;
    char *vecName;
    
    vecName = Tcl_GetString(vectorObjPtr);
    if (!Blt_VectorExists2(interp, vecName)) {
	return TCL_CONTINUE;
    }
    evPtr->vectorInfo.id = Blt_AllocVectorId(interp, vecName);
    if (Blt_GetVectorById(interp, evPtr->vectorInfo.id, &vector) != TCL_OK) {
	return TCL_ERROR;
    }
    if (FetchVectorValues(interp, evPtr, vector) != TCL_OK) {
	goto error;
    }
    Blt_SetVectorChangedProc(evPtr->vectorInfo.id, VectorChangedProc, evPtr);
    evPtr->type = ELEM_SOURCE_VECTOR;
    return TCL_OK;
 error:
    FreeVector(evPtr);
    return TCL_ERROR;
}

static int
FetchTableValues(
    Tcl_Interp *interp,
    ElemVector *evPtr, 
    Blt_Dt_Column col)
{
    long i, j;
    double *array;
    Blt_Dt table;

    table = evPtr->tableInfo.table;
    array = Blt_Malloc(sizeof(double) * Blt_Dt_NumRows(table));
    if (array == NULL) {
	return TCL_ERROR;
    }
    for (j = 0, i = 1; i < Blt_Dt_NumRows(table); i++) {
	Blt_Dt_Row row;
	Tcl_Obj *objPtr;
	double value;

	row = Blt_Dt_GetRowByIndex(table, i);
	objPtr  = Blt_Dt_GetValue(table, row, col);
	if (objPtr == NULL) {
	    continue;
	}
	if (Tcl_GetDoubleFromObj(interp, objPtr, &value) != TCL_OK) {
	    return TCL_ERROR;
	}
	if (FINITE(value)) {
	    array[j] = value;
	    j++;
	}
    }
    if (evPtr->values != NULL) {
	Blt_Free(evPtr->values);
    }
    evPtr->nValues = j;
    evPtr->values = array;
    FindRange(evPtr);
    return TCL_OK;
}

static void
FreeTable(ElemVector *evPtr)
{
    Blt_HashEntry *hPtr;
    GraphInterpData *dataPtr;
    const char *tableName;

    if (evPtr->tableInfo.trace != NULL) {
	Blt_Dt_DeleteTrace(evPtr->tableInfo.trace);
    }
    if (evPtr->tableInfo.notifier != NULL) {
	Blt_Dt_DeleteNotifier(evPtr->tableInfo.notifier);
    }
    tableName = Blt_Dt_TableName(evPtr->tableInfo.table);
    dataPtr = GetGraphInterpData(evPtr->elemPtr->object.graphPtr->interp);
    hPtr = Blt_FindHashEntry(&dataPtr->tableClients, tableName);
    if (hPtr != NULL) {
	TableClient *clientPtr;

	clientPtr = Blt_GetHashValue(hPtr);
	clientPtr->refCount--;
	if (clientPtr->refCount == 0) {
	    if (evPtr->tableInfo.table != NULL) {
		Blt_Dt_Close(evPtr->tableInfo.table);
	    }
	    Blt_Free(clientPtr);
	    Blt_DeleteHashEntry(&dataPtr->tableClients, hPtr);
	}
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * TableNotifyProc --
 *
 *
 * Results:
 *     	None.
 *
 * Side Effects:
 *	Graph is redrawn.
 *
 *---------------------------------------------------------------------------
 */
static int
TableNotifyProc(ClientData clientData, Blt_Dt_NotifyEvent *eventPtr)
{
    ElemVector *evPtr = clientData;
    Element *elemPtr = evPtr->elemPtr;
    Graph *graphPtr = elemPtr->object.graphPtr;

    if ((eventPtr->type == DT_NOTIFY_COLUMN_DELETED) || 
	(FetchTableValues(graphPtr->interp, evPtr, 
			  (Blt_Dt_Column)eventPtr->header)) != TCL_OK) {
	FreeTable(evPtr);
	return TCL_ERROR;
    } 
    /* Always redraw the element. */
    graphPtr->flags |= RESET_AXES;
    elemPtr->flags |= MAP_ITEM;
    if ((elemPtr->flags & (HIDDEN|DELETE_PENDING)) == 0) {
	graphPtr->flags |= REDRAW_BACKING_STORE;
	Blt_EventuallyRedrawGraph(graphPtr);
    }
    return TCL_OK;
}
 
/*
 *---------------------------------------------------------------------------
 *
 * TableTraceProc --
 *
 *
 * Results:
 *     	None.
 *
 * Side Effects:
 *	Graph is redrawn.
 *
 *---------------------------------------------------------------------------
 */
static int
TableTraceProc(ClientData clientData, Blt_Dt_TraceEvent *eventPtr)
{
    ElemVector *evPtr = clientData;
    Element *elemPtr = evPtr->elemPtr;
    Graph *graphPtr = elemPtr->object.graphPtr;

    assert((Blt_Dt_Column)eventPtr->column == evPtr->tableInfo.column);

    if (FetchTableValues(eventPtr->interp, evPtr, eventPtr->column) != TCL_OK) {
	FreeTable(evPtr);
	return TCL_ERROR;
    }
    graphPtr->flags |= RESET_AXES;
    elemPtr->flags |= MAP_ITEM;
    if ((elemPtr->flags & (HIDDEN|DELETE_PENDING)) == 0) {
	graphPtr->flags |= REDRAW_BACKING_STORE;
	Blt_EventuallyRedrawGraph(graphPtr);
    }
    return TCL_OK;
}

static int
GetTable(
    Tcl_Interp *interp, 
    ElemVector *evPtr, 
    Tcl_Obj *tableObjPtr, 
    Tcl_Obj *colObjPtr)
{
    Blt_HashEntry *hPtr;
    DataTableInfo *infoPtr;
    GraphInterpData *dataPtr;
    TableClient *clientPtr;
    const char *tableName;
    int isNew;

    tableName = Tcl_GetString(tableObjPtr);
    if (!Blt_Dt_TableExists(interp, tableName)) {
	return TCL_CONTINUE;
    }
    memset(&evPtr->tableInfo, sizeof(DataTableInfo), 0);
    infoPtr = &evPtr->tableInfo;

    dataPtr = GetGraphInterpData(interp);
    hPtr = Blt_CreateHashEntry(&dataPtr->tableClients, tableName, &isNew);
    if (isNew) {
	if (Blt_Dt_Open(interp, tableName, &infoPtr->table) != TCL_OK) {
	    return TCL_ERROR;
	}
	clientPtr = Blt_MallocAssert(sizeof(TableClient));
	clientPtr->table = infoPtr->table;
	clientPtr->refCount = 1;
	Blt_SetHashValue(hPtr, clientPtr);
    } else {
	clientPtr = Blt_GetHashValue(hPtr);
	infoPtr->table = clientPtr->table;
	clientPtr->refCount++;
    }
    infoPtr->column = Blt_Dt_FindColumn(interp,infoPtr->table, colObjPtr);
    if (infoPtr->column == NULL) {
	goto error;
    }
    if (FetchTableValues(interp, evPtr, infoPtr->column) != TCL_OK) {
	goto error;
    }
    infoPtr->notifier = Blt_Dt_CreateColumnNotifier(interp, infoPtr->table, 
	infoPtr->column, DT_NOTIFY_COLUMN_CHANGED, TableNotifyProc, 
	(Blt_Dt_NotifierDeleteProc *)NULL, evPtr);
    infoPtr->trace = Blt_Dt_CreateColumnTrace(infoPtr->table, infoPtr->column, 
	(DT_TRACE_WRITES | DT_TRACE_UNSETS | DT_TRACE_CREATES), TableTraceProc,
	(Blt_Dt_TraceDeleteProc *)NULL, evPtr);
    evPtr->type = ELEM_SOURCE_TABLE;
    return TCL_OK;
 error:
    FreeTable(evPtr);
    return TCL_ERROR;
}

static int
ParseValues(
    Tcl_Interp *interp,
    Tcl_Obj *objPtr,
    int *nValuesPtr,
    double **arrayPtr)
{
    int objc;
    Tcl_Obj **objv;

    if (Tcl_ListObjGetElements(interp, objPtr, &objc, &objv) != TCL_OK) {
	return TCL_ERROR;
    }
    *arrayPtr = NULL;
    *nValuesPtr = 0;
    if (objc > 0) {
	double *array;
	double *valuePtr;
	int i;

	array = Blt_Malloc(sizeof(double) * objc);
	if (array == NULL) {
	    Tcl_AppendResult(interp, "can't allocate new vector", (char *)NULL);
	    return TCL_ERROR;
	}
	valuePtr = array;
	for (i = 0; i < objc; i++) {
	    if (Blt_ExprDoubleFromObj(interp, objv[i], valuePtr) != TCL_OK) {
		Blt_Free(array);
		return TCL_ERROR;
	    }
	    valuePtr++;
	}
	*arrayPtr = array;
	*nValuesPtr = objc;
    }
    return TCL_OK;
}

static void
FreeDataValues(ElemVector *evPtr)
{
    switch (evPtr->type) {
    case ELEM_SOURCE_VECTOR: 
	FreeVector(evPtr); break;
    case ELEM_SOURCE_TABLE:
	FreeTable(evPtr); break;
    case ELEM_SOURCE_VALUES:
	break;
    }
    if (evPtr->values != NULL) {
	Blt_Free(evPtr->values);
    }
    evPtr->values = NULL;
    evPtr->nValues = 0;
    evPtr->type = ELEM_SOURCE_VALUES;
}

/*
 *---------------------------------------------------------------------------
 *
 * FindRange --
 *
 *	Find the minimum, positive minimum, and maximum values in a
 *	given vector and store the results in the vector structure.
 *
 * Results:
 *     	None.
 *
 * Side Effects:
 *	Minimum, positive minimum, and maximum values are stored in
 *	the vector.
 *
 *---------------------------------------------------------------------------
 */
static void
FindRange(ElemVector *evPtr)
{
    int i;
    double *x;
    double min, max;

    if ((evPtr->nValues < 1) || (evPtr->values == NULL)) {
	return;			/* This shouldn't ever happen. */
    }
    x = evPtr->values;

    min = DBL_MAX, max = -DBL_MAX;
    for(i = 0; i < evPtr->nValues; i++) {
	if (FINITE(x[i])) {
	    min = max = x[i];
	    break;
	}
    }
    /*  Initialize values to track the vector range */
    for (/* empty */; i < evPtr->nValues; i++) {
	if (FINITE(x[i])) {
	    if (x[i] < min) {
		min = x[i];
	    } else if (x[i] > max) {
		max = x[i];
	    }
	}
    }
    evPtr->min = min, evPtr->max = max;
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_FindElemVectorMinimum --
 *
 *	Find the minimum, positive minimum, and maximum values in a
 *	given vector and store the results in the vector structure.
 *
 * Results:
 *     	None.
 *
 * Side Effects:
 *	Minimum, positive minimum, and maximum values are stored in
 *	the vector.
 *
 *---------------------------------------------------------------------------
 */
double
Blt_FindElemVectorMinimum(ElemVector *evPtr, double minLimit)
{
    int i;
    double min;

    min = DBL_MAX;
    for (i = 0; i < evPtr->nValues; i++) {
	double x;

	x = evPtr->values[i];
	if (x < 0.0) {
	    /* What do you do about negative values when using log
	     * scale values seems like a grey area.  Mirror. */
	    x = -x;
	}
	if ((x > minLimit) && (min > x)) {
	    min = x;
	}
    }
    if (min == DBL_MAX) {
	min = minLimit;
    }
    return min;
}

/*ARGSUSED*/
static void
FreeValues(
    ClientData clientData,	/* Not used. */
    Display *display,		/* Not used. */
    char *widgRec,
    int offset)
{
    ElemVector *evPtr = (ElemVector *)(widgRec + offset);

    FreeDataValues(evPtr);
}

/*
 *---------------------------------------------------------------------------
 *
 * ObjToValues --
 *
 *	Given a Tcl list of numeric expression representing the element
 *	values, convert into an array of double precision values. In
 *	addition, the minimum and maximum values are saved.  Since
 *	elastic values are allow (values which translate to the
 *	min/max of the graph), we must try to get the non-elastic
 *	minimum and maximum.
 *
 * Results:
 *	The return value is a standard Tcl result.  The vector is passed
 *	back via the evPtr.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToValues(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,		/* Interpreter to send results back to */
    Tk_Window tkwin,		/* Not used. */
    Tcl_Obj *objPtr,		/* Tcl list of expressions */
    char *widgRec,		/* Element record */
    int offset,			/* Offset to field in structure */
    int flags)			/* Not used. */
{
    ElemVector *evPtr = (ElemVector *)(widgRec + offset);
    Element *elemPtr = (Element *)widgRec;
    Tcl_Obj **objv;
    int objc;
    int result;

    if (Tcl_ListObjGetElements(interp, objPtr, &objc, &objv) != TCL_OK) {
	return TCL_ERROR;
    }
    evPtr->elemPtr = elemPtr;
    elemPtr->flags |= MAP_ITEM;
    result = TCL_CONTINUE;
    if (objc == 1) {
	result = GetVector(interp, evPtr, objv[0]);
    } else if (objc == 2) {
	result = GetTable(interp, evPtr, objv[0], objv[1]);
    }
    if (result == TCL_ERROR) {
	return TCL_ERROR;
    }
    if (result == TCL_CONTINUE) {
	double *values;
	int nValues;

	if (ParseValues(interp, objPtr, &nValues, &values) != TCL_OK) {
	    return TCL_ERROR;
	}
	FreeDataValues(evPtr);
	if (nValues > 0) {
	    evPtr->values = values;
	}
	evPtr->nValues = nValues;
	FindRange(evPtr);
	evPtr->type = ELEM_SOURCE_VALUES;
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ValuesToObj --
 *
 *	Convert the vector of floating point values into a Tcl list.
 *
 * Results:
 *	The string representation of the vector is returned.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
ValuesToObj(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,
    Tk_Window tkwin,		/* Not used. */
    char *widgRec,		/* Element record */
    int offset,			/* Offset to field in structure */
    int flags)			/* Not used. */
{
    ElemVector *evPtr = (ElemVector *)(widgRec + offset);

    switch (evPtr->type) {
    case ELEM_SOURCE_VECTOR:
	{
	    const char *vecName;
	    
	    vecName = Blt_NameOfVectorId(evPtr->vectorInfo.id);
	    return Tcl_NewStringObj(vecName, -1);
	}
    case ELEM_SOURCE_TABLE:
	{
	    Tcl_Obj *listObjPtr;
	    const char *tableName;
	    size_t i;
	    
	    listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
	    tableName = Blt_Dt_TableName(evPtr->tableInfo.table);
	    Tcl_ListObjAppendElement(interp, listObjPtr, 
		Tcl_NewStringObj(tableName, -1));
	    
	    i = Blt_Dt_ColumnIndex(evPtr->tableInfo.column);
	    Tcl_ListObjAppendElement(interp, listObjPtr, Tcl_NewLongObj(i));
	    return listObjPtr;
	}
    case ELEM_SOURCE_VALUES:
	{
	    Tcl_Obj *listObjPtr;
	    double *vp, *vend; 
	    
	    listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
	    for (vp = evPtr->values, vend = vp + evPtr->nValues; vp < vend; 
		 vp++) {
		Tcl_ListObjAppendElement(interp, listObjPtr, 
					 Tcl_NewDoubleObj(*vp));
	    }
	    return listObjPtr;
	}
    }
    return Tcl_NewStringObj("???", 3);
}

/*ARGSUSED*/
static void
FreeValuePairs(
    ClientData clientData,	/* Not used. */
    Display *display,		/* Not used. */
    char *widgRec,
    int offset)			/* Not used. */
{
    Element *elemPtr = (Element *)widgRec;

    FreeDataValues(&elemPtr->x);
    FreeDataValues(&elemPtr->y);
}


/*
 *---------------------------------------------------------------------------
 *
 * ObjToValuePairs --
 *
 *	This procedure is like ObjToValues except that it interprets
 *	the list of numeric expressions as X Y coordinate pairs.  The
 *	minimum and maximum for both the X and Y vectors are
 *	determined.
 *
 * Results:
 *	The return value is a standard Tcl result.  The vectors are
 *	passed back via the widget record (elemPtr).
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToValuePairs(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,		/* Interpreter to send results back to */
    Tk_Window tkwin,		/* Not used. */
    Tcl_Obj *objPtr,		/* Tcl list of numeric expressions */
    char *widgRec,		/* Element record */
    int offset,			/* Not used. */
    int flags)			/* Not used. */
{
    Element *elemPtr = (Element *)widgRec;
    double *values;
    int nValues;
    size_t newSize;

    if (ParseValues(interp, objPtr, &nValues, &values) != TCL_OK) {
	return TCL_ERROR;
    }
    if (nValues & 1) {
	Tcl_AppendResult(interp, "odd number of data points", (char *)NULL);
	Blt_Free(values);
	return TCL_ERROR;
    }
    nValues /= 2;
    newSize = nValues * sizeof(double);
    FreeDataValues(&elemPtr->x);
    FreeDataValues(&elemPtr->y);
    if (newSize > 0) {
	double *vp;
	int i;

	elemPtr->x.values = Blt_MallocAssert(newSize);
	elemPtr->y.values = Blt_MallocAssert(newSize);
	elemPtr->x.nValues = elemPtr->y.nValues = nValues;
	for (vp = values, i = 0; i < nValues; i++) {
	    elemPtr->x.values[i] = *vp++;
	    elemPtr->y.values[i] = *vp++;
	}
	Blt_Free(values);
	FindRange(&elemPtr->x);
	FindRange(&elemPtr->y);
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ValuePairsToObj --
 *
 *	Convert pairs of floating point values in the X and Y arrays
 *	into a Tcl list.
 *
 * Results:
 *	The return value is a string (Tcl list).
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
ValuePairsToObj(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,
    Tk_Window tkwin,		/* Not used. */
    char *widgRec,		/* Element information record */
    int offset,			/* Not used. */
    int flags)			/* Not used. */
{
    Element *elemPtr = (Element *)widgRec;
    Tcl_Obj *listObjPtr;
    int i;
    int length;

    length = NUMBEROFPOINTS(elemPtr);
    listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
    for (i = 0; i < length; i++) {
	Tcl_ListObjAppendElement(interp, listObjPtr, 
				 Tcl_NewDoubleObj(elemPtr->x.values[i]));
	Tcl_ListObjAppendElement(interp, listObjPtr, 
				 Tcl_NewDoubleObj(elemPtr->y.values[i]));
    }
    return listObjPtr;
}

/*
 *---------------------------------------------------------------------------
 *
 * ObjToAlong --
 *
 *	Given a Tcl list of numeric expression representing the element
 *	values, convert into an array of double precision values. In
 *	addition, the minimum and maximum values are saved.  Since
 *	elastic values are allow (values which translate to the
 *	min/max of the graph), we must try to get the non-elastic
 *	minimum and maximum.
 *
 * Results:
 *	The return value is a standard Tcl result.  The vector is passed
 *	back via the evPtr.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToAlong(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,		/* Interpreter to send results back to */
    Tk_Window tkwin,		/* Not used. */
    Tcl_Obj *objPtr,		/* String representation of value. */
    char *widgRec,		/* Widget record. */
    int offset,			/* Offset to field in structure */
    int flags)			/* Not used. */
{
    int *intPtr = (int *)(widgRec + offset);
    char *string;

    string = Tcl_GetString(objPtr);
    if ((string[0] == 'x') && (string[1] == '\0')) {
	*intPtr = SEARCH_X;
    } else if ((string[0] == 'y') && (string[1] == '\0')) { 
	*intPtr = SEARCH_Y;
    } else if ((string[0] == 'b') && (strcmp(string, "both") == 0)) {
	*intPtr = SEARCH_BOTH;
    } else {
	Tcl_AppendResult(interp, "bad along value \"", string, "\"",
			 (char *)NULL);
	return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * AlongToObj --
 *
 *	Convert the vector of floating point values into a Tcl list.
 *
 * Results:
 *	The string representation of the vector is returned.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
AlongToObj(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,		/* Not used. */
    Tk_Window tkwin,		/* Not used. */
    char *widgRec,		/* Widget record */
    int offset,			/* Offset to field in structure */
    int flags)			/* Not used. */
{
    int along = *(int *)(widgRec + offset);
    Tcl_Obj *objPtr;

    switch (along) {
    case SEARCH_X:
	objPtr = Tcl_NewStringObj("x", 1);
	break;
    case SEARCH_Y:
	objPtr = Tcl_NewStringObj("y", 1);
	break;
    case SEARCH_BOTH:
	objPtr = Tcl_NewStringObj("both", 4);
	break;
    default:
	objPtr = Tcl_NewStringObj("unknown along value", 4);
	break;
    }
    return objPtr;
}

void
Blt_FreeStylePalette(Blt_Chain stylePalette)
{
    Blt_ChainLink link;

    /* Skip the first slot. It contains the built-in "normal" pen of
     * the element.  */
    link = Blt_ChainFirstLink(stylePalette);
    if (link != NULL) {
	Blt_ChainLink next;

	for (link = Blt_ChainNextLink(link); link != NULL; link = next) {
	    PenStyle *stylePtr;

	    next = Blt_ChainNextLink(link);
	    stylePtr = Blt_ChainGetValue(link);
	    Blt_FreePen(stylePtr->penPtr);
	    Blt_ChainDeleteLink(stylePalette, link);
	}
    }
}

/*ARGSUSED*/
static void
FreeStyles(
    ClientData clientData,	/* Not used. */
    Display *display,		/* Not used. */
    char *widgRec,
    int offset)
{
    Blt_Chain stylePalette = *(Blt_Chain *)(widgRec + offset);

    Blt_FreeStylePalette(stylePalette);
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_ObjToStyles --
 *
 *	Parse the list of style names.
 *
 * Results:
 *	The return value is a standard Tcl result.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToStyles(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,		/* Interpreter to send results back to */
    Tk_Window tkwin,		/* Not used. */
    Tcl_Obj *objPtr,		/* String representing style list */
    char *widgRec,		/* Element information record */
    int offset,			/* Offset to field in structure */
    int flags)			/* Not used. */
{
    Blt_Chain stylePalette = *(Blt_Chain *)(widgRec + offset);
    Blt_ChainLink link;
    Element *elemPtr = (Element *)(widgRec);
    PenStyle *stylePtr;
    Tcl_Obj **objv;
    int objc;
    int i;
    size_t size = (size_t)clientData;


    if (Tcl_ListObjGetElements(interp, objPtr, &objc, &objv) != TCL_OK) {
	return TCL_ERROR;
    }
    /* Reserve the first entry for the "normal" pen. We'll set the
     * style later */
    Blt_FreeStylePalette(stylePalette);
    link = Blt_ChainFirstLink(stylePalette);
    if (link == NULL) {
	link = Blt_ChainAllocLink(size);
	Blt_ChainLinkBefore(stylePalette, link, NULL);
    }
    stylePtr = Blt_ChainGetValue(link);
    stylePtr->penPtr = elemPtr->normalPenPtr;
    for (i = 0; i < objc; i++) {
	link = Blt_ChainAllocLink(size);
	stylePtr = Blt_ChainGetValue(link);
	stylePtr->weight.min = (double)i;
	stylePtr->weight.max = (double)i + 1.0;
	stylePtr->weight.range = 1.0;
	if (GetPenStyleFromObj(interp, elemPtr->object.graphPtr, objv[i], 
		elemPtr->object.classId, (PenStyle *)stylePtr) != TCL_OK) {
	    Blt_FreeStylePalette(stylePalette);
	    return TCL_ERROR;
	}
	Blt_ChainLinkBefore(stylePalette, link, NULL);
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * StylesToObj --
 *
 *	Convert the style information into a Tcl_Obj.
 *
 * Results:
 *	The string representing the style information is returned.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
StylesToObj(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,
    Tk_Window tkwin,		/* Not used. */
    char *widgRec,		/* Element information record */
    int offset,			/* Offset to field in structure */
    int flags)			/* Not used. */
{
    Blt_Chain stylePalette = *(Blt_Chain *)(widgRec + offset);
    Blt_ChainLink link;
    Tcl_Obj *listObjPtr;

    listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
    link = Blt_ChainFirstLink(stylePalette);
    if (link != NULL) {
	/* Skip the first style (it's the default) */
	for (link = Blt_ChainNextLink(link); link != NULL; 
	     link = Blt_ChainNextLink(link)) {
	    PenStyle *stylePtr;
	    Tcl_Obj *subListObjPtr;

	    stylePtr = Blt_ChainGetValue(link);
	    subListObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
	    Tcl_ListObjAppendElement(interp, subListObjPtr, 
		Tcl_NewStringObj(stylePtr->penPtr->name, -1));
	    Tcl_ListObjAppendElement(interp, subListObjPtr, 
				     Tcl_NewDoubleObj(stylePtr->weight.min));
	    Tcl_ListObjAppendElement(interp, subListObjPtr, 
				     Tcl_NewDoubleObj(stylePtr->weight.max));
	    Tcl_ListObjAppendElement(interp, listObjPtr, subListObjPtr);
	}
    }
    return listObjPtr;
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_StyleMap --
 *
 *	Creates an array of style indices and fills it based on the weight
 *	of each data point.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Memory is freed and allocated for the index array.
 *
 *---------------------------------------------------------------------------
 */

PenStyle **
Blt_StyleMap(Element *elemPtr)
{
    int i;
    int nWeights;		/* Number of weights to be examined.
				 * If there are more data points than
				 * weights, they will default to the
				 * normal pen. */

    PenStyle **dataToStyle;	/* Directory of styles.  Each array
				 * element represents the style for
				 * the data point at that index */
    Blt_ChainLink link;
    PenStyle *stylePtr;
    double *w;			/* Weight vector */
    int nPoints;

    nPoints = NUMBEROFPOINTS(elemPtr);
    nWeights = MIN(elemPtr->w.nValues, nPoints);
    w = elemPtr->w.values;
    link = Blt_ChainFirstLink(elemPtr->stylePalette);
    stylePtr = Blt_ChainGetValue(link);

    /* 
     * Create a style mapping array (data point index to style), 
     * initialized to the default style.
     */
    dataToStyle = Blt_MallocAssert(nPoints * sizeof(PenStyle *));
    for (i = 0; i < nPoints; i++) {
	dataToStyle[i] = stylePtr;
    }

    for (i = 0; i < nWeights; i++) {
	for (link = Blt_ChainLastLink(elemPtr->stylePalette); link != NULL; 
	     link = Blt_ChainPrevLink(link)) {
	    stylePtr = Blt_ChainGetValue(link);

	    if (stylePtr->weight.range > 0.0) {
		double norm;

		norm = (w[i] - stylePtr->weight.min) / stylePtr->weight.range;
		if (((norm - 1.0) <= DBL_EPSILON) && 
		    (((1.0 - norm) - 1.0) <= DBL_EPSILON)) {
		    dataToStyle[i] = stylePtr;
		    break;		/* Done: found range that matches. */
		}
	    }
	}
    }
    return dataToStyle;
}


/*
 *---------------------------------------------------------------------------
 *
 * GetIndex --
 *
 *	Given a string representing the index of a pair of x,y
 *	coordinates, return the numeric index.
 *
 * Results:
 *     	A standard TCL result.
 *
 *---------------------------------------------------------------------------
 */
static int
GetIndex(Tcl_Interp *interp, Element *elemPtr, Tcl_Obj *objPtr, int *indexPtr)
{
    char *string;

    string = Tcl_GetString(objPtr);
    if ((*string == 'e') && (strcmp("end", string) == 0)) {
	*indexPtr = NUMBEROFPOINTS(elemPtr) - 1;
    } else if (Blt_ExprIntFromObj(interp, objPtr, indexPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * GetElementFromObj --
 *
 *	Find the element represented the given name,  returning
 *	a pointer to its data structure via elemPtrPtr.
 *
 * Results:
 *     	A standard TCL result.
 *
 *---------------------------------------------------------------------------
 */
static int
GetElementFromObj(
    Tcl_Interp *interp,
    Graph *graphPtr,
    Tcl_Obj *objPtr,
    Element **elemPtrPtr)
{
    Blt_HashEntry *hPtr;
    char *name;

    name = Tcl_GetString(objPtr);
    hPtr = Blt_FindHashEntry(&graphPtr->elements.table, name);
    if (hPtr == NULL) {
	if (interp != NULL) {
 	    Tcl_AppendResult(interp, "can't find element \"", name,
			     "\" in \"", Tk_PathName(graphPtr->tkwin), "\"", (char *)NULL);
	}
	return TCL_ERROR;
    }
    *elemPtrPtr = (Element *)Blt_GetHashValue(hPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * DestroyElement --
 *
 *	Add a new element to the graph.
 *
 * Results:
 *	The return value is a standard Tcl result.
 *
 *---------------------------------------------------------------------------
 */
static void
DestroyElement(Element *elemPtr)
{
    Blt_ChainLink link;
    Graph *graphPtr = elemPtr->object.graphPtr;

    Blt_DeleteBindings(graphPtr->bindTable, elemPtr);
    Blt_LegendRemoveElement(graphPtr->legend, elemPtr);

    Blt_FreeOptions(elemPtr->configSpecs, (char *)elemPtr,graphPtr->display, 0);
    /*
     * Call the element's own destructor to release the memory and
     * resources allocated for it.
     */
    (*elemPtr->procsPtr->destroyProc) (graphPtr, elemPtr);

    /* Remove it also from the element display list */
    for (link = Blt_ChainFirstLink(graphPtr->elements.displayList); 
	 link != NULL; link = Blt_ChainNextLink(link)) {
	if (elemPtr == Blt_ChainGetValue(link)) {
	    Blt_ChainDeleteLink(graphPtr->elements.displayList, link);
	    if ((elemPtr->flags & (HIDDEN|DELETE_PENDING)) == 0) {
		graphPtr->flags |= RESET_WORLD;
		Blt_EventuallyRedrawGraph(graphPtr);
	    }
	    break;
	}
    }
    /* Remove the element for the graph's hash table of elements */
    if (elemPtr->hashPtr != NULL) {
	Blt_DeleteHashEntry(&graphPtr->elements.table, elemPtr->hashPtr);
    }
    if (elemPtr->object.name != NULL) {
	Blt_Free(elemPtr->object.name);
    }
    if (elemPtr->label != NULL) {
	Blt_Free(elemPtr->label);
    }
    Blt_Free(elemPtr);
}

static void
FreeElement(DestroyData data)
{
    Element *elemPtr = (Element *)data;
    DestroyElement(elemPtr);
}

/*
 *---------------------------------------------------------------------------
 *
 * CreateElement --
 *
 *	Add a new element to the graph.
 *
 * Results:
 *	The return value is a standard Tcl result.
 *
 *---------------------------------------------------------------------------
 */
static int
CreateElement(
    Graph *graphPtr,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv,
    ClassId classId)
{
    Element *elemPtr;
    Blt_HashEntry *hPtr;
    int isNew;
    char *string;

    string = Tcl_GetString(objv[3]);
    if (string[0] == '-') {
	Tcl_AppendResult(graphPtr->interp, "name of element \"", string, 
			 "\" can't start with a '-'", (char *)NULL);
	return TCL_ERROR;
    }
    hPtr = Blt_CreateHashEntry(&graphPtr->elements.table, string, &isNew);
    if (!isNew) {
	Tcl_AppendResult(interp, "element \"", string, 
			 "\" already exists in \"", Tcl_GetString(objv[0]), 
			 "\"", (char *)NULL);
	return TCL_ERROR;
    }
    if (classId == CID_ELEM_BAR) {
	elemPtr = Blt_BarElement(graphPtr, string, classId);
    } else { 
	/* Stripcharts are line graphs with some options enabled. */	
	elemPtr = Blt_LineElement(graphPtr, string, classId);
    }
    elemPtr->hashPtr = hPtr;
    Blt_SetHashValue(hPtr, elemPtr);

    if (Blt_ConfigureComponentFromObj(interp, graphPtr->tkwin, 
	elemPtr->object.name, "Element", elemPtr->configSpecs, 
	objc - 4, objv + 4, (char *)elemPtr, 0) != TCL_OK) {
	DestroyElement(elemPtr);
	return TCL_ERROR;
    }
    (*elemPtr->procsPtr->configProc) (graphPtr, elemPtr);
    Blt_ChainPrepend(graphPtr->elements.displayList, elemPtr);
    if ((elemPtr->flags & (HIDDEN|DELETE_PENDING)) == 0) {
	/* If the new element isn't hidden then redraw the graph.  */
	graphPtr->flags |= REDRAW_BACKING_STORE;
	Blt_EventuallyRedrawGraph(graphPtr);
    }
    elemPtr->flags |= MAP_ITEM;
    graphPtr->flags |= RESET_AXES;
    Tcl_SetObjResult(interp, objv[3]);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * RebuildDisplayList --
 *
 *	Given a Tcl list of element names, this procedure rebuilds the
 *	display list, ignoring invalid element names. This list describes
 *	not only only which elements to draw, but in what order.  This is
 *	only important for bar and pie charts.
 *
 * Results:
 *	The return value is a standard Tcl result.  Only if the Tcl list
 *	can not be split, a TCL_ERROR is returned and interp->result contains
 *	an error message.
 *
 * Side effects:
 *	The graph is eventually redrawn using the new display list.
 *
 *---------------------------------------------------------------------------
 */
static int
RebuildDisplayList(
    Tcl_Interp *interp,
    Graph *graphPtr,		/* Graph widget record */
    Tcl_Obj *objPtr)		/* Tcl list of element names */
{
    Tcl_Obj **objv;		/* Broken out array of element names */
    int objc;			/* Number of names found in Tcl name list */
    int i;

    if (Tcl_ListObjGetElements(interp, objPtr, &objc, &objv) != TCL_OK) {
	return TCL_ERROR;
    }
    /* Clear the display list.  */
    Blt_ChainReset(graphPtr->elements.displayList);

    /* 
     * Then rebuild it, checking that each name it exists (ignore bad
     * element names).  Keep the "hidden" flags as it was.  
     */
    for (i = 0; i < objc; i++) {
	Element *elemPtr;	/* Element information record */

	if (GetElementFromObj((Tcl_Interp *)NULL, graphPtr, objv[i], 
		&elemPtr) == TCL_OK) {
	    Blt_ChainAppend(graphPtr->elements.displayList, elemPtr);
	}
    }
    graphPtr->flags |= RESET_WORLD;
    Blt_EventuallyRedrawGraph(graphPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_DestroyElements --
 *
 *	Removes all the graph's elements. This routine is called when
 *	the graph is destroyed.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Memory allocated for the graph's elements is freed.
 *
 *---------------------------------------------------------------------------
 */
void
Blt_DestroyElements(Graph *graphPtr)
{
    Blt_HashEntry *hPtr;
    Blt_HashSearch cursor;
    Element *elemPtr;

    for (hPtr = Blt_FirstHashEntry(&graphPtr->elements.table, &cursor);
	 hPtr != NULL; hPtr = Blt_NextHashEntry(&cursor)) {
	elemPtr = (Element *)Blt_GetHashValue(hPtr);
	elemPtr->hashPtr = NULL;
	DestroyElement(elemPtr);
    }
    Blt_DeleteHashTable(&graphPtr->elements.table);
    Blt_DeleteHashTable(&graphPtr->elements.tagTable);
    Blt_ChainDestroy(graphPtr->elements.displayList);
}

void
Blt_MapElements(Graph *graphPtr)
{
    Blt_ChainLink link;

    if (graphPtr->mode != MODE_INFRONT) {
	Blt_ResetStacks(graphPtr);
    }
    for (link = Blt_ChainFirstLink(graphPtr->elements.displayList); 
	 link != NULL; link = Blt_ChainNextLink(link)) {
	Element *elemPtr;

	elemPtr = Blt_ChainGetValue(link);
	if (elemPtr->flags & (HIDDEN|DELETE_PENDING)) {
	    continue;
	}
	if ((graphPtr->flags & MAP_ALL) || (elemPtr->flags & MAP_ITEM)) {
	    (*elemPtr->procsPtr->mapProc) (graphPtr, elemPtr);
	    elemPtr->flags &= ~MAP_ITEM;
	}
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_DrawElements --
 *
 *	Calls the individual element drawing routines for each
 *	element.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Elements are drawn into the drawable (pixmap) which will
 *	eventually be displayed in the graph window.
 *
 *---------------------------------------------------------------------------
 */
void
Blt_DrawElements(Graph *graphPtr, Drawable drawable)
{
    Blt_ChainLink link;

    for (link = Blt_ChainFirstLink(graphPtr->elements.displayList); 
	 link != NULL; link = Blt_ChainNextLink(link)) {
	Element *elemPtr;

	elemPtr = Blt_ChainGetValue(link);
	if ((elemPtr->flags & (HIDDEN|DELETE_PENDING)) == 0) {
	    (*elemPtr->procsPtr->drawNormalProc) (graphPtr, drawable, elemPtr);
	}
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_DrawActiveElements --
 *
 *	Calls the individual element drawing routines to display
 *	the active colors for each element.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Elements are drawn into the drawable (pixmap) which will
 *	eventually be displayed in the graph window.
 *
 *---------------------------------------------------------------------------
 */
void
Blt_DrawActiveElements(Graph *graphPtr, Drawable drawable)
{
    Blt_ChainLink link;

    for (link = Blt_ChainFirstLink(graphPtr->elements.displayList); 
	 link != NULL; link = Blt_ChainNextLink(link)) {
	Element *elemPtr;

	elemPtr = Blt_ChainGetValue(link);
	if ((elemPtr->flags & (HIDDEN|ACTIVE|DELETE_PENDING)) == ACTIVE) {
	    (*elemPtr->procsPtr->drawActiveProc) (graphPtr, drawable, elemPtr);
	}
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_ElementsToPostScript --
 *
 *	Generates PostScript output for each graph element in the
 *	element display list.
 *
 *---------------------------------------------------------------------------
 */
void
Blt_ElementsToPostScript(Graph *graphPtr, Blt_Ps ps)
{
    Blt_ChainLink link;

    for (link = Blt_ChainFirstLink(graphPtr->elements.displayList); 
	 link != NULL; link = Blt_ChainNextLink(link)) {
	Element *elemPtr;

	elemPtr = Blt_ChainGetValue(link);
	if (elemPtr->flags & (HIDDEN|DELETE_PENDING)) {
	    continue;
	}
	/* Comment the PostScript to indicate the start of the element */
	Blt_Ps_Format(ps, "\n%% Element \"%s\"\n\n", 
			       elemPtr->object.name);
	(*elemPtr->procsPtr->printNormalProc) (graphPtr, ps, elemPtr);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_ActiveElementsToPostScript --
 *
 *---------------------------------------------------------------------------
 */
void
Blt_ActiveElementsToPostScript( Graph *graphPtr, Blt_Ps ps)
{
    Blt_ChainLink link;

    for (link = Blt_ChainFirstLink(graphPtr->elements.displayList); 
	 link != NULL; link = Blt_ChainNextLink(link)) {
	Element *elemPtr;

	elemPtr = Blt_ChainGetValue(link);
	if ((elemPtr->flags & (DELETE_PENDING|HIDDEN|ACTIVE)) == ACTIVE) {
	    Blt_Ps_Format(ps, "\n%% Active Element \"%s\"\n\n",
		elemPtr->object.name);
	    (*elemPtr->procsPtr->printActiveProc) (graphPtr, ps, elemPtr);
	}
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * ActivateOp --
 *
 *	Marks data points of elements (given by their index) as active.
 *
 * Results:
 *	Returns TCL_OK if no errors occurred.
 *
 *---------------------------------------------------------------------------
 */
static int
ActivateOp(
    Graph *graphPtr,		/* Graph widget */
    Tcl_Interp *interp,		/* Interpreter to report errors to */
    int objc,			/* Number of element names */
    Tcl_Obj *const *objv)	/* List of element names */
{
    Element *elemPtr;
    int i;
    int *indices;
    int nIndices;

    if (objc == 3) {
	Blt_HashEntry *hPtr;
	Blt_HashSearch cursor;
	Tcl_Obj *listObjPtr;

	listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
	/* List all the currently active elements */
	for (hPtr = Blt_FirstHashEntry(&graphPtr->elements.table, &cursor);
	     hPtr != NULL; hPtr = Blt_NextHashEntry(&cursor)) {
	    elemPtr = (Element *)Blt_GetHashValue(hPtr);
	    if (elemPtr->flags & ACTIVE) {
		Tcl_ListObjAppendElement(interp, listObjPtr, 
			Tcl_NewStringObj(elemPtr->object.name, -1));
	    }
	}
	Tcl_SetObjResult(interp, listObjPtr);
	return TCL_OK;
    }
    if (GetElementFromObj(interp, graphPtr, objv[3], &elemPtr) != TCL_OK) {
	return TCL_ERROR;	/* Can't find named element */
    }
    elemPtr->flags |= ACTIVE | ACTIVE_PENDING;

    indices = NULL;
    nIndices = -1;
    if (objc > 4) {
	int *activePtr;

	nIndices = objc - 4;
	activePtr = indices = Blt_MallocAssert(sizeof(int) * nIndices);
	for (i = 4; i < objc; i++) {
	    if (GetIndex(interp, elemPtr, objv[i], activePtr) != TCL_OK) {
		return TCL_ERROR;
	    }
	    activePtr++;
	}
    }
    if (elemPtr->activeIndices != NULL) {
	Blt_Free(elemPtr->activeIndices);
    }
    elemPtr->nActiveIndices = nIndices;
    elemPtr->activeIndices = indices;
    Blt_EventuallyRedrawGraph(graphPtr);
    return TCL_OK;
}

ClientData
Blt_MakeElementTag(Graph *graphPtr, const char *tagName)
{
    Blt_HashEntry *hPtr;
    int isNew;

    hPtr = Blt_CreateHashEntry(&graphPtr->elements.tagTable, tagName, &isNew);
    return Blt_GetHashKey(&graphPtr->elements.tagTable, hPtr);
}

/*
 *---------------------------------------------------------------------------
 *
 * BindOp --
 *
 *	.g element bind elemName sequence command
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
	Blt_HashEntry *hPtr;
	Blt_HashSearch cursor;
	char *tagName;
	Tcl_Obj *listObjPtr;

	listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
	for (hPtr = Blt_FirstHashEntry(&graphPtr->elements.tagTable, &cursor);
	     hPtr != NULL; hPtr = Blt_NextHashEntry(&cursor)) {
	    tagName = Blt_GetHashKey(&graphPtr->elements.tagTable, hPtr);
	    Tcl_ListObjAppendElement(interp, listObjPtr, 
				     Tcl_NewStringObj(tagName, -1));
	}
	Tcl_SetObjResult(interp, listObjPtr);
	return TCL_OK;
    }
    return Blt_ConfigureBindingsFromObj(interp, graphPtr->bindTable,
	Blt_MakeElementTag(graphPtr, Tcl_GetString(objv[3])), 
	objc - 4, objv + 4);
}

/*
 *---------------------------------------------------------------------------
 *
 * CreateOp --
 *
 *	Add a new element to the graph (using the default type of the
 *	graph).
 *
 * Results:
 *	The return value is a standard Tcl result.
 *
 *---------------------------------------------------------------------------
 */
static int
CreateOp(
    Graph *graphPtr,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv,
    ClassId classId)
{
    return CreateElement(graphPtr, interp, objc, objv, classId);
}

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
    Element *elemPtr;

    if (GetElementFromObj(interp, graphPtr, objv[3], &elemPtr) != TCL_OK) {
	return TCL_ERROR;	/* Can't find named element */
    }
    if (Blt_ConfigureValueFromObj(interp, graphPtr->tkwin, elemPtr->configSpecs,
				  (char *)elemPtr, objv[4], 0) != TCL_OK) {
	return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ClosestOp --
 *
 *	Find the element closest to the specified screen coordinates.
 *	Options:
 *	-halo		Consider points only with this maximum distance
 *			from the picked coordinate.
 *	-interpolate	Find closest point along element traces, not just
 *			data points.
 *	-along
 *
 * Results:
 *	A standard Tcl result. If an element could be found within
 *	the halo distance, the interpreter result is "1", otherwise
 *	"0".  If a closest element exists, the designated Tcl array
 *	variable will be set with the following information:
 *
 *	1) the element name,
 *	2) the index of the closest point,
 *	3) the distance (in screen coordinates) from the picked X-Y
 *	   coordinate and the closest point,
 *	4) the X coordinate (graph coordinate) of the closest point,
 *	5) and the Y-coordinate.
 *
 *---------------------------------------------------------------------------
 */

static Blt_ConfigSpec closestSpecs[] = {
    {BLT_CONFIG_PIXELS_NNEG, "-halo", (char *)NULL, (char *)NULL,
	(char *)NULL, Blt_Offset(ClosestSearch, halo), 0},
    {BLT_CONFIG_BOOLEAN, "-interpolate", (char *)NULL, (char *)NULL,
	(char *)NULL, Blt_Offset(ClosestSearch, mode), 0 }, 
    {BLT_CONFIG_CUSTOM, "-along", (char *)NULL, (char *)NULL,
	(char *)NULL, Blt_Offset(ClosestSearch, along), 0, &alongOption},
    {BLT_CONFIG_END, (char *)NULL, (char *)NULL, (char *)NULL,
	(char *)NULL, 0, 0}
};

static int
ClosestOp(
    Graph *graphPtr,		/* Graph widget */
    Tcl_Interp *interp,		/* Interpreter to report results to */
    int objc,			/* Number of element names */
    Tcl_Obj *const *objv)	/* List of element names */
{
    Element *elemPtr;
    ClosestSearch search;
    int i, x, y;
    char *string;

    if (graphPtr->flags & RESET_AXES) {
	Blt_ResetAxes(graphPtr);
    }
    if (Tcl_GetIntFromObj(interp, objv[3], &x) != TCL_OK) {
	Tcl_AppendResult(interp, ": bad window x-coordinate", (char *)NULL);
	return TCL_ERROR;
    }
    if (Tcl_GetIntFromObj(interp, objv[4], &y) != TCL_OK) {
	Tcl_AppendResult(interp, ": bad window y-coordinate", (char *)NULL);
	return TCL_ERROR;
    }
    if (graphPtr->inverted) {
	int temp;

	temp = x, x = y, y = temp;
    }
    for (i = 5; i < objc; i += 2) {	/* Count switches-value pairs */
	string = Tcl_GetString(objv[i]);
	if ((string[0] != '-') || 
	    ((string[1] == '-') && (string[2] == '\0'))) {
	    break;
	}
    }
    if (i > objc) {
	i = objc;
    }

    search.mode = SEARCH_POINTS;
    search.halo = graphPtr->halo;
    search.index = -1;
    search.along = SEARCH_BOTH;
    search.x = x;
    search.y = y;

    if (Blt_ConfigureWidgetFromObj(interp, graphPtr->tkwin, closestSpecs, i - 5,
	objv + 5, (char *)&search, BLT_CONFIG_OBJV_ONLY) != TCL_OK) {
	return TCL_ERROR;	/* Error occurred processing an option. */
    }
    if (i < objc) {
	string = Tcl_GetString(objv[i]);
	if (string[0] == '-') {
	    i++;			/* Skip "--" */
	}
    }
    search.dist = (double)(search.halo + 1);

    if (i < objc) {
	Blt_ChainLink link;
	int found;

	for ( /* empty */ ; i < objc; i++) {
	    if (GetElementFromObj(interp, graphPtr, objv[i], &elemPtr) 
		!= TCL_OK) {
		return TCL_ERROR; /* Can't find named element */
	    }
	    if (elemPtr->flags & (HIDDEN|MAP_ITEM|DELETE_PENDING)) {
		continue;
	    }
	    found = FALSE;
	    for (link = Blt_ChainFirstLink(graphPtr->elements.displayList);
		 link == NULL; link = Blt_ChainNextLink(link)) {
		if (elemPtr == Blt_ChainGetValue(link)) {
		    found = TRUE;
		    break;
		}
	    }
	    if ((!found) || (elemPtr->flags & HIDDEN)) {
		Tcl_AppendResult(interp, "element \"", Tcl_GetString(objv[i]), 
				 "\" is hidden", (char *)NULL);
		return TCL_ERROR;	/* Element isn't visible */
	    }
	    (*elemPtr->procsPtr->closestProc) (graphPtr, elemPtr, &search);
	}
    } else {
	Blt_ChainLink link;

	/* 
	 * Find the closest point from the set of displayed elements,
	 * searching the display list from back to front.  That way if
	 * the points from two different elements overlay each other
	 * exactly, the last one picked will be the topmost.  
	 */
	for (link = Blt_ChainLastLink(graphPtr->elements.displayList); 
	     link != NULL; link = Blt_ChainPrevLink(link)) {
	    elemPtr = Blt_ChainGetValue(link);
	    /* Check if the X or Y vectors have notifications pending. */
	    if (elemPtr->flags & (HIDDEN|MAP_ITEM|DELETE_PENDING)) {
		continue;
	    }
	    (*elemPtr->procsPtr->closestProc) (graphPtr, elemPtr, &search);
	}
    }
    if (search.dist < (double)search.halo) {
	Tcl_Obj *listObjPtr;
	/*
	 *  Return a list of name value pairs.
	 */
	listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
	Tcl_ListObjAppendElement(interp, listObjPtr, 
				 Tcl_NewStringObj("name", -1));
	Tcl_ListObjAppendElement(interp, listObjPtr, 
		Tcl_NewStringObj(search.elemPtr->object.name, -1)); 
	Tcl_ListObjAppendElement(interp, listObjPtr, 
				 Tcl_NewStringObj("index", -1));
	Tcl_ListObjAppendElement(interp, listObjPtr, 
				 Tcl_NewIntObj(search.index));
	Tcl_ListObjAppendElement(interp, listObjPtr, 
				 Tcl_NewStringObj("x", -1));
	Tcl_ListObjAppendElement(interp, listObjPtr, 
				 Tcl_NewDoubleObj(search.point.x));
	Tcl_ListObjAppendElement(interp, listObjPtr, 
				 Tcl_NewStringObj("y", -1));
	Tcl_ListObjAppendElement(interp, listObjPtr, 
				 Tcl_NewDoubleObj(search.point.y));
	Tcl_ListObjAppendElement(interp, listObjPtr, 
				 Tcl_NewStringObj("dist", -1));
	Tcl_ListObjAppendElement(interp, listObjPtr, 
				 Tcl_NewDoubleObj(search.dist));
	Tcl_SetObjResult(interp, listObjPtr);
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ConfigureOp --
 *
 *	Sets the element specifications by the given the command line
 *	arguments and calls the element specification configuration
 *	routine. If zero or one command line options are given, only
 *	information about the option(s) is returned in interp->result.
 *	If the element configuration has changed and the element is
 *	currently displayed, the axis limits are updated and
 *	recomputed.
 *
 * Results:
 *	The return value is a standard Tcl result.
 *
 * Side Effects:
 *	Graph will be redrawn to reflect the new display list.
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
    int nNames, nOpts;
    Tcl_Obj *const *options;
    int i;

    /* Figure out where the option value pairs begin */
    objc -= 3;
    objv += 3;
    for (i = 0; i < objc; i++) {
	Element *elemPtr;
	char *string;

	string = Tcl_GetString(objv[i]);
	if (string[0] == '-') {
	    break;
	}
	if (GetElementFromObj(interp, graphPtr, objv[i], &elemPtr) != TCL_OK) {
	    return TCL_ERROR;	/* Can't find named element */
	}
    }
    nNames = i;			/* Number of element names specified */
    nOpts = objc - i;		/* Number of options specified */
    options = objv + nNames;	/* Start of options in objv  */

    for (i = 0; i < nNames; i++) {
	Element *elemPtr;
	int flags;

	if (GetElementFromObj(interp, graphPtr, objv[i], &elemPtr) != TCL_OK) {
	    return TCL_ERROR;
	}
	flags = BLT_CONFIG_OBJV_ONLY;
	if (nOpts == 0) {
	    return Blt_ConfigureInfoFromObj(interp, graphPtr->tkwin, 
		elemPtr->configSpecs, (char *)elemPtr, (Tcl_Obj *)NULL, flags);
	} else if (nOpts == 1) {
	    return Blt_ConfigureInfoFromObj(interp, graphPtr->tkwin, 
		elemPtr->configSpecs, (char *)elemPtr, options[0], flags);
	}
	if (Blt_ConfigureWidgetFromObj(interp, graphPtr->tkwin, 
		elemPtr->configSpecs, nOpts, options, (char *)elemPtr, flags) 
		!= TCL_OK) {
	    return TCL_ERROR;
	}
	if ((*elemPtr->procsPtr->configProc) (graphPtr, elemPtr) != TCL_OK) {
	    return TCL_ERROR;	/* Failed to configure element */
	}
	if (Blt_ConfigModified(elemPtr->configSpecs, "-hide", (char *)NULL)) {
	    graphPtr->flags |= RESET_AXES;
	    elemPtr->flags |= MAP_ITEM;
	}
	/* If data points or axes have changed, reset the axes (may
	 * affect autoscaling) and recalculate the screen points of
	 * the element. */

	if (Blt_ConfigModified(elemPtr->configSpecs, "-*data", "-map*", "-x",
		"-y", (char *)NULL)) {
	    graphPtr->flags |= RESET_WORLD;
	    elemPtr->flags |= MAP_ITEM;
	}
	/* The new label may change the size of the legend */
	if (Blt_ConfigModified(elemPtr->configSpecs, "-label", (char *)NULL)) {
	    graphPtr->flags |= (MAP_WORLD | REDRAW_WORLD);
	}
    }
    /* Update the pixmap if any configuration option changed */
    graphPtr->flags |= (REDRAW_BACKING_STORE | DRAW_MARGINS);
    Blt_EventuallyRedrawGraph(graphPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * DeactivateOp --
 *
 *	Clears the active bit for the named elements.
 *
 * Results:
 *	Returns TCL_OK if no errors occurred.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
DeactivateOp(
    Graph *graphPtr,		/* Graph widget */
    Tcl_Interp *interp,		/* Not used. */
    int objc,			/* Number of element names */
    Tcl_Obj *const *objv)	/* List of element names */
{
    int i;

    for (i = 3; i < objc; i++) {
	Element *elemPtr;

	if (GetElementFromObj(interp, graphPtr, objv[i], &elemPtr) != TCL_OK) {
	    return TCL_ERROR;	/* Can't find named element */
	}
	elemPtr->flags &= ~ACTIVE;
	if (elemPtr->activeIndices != NULL) {
	    Blt_Free(elemPtr->activeIndices);
	    elemPtr->activeIndices = NULL;
	}
	elemPtr->nActiveIndices = 0;
    }
    Blt_EventuallyRedrawGraph(graphPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * DeleteOp --
 *
 *	Delete the named elements from the graph.
 *
 * Results:
 *	TCL_ERROR is returned if any of the named elements can not be
 *	found.  Otherwise TCL_OK is returned;
 *
 * Side Effects:
 *	If the element is currently displayed, the plotting area of
 *	the graph is redrawn. Memory and resources allocated by the
 *	elements are released.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
DeleteOp(
    Graph *graphPtr,		/* Graph widget */
    Tcl_Interp *interp,		/* Not used. */
    int objc,			/* Number of element names */
    Tcl_Obj *const *objv)	/* List of element names */
{
    int i;

    for (i = 3; i < objc; i++) {
	Element *elemPtr;

	if (GetElementFromObj(interp, graphPtr, objv[i], &elemPtr) != TCL_OK) {
	    return TCL_ERROR;	/* Can't find named element */
	}
	elemPtr->flags |= DELETE_PENDING;
	Tcl_EventuallyFree(elemPtr, FreeElement);
    }
    Blt_EventuallyRedrawGraph(graphPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ExistsOp --
 *
 *	Indicates if the named element exists in the graph.
 *
 * Results:
 *	The return value is a standard Tcl result.  The interpreter
 *	result will contain "1" or "0".
 *
 *---------------------------------------------------------------------------
 */
/* ARGSUSED */
static int
ExistsOp(
    Graph *graphPtr,
    Tcl_Interp *interp,
    int objc,			/* Not used. */
    Tcl_Obj *const *objv)
{
    Blt_HashEntry *hPtr;

    hPtr = Blt_FindHashEntry(&graphPtr->elements.table, Tcl_GetString(objv[3]));
    Tcl_SetBooleanObj(Tcl_GetObjResult(interp), (hPtr != NULL));
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * GetOp --
 *
 * 	Returns the name of the picked element (using the element
 *	bind operation).  Right now, the only name accepted is
 *	"current".
 *
 * Results:
 *	A standard Tcl result.  The interpreter result will contain
 *	the name of the element.
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
    char *string;

    string = Tcl_GetString(objv[3]);
    if ((string[0] == 'c') && (strcmp(string, "current") == 0)) {
	Element *elemPtr;

	elemPtr = (Element *)Blt_GetCurrentItem(graphPtr->bindTable);
	/* Report only on elements. */
	if ((elemPtr != NULL) && ((elemPtr->flags & DELETE_PENDING) == 0) &&
	    (elemPtr->object.classId >= CID_ELEM_BAR) &&
	    (elemPtr->object.classId <= CID_ELEM_STRIP)) {
	    Tcl_SetStringObj(Tcl_GetObjResult(interp), elemPtr->object.name,-1);
	}
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * NamesOp --
 *
 *	Returns the names of the elements is the graph matching
 *	one of more patterns provided.  If no pattern arguments
 *	are given, then all element names will be returned.
 *
 * Results:
 *	The return value is a standard Tcl result. The interpreter
 *	result will contain a Tcl list of the element names.
 *
 *---------------------------------------------------------------------------
 */
static int
NamesOp(
    Graph *graphPtr,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    Tcl_Obj *listObjPtr;

    listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
    if (objc == 3) {
	Blt_HashEntry *hp;
	Blt_HashSearch cursor;

	for (hp = Blt_FirstHashEntry(&graphPtr->elements.table, &cursor);
	     hp != NULL; hp = Blt_NextHashEntry(&cursor)) {
	    Element *elemPtr;
	    Tcl_Obj *objPtr;

	    elemPtr = (Element *)Blt_GetHashValue(hp);
	    objPtr = Tcl_NewStringObj(elemPtr->object.name, -1);
	    Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
	}
    } else {
	Blt_HashEntry *hp;
	Blt_HashSearch cursor;

	for (hp = Blt_FirstHashEntry(&graphPtr->elements.table, &cursor);
	     hp != NULL; hp = Blt_NextHashEntry(&cursor)) {
	    Element *elemPtr;
	    int i;

	    elemPtr = (Element *)Blt_GetHashValue(hp);
	    for (i = 3; i < objc; i++) {
		if (Tcl_StringMatch(elemPtr->object.name, 
				    Tcl_GetString(objv[i]))) {
		    Tcl_Obj *objPtr;

		    objPtr = Tcl_NewStringObj(elemPtr->object.name, -1);
		    Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
		    break;
		}
	    }
	}
    }
    Tcl_SetObjResult(interp, listObjPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ShowOp --
 *
 *	Queries or resets the element display list.
 *
 * Results:
 *	The return value is a standard Tcl result. The interpreter
 *	result will contain the new display list of element names.
 *
 *---------------------------------------------------------------------------
 */
static int
ShowOp(
    Graph *graphPtr,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    Blt_ChainLink link;
    Tcl_Obj *listObjPtr;

    if (objc == 4) {
	if (RebuildDisplayList(interp, graphPtr, objv[3]) != TCL_OK) {
	    return TCL_ERROR;
	}
    }
    listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
    for (link = Blt_ChainFirstLink(graphPtr->elements.displayList); 
	 link != NULL; link = Blt_ChainNextLink(link)) {
	Element *elemPtr;

	elemPtr = Blt_ChainGetValue(link);
	Tcl_ListObjAppendElement(interp, listObjPtr, 
				 Tcl_NewStringObj(elemPtr->object.name, -1));
    }
    Tcl_SetObjResult(interp, listObjPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * TypeOp --
 *
 *	Returns the name of the type of the element given by some
 *	element name.
 *
 * Results:
 *	A standard Tcl result. Returns the type of the element in
 *	interp->result. If the identifier given doesn't represent an
 *	element, then an error message is left in interp->result.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
TypeOp(
    Graph *graphPtr,		/* Graph widget */
    Tcl_Interp *interp,
    int objc,			/* Not used. */
    Tcl_Obj *const *objv)	/* Element name */
{
    Element *elemPtr;

    if (GetElementFromObj(interp, graphPtr, objv[3], &elemPtr) != TCL_OK) {
	return TCL_ERROR;	/* Can't find named element */
    }
    Tcl_SetStringObj(Tcl_GetObjResult(interp), elemPtr->object.className, -1);
    return TCL_OK;
}

/*
 * Global routines:
 */
static Blt_OpSpec elemOps[] = {
    {"activate", 1, ActivateOp, 3, 0, "?elemName? ?index...?",},
    {"bind", 1, BindOp, 3, 6, "elemName sequence command",},
    {"cget", 2, CgetOp, 5, 5, "elemName option",},
    {"closest", 2, ClosestOp, 5, 0,
	"x y ?option value?... ?elemName?...",},
    {"configure", 2, ConfigureOp, 4, 0,
	"elemName ?elemName?... ?option value?...",},
    {"create", 2, CreateOp, 4, 0, "elemName ?option value?...",},
    {"deactivate", 3, DeactivateOp, 3, 0, "?elemName?...",},
    {"delete", 3, DeleteOp, 3, 0, "?elemName?...",},
    {"exists", 1, ExistsOp, 4, 4, "elemName",},
    {"get", 1, GetOp, 4, 4, "name",},
    {"names", 1, NamesOp, 3, 0, "?pattern?...",},
    {"show", 1, ShowOp, 3, 4, "?elemList?",},
    {"type", 1, TypeOp, 4, 4, "elemName",},
};
static int numElemOps = sizeof(elemOps) / sizeof(Blt_OpSpec);


/*
 *---------------------------------------------------------------------------
 *
 * Blt_ElementOp --
 *
 *	This procedure is invoked to process the Tcl command that
 *	corresponds to a widget managed by this module.  See the user
 *	documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *---------------------------------------------------------------------------
 */
int
Blt_ElementOp(
    Graph *graphPtr,		/* Graph widget record */
    Tcl_Interp *interp,
    int objc,			/* # arguments */
    Tcl_Obj *const *objv,	/* Argument list */
    ClassId classId)
{
    void *ptr;
    int result;

    ptr = Blt_GetOpFromObj(interp, numElemOps, elemOps, BLT_OP_ARG2, 
			    objc, objv, 0);
    if (ptr == NULL) {
	return TCL_ERROR;
    }
    if (ptr == CreateOp) {
	result = CreateOp(graphPtr, interp, objc, objv, classId);
    } else {
	GraphElementProc *proc;
	
	proc = ptr;
	result = (*proc) (graphPtr, interp, objc, objv);
    }
    return result;
}
