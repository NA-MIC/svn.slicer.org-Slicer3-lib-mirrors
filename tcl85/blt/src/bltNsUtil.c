
/*
 * bltNsUtil.c --
 *
 * This module implements utility namespace procedures for the BLT
 * toolkit.
 *
 *	Copyright 1997-2004 George A Howlett.
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
#include <bltHash.h>
#include "bltNsUtil.h"
#include "bltVar.h"
#include <bltList.h>

/* Namespace related routines */

typedef struct {
    char *result;
    Tcl_FreeProc *freeProc;
    int errorLine;
    Tcl_HashTable commandTable;
    Tcl_HashTable mathFuncTable;

    Tcl_HashTable globalTable;	/* This is the only field we care about */

    int nLevels;
    int maxNestingDepth;
} TclInterp;


typedef struct ArraySearchStruct ArraySearch;
typedef struct VarTraceStruct VarTrace;
typedef struct NamespaceStruct Namespace;

typedef struct Var {
    union {
	Tcl_Obj *objPtr;	/* The variable's object value. Used for 
				 * scalar variables and array elements. */
	Tcl_HashTable *tablePtr;/* For array variables, this points to
				 * information about the hash table used
				 * to implement the associative array. 
				 * Points to malloc-ed data. */
	struct Var *linkPtr;	/* If this is a global variable being
				 * referred to in a procedure, or a variable
				 * created by "upvar", this field points to
				 * the referenced variable's Var struct. */
    } value;
    char *name;			/* NULL if the variable is in a hashtable,
				 * otherwise points to the variable's
				 * name. It is used, e.g., by TclLookupVar
				 * and "info locals". The storage for the
				 * characters of the name is not owned by
				 * the Var and must not be freed when
				 * freeing the Var. */
    Namespace *nsPtr;		/* Points to the namespace that contains
				 * this variable or NULL if the variable is
				 * a local variable in a Tcl procedure. */
    Tcl_HashEntry *hPtr;	/* If variable is in a hashtable, either the
				 * hash table entry that refers to this
				 * variable or NULL if the variable has been
				 * detached from its hash table (e.g. an
				 * array is deleted, but some of its
				 * elements are still referred to in
				 * upvars). NULL if the variable is not in a
				 * hashtable. This is used to delete an
				 * variable from its hashtable if it is no
				 * longer needed. */
    int refCount;		/* Counts number of active uses of this
				 * variable, not including its entry in the
				 * call frame or the hash table: 1 for each
				 * additional variable whose linkPtr points
				 * here, 1 for each nested trace active on
				 * variable, and 1 if the variable is a 
				 * namespace variable. This record can't be
				 * deleted until refCount becomes 0. */
    VarTrace *tracePtr;		/* First in list of all traces set for this
				 * variable. */
    ArraySearch *searchPtr;	/* First in list of all searches active
				 * for this variable, or NULL if none. */
    int flags;			/* Miscellaneous bits of information about
				 * variable. See below for definitions. */
} Var;

#define VAR_SCALAR		0x1
#define VAR_ARRAY		0x2
#define VAR_LINK		0x4
#define VAR_UNDEFINED		0x8
#define VAR_IN_HASHTABLE	0x10
#define VAR_TRACE_ACTIVE	0x20
#define VAR_ARRAY_ELEMENT	0x40
#define VAR_NAMESPACE_VAR	0x80

#define VAR_ARGUMENT		0x100
#define VAR_TEMPORARY		0x200
#define VAR_RESOLVED		0x400	

/*
 * A Command structure exists for each command in a namespace. The
 * Tcl_Command opaque type actually refers to these structures.
 */

typedef struct CompileProcStruct CompileProc;
typedef struct ImportRefStruct ImportRef;

typedef struct {
    Tcl_HashEntry *hPtr;	/* Pointer to the hash table entry that
				 * refers to this command. The hash table is
				 * either a namespace's command table or an
				 * interpreter's hidden command table. This
				 * pointer is used to get a command's name
				 * from its Tcl_Command handle. NULL means
				 * that the hash table entry has been
				 * removed already (this can happen if
				 * deleteProc causes the command to be
				 * deleted or recreated). */
    Tcl_Namespace *nsPtr;	/* Points to the namespace containing this
				 * command. */
    int refCount;		/* 1 if in command hashtable plus 1 for each
				 * reference from a CmdName Tcl object
				 * representing a command's name in a
				 * ByteCode instruction sequence. This
				 * structure can be freed when refCount
				 * becomes zero. */
    int cmdEpoch;		/* Incremented to invalidate any references
				 * that point to this command when it is
				 * renamed, deleted, hidden, or exposed. */
    CompileProc *compileProc;	/* Procedure called to compile command. NULL
				 * if no compile proc exists for command. */
    Tcl_ObjCmdProc *objProc;	/* Object-based command procedure. */
    ClientData objClientData;	/* Arbitrary value passed to object proc. */
    Tcl_CmdProc *proc;		/* String-based command procedure. */
    ClientData clientData;	/* Arbitrary value passed to string proc. */
    Tcl_CmdDeleteProc *deleteProc;
				/* Procedure invoked when deleting command
				 * to, e.g., free all client data. */
    ClientData deleteData;	/* Arbitrary value passed to deleteProc. */
    int deleted;		/* Means that the command is in the process
				 * of being deleted (its deleteProc is
				 * currently executing). Other attempts to
				 * delete the command should be ignored. */
    ImportRef *importRefPtr;	/* List of each imported Command created in
				 * another namespace when this command is
				 * imported. These imported commands
				 * redirect invocations back to this
				 * command. The list is used to remove all
				 * those imported commands when deleting
				 * this "real" command. */
} Command;


/*
 *---------------------------------------------------------------------------
 *
 * Blt_GetVariableNamespace --
 *
 *	Returns the namespace context of the variable.  If NULL, this
 *	indicates that the variable is local to the call frame.
 *
 * Results:
 *	Returns the context of the namespace in an opaque type.
 *
 *---------------------------------------------------------------------------
 */
Tcl_Namespace *
Blt_GetVariableNamespace(Tcl_Interp *interp, const char *path)
{
    Blt_ObjectName objName;

    if (!Blt_ParseObjectName(interp, path, &objName, BLT_NO_DEFAULT_NS)) {
	return NULL;
    }
    if (objName.nsPtr == NULL) {
	Var *varPtr;

	varPtr = (Var *)Tcl_FindNamespaceVar(interp, (char *)path, 
		(Tcl_Namespace *)NULL, TCL_NAMESPACE_ONLY);
	if (varPtr != NULL) {
	    return (Tcl_Namespace *)varPtr->nsPtr;
	}
	varPtr = (Var *)Tcl_FindNamespaceVar(interp, (char *)path, 
		(Tcl_Namespace *)NULL, TCL_GLOBAL_ONLY);
	if (varPtr != NULL) {
	    return (Tcl_Namespace *)varPtr->nsPtr;
	}
    }
    return objName.nsPtr;    
}

/*ARGSUSED*/
Tcl_Namespace *
Blt_GetCommandNamespace(Tcl_Command cmdToken)
{
    Command *cmdPtr = (Command *)cmdToken;

    return (Tcl_Namespace *)cmdPtr->nsPtr;
}

Tcl_CallFrame *
Blt_EnterNamespace(Tcl_Interp *interp, Tcl_Namespace *nsPtr)
{
    Tcl_CallFrame *framePtr;

    framePtr = Blt_MallocAssert(sizeof(Tcl_CallFrame));
    if (Tcl_PushCallFrame(interp, framePtr, (Tcl_Namespace *)nsPtr, 0)
	!= TCL_OK) {
	Blt_Free(framePtr);
	return NULL;
    }
    return framePtr;
}

void
Blt_LeaveNamespace(Tcl_Interp *interp, Tcl_CallFrame *framePtr)
{
    Tcl_PopCallFrame(interp);
    Blt_Free(framePtr);
}

int
Blt_ParseObjectName(
    Tcl_Interp *interp,
    const char *path,
    Blt_ObjectName *objNamePtr,
    unsigned int flags)
{
    char *last, *colon;

    objNamePtr->nsPtr = NULL;
    objNamePtr->name = NULL;
    colon = NULL;
    /* Find the last namespace separator in the qualified name. */
    last = (char *)(path + strlen(path));
    while (--last > path) {
	if ((*last == ':') && (*(last - 1) == ':')) {
	    last++;		/* just after the last "::" */
	    colon = last - 2;
	    break;
	}
    }
    if (colon == NULL) {
	objNamePtr->name = path;
	if ((flags & BLT_NO_DEFAULT_NS) == 0) {
	    objNamePtr->nsPtr = Tcl_GetCurrentNamespace(interp);
	}
	return TRUE;		/* No namespace designated in name. */
    }

    /* Separate the namespace and the object name. */
    *colon = '\0';
    if (path[0] == '\0') {
	objNamePtr->nsPtr = Tcl_GetGlobalNamespace(interp);
    } else {
	objNamePtr->nsPtr = Tcl_FindNamespace(interp, (char *)path, NULL, 
		(flags & BLT_NO_ERROR_MSG) ? 0 : TCL_LEAVE_ERR_MSG);
    }
    /* Repair the string. */    *colon = ':';

    if (objNamePtr->nsPtr == NULL) {
	return FALSE;		/* Namespace doesn't exist. */
    }
    objNamePtr->name =last;
    return TRUE;
}

char *
Blt_MakeQualifiedName(
    Blt_ObjectName *objNamePtr,
    Tcl_DString *resultPtr)
{
    Tcl_DStringInit(resultPtr);
    if ((objNamePtr->nsPtr->fullName[0] != ':') || 
	(objNamePtr->nsPtr->fullName[1] != ':') ||
	(objNamePtr->nsPtr->fullName[2] != '\0')) {
	Tcl_DStringAppend(resultPtr, objNamePtr->nsPtr->fullName, -1);
    }
    Tcl_DStringAppend(resultPtr, "::", -1);
    Tcl_DStringAppend(resultPtr, (char *)objNamePtr->name, -1);
    return Tcl_DStringValue(resultPtr);
}

#ifdef notdef
/*
 *---------------------------------------------------------------------------
 *
 * Blt_CreateCommand --
 *
 *	Like Tcl_CreateCommand, but creates command in current namespace
 *	instead of global, if one isn't defined.  Not a problem with
 *	[incr Tcl] namespaces.
 *
 * Results:
 *	The return value is a token for the command, which can
 *	be used in future calls to Tcl_GetCommandName.
 *
 *---------------------------------------------------------------------------
 */
Tcl_Command
Blt_CreateCommand(
    Tcl_Interp *interp,		/* Token for command interpreter returned by
				 * a previous call to Tcl_CreateInterp. */
    const char *cmdName,	/* Name of command. If it contains namespace
				 * qualifiers, the new command is put in the
				 * specified namespace; otherwise it is put
				 * in the global namespace. */
    Tcl_CmdProc *proc,		/* Procedure to associate with cmdName. */
    ClientData clientData,	/* Arbitrary value passed to string proc. */
    Tcl_CmdDeleteProc *deleteProc) /* If not NULL, gives a procedure to call
				    * when this command is deleted. */
{
    const char *p;

    p = cmdName + strlen(cmdName);
    while (--p > cmdName) {
	if ((*p == ':') && (*(p - 1) == ':')) {
	    p++;		/* just after the last "::" */
	    break;
	}
    }
    if (cmdName == p) {
	Tcl_DString dString;
	Tcl_Namespace *nsPtr;
	Tcl_Command cmdToken;

	Tcl_DStringInit(&dString);
	nsPtr = Tcl_GetCurrentNamespace(interp);
	Tcl_DStringAppend(&dString, nsPtr->fullName, -1);
	Tcl_DStringAppend(&dString, "::", -1);
	Tcl_DStringAppend(&dString, cmdName, -1);
	cmdToken = Tcl_CreateCommand(interp, Tcl_DStringValue(&dString), proc,
	    clientData, deleteProc);
	Tcl_DStringFree(&dString);
	return cmdToken;
    }
    return Tcl_CreateCommand(interp, (char *)cmdName, proc, clientData, 
	deleteProc);
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_CreateCommandObj --
 *
 *	Like Tcl_CreateCommand, but creates command in current namespace
 *	instead of global, if one isn't defined.  Not a problem with
 *	[incr Tcl] namespaces.
 *
 * Results:
 *	The return value is a token for the command, which can
 *	be used in future calls to Tcl_GetCommandName.
 *
 *---------------------------------------------------------------------------
 */
Tcl_Command
Blt_CreateCommandObj(
    Tcl_Interp *interp,		/* Token for command interpreter returned by
				 * a previous call to Tcl_CreateInterp. */
    const char *cmdName,	/* Name of command. If it contains namespace
				 * qualifiers, the new command is put in the
				 * specified namespace; otherwise it is put
				 * in the global namespace. */
    Tcl_ObjCmdProc *proc,	/* Procedure to associate with cmdName. */
    ClientData clientData,	/* Arbitrary value passed to string proc. */
    Tcl_CmdDeleteProc *deleteProc) /* If not NULL, gives a procedure to call
				    * when this command is deleted. */
{
    const char *p;

    p = cmdName + strlen(cmdName);
    while (--p > cmdName) {
	if ((*p == ':') && (*(p - 1) == ':')) {
	    p++;		/* just after the last "::" */
	    break;
	}
    }
    if (cmdName == p) {
	Tcl_DString dString;
	Tcl_Namespace *nsPtr;
	Tcl_Command cmdToken;

	Tcl_DStringInit(&dString);
	nsPtr = Tcl_GetCurrentNamespace(interp);
	Tcl_DStringAppend(&dString, nsPtr->fullName, -1);
	Tcl_DStringAppend(&dString, "::", -1);
	Tcl_DStringAppend(&dString, cmdName, -1);
	cmdToken = Tcl_CreateObjCommand(interp, Tcl_DStringValue(&dString), 
		proc, clientData, deleteProc);
	Tcl_DStringFree(&dString);
	return cmdToken;
    }
    return Tcl_CreateObjCommand(interp, (char *)cmdName, proc, clientData, 
	deleteProc);
}

typedef struct {
    Tcl_HashTable clientTable;

    /* Original clientdata and delete procedure. */
    ClientData origClientData;
    Tcl_NamespaceDeleteProc *origDeleteProc;

} Callback;

static Tcl_CmdProc NamespaceDeleteCmd;
static Tcl_NamespaceDeleteProc NamespaceDeleteNotify;

#define NS_DELETE_CMD	"#NamespaceDeleteNotifier"

/*ARGSUSED*/
static int
NamespaceDeleteCmd(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,		/*  */
    int argc,
    const char **argv)
{
    Tcl_AppendResult(interp, "command \"", argv[0], "\" shouldn't be invoked",
	(char *)NULL);
    return TCL_ERROR;
}

static void
NamespaceDeleteNotify(ClientData clientData)
{
    Blt_List list;
    Blt_ListNode node;
    Tcl_CmdDeleteProc *deleteProc;

    list = (Blt_List)clientData;
    for (node = Blt_ListFirstNode(list); node != NULL;
	node = Blt_ListNextNode(node)) {
	deleteProc = (Tcl_CmdDeleteProc *)Blt_ListGetValue(node);
	clientData = (ClientData)Blt_ListGetKey(node);
	(*deleteProc) (clientData);
    }
    Blt_ListDestroy(list);
}

void
Blt_DestroyNsDeleteNotify(
    Tcl_Interp *interp,
    Tcl_Namespace *nsPtr,
    ClientData clientData)
{
    Blt_List list;
    Blt_ListNode node;
    char *string;
    Tcl_CmdInfo cmdInfo;
    size_t size;

    size = sizeof(nsPtr->fullName) + strlen(NS_DELETE_CMD) + 4;
    string = Blt_MallocAssert(size);
    sprintf_s(string, size, "%s::%s", nsPtr->fullName, NS_DELETE_CMD);
    if (!Tcl_GetCommandInfo(interp, string, &cmdInfo)) {
	goto done;
    }
    list = (Blt_List)cmdInfo.clientData;
    node = Blt_ListGetNode(list, clientData);
    if (node != NULL) {
	Blt_ListDeleteNode(node);
    }
  done:
    Blt_Free(string);
}

int
Blt_CreateNsDeleteNotify(
    Tcl_Interp *interp,
    Tcl_Namespace *nsPtr,
    ClientData clientData,
    Tcl_CmdDeleteProc *deleteProc)
{
    Blt_List list;
    char *string;
    Tcl_CmdInfo cmdInfo;
    size_t size;

    size = sizeof(nsPtr->fullName) + strlen(NS_DELETE_CMD) + 4;
    string = Blt_MallocAssert(size);
    sprintf_s(string, size, "%s::%s", nsPtr->fullName, NS_DELETE_CMD);
    if (!Tcl_GetCommandInfo(interp, string, &cmdInfo)) {
	list = Blt_ListCreate(BLT_ONE_WORD_KEYS);
	Blt_CreateCommand(interp, string, NamespaceDeleteCmd, list, 
		NamespaceDeleteNotify);
    } else {
	list = (Blt_List)cmdInfo.clientData;
    }
    Blt_Free(string);
    Blt_ListAppend(list, clientData, (ClientData)deleteProc);
    return TCL_OK;
}
#endif

/* 
 * Variable resolver routines.
 *
 * The following bit of magic is from [incr Tcl].  The following
 * routine are taken from [incr Tcl] to roughly duplicate how Tcl
 * internally creates variables.
 *
 * Note: There is no API for the variable resolver routines in the Tcl
 *	 library.  The resolver callback is supposed to return a Tcl_Var
 *	 back. But the definition of Tcl_Var in tcl.h is opaque.
 */

/*
 *---------------------------------------------------------------------------
 *
 * NewVar --
 *
 *	Create a new heap-allocated variable that will eventually be
 *	entered into a hashtable.
 *
 * Results:
 *	The return value is a pointer to the new variable structure. It is
 *	marked as a scalar variable (and not a link or array variable). Its
 *	value initially is NULL. The variable is not part of any hash table
 *	yet. Since it will be in a hashtable and not in a call frame, its
 *	name field is set NULL. It is initially marked as undefined.
 *
 * Side effects:
 *	Storage gets allocated.
 *
 *---------------------------------------------------------------------------
 */
static Var *
NewVar(const char *label, Tcl_Obj *objPtr)
{
    Var *varPtr;

    varPtr = Blt_MallocAssert(sizeof(Var));
    varPtr->value.objPtr = objPtr;
#ifdef notdef
    if (objPtr != NULL) {
	Tcl_IncrRefCount(objPtr);
    }
#endif
    varPtr->name = (char *)label;
    varPtr->nsPtr = NULL;
    /*
     *  NOTE:  Tcl reports a "dangling upvar" error for variables
     *         with a null "hPtr" field.  Put something non-zero
     *         in here to keep Tcl_SetVar2() happy.  The only time
     *         this field is really used is it remove a variable
     *         from the hash table that contains it in CleanupVar,
     *         but since these variables are protected by their
     *         higher refCount, they will not be deleted by CleanupVar
     *         anyway.  These variables are unset and removed in
     *         FreeCachedVars.
     */
    varPtr->hPtr = (Tcl_HashEntry *)0x01;
    varPtr->refCount = 1;  /* protect from being deleted */
    varPtr->tracePtr = NULL;
    varPtr->searchPtr = NULL;
    varPtr->flags = (VAR_SCALAR | VAR_IN_HASHTABLE);
    return varPtr;
}

Tcl_Var
Blt_GetCachedVar(
    Blt_HashTable *cacheTablePtr, 
    const char *label, 
    Tcl_Obj *objPtr)
{
    Blt_HashEntry *hPtr;
    int isNew;
    Var *varPtr;

    /* Check if the variable has been cached already. */
    hPtr = Blt_CreateHashEntry(cacheTablePtr, label, &isNew);
    if (isNew) {
	varPtr = NewVar(label, objPtr);
	Blt_SetHashValue(hPtr, varPtr);
    } else {
	varPtr = Blt_GetHashValue(hPtr);
	varPtr->value.objPtr = objPtr;
#ifdef notdef
	if (objPtr != NULL) {
	    Tcl_IncrRefCount(objPtr);
	}
#endif
    }
    return (Tcl_Var)varPtr;
}

void
Blt_FreeCachedVars(Blt_HashTable *cacheTablePtr)
{
    Blt_HashEntry *hPtr;
    Blt_HashSearch cursor;

    for (hPtr = Blt_FirstHashEntry(cacheTablePtr, &cursor); hPtr != NULL; 
	 hPtr = Blt_NextHashEntry(&cursor)) {
	Var *varPtr;

	varPtr = Blt_GetHashValue(hPtr);
	varPtr->refCount--;
	if (varPtr->refCount > 1) {
#ifdef notdef
	    if (varPtr->value.objPtr != NULL) {
		Tcl_DecrRefCount(varPtr->value.objPtr);
	    }
#endif
	    Blt_Free(varPtr);
	}
    }
    Blt_DeleteHashTable(cacheTablePtr);
}
