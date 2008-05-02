
/*
 *
 * bltDtVec.c --
 *
 *	Copyright 1998-2005 George A Howlett.
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

#include <blt.h>

#include "config.h"
#include <tcl.h>

#include <bltSwitch.h>
#include <bltDataTable.h>
#include <bltVector.h>

extern double Blt_NaN(void);

DLLEXPORT extern Tcl_AppInitProc Blt_Dt_VectorInit;

/*
 * Format	Import		Export
 * csv		file/data	file/data
 * tree		data		data
 * vector	data		data
 * xml		file/data	file/data
 * sql		data		data
 *
 * $table import csv -file fileName -data dataString 
 * $table export csv -file defaultFileName 
 * $table import tree $treeName $node ?switches? 
 * $table export tree $treeName $node "label" "label" "label"
 * $table import vector $vecName label $vecName label...
 * $table export vector label $vecName label $vecName...
 * $table import xml -file fileName -data dataString ?switches?
 * $table export xml -file fileName -data dataString ?switches?
 * $table import sql -host $host -password $pw -db $db -port $port 
 */

static Blt_Dt_ImportProc ImportVecProc;
static Blt_Dt_ExportProc ExportVecProc;

/* 
 * $table export -file fileName ?switches...?
 * $table export ?switches...?
 */
static int
ExportVecProc(
    Blt_Dt table, 
    Tcl_Interp *interp, 
    int objc, 
    Tcl_Obj *const *objv)
{
    int i;
    long nRows;
    
    if ((objc - 3) & 1) {
	Tcl_AppendResult(interp, "odd # of column/vector pairs: should be \"", 
		Tcl_GetString(objv[0]), 
		" export vector col vecName ?col vecName?...", (char *)NULL);
	return TCL_ERROR;
    }
    nRows = Blt_Dt_NumRows(table);
    for (i = 3; i < objc; i += 2) {
	Blt_Vector *vector;
	size_t size;
	double *array;
	int j, k;
	Blt_Dt_Column col;

	col = Blt_Dt_FindColumn(interp, table, objv[i]);
	if (col == NULL) {
	    return TCL_ERROR;
	}
	if (Blt_GetVectorFromObj(interp, objv[i+1], &vector) != TCL_OK) {
	    return TCL_ERROR;
	}
	if (Blt_VecLength(vector) != nRows) {
	    if (Blt_ResizeVector(vector, nRows) != TCL_OK) {
		return TCL_ERROR;
	    }
	}
	array = Blt_VecData(vector);
	size = Blt_VecSize(vector);
	for (j = 0, k = 1; j <= Blt_VecLength(vector); j++, k++) {
	    Blt_Dt_Row row;
	    Tcl_Obj *objPtr;
	    double value;

	    row = Blt_Dt_GetRowByIndex(table, k);
	    objPtr = Blt_Dt_GetValue(table, row, col);
	    if (Tcl_GetDoubleFromObj(interp, objPtr, &value) != TCL_OK) {
		return TCL_ERROR;
	    }
	    array[j] = (objPtr == NULL) ? Blt_NaN() : value;
	}
	if (Blt_ResetVector(vector, array, nRows, size, TCL_STATIC) != TCL_OK) {
	    return TCL_ERROR;
	}
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ImportVecProc --
 *
 *	Parses the given command line and calls one of several
 *	export-specific operations.
 *	
 * Results:
 *	Returns a standard Tcl result.  It is the result of 
 *	operation called.
 *
 *	$table import vector v1 col v2 col v3 col
 *
 *---------------------------------------------------------------------------
 */
static int
ImportVecProc(
    Blt_Dt table,
    Tcl_Interp *interp, 
    int objc, 
    Tcl_Obj *const *objv)
{
    int maxLen;
    int i;

    if ((objc - 3) & 1) {
	Tcl_AppendResult(interp, "odd # of vector/column pairs: should be \"", 
		Tcl_GetString(objv[0]), 
		" import vector vecName col vecName col...", (char *)NULL);
	return TCL_ERROR;
    }
    maxLen = 0;
    for (i = 3; i < objc; i += 2) {
	Blt_Dt_Column col;
	Blt_Vector *vector;
	
	if (Blt_GetVectorFromObj(interp, objv[i], &vector) != TCL_OK) {
	    return TCL_ERROR;
	}
	col = Blt_Dt_FindColumn(NULL, table, objv[i+1]);
	if (col == NULL) {
	    const char *label;

	    label = Tcl_GetString(objv[i+1]);
	    col = Blt_Dt_CreateColumn(interp, table, label);
	    if (col == NULL) {
		return TCL_ERROR;
	    }
	}
	if (Blt_VecLength(vector) > maxLen) {
	    maxLen = Blt_VecLength(vector);
	}
    }
    if (maxLen > Blt_Dt_NumRows(table)) {
	size_t needed;

	needed = maxLen - Blt_Dt_NumRows(table);
	if (Blt_Dt_ExtendRows(interp, table, needed, NULL) != TCL_OK) {
	    return TCL_ERROR;
	}
    }
    for (i = 3; i < objc; i += 2) {
	Blt_Dt_Column col;
	Blt_Vector *vector;
	double *array;
	size_t j, k;
	size_t nElems;

	if (Blt_GetVectorFromObj(interp, objv[i], &vector) != TCL_OK) {
	    return TCL_ERROR;
	}
	col = Blt_Dt_FindColumn(interp, table, objv[i+1]);
	if (col == NULL) {
	    return TCL_ERROR;
	}
	array = Blt_VecData(vector);
	nElems = Blt_VecLength(vector);
	for (j = 0, k = 1; j < nElems; j++, k++) {
	    Blt_Dt_Row row;

	    row = Blt_Dt_GetRowByIndex(table, k);
	    if (array[j] == Blt_NaN()) {
		if (Blt_Dt_UnsetValue(table, row, col) != TCL_OK) {
		    return TCL_ERROR;
		}
	    } else {
		if (Blt_Dt_SetValue(table, row, col, Tcl_NewDoubleObj(array[j]))
		    != TCL_OK) {
		    return TCL_ERROR;
		}
	    }
	}
    }
    return TCL_OK;
}
    
int 
Blt_Dt_VectorInit(Tcl_Interp *interp)
{
#ifdef USE_TCL_STUBS
    if (Tcl_InitStubs(interp, TCL_VERSION, 1) == NULL) {
	return TCL_ERROR;
    };
#endif
    if (Tcl_PkgRequire(interp, "bltcore", BLT_VERSION, /*Exact*/1) == NULL) {
	return TCL_ERROR;
    }
    if (Tcl_PkgProvide(interp, "bltdatatablevector", BLT_VERSION) != TCL_OK){ 
	return TCL_ERROR;
    }
    return Blt_Dt_RegisterFormat(interp,
        "vector",		/* Name of format. */
	ImportVecProc,		/* Import procedure. */
	ExportVecProc);		/* Export procedure. */
}

