
/*
 * bltTree.h --
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

#ifndef _BLT_TREE_H
#define _BLT_TREE_H

#include <bltChain.h>
#include <bltHash.h>
#include <bltPool.h>

typedef struct Blt_TreeSlinkStruct *Blt_TreeSlink;
typedef struct Blt_TreeNodeStruct *Blt_TreeNode;
typedef struct Blt_TreeObjectStruct *Blt_TreeObject;
typedef struct Blt_TreeStruct *Blt_Tree;
typedef struct Blt_TreeTraceStruct *Blt_TreeTrace;
typedef struct Blt_TreeValueStruct *Blt_TreeValue;
typedef struct Blt_TreeTagEntryStruct Blt_TreeTagEntry;
typedef struct Blt_TreeTagTableStruct Blt_TreeTagTable;
typedef struct Blt_TreeInterpDataStruct Blt_TreeInterpData;

typedef CONST char *Blt_TreeKey;

#define TREE_CREATE		(1<<0)
#define TREE_NEWTAGS		(1<<1)

#define TREE_PREORDER		(1<<0)
#define TREE_POSTORDER		(1<<1)
#define TREE_INORDER		(1<<2)
#define TREE_BREADTHFIRST	(1<<3)

#define TREE_NODE_LINK		(1<<0)

#define TREE_TRACE_UNSET	(1<<3)
#define TREE_TRACE_WRITE	(1<<4)
#define TREE_TRACE_READ		(1<<5)
#define TREE_TRACE_CREATE	(1<<6)
#define TREE_TRACE_ALL		\
    (TREE_TRACE_UNSET | TREE_TRACE_WRITE | TREE_TRACE_READ | TREE_TRACE_CREATE)
#define TREE_TRACE_MASK		(TREE_TRACE_ALL)

#define TREE_TRACE_FOREIGN_ONLY	(1<<8)
#define TREE_TRACE_ACTIVE	(1<<9)

#define TREE_NOTIFY_CREATE	(1<<0)
#define TREE_NOTIFY_DELETE	(1<<1)
#define TREE_NOTIFY_MOVE	(1<<2)
#define TREE_NOTIFY_SORT	(1<<3)
#define TREE_NOTIFY_RELABEL	(1<<4)
#define TREE_NOTIFY_ALL		\
    (TREE_NOTIFY_CREATE | TREE_NOTIFY_DELETE | TREE_NOTIFY_MOVE | \
	TREE_NOTIFY_SORT | TREE_NOTIFY_RELABEL)
#define TREE_NOTIFY_MASK	(TREE_NOTIFY_ALL)

#define TREE_NOTIFY_WHENIDLE	 (1<<8)
#define TREE_NOTIFY_FOREIGN_ONLY (1<<9)
#define TREE_NOTIFY_ACTIVE	 (1<<10)

#define TREE_RESTORE_NO_TAGS	(1<<0)
#define TREE_RESTORE_OVERWRITE	(1<<1)

#define TREE_INCLUDE_ROOT	(1<<0)

typedef struct {
    int type;
    Blt_Tree tree;
    long inode;			/* Node of event */
    Tcl_Interp *interp;
} Blt_TreeNotifyEvent;

typedef struct {
    Blt_TreeNode node;		/* Node being searched. */
    unsigned long nextIndex;	/* Index of next bucket to be enumerated after
				 * present one. */
    Blt_TreeValue nextValue;	/* Next entry to be enumerated in the the
				 * current bucket. */
} Blt_TreeKeySearch;

/*
 * Blt_TreeObject --
 *
 *	Structure providing the internal representation of the tree object. A
 *	tree is uniquely identified by a combination of its name and
 *	originating namespace.  Two trees in the same interpreter can have the
 *	same names but must reside in different namespaces.
 *
 *	The tree object represents a general-ordered tree of nodes.  Each node
 *	may contain a heterogeneous collection of data values. Each value is
 *	identified by a field name and nodes do not need to contain the same
 *	data fields. Data field names are saved as reference counted strings
 *	and can be shared among nodes.
 *
 *	The tree is threaded.  Each node contains pointers to back its parents
 *	to its next sibling.  
 * 
 *	A tree object can be shared by several clients.  When a client wants
 *	to use a tree object, it is given a token that represents the tree.
 *	The tree object uses the tokens to keep track of its clients.  When
 *	all clients have released their tokens the tree is automatically
 *	destroyed.
 */

struct Blt_TreeObjectStruct {
    Blt_TreeNode root;		/* Root of the entire tree. */

    CONST char *sortNodesCmd;	/* Tcl command to invoke to sort entries */

    Blt_Chain clients;		/* List of clients using this tree */

    Blt_Pool nodePool;
    Blt_Pool valuePool;
    
    Blt_HashTable nodeTable;	/* Table of node identifiers. Used to
				 * search for a node pointer given an inode.*/
    Blt_TreeInterpData *dataPtr;
    long nextInode;

    long nNodes;		/* Always counts root node. */

    long depth;			/* Maximum depth of the tree. */

    unsigned int flags;		/* Internal flags. See definitions below. */
    unsigned int notifyFlags;	/* Notification flags. See definitions
				 * below. */
};

/*
 * Blt_TreeNodeStruct --
 *
 *	Structure representing a node in a general ordered tree.  Nodes are
 *	identified by their index, or inode.  Nodes also have names, but nodes
 *	names are not unique and can be changed.  Inodes are valid even if the
 *	node is moved.
 *
 *	Each node can contain a list of data fields.  Fields are name-value
 *	pairs.  The values are represented by Tcl_Objs.
 *	
 */
struct Blt_TreeNodeStruct {
    Blt_TreeNode parent;	/* Parent node. If NULL, then this is the root
				   node. */
    Blt_TreeNode next, prev;	/* Next/previous sibling nodes. */
    Blt_TreeNode hnext;		/* Next node in the hash bucket. */
    
    Blt_TreeKey label;		/* Node label (doesn't have to be unique). */
    long inode;			/* Serial number of the node. */

    Blt_TreeObject corePtr;	/* Pointer back to the tree object that
				 * contains this node. */
    size_t depth;		/* The depth of this node in the tree. */
    size_t nChildren;		/* # of children for this node. */
    Blt_TreeNode first, last;	/* First/last nodes of child nodes stored as a
				 * linked list. */
    Blt_TreeNode *nodeTable;    /* Hash table of child nodes. */
    size_t nodeTableSize2;	/* Log2 size of child node hash table. */

    Blt_TreeValue values;	/* Chain of Blt_TreeValue structures.  Each
				 * value structure contains a key/value data
				 * pair.  The data value is a Tcl_Obj. */

    Blt_TreeValue *valueTable;	/* Hash table for values. When the number of
				 * values reaches exceeds a threshold, values
				 * will also be linked into this hash
				 * table. */

    unsigned short nValues;	/* # of values for this node. */
    unsigned short valueTableSize2; /* Size of hash table indicated as a power
				 * of 2 (e.g. if logSize=3, then table size is
				 * 8). If 0, this indicates that the node's
				 * values are stored as a list. */
    unsigned int flags;		/* Indicates if this node is currently used
				 * within an active trace. */
};

struct Blt_TreeTagEntryStruct {
    CONST char *tagName;
    Blt_HashEntry *hashPtr;
    Blt_HashTable nodeTable;
};

struct Blt_TreeTagTableStruct {
    Blt_HashTable tagTable;
    int refCount;
};

/*
 * Blt_TreeStruct --
 *
 *	A tree may be shared by several clients.  Each client allocates this
 *	structure which acts as a ticket for using the tree.  Each client can
 *
 *      - Designate notifier routines that are automatically invoked by the
 *        tree object when nodes are created, deleted, moved, etc. by other
 *        clients.
 *      - Place traces on the values of specific nodes.
 *      - Manage its own set or common of tags for nodes of the tree. By
 *        default, clients share tags.
 */

struct Blt_TreeStruct {
    unsigned int magic;		/* Magic value indicating whether a generic
				 * pointer is really a datatable token or
				 * not. */
    CONST char *name;		/* Fully namespace-qualified name of the
				 * client. */
    Blt_TreeObject corePtr;	/* Pointer to the structure containing the
				 * master information about the tree used by
				 * the client.  If NULL, this indicates that
				 * the tree has been destroyed (but as of yet,
				 * this client hasn't recognized it). */
    Tcl_Interp *interp;		/* Interpreter associated with this tree. */

    Blt_HashEntry *hPtr;	/* This client's entry in the above
				 * table. This is a list of clients that all
				 * have the same qualified table name
				 * (i.e. are sharing the same table. */

    Blt_ChainLink link;		/* Pointer to this link in the server's chain
				 * of clients. */
    
    Blt_Chain events;		/* Chain of node event handlers. */
    Blt_Chain traces;		/* Chain of data field callbacks. */
    Blt_TreeNode root;		/* Designated root for this client */
    Blt_TreeTagTable *tagTablePtr; /* Tag table used by this client. */ 
};


typedef int (Blt_TreeNotifyEventProc) _ANSI_ARGS_((ClientData clientData, 
	Blt_TreeNotifyEvent *eventPtr));

typedef int (Blt_TreeTraceProc) _ANSI_ARGS_((ClientData clientData, 
	Tcl_Interp *interp, Blt_TreeNode node, Blt_TreeKey key, 
	unsigned int flags));

typedef int (Blt_TreeEnumProc) _ANSI_ARGS_((Blt_TreeNode node, Blt_TreeKey key,
	Tcl_Obj *valuePtr));

typedef int (Blt_TreeCompareNodesProc) _ANSI_ARGS_((Blt_TreeNode *n1Ptr, 
	Blt_TreeNode *n2Ptr));

typedef int (Blt_TreeApplyProc) _ANSI_ARGS_((Blt_TreeNode node, 
	ClientData clientData, int order));

struct Blt_TreeTraceStruct {
    ClientData clientData;
    Blt_TreeKey key;
    Blt_TreeNode node;
    unsigned int mask;
    Blt_TreeTraceProc *proc;
};

/*
 * Structure definition for information used to keep track of searches through
 * hash tables:
 */
struct Blt_TreeKeySearchStruct {
    Blt_TreeNode node;		/* Table being searched. */
    unsigned long nextIndex;	/* Index of next bucket to be enumerated after
				 * present one. */
    Blt_TreeValue nextValue;	/* Next entry to be enumerated in the the
				 * current bucket. */
};

BLT_EXTERN Blt_TreeKey Blt_TreeGetKey _ANSI_ARGS_((Blt_Tree tree, 
	CONST char *string));
BLT_EXTERN Blt_TreeKey Blt_TreeGetKeyFromNode _ANSI_ARGS_((Blt_TreeNode node, 
	CONST char *string));
BLT_EXTERN Blt_TreeKey Blt_TreeGetKeyFromInterp _ANSI_ARGS_((Tcl_Interp *interp,
	CONST char *string));

BLT_EXTERN Blt_TreeNode Blt_TreeCreateNode _ANSI_ARGS_((Blt_Tree tree, 
	Blt_TreeNode parent, CONST char *name, long position)); 

BLT_EXTERN Blt_TreeNode Blt_TreeCreateNodeWithId _ANSI_ARGS_((Blt_Tree tree, 
	Blt_TreeNode parent, CONST char *name, long inode, long position)); 

BLT_EXTERN int Blt_TreeDeleteNode _ANSI_ARGS_((Blt_Tree tree, 
	Blt_TreeNode node));

BLT_EXTERN int Blt_TreeMoveNode _ANSI_ARGS_((Blt_Tree tree, Blt_TreeNode node, 
	Blt_TreeNode parent, Blt_TreeNode before));

BLT_EXTERN Blt_TreeNode Blt_TreeGetNode _ANSI_ARGS_((Blt_Tree tree, 
	long inode));

BLT_EXTERN Blt_TreeNode Blt_TreeFindChild _ANSI_ARGS_((Blt_TreeNode parent, 
	CONST char *name));

BLT_EXTERN Blt_TreeNode Blt_TreeNextNode _ANSI_ARGS_((Blt_TreeNode root, 
	Blt_TreeNode node));

BLT_EXTERN Blt_TreeNode Blt_TreePrevNode _ANSI_ARGS_((Blt_TreeNode root,
	Blt_TreeNode node));

BLT_EXTERN Blt_TreeNode Blt_TreeFirstChild _ANSI_ARGS_((Blt_TreeNode parent));

BLT_EXTERN Blt_TreeNode Blt_TreeLastChild _ANSI_ARGS_((Blt_TreeNode parent));

BLT_EXTERN Blt_TreeNode Blt_TreeChangeRoot _ANSI_ARGS_((Blt_Tree tree,
	Blt_TreeNode node));

BLT_EXTERN Blt_TreeNode Blt_TreeEndNode _ANSI_ARGS_((Blt_TreeNode node,
	unsigned int nodeFlags));

BLT_EXTERN int Blt_TreeIsBefore _ANSI_ARGS_((Blt_TreeNode node1, 
	Blt_TreeNode node2));

BLT_EXTERN int Blt_TreeIsAncestor _ANSI_ARGS_((Blt_TreeNode node1, 
	Blt_TreeNode node2));

BLT_EXTERN int Blt_TreePrivateValue _ANSI_ARGS_((Tcl_Interp *interp, 
	Blt_Tree tree, Blt_TreeNode node, Blt_TreeKey key));

BLT_EXTERN int Blt_TreePublicValue _ANSI_ARGS_((Tcl_Interp *interp, 
	Blt_Tree tree, Blt_TreeNode node, Blt_TreeKey key));

BLT_EXTERN int Blt_TreeGetValue _ANSI_ARGS_((Tcl_Interp *interp, Blt_Tree tree, 
	Blt_TreeNode node, CONST char *string, Tcl_Obj **valuePtr));

BLT_EXTERN int Blt_TreeValueExists _ANSI_ARGS_((Blt_Tree tree, 
	Blt_TreeNode node, CONST char *string));

BLT_EXTERN int Blt_TreeSetValue _ANSI_ARGS_((Tcl_Interp *interp, Blt_Tree tree, 
	Blt_TreeNode node, CONST char *string, Tcl_Obj *valuePtr));

BLT_EXTERN int Blt_TreeUnsetValue _ANSI_ARGS_((Tcl_Interp *interp, 
	Blt_Tree tree, Blt_TreeNode node, CONST char *string));

BLT_EXTERN int Blt_TreeGetArrayValue _ANSI_ARGS_((Tcl_Interp *interp, 
	Blt_Tree tree, Blt_TreeNode node, CONST char *arrayName, 
	CONST char *elemName, Tcl_Obj **valueObjPtrPtr));

BLT_EXTERN int Blt_TreeSetArrayValue _ANSI_ARGS_((Tcl_Interp *interp, 
	Blt_Tree tree, Blt_TreeNode node, CONST char *arrayName, 
	CONST char *elemName, Tcl_Obj *valueObjPtr));

BLT_EXTERN int Blt_TreeUnsetArrayValue _ANSI_ARGS_((Tcl_Interp *interp, 
	Blt_Tree tree, Blt_TreeNode node, CONST char *arrayName, 
	CONST char *elemName));

BLT_EXTERN int Blt_TreeArrayValueExists _ANSI_ARGS_((Blt_Tree tree, 
	Blt_TreeNode node, CONST char *arrayName, CONST char *elemName));

BLT_EXTERN int Blt_TreeArrayNames _ANSI_ARGS_((Tcl_Interp *interp, 
	Blt_Tree tree, Blt_TreeNode node, CONST char *arrayName, 
	Tcl_Obj *listObjPtr));

BLT_EXTERN int Blt_TreeGetValueByKey _ANSI_ARGS_((Tcl_Interp *interp, 
	Blt_Tree tree, Blt_TreeNode node, Blt_TreeKey key, 
	Tcl_Obj **valuePtr));

BLT_EXTERN int Blt_TreeSetValueByKey _ANSI_ARGS_((Tcl_Interp *interp, 
	Blt_Tree tree, Blt_TreeNode node, Blt_TreeKey key, Tcl_Obj *valuePtr));

BLT_EXTERN int Blt_TreeUnsetValueByKey _ANSI_ARGS_((Tcl_Interp *interp, 
	Blt_Tree tree, Blt_TreeNode node, Blt_TreeKey key));

BLT_EXTERN int Blt_TreeValueExistsByKey _ANSI_ARGS_((Blt_Tree tree, 
	Blt_TreeNode node, Blt_TreeKey key));

BLT_EXTERN Blt_TreeKey Blt_TreeFirstKey _ANSI_ARGS_((Blt_Tree tree, 
	Blt_TreeNode node, Blt_TreeKeySearch *cursorPtr));

BLT_EXTERN Blt_TreeKey Blt_TreeNextKey _ANSI_ARGS_((Blt_Tree tree, 
	Blt_TreeKeySearch *cursorPtr));

BLT_EXTERN int Blt_TreeApply _ANSI_ARGS_((Blt_TreeNode root, 
	Blt_TreeApplyProc *proc, ClientData clientData));

BLT_EXTERN int Blt_TreeApplyDFS _ANSI_ARGS_((Blt_TreeNode root, 
	Blt_TreeApplyProc *proc, ClientData clientData, int order));

BLT_EXTERN int Blt_TreeApplyBFS _ANSI_ARGS_((Blt_TreeNode root, 
	Blt_TreeApplyProc *proc, ClientData clientData));

BLT_EXTERN int Blt_TreeSortNode _ANSI_ARGS_((Blt_Tree tree, Blt_TreeNode node, 
	Blt_TreeCompareNodesProc *proc));

BLT_EXTERN int Blt_TreeExists _ANSI_ARGS_((Tcl_Interp *interp, 
	CONST char *name));

BLT_EXTERN Blt_Tree Blt_TreeOpen _ANSI_ARGS_((Tcl_Interp *interp, 
	CONST char *name, int flags));

BLT_EXTERN void Blt_TreeClose _ANSI_ARGS_((Blt_Tree tree));

BLT_EXTERN int Blt_TreeAttach _ANSI_ARGS_((Tcl_Interp *interp, Blt_Tree tree,
	CONST char *name));

BLT_EXTERN int Blt_TreeSize _ANSI_ARGS_((Blt_TreeNode node));

BLT_EXTERN Blt_TreeTrace Blt_TreeCreateTrace _ANSI_ARGS_((Blt_Tree tree, 
	Blt_TreeNode node, CONST char *keyPattern, CONST char *tagName,
	unsigned int mask, Blt_TreeTraceProc *proc, ClientData clientData));

BLT_EXTERN void Blt_TreeDeleteTrace _ANSI_ARGS_((Blt_TreeTrace token));

BLT_EXTERN void Blt_TreeCreateEventHandler _ANSI_ARGS_((Blt_Tree tree, 
	unsigned int mask, Blt_TreeNotifyEventProc *proc, 
	ClientData clientData));

BLT_EXTERN void Blt_TreeDeleteEventHandler _ANSI_ARGS_((Blt_Tree tree, 
	unsigned int mask, Blt_TreeNotifyEventProc *proc, 
	ClientData clientData));

BLT_EXTERN void Blt_TreeRelabelNode _ANSI_ARGS_((Blt_Tree tree, 
	Blt_TreeNode node, CONST char *string));
BLT_EXTERN void Blt_TreeRelabelNodeWithoutNotify _ANSI_ARGS_((Blt_TreeNode node,
	CONST char *string));

BLT_EXTERN CONST char *Blt_TreeNodeIdAscii _ANSI_ARGS_((Blt_TreeNode node));

BLT_EXTERN CONST char *Blt_TreeNodePath _ANSI_ARGS_((Blt_TreeNode node, 
	Tcl_DString *resultPtr));	

BLT_EXTERN CONST char *Blt_TreeNodeRelativePath _ANSI_ARGS_((Blt_TreeNode root, 
	Blt_TreeNode node, CONST char *separator, unsigned int flags, 
	Tcl_DString *resultPtr));

BLT_EXTERN long Blt_TreeNodePosition _ANSI_ARGS_((Blt_TreeNode node));

BLT_EXTERN void Blt_TreeClearTags _ANSI_ARGS_((Blt_Tree tree, 
	Blt_TreeNode node));
BLT_EXTERN int Blt_TreeHasTag _ANSI_ARGS_((Blt_Tree tree, Blt_TreeNode node, 
	CONST char *tagName));
BLT_EXTERN void Blt_TreeAddTag _ANSI_ARGS_((Blt_Tree tree, Blt_TreeNode node, 
	CONST char *tagName));
BLT_EXTERN void Blt_TreeRemoveTag _ANSI_ARGS_((Blt_Tree tree, Blt_TreeNode node,
	CONST char *tagName));
BLT_EXTERN void Blt_TreeForgetTag _ANSI_ARGS_((Blt_Tree tree, 
	CONST char *tagName));
BLT_EXTERN Blt_HashTable *Blt_TreeTagHashTable _ANSI_ARGS_((Blt_Tree tree, 
	CONST char *tagName));
BLT_EXTERN int Blt_TreeTagTableIsShared _ANSI_ARGS_((Blt_Tree tree));
BLT_EXTERN void Blt_TreeNewTagTable _ANSI_ARGS_((Blt_Tree tree));

BLT_EXTERN Blt_HashEntry *Blt_TreeFirstTag _ANSI_ARGS_((Blt_Tree tree, 
	Blt_HashSearch *searchPtr));

BLT_EXTERN int Blt_TreeImportData _ANSI_ARGS_((Tcl_Interp *interp, 
	Blt_Tree tree, Blt_TreeNode root, Tcl_Obj *objPtr,  
	unsigned int flags));

BLT_EXTERN int Blt_TreeImportFile _ANSI_ARGS_((Tcl_Interp *interp, 
	Blt_Tree tree, Blt_TreeNode root, Tcl_Obj *objPtr,  
	unsigned int flags));

BLT_EXTERN void Blt_TreeDumpNode _ANSI_ARGS_((Blt_Tree tree, Blt_TreeNode root,
	Blt_TreeNode node, Tcl_DString *resultPtr));

BLT_EXTERN int Blt_TreeDump _ANSI_ARGS_((Blt_Tree tree, Blt_TreeNode root, 
	Tcl_DString *resultPtr));

BLT_EXTERN int Blt_TreeDumpToFile _ANSI_ARGS_((Tcl_Interp *interp, 
	Blt_Tree tree, Blt_TreeNode root, CONST char *fileName));

BLT_EXTERN int Blt_TreeRestore _ANSI_ARGS_((Tcl_Interp *interp, Blt_Tree tree, 
	Blt_TreeNode root, CONST char *string, unsigned int flags));

BLT_EXTERN int Blt_TreeRestoreFromFile _ANSI_ARGS_((Tcl_Interp *interp, 
	Blt_Tree tree, Blt_TreeNode root, CONST char *fileName, 
	unsigned int flags));

#define Blt_TreeName(token)	((token)->name)
#define Blt_TreeRootNode(token)	((token)->root)

#define Blt_TreeNodeDepth(node)	 ((node)->depth)
#define Blt_TreeNodeLabel(node)	 ((node)->label)
#define Blt_TreeNodeId(node)	 ((node)->inode)
#define Blt_TreeNodeParent(node) ((node == NULL) ? NULL : (node)->parent)
#define Blt_TreeNodeDegree(node) ((node)->nChildren)

#define Blt_TreeIsLeaf(node)     ((node)->nChildren == 0)
#define Blt_TreeIsLink(node)     ((node)->flags & TREE_NODE_LINK)
#define Blt_TreeNextNodeId(token)     ((token)->corePtr->nextInode)

#define Blt_TreeNextSibling(node) (((node) == NULL) ? NULL : (node)->next)
#define Blt_TreePrevSibling(node) (((node) == NULL) ? NULL : (node)->prev)

typedef int (Blt_TreeImportProc)(Tcl_Interp *interp, Blt_Tree tree, int objc, 
	Tcl_Obj *CONST *objv);

typedef int (Blt_TreeExportProc)(Tcl_Interp *interp, Blt_Tree tree, int objc, 
	Tcl_Obj *CONST *objv);

BLT_EXTERN int Blt_TreeRegisterFormat _ANSI_ARGS_((Tcl_Interp *interp, 
	CONST char *fmtName, Blt_TreeImportProc *importProc, 
	Blt_TreeExportProc *exportProc));

BLT_EXTERN Blt_TreeTagEntry *Blt_TreeRememberTag(Blt_Tree tree, 
	CONST char *name);

#endif /* _BLT_TREE_H */

