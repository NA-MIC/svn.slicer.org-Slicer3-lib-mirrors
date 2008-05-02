
/*
 *
 * bltDtTree.c --
 *
 *	Copyright 1998-2005 George A Howlett.
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

#include <blt.h>

#include "config.h"
#ifndef NO_DATATABLE

#include <tcl.h>
#include <bltDataTable.h>
#include <bltTree.h>
#include <bltSwitch.h>
#ifdef HAVE_MEMORY_H
#  include <memory.h>
#endif /* HAVE_MEMORY_H */

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

/*
 * ExportSwitches --
 */
typedef struct {
    /* Private data. */
    Blt_TreeNode node;

    /* Public fields */
    Blt_Dt_Iterator rIter, cIter;
    Blt_Dt_Iterator hIter;
    Tcl_Obj *nodeObjPtr;
} ExportSwitches;

BLT_EXTERN Blt_SwitchFreeProc Blt_Dt_ColumnIterFreeProc;
BLT_EXTERN Blt_SwitchFreeProc Blt_Dt_RowIterFreeProc;
BLT_EXTERN Blt_SwitchParseProc Blt_Dt_ColumnIterSwitchProc;
BLT_EXTERN Blt_SwitchParseProc Blt_Dt_RowIterSwitchProc;

static Blt_SwitchCustom columnIterSwitch = {
    Blt_Dt_ColumnIterSwitchProc, Blt_Dt_ColumnIterFreeProc, 0,
};
static Blt_SwitchCustom rowIterSwitch = {
    Blt_Dt_RowIterSwitchProc, Blt_Dt_RowIterFreeProc, 0,
};

static Blt_SwitchSpec exportSwitches[] = 
{
    {BLT_SWITCH_CUSTOM, "-columns", "columns",
	Blt_Offset(ExportSwitches, cIter), 0, 0, &columnIterSwitch},
    {BLT_SWITCH_CUSTOM, "-hierarchy", "columns",
	Blt_Offset(ExportSwitches, hIter), 0, 0, &columnIterSwitch},
    {BLT_SWITCH_OBJ, "-node", "node",
	Blt_Offset(ExportSwitches, nodeObjPtr), 0},
    {BLT_SWITCH_CUSTOM, "-rows", "rows",
        Blt_Offset(ExportSwitches, rIter), 0, 0, &rowIterSwitch},
    {BLT_SWITCH_END}
};

DLLEXPORT extern Tcl_AppInitProc Blt_Dt_TreeInit;

static Blt_Dt_ImportProc ImportTreeProc;
static Blt_Dt_ExportProc ExportTreeProc;

static int
ImportTree(
    Tcl_Interp *interp,    
    Blt_Dt table, 
    Blt_Tree tree, 
    Blt_TreeNode top)
{
    Blt_TreeNode node;
    int maxDepth, topDepth;
    long iRow, nCols;

    /* 
     * Fill in the table data in 2 passes.  We need to know the
     * maximum depth of the leaf nodes, to generate columns for each
     * level of the hierarchy.  We have to do this before adding
     * node data values.
     */

    /* Pass 1.  Create entries for all the nodes. Add entries for 
     *          the node and it's ancestor's labels. */
    maxDepth = topDepth = Blt_TreeNodeDepth(top);
    nCols = Blt_Dt_NumColumns(table);
    for (node = Blt_TreeNextNode(top, top); node != NULL;
	 node = Blt_TreeNextNode(top, node)) {
	Blt_TreeNode parent;
	int depth;
	Blt_Dt_Row row;
	size_t iCol;

	depth = Blt_TreeNodeDepth(node);
	if (depth > maxDepth) {
	    Blt_Dt_Column col;

	    if (Blt_Dt_ExtendColumns(interp, table, 1, &col) != TCL_OK) {
		return TCL_ERROR;
	    }
	    iCol = Blt_Dt_ColumnIndex(col);
	    maxDepth = depth;
	} else {
	    iCol = depth - topDepth;
	}
	if (Blt_Dt_ExtendRows(interp, table, 1, &row) != TCL_OK) {
	    return TCL_ERROR;
	}
	for (parent = node; parent != top; parent = Blt_TreeNodeParent(parent)){
	    Tcl_Obj *objPtr;
	    const char *label;
	    Blt_Dt_Column col;

	    col = Blt_Dt_GetColumnByIndex(table, iCol);
	    label = Blt_TreeNodeLabel(parent);
	    objPtr = Tcl_NewStringObj(label, -1);
	    if (Blt_Dt_SetValue(table, row, col, objPtr) != TCL_OK) {
		return TCL_ERROR;
	    }
	    iCol--;
	}
    }
    /* Pass 2.  Fill in entries for all the data fields found. */
    for (iRow = 1, node = Blt_TreeNextNode(top, top); node != NULL;
	 node = Blt_TreeNextNode(top, node), iRow++) {
	Blt_TreeKey key;
	Blt_TreeKeySearch cursor;
	Blt_Dt_Row row;

	row = Blt_Dt_GetRowByIndex(table, iRow);
	for (key = Blt_TreeFirstKey(tree, node, &cursor); key != NULL;
	     key = Blt_TreeNextKey(tree, &cursor)) {
	    Blt_Dt_Column col;
	    Tcl_Obj *objPtr;

	    if (Blt_TreeGetValue(interp, tree, node, key, &objPtr) != TCL_OK) {
		return TCL_ERROR;
	    }
	    col = Blt_Dt_GetColumnByLabel(table, key);
	    if (col == NULL) {
		col = Blt_Dt_CreateColumn(interp, table, key);
		if (col == NULL) {
		    return TCL_ERROR;
		}
	    }
	    if (Blt_Dt_SetValue(table, row, col, objPtr) != TCL_OK) {
		return TCL_ERROR;
	    }
	}
    }
    return TCL_OK;
}

static int
ImportTreeProc(
    Blt_Dt table, 
    Tcl_Interp *interp, 
    int objc, 
    Tcl_Obj *const *objv)
{
    Blt_Tree tree;
    Blt_TreeNode node;

    /* FIXME: 
     *	 2. Export *GetNode tag parsing routines from bltTreeCmd.c,
     *	    instead of using node id to select the top node.
     */
    tree = Blt_TreeOpen(interp, Tcl_GetString(objv[3]), 0);
    if (tree == NULL) {
	return TCL_ERROR;
    }
    if (objc == 5) {
	long inode;

	if (Tcl_GetLongFromObj(interp, objv[4], &inode) != TCL_OK) {
	    return TCL_ERROR;
	}
	node = Blt_TreeGetNode(tree, inode);
	if (node == NULL) {
	    return TCL_ERROR;
	}
    } else {
	node = Blt_TreeRootNode(tree);
    }
    return ImportTree(interp, table, tree, node);
}

static int
ExportTree(
    Tcl_Interp *interp,    
    Blt_Dt table, 
    Blt_Tree tree, 
    Blt_TreeNode parent,
    ExportSwitches *switchesPtr) 
{
    Blt_Dt_Row row;

    for (row = Blt_Dt_FirstRow(&switchesPtr->rIter); row != NULL;
	 row = Blt_Dt_NextRow(&switchesPtr->rIter)) {
	Blt_Dt_Column col;
	Blt_TreeNode node;
	const char *label;

	label = Blt_Dt_RowLabel(row);
	node = Blt_TreeCreateNode(tree, parent, label, -1);
	for (col = Blt_Dt_FirstColumn(&switchesPtr->cIter); col != NULL;
	     col = Blt_Dt_NextColumn(&switchesPtr->cIter)) {
	    Tcl_Obj *objPtr;
	    const char *key;

	    objPtr = Blt_Dt_GetValue(table, row, col);
	    key = Blt_Dt_ColumnLabel(col);
	    if (Blt_TreeSetValue(interp, tree, node, key, objPtr) != TCL_OK) {
		return TCL_ERROR;
	    }		
	}
    }
    return TCL_OK;
}


static int
ExportTreeProc(
    Blt_Dt table, 
    Tcl_Interp *interp, 
    int objc, 
    Tcl_Obj *const *objv)
{
    Blt_Tree tree;
    Blt_TreeNode node;
    ExportSwitches switches;
    int result;

    tree = Blt_TreeOpen(interp, Tcl_GetString(objv[3]), 0);
    if (tree == NULL) {
	return TCL_ERROR;
    }
    memset(&switches, 0, sizeof(switches));
    rowIterSwitch.clientData = table;
    columnIterSwitch.clientData = table;
    Blt_Dt_IterateAllRows(table, &switches.rIter);
    Blt_Dt_IterateAllColumns(table, &switches.cIter);
    if (Blt_ParseSwitches(interp, exportSwitches, objc, objv, &switches, 
	BLT_SWITCH_DEFAULTS) < 0) {
	return TCL_ERROR;
    }
    if (switches.nodeObjPtr != NULL) {
	long inode;

	if (Tcl_GetLongFromObj(interp, switches.nodeObjPtr, &inode) != TCL_OK) {
	    return TCL_ERROR;
	}
	node = Blt_TreeGetNode(tree, inode);
	if (node == NULL) {
	    return TCL_ERROR;
	}
    } else {
	node = Blt_TreeRootNode(tree);
    }
    result = ExportTree(interp, table, tree, node, &switches);
    Blt_FreeSwitches(exportSwitches, (char *)&switches, 0);
    return result;
}

int 
Blt_Dt_TreeInit(Tcl_Interp *interp)
{
#ifdef USE_TCL_STUBS
    if (Tcl_InitStubs(interp, TCL_VERSION, 1) == NULL) {
	return TCL_ERROR;
    };
#endif
    if (Tcl_PkgRequire(interp, "bltcore", BLT_VERSION, /*Exact*/1) == NULL) {
	return TCL_ERROR;
    }
    if (Tcl_PkgProvide(interp, "bltdatatabletree", BLT_VERSION) != TCL_OK) { 
	return TCL_ERROR;
    }
    return Blt_Dt_RegisterFormat(interp,
        "tree",			/* Name of format. */
	ImportTreeProc,		/* Import procedure. */
	ExportTreeProc);	/* Export procedure. */

}
#endif /* NO_DATATABLE */

