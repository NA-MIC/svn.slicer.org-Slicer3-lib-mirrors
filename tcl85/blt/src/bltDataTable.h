
/*
 * bltDataTable.h --
 *
 *	Copyright 1998-2004 George A Howlett.
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

#ifndef _BLT_DATATABLE_H
#define _BLT_DATATABLE_H

#include <bltChain.h>
#include <bltHash.h>

typedef struct Blt_Dt_TagsStruct *Blt_Dt_Tags;

typedef struct Blt_Dt_HeaderStruct {
    char *label;		/* Label of row or column. */
    long index;			/* Reverse lookup offset-to-index. */
    long offset;
    unsigned int flags;
} *Blt_Dt_Header;

typedef struct Blt_Dt_RowStruct {
    char *label;		/* Label of row or column. */
    long index;			/* Reverse lookup offset-to-index. */
    long offset;
    unsigned int flags;
} *Blt_Dt_Row;

typedef struct Blt_Dt_ColumnStruct {
    char *label;		/* Label of row or column. */
    long index;			/* Reverse lookup offset-to-index. */
    long offset;
    unsigned short flags;
    unsigned short type;
} *Blt_Dt_Column;

typedef struct {
    const char *name;
    long headerSize;
} Blt_Dt_RowColumnClass;

/*
 * Blt_Dt_RowColumn --
 *
 *	Structure representing a row or column in the table. 
 */
typedef struct Blt_Dt_RowColumnStruct {
    Blt_Dt_RowColumnClass *classPtr;
    Blt_Pool headerPool;

    long nAllocated;		/* Length of allocated array below. May exceed
				 * the number of row or column headers
				 * used. */
    long nUsed;

    Blt_Dt_Header *map;		/* Array of row or column headers. */
    
    Blt_Chain freeList;		/* Tracks free row or column headers.  */

    Blt_HashTable labels;	/* Hash table of labels. Maps labels to table
				 * offsets. */

    long nextId;		/* Used to generate default labels. */

} Blt_Dt_RowColumn;

/*
 * Blt_Dt_Core --
 *
 *	Structure representing a table object. 
 *
 *	The table object is an array of column vectors. Each vector is an
 *	array of Tcl_Obj pointers, representing the data for the column.
 *	Empty row entries are designated by NULL values.  Column vectors are
 *	allocated when needed.  Every column in the table has the same length.
 *
 *	Rows and columns are indexed by a map of pointers to headers.  This
 *	map represents the order of the rows or columns.  A table object can
 *	be shared by several clients.  When a client wants to use a table
 *	object, it is given a token that represents the table.  The object
 *	tracks its clients by its token. When all clients have released their
 *	tokens, the tuple object is automatically destroyed.
 */
typedef struct Blt_Dt_CoreStruct {
    Blt_Dt_RowColumn rowData, colData;

    Tcl_Obj ***vectors;		/* Array of vector pointers */

    unsigned int flags;		/* Internal flags. See definitions below. */

    Blt_Chain clients;		/* List of clients using this table */

    unsigned long mtime, ctime;

    unsigned int notifyFlags;	/* Notification flags. See definitions
				 * below. */
    int notifyHold;
    
} Blt_Dt_Core;

/*
 * Blt_Dt --
 *
 *	A client is uniquely identified by a combination of its name and the
 *	originating namespace.  Two table objects in the same interpreter can
 *	have similar names but must reside in different namespaces.
 *
 *	Two or more clients can share the same table object.  Each client
 *	structure which acts as a ticket for the underlying table object.
 *	Clients can designate notifier routines that are automatically invoked
 *	by the table object whenever the table is changed is specific ways by
 *	other clients.
 */
typedef struct Blt_Dt_TableStruct {
    unsigned int magic;		/* Magic value indicating whether a generic
				 * pointer is really a datatable token or
				 * not. */
    const char *name;		/* Fully namespace-qualified name of the
				 * client. */
    const char *emptyValue;

    long *nRowsPtr, *nColumnsPtr;

    Blt_Dt_Core *corePtr;	/* Pointer to the structure containing the
				 * master information about the table used by
				 * the client.  If NULL, this indicates that
				 * the table has been destroyed (but as of
				 * yet, this client hasn't recognized it). */
    Tcl_Interp *interp;

    Blt_HashTable *tablePtr;	/* Interpreter-global hash table of all
				 * datatable clients. Each entry is a chain of
				 * clients that have sharing the same
				 * table. */

    Blt_HashEntry *hPtr;	/* This client's entry in the above
				 * table. This is a list of clients that all
				 * have the same qualified table name
				 * (i.e. are sharing the same table. */

    Blt_ChainLink link2;	/* This client's entry in the above
				 * label hashtable chain. */

    Blt_ChainLink link;		/* Pointer into the server's chain of
				 * clients. */

    Blt_HashTable *rowTags;
    Blt_HashTable *columnTags;

    Blt_Chain traces;		/* Chain of traces. */
    Blt_Chain columnNotifiers;	/* Chain of event handlers. */
    Blt_Chain rowNotifiers;	/* Chain of event handlers. */
    Blt_Dt_Tags tags;

    Blt_HashTable *keyTables;	/* Array of primary keys. */
    long nKeys;			/* Length of the above array. */
    Blt_Dt_Row *masterKey;	/* Master key entry. */
    Blt_HashTable masterKeyTable;
    Blt_Chain primaryKeys;
    unsigned int flags;
} *Blt_Dt;

BLT_EXTERN void Blt_Dt_ReleaseTags(Blt_Dt table);

BLT_EXTERN int Blt_Dt_TableExists(Tcl_Interp *interp, CONST char *name);
BLT_EXTERN int Blt_Dt_CreateTable(Tcl_Interp *interp, CONST char *name, 
	Blt_Dt *tablePtr);
BLT_EXTERN int Blt_Dt_Open(Tcl_Interp *interp, CONST char *name, 
	Blt_Dt *tablePtr);
BLT_EXTERN void Blt_Dt_Close(Blt_Dt table);

BLT_EXTERN int Blt_Dt_SameTableObject(Blt_Dt table1, Blt_Dt table2);

BLT_EXTERN CONST char *Blt_Dt_Name(Blt_Dt table);

BLT_EXTERN Blt_Dt_Row    Blt_Dt_GetRowByLabel(Blt_Dt table, CONST char *label);
BLT_EXTERN Blt_Dt_Column Blt_Dt_GetColumnByLabel(Blt_Dt table, 
	CONST char *label);

BLT_EXTERN Blt_Dt_Row    Blt_Dt_GetRowByIndex(Blt_Dt table, long index);
BLT_EXTERN Blt_Dt_Column Blt_Dt_GetColumnByIndex(Blt_Dt table, long index);

BLT_EXTERN int Blt_Dt_SetRowLabel(Tcl_Interp *interp, Blt_Dt table, 
	Blt_Dt_Row row, CONST char *label);
BLT_EXTERN int Blt_Dt_SetColumnLabel(Tcl_Interp *interp, Blt_Dt table, 
	Blt_Dt_Column column, CONST char *label);

BLT_EXTERN int Blt_Dt_ParseColumnType(CONST char *typeName);

BLT_EXTERN CONST char *Blt_Dt_NameOfColumnType(int type);


BLT_EXTERN int Blt_Dt_GetColumnType(Blt_Dt_Column column);
BLT_EXTERN int Blt_Dt_SetColumnType(Blt_Dt_Column column, int type);

BLT_EXTERN int Blt_Dt_SetColumnTag(Tcl_Interp *interp, Blt_Dt table, 
	Blt_Dt_Column column, CONST char *tagName);
BLT_EXTERN int Blt_Dt_SetRowTag(Tcl_Interp *interp, Blt_Dt table, 
	Blt_Dt_Row row, CONST char *tagName);

BLT_EXTERN Blt_Dt_Row Blt_Dt_CreateRow(Tcl_Interp *interp, Blt_Dt table, 
	CONST char *label);
BLT_EXTERN Blt_Dt_Column Blt_Dt_CreateColumn(Tcl_Interp *interp, Blt_Dt table, 
	CONST char *label);

BLT_EXTERN int Blt_Dt_ExtendRows(Tcl_Interp *interp, Blt_Dt table, long n, 
	Blt_Dt_Row *rows);
BLT_EXTERN int Blt_Dt_ExtendColumns(Tcl_Interp *interp, Blt_Dt table, long n, 
	Blt_Dt_Column *columms);
BLT_EXTERN int Blt_Dt_DeleteRow(Blt_Dt table, Blt_Dt_Row row);
BLT_EXTERN int Blt_Dt_DeleteColumn(Blt_Dt table, Blt_Dt_Column column);
BLT_EXTERN int Blt_Dt_MoveRows(Tcl_Interp *interp, Blt_Dt table, 
	Blt_Dt_Row from, Blt_Dt_Row to, long n);
BLT_EXTERN int Blt_Dt_MoveColumns(Tcl_Interp *interp, Blt_Dt table, 
	Blt_Dt_Column from,  Blt_Dt_Column to, long n);

BLT_EXTERN Tcl_Obj *Blt_Dt_GetValue(Blt_Dt table, Blt_Dt_Row row,
	Blt_Dt_Column col);
BLT_EXTERN int Blt_Dt_SetValue(Blt_Dt table, Blt_Dt_Row row, 
	Blt_Dt_Column column, Tcl_Obj *objPtr);
BLT_EXTERN int Blt_Dt_UnsetValue(Blt_Dt table, Blt_Dt_Row row, 
	Blt_Dt_Column column);
BLT_EXTERN int Blt_Dt_ValueExists(Blt_Dt table, Blt_Dt_Row row, 
	Blt_Dt_Column column);
BLT_EXTERN int Blt_Dt_GetArrayValue(Blt_Dt table, Blt_Dt_Row row, 
	Blt_Dt_Column column, CONST char *key, Tcl_Obj **objPtrPtr);
BLT_EXTERN int Blt_Dt_SetArrayValue(Blt_Dt table, Blt_Dt_Row row, 
	Blt_Dt_Column column, CONST char *key, Tcl_Obj *objPtr);
BLT_EXTERN int Blt_Dt_UnsetArrayValue(Blt_Dt table, Blt_Dt_Row row, 
	Blt_Dt_Column column, CONST char *key);
BLT_EXTERN int Blt_Dt_ArrayValueExists(Blt_Dt table, Blt_Dt_Row row, 
	Blt_Dt_Column column, CONST char *key);
BLT_EXTERN Tcl_Obj *Blt_Dt_ArrayNames(Blt_Dt table, Blt_Dt_Row row, 
	Blt_Dt_Column column);


BLT_EXTERN Blt_HashTable *Blt_Dt_GetRowTagTable(Blt_Dt table, 
	CONST char *tagName);
BLT_EXTERN Blt_HashTable *Blt_Dt_GetColumnTagTable(Blt_Dt table, 
	CONST char *tagName);
BLT_EXTERN Blt_Chain Blt_Dt_GetRowTags(Blt_Dt table, Blt_Dt_Row row);
BLT_EXTERN Blt_Chain Blt_Dt_GetColumnTags(Blt_Dt table, Blt_Dt_Column column);

BLT_EXTERN long Blt_Dt_GetNumColumns(Blt_Dt table);
BLT_EXTERN Blt_Chain Blt_Dt_Traces(Blt_Dt table);
BLT_EXTERN int Blt_Dt_TagsAreShared(Blt_Dt table);

BLT_EXTERN int Blt_Dt_HasRowTag(Blt_Dt table, Blt_Dt_Row row, 
	CONST char *tagName);
BLT_EXTERN int Blt_Dt_HasColumnTag(Blt_Dt table, Blt_Dt_Column column, 
	CONST char *tagName);
BLT_EXTERN void Blt_Dt_AddColumnTag(Blt_Dt table, Blt_Dt_Column column, 
	CONST char *tagName);
BLT_EXTERN void Blt_Dt_AddRowTag(Blt_Dt table, Blt_Dt_Row row, 
	CONST char *tagName);
BLT_EXTERN int Blt_Dt_ForgetRowTag(Tcl_Interp *interp, Blt_Dt table, 
	CONST char *tagName);
BLT_EXTERN int Blt_Dt_ForgetColumnTag(Tcl_Interp *interp, Blt_Dt table,
	CONST char *tagName);
BLT_EXTERN int Blt_Dt_UnsetRowTag(Tcl_Interp *interp, Blt_Dt table, 
	Blt_Dt_Row row, CONST char *tagName);
BLT_EXTERN int Blt_Dt_UnsetColumnTag(Tcl_Interp *interp, Blt_Dt table, 
	Blt_Dt_Column column, CONST char *tagName);
BLT_EXTERN Blt_HashEntry *Blt_Dt_FirstRowTag(Blt_Dt table, 
	Blt_HashSearch *cursorPtr);
BLT_EXTERN Blt_HashEntry *Blt_Dt_FirstColumnTag(Blt_Dt table, 
	Blt_HashSearch *cursorPtr);

BLT_EXTERN Blt_HashTable *Blt_Dt_GetTagHashTable(Blt_Dt table, 
	CONST char *tagName);

BLT_EXTERN int Blt_Dt_TagTableIsShared(Blt_Dt table);

#define DT_COLUMN_UNKNOWN	(0) 
#define DT_COLUMN_STRING	(1<<0)
#define DT_COLUMN_INTEGER	(1<<1)
#define DT_COLUMN_DOUBLE	(1<<2)
#define DT_COLUMN_BINARY	(1<<3)
#define DT_COLUMN_IMAGE		(1<<4)
#define DT_COLUMN_MASK		(DT_COLUMN_INTEGER | DT_COLUMN_BINARY | \
				 DT_COLUMN_DOUBLE | DT_COLUMN_STRING | \
				 DT_COLUMN_IMAGE)

typedef enum { 
    DT_SPEC_UNKNOWN, DT_SPEC_INDEX, DT_SPEC_RANGE, DT_SPEC_LABEL, DT_SPEC_TAG, 
} Blt_Dt_RowColumnSpec;

BLT_EXTERN Blt_Dt_RowColumnSpec Blt_Dt_GetRowSpec(Blt_Dt table, 
	Tcl_Obj *objPtr, CONST char **stringPtr);
BLT_EXTERN Blt_Dt_RowColumnSpec Blt_Dt_GetColumnSpec(Blt_Dt table, 
	Tcl_Obj *objPtr, CONST char **stringPtr);

/*
 * Blt_Dt_Iterator --
 *
 *	Structure representing a trace used by a client of the table.
 *
 *	Table rows and columns may be tagged with strings.  A row may
 *	have many tags.  The same tag may be used for many rows.  Tags
 *	are used and stored by clients of a table.  Tags can also be
 *	shared between clients of the same table.
 *	
 *	Both rowTable and columnTable are hash tables keyed by the
 *	physical row or column location in the table respectively.
 *	This is not the same as the client's view (the order of rows
 *	or columns as seen by the client).  This is so that clients
 *	(which may have different views) can share tags without
 *	sharing the same view.
 */


typedef enum { 
    DT_ITER_INDEX, DT_ITER_LABEL, DT_ITER_TAG, 
    DT_ITER_RANGE, DT_ITER_ALL, DT_ITER_CHAIN
} Blt_Dt_IteratorType;

typedef struct Blt_Dt_IteratorStruct {
    Blt_Dt table;		/* Table that we're iterating over. */

    Blt_Dt_IteratorType type;	/* Type of iteration:
				 * DT_ITER_TAG	by row or column tag.
				 * DT_ITER_ALL	by every row or column.
				 * DT_ITER_INDEX	single item: either 
				 *			label or index.
				 * DT_ITER_RANGE	over a consecutive 
				 *			range of indices.
				 * DT_ITER_CHAIN	over an expanded,
				 *			non-overlapping
				 *			list of tags, labels,
				 *			and indices.
				 */

    CONST char *tagName;	/* Used by notification routines to determine
				 * if a tag is being used. */
    long start;			/* Starting index.  Starting point of search,
				 * saved if iterator is reused.  Used for
				 * DT_ITER_ALL and DT_ITER_INDEX searches. */
    long end;			/* Ending index (inclusive). */

    long next;			/* Next index. */

    /* For tag-based searches. */
    Blt_HashTable *tablePtr;	/* Pointer to tag hash table. */
    Blt_HashSearch cursor;	/* Search iterator for tag hash table. */

    /* For chain-based searches (multiple tags). */
    Blt_Chain chain;		/* This chain, unlike the above hash table
				 * must be freed after it's use. */
    Blt_ChainLink link;		/* Search iterator for chain. */
} Blt_Dt_Iterator;

BLT_EXTERN int Blt_Dt_IterateRows(Tcl_Interp *interp, Blt_Dt table, 
	Tcl_Obj *objPtr, Blt_Dt_Iterator *iter);

BLT_EXTERN int Blt_Dt_IterateColumns(Tcl_Interp *interp, Blt_Dt table, 
	Tcl_Obj *objPtr, Blt_Dt_Iterator *iter);

BLT_EXTERN int Blt_Dt_IterateRowsObjv(Tcl_Interp *interp, Blt_Dt table, 
	int objc, Tcl_Obj *CONST *objv, Blt_Dt_Iterator *iterPtr);

BLT_EXTERN int Blt_Dt_IterateColumnsObjv(Tcl_Interp *interp, Blt_Dt table, 
	int objc, Tcl_Obj *CONST *objv, Blt_Dt_Iterator *iterPtr);

BLT_EXTERN void Blt_Dt_FreeIteratorObjv(Blt_Dt_Iterator *iterPtr);

BLT_EXTERN void Blt_Dt_IterateAllRows(Blt_Dt table, Blt_Dt_Iterator *iterPtr);

BLT_EXTERN void Blt_Dt_IterateAllColumns(Blt_Dt table, 
	Blt_Dt_Iterator *iterPtr);

BLT_EXTERN Blt_Dt_Row Blt_Dt_FirstRow(Blt_Dt_Iterator *iter);

BLT_EXTERN Blt_Dt_Column Blt_Dt_FirstColumn(Blt_Dt_Iterator *iter);

BLT_EXTERN Blt_Dt_Row Blt_Dt_NextRow(Blt_Dt_Iterator *iter);

BLT_EXTERN Blt_Dt_Column Blt_Dt_NextColumn(Blt_Dt_Iterator *iter);

BLT_EXTERN Blt_Dt_Row Blt_Dt_FindRow(Tcl_Interp *interp, Blt_Dt table, 
	Tcl_Obj *objPtr);

BLT_EXTERN Blt_Dt_Column Blt_Dt_FindColumn(Tcl_Interp *interp, Blt_Dt table, 
	Tcl_Obj *objPtr);

BLT_EXTERN int Blt_Dt_ListRows(Tcl_Interp *interp, Blt_Dt table, int objc, 
	Tcl_Obj *CONST *objv, Blt_Chain chain);

BLT_EXTERN int Blt_Dt_ListColumns(Tcl_Interp *interp, Blt_Dt table, int objc, 
	Tcl_Obj *CONST *objv, Blt_Chain chain);

/*
 * Blt_Dt_TraceEvent --
 *
 *	Structure representing an event matching a trace set by a client of
 *	the table.
 *
 *	Table rows and columns may be tagged with strings.  A row may have
 *	many tags.  The same tag may be used for many rows.  Tags are used and
 *	stored by clients of a table.  Tags can also be shared between clients
 *	of the same table.
 *	
 *	Both rowTable and columnTable are hash tables keyed by the physical
 *	row or column location in the table respectively.  This is not the
 *	same as the client's view (the order of rows or columns as seen by the
 *	client).  This is so that clients (which may have different views) can
 *	share tags without sharing the same view.
 */
typedef struct {
    Tcl_Interp *interp;		/* Interpreter to report to */
    Blt_Dt table;		/* Table object client that received the
				 * event. */
    Blt_Dt_Row row;		/* Matching row and column. */
    Blt_Dt_Column column;
    unsigned int mask;		/* Indicates type of event received. */
} Blt_Dt_TraceEvent;

typedef int (Blt_Dt_TraceProc)(ClientData clientData, 
	Blt_Dt_TraceEvent *eventPtr);

typedef void (Blt_Dt_TraceDeleteProc)(ClientData clientData);

/*
 * Blt_Dt_Trace --
 *
 *	Structure representing a trace used by a client of the table.
 *
 *	Table rows and columns may be tagged with strings.  A row may have
 *	many tags.  The same tag may be used for many rows.  Tags are used and
 *	stored by clients of a table.  Tags can also be shared between clients
 *	of the same table.
 *	
 *	Both rowTable and columnTable are hash tables keyed by the physical
 *	row or column location in the table respectively.  This is not the
 *	same as the client's view (the order of rows or columns as seen by the
 *	client).  This is so that clients (which may have different views) can
 *	share tags without sharing the same view.
 */
typedef struct Blt_Dt_TraceStruct {
    unsigned int flags;
    char *rowTag, *colTag;
    Blt_Dt_Row row;
    Blt_Dt_Column column;
    Blt_Dt_TraceProc *proc;
    Blt_Dt_TraceDeleteProc *deleteProc;
    ClientData clientData;
    Blt_Chain chain;
    Blt_ChainLink link;
} *Blt_Dt_Trace;


#define DT_TRACE_READS		(1<<0)
#define DT_TRACE_CREATES	(1<<1)
#define DT_TRACE_WRITES		(1<<2)
#define DT_TRACE_UNSETS		(1<<3)
#define DT_TRACE_ALL		(DT_TRACE_UNSETS | DT_TRACE_WRITES | \
				 DT_TRACE_READS  | DT_TRACE_CREATES)
#define DT_TRACE_MASK		(TRACE_ALL)

#define DT_TRACE_FOREIGN_ONLY	(1<<8)
#define DT_TRACE_ACTIVE		(1<<9)
#define DT_TRACE_SELF		(1<<10)
#define DT_TRACE_DESTROYED	(1<<11)

BLT_EXTERN void Blt_Dt_ClearRowTags(Blt_Dt table, Blt_Dt_Row row);

BLT_EXTERN void Blt_Dt_ClearColumnTags(Blt_Dt table, Blt_Dt_Column column);

BLT_EXTERN void Blt_Dt_ClearRowTraces(Blt_Dt table, Blt_Dt_Row row);

BLT_EXTERN void Blt_Dt_ClearColumnTraces(Blt_Dt table, Blt_Dt_Column column);

BLT_EXTERN Blt_Dt_Trace Blt_Dt_CreateTrace(Blt_Dt table, Blt_Dt_Row row, 
	Blt_Dt_Column column, CONST char *rowTag, CONST char *columnTag, 
	unsigned int mask, Blt_Dt_TraceProc *proc, 
	Blt_Dt_TraceDeleteProc *deleteProc, ClientData clientData);

BLT_EXTERN Blt_Dt_Trace Blt_Dt_CreateColumnTrace(Blt_Dt table, 
	Blt_Dt_Column col, 	unsigned int mask, Blt_Dt_TraceProc *proc, 
	Blt_Dt_TraceDeleteProc *deleteProc, ClientData clientData);

BLT_EXTERN Blt_Dt_Trace Blt_Dt_CreateColumnTagTrace(Blt_Dt table, 
	CONST char *tag, unsigned int mask, Blt_Dt_TraceProc *proc, 
	Blt_Dt_TraceDeleteProc *deleteProc, ClientData clientData);

BLT_EXTERN Blt_Dt_Trace Blt_Dt_CreateRowTrace(Blt_Dt table, Blt_Dt_Row row, 
	unsigned int mask, Blt_Dt_TraceProc *proc, 
	Blt_Dt_TraceDeleteProc *deleteProc, ClientData clientData);

BLT_EXTERN Blt_Dt_Trace Blt_Dt_CreateRowTagTrace(Blt_Dt table, CONST char *tag, 
	unsigned int mask, Blt_Dt_TraceProc *proc, 
	Blt_Dt_TraceDeleteProc *deleteProc, ClientData clientData);

BLT_EXTERN void Blt_Dt_DeleteTrace(Blt_Dt_Trace trace);

/*
 * Blt_Dt_NotifyEvent --
 *
 *	Structure representing a trace used by a client of the table.
 *
 *	Table rows and columns may be tagged with strings.  A row may have
 *	many tags.  The same tag may be used for many rows.  Tags are used and
 *	stored by clients of a table.  Tags can also be shared between clients
 *	of the same table.
 *	
 *	Both rowTable and columnTable are hash tables keyed by the physical
 *	row or column location in the table respectively.  This is not the
 *	same as the client's view (the order of rows or columns as seen by the
 *	client).  This is so that clients (which may have different views) can
 *	share tags without sharing the same view.
 */
typedef struct {
    Tcl_Interp *interp;		/* Interpreter to report to */
    Blt_Dt table;		/* Table object client that received the
				 * event. */
    Blt_Dt_Header header;	/* Matching row or column. */
    int self;			/* Indicates if this table client generated
				 * the event. */
    int type;			/* Indicates type of event received. */

} Blt_Dt_NotifyEvent;

typedef int (Blt_Dt_NotifyEventProc)(ClientData clientData, 
	Blt_Dt_NotifyEvent *eventPtr);

typedef void (Blt_Dt_NotifierDeleteProc)(ClientData clientData);

typedef struct Blt_Dt_NotifierStruct {
    Blt_Dt table;
    Blt_ChainLink link;
    Blt_Chain chain;
    Blt_Dt_NotifyEvent event;
    Blt_Dt_NotifyEventProc *proc;
    Blt_Dt_NotifierDeleteProc *deleteProc;
    ClientData clientData;
    Tcl_Interp *interp;
    Blt_Dt_Header header;
    char *tag;
    unsigned int flags;
} *Blt_Dt_Notifier;


#define DT_NOTIFY_ROW_CREATED		(1<<0)
#define DT_NOTIFY_COLUMN_CREATED	(1<<1)
#define DT_NOTIFY_CREATE		(DT_NOTIFY_COLUMN_CREATED | \
					 DT_NOTIFY_ROW_CREATED)
#define DT_NOTIFY_ROW_DELETED		(1<<2)
#define DT_NOTIFY_COLUMN_DELETED	(1<<3)
#define DT_NOTIFY_DELETE		(DT_NOTIFY_COLUMN_DELETED | \
					 DT_NOTIFY_ROW_DELETED)
#define DT_NOTIFY_ROW_MOVED		(1<<4)
#define DT_NOTIFY_COLUMN_MOVED		(1<<5)
#define DT_NOTIFY_MOVE			(DT_NOTIFY_COLUMN_MOVED | \
					 DT_NOTIFY_ROW_MOVED)
#define DT_NOTIFY_COLUMN_CHANGED \
	(DT_NOTIFY_COLUMN_CREATED | DT_NOTIFY_COLUMN_DELETED | \
		DT_NOTIFY_COLUMN_MOVED)
#define DT_NOTIFY_ROW_CHANGED \
	(DT_NOTIFY_ROW_CREATED | DT_NOTIFY_ROW_DELETED | \
		DT_NOTIFY_ROW_MOVED)
    
#define DT_NOTIFY_ALL_EVENTS (DT_NOTIFY_ROW_CHANGED | DT_NOTIFY_COLUMN_CHANGED)
#define DT_NOTIFY_ROW		(1<<6)
#define DT_NOTIFY_COLUMN	(1<<7)
#define DT_NOTIFY_TYPE_MASK	(DT_NOTIFY_ROW | DT_NOTIFY_COLUMN)

#define DT_NOTIFY_EVENT_MASK	DT_NOTIFY_ALL_EVENTS
#define DT_NOTIFY_MASK		(DT_NOTIFY_EVENT_MASK | DT_NOTIFY_TYPE_MASK)

#define DT_NOTIFY_WHENIDLE	(1<<10)
#define DT_NOTIFY_FOREIGN_ONLY	(1<<11)
#define DT_NOTIFY_PENDING	(1<<12)
#define DT_NOTIFY_ACTIVE	(1<<13)
#define DT_NOTIFY_DESTROYED	(1<<14)

#define DT_NOTIFY_ALL		(NULL)

BLT_EXTERN Blt_Dt_Notifier Blt_Dt_CreateRowNotifier(Tcl_Interp *interp, 
	Blt_Dt table, Blt_Dt_Row row, unsigned int mask, 
	Blt_Dt_NotifyEventProc *proc, Blt_Dt_NotifierDeleteProc *deleteProc, 
	ClientData clientData);

BLT_EXTERN Blt_Dt_Notifier Blt_Dt_CreateRowTagNotifier(Tcl_Interp *interp, 
	Blt_Dt table, CONST char *tag, unsigned int mask, 
	Blt_Dt_NotifyEventProc *proc, Blt_Dt_NotifierDeleteProc *deleteProc, 
	ClientData clientData);

BLT_EXTERN Blt_Dt_Notifier Blt_Dt_CreateColumnNotifier(Tcl_Interp *interp, 
	Blt_Dt table, Blt_Dt_Column col, unsigned int mask, 
	Blt_Dt_NotifyEventProc *proc, Blt_Dt_NotifierDeleteProc *deleteProc, 
	ClientData clientData);

BLT_EXTERN Blt_Dt_Notifier Blt_Dt_CreateColumnTagNotifier(Tcl_Interp *interp, 
	Blt_Dt table, CONST char *tag, unsigned int mask, 
	Blt_Dt_NotifyEventProc *proc, Blt_Dt_NotifierDeleteProc *deleteProc, 
	ClientData clientData);


BLT_EXTERN void Blt_Dt_DeleteNotifier(Blt_Dt_Notifier notifier);

/*
 * Blt_Dt_SortOrder --
 *
 */

typedef int (Blt_Dt_SortProc) _ANSI_ARGS_((ClientData clientData, 
	Tcl_Obj *valueObjPtr1, Tcl_Obj *valueObjPtr2));

typedef struct {
    int type;			/* Type of sort to be performed: see flags
				 * below. */
    Blt_Dt_SortProc *proc;	/* Procedures to be called to compare two
				 * entries in the same row or column. */
    ClientData clientData;	/* One word of data passed to the sort
				 * comparison procedure above. */
    Blt_Dt_Column column;	/* Column to be compared. */

} Blt_Dt_SortOrder;

#define DT_SORT_NONE		0
#define DT_SORT_INTEGER		(1<<0)
#define DT_SORT_DOUBLE		(1<<1)
#define DT_SORT_DICTIONARY	(1<<2)
#define DT_SORT_ASCII		(1<<3)
#define DT_SORT_CUSTOM		(1<<4)
#define DT_SORT_DECREASING	(1<<6)

BLT_EXTERN Blt_Dt_Row *Blt_Dt_SortRows(Blt_Dt table, Blt_Dt_SortOrder *order, 
	long nCompares, unsigned int flags);

BLT_EXTERN Blt_Dt_Row *Blt_Dt_RowMap(Blt_Dt table);
BLT_EXTERN Blt_Dt_Column *Blt_Dt_ColumnMap(Blt_Dt table);

BLT_EXTERN void Blt_Dt_SetRowMap(Blt_Dt table, Blt_Dt_Row *map);
BLT_EXTERN void Blt_Dt_SetColumnMap(Blt_Dt table, Blt_Dt_Column *map);

#define DT_RESTORE_NO_TAGS	    (1<<0)
#define DT_RESTORE_OVERWRITE	    (1<<1)

BLT_EXTERN int Blt_Dt_Restore(Tcl_Interp *interp, Blt_Dt table, char *string, 
	unsigned int flags);
BLT_EXTERN int Blt_Dt_FileRestore(Tcl_Interp *interp, Blt_Dt table, 
	CONST char *fileName, unsigned int flags);
BLT_EXTERN int Blt_Dt_Dump(Tcl_Interp *interp, Blt_Dt table, 
	Blt_Dt_Row *rowMap, Blt_Dt_Column *colMap, Tcl_DString *dsPtr);
BLT_EXTERN int Blt_Dt_FileDump(Tcl_Interp *interp, Blt_Dt table, 
	Blt_Dt_Row *rowMap, 	Blt_Dt_Column *colMap, CONST char *fileName);

typedef int (Blt_Dt_ImportProc)(Blt_Dt table, Tcl_Interp *interp, int objc, 
	Tcl_Obj *CONST *objv);

typedef int (Blt_Dt_ExportProc)(Blt_Dt table, Tcl_Interp *interp, int objc, 
	Tcl_Obj *CONST *objv);

BLT_EXTERN int Blt_Dt_RegisterFormat(Tcl_Interp *interp, CONST char *name, 
	Blt_Dt_ImportProc *importProc, Blt_Dt_ExportProc *exportProc);

BLT_EXTERN void Blt_Dt_UnsetKeys(Blt_Dt table);
BLT_EXTERN Blt_Chain Blt_Dt_GetKeys(Blt_Dt table);
BLT_EXTERN int Blt_Dt_SetKeys(Blt_Dt table, Blt_Chain keys, int unique);
BLT_EXTERN int Blt_Dt_KeyLookup(Tcl_Interp *interp, Blt_Dt table, int objc, 
	Tcl_Obj *CONST *objv, Blt_Dt_Row *rowPtr);


#define Blt_Dt_NumRows(t)	   ((t)->corePtr->rowData.nUsed)
#define Blt_Dt_RowIndex(r)	   ((r)->index)
#define Blt_Dt_RowLabel(r)	   ((r)->label)
#define Blt_Dt_GetRow(t,i)	\
    (Blt_Dt_Row)((t)->corePtr->rowData.map[(i)-1])

#define Blt_Dt_NumColumns(t)	   ((t)->corePtr->colData.nUsed)
#define Blt_Dt_ColumnIndex(c)      ((c)->index)
#define Blt_Dt_ColumnLabel(c)	   ((c)->label)
#define Blt_Dt_ColumnType(c)	   ((c)->type)
#define Blt_Dt_GetColumn(t,i)	\
    (Blt_Dt_Column)((t)->corePtr->colData.map[(i)-1])

#define Blt_Dt_TableName(t)	   ((t)->name)
#define Blt_Dt_EmptyValue(t)	   ((t)->emptyValue)


#endif /* BLT_DATATABLE_H */
