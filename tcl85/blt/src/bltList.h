
/*
 * bltList.h --
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
#ifndef _BLT_LIST_H
#define _BLT_LIST_H

/*
 * Acceptable key types for hash tables:
 */
#ifndef BLT_STRING_KEYS
#define BLT_STRING_KEYS		0
#endif
#ifndef BLT_ONE_WORD_KEYS
#define BLT_ONE_WORD_KEYS	((size_t)-1)
#endif

typedef struct Blt_ListStruct *Blt_List;
typedef struct Blt_ListNodeStruct *Blt_ListNode;

/*
 * A Blt_ListNode is the container structure for the Blt_List.
 */
struct Blt_ListNodeStruct {
    Blt_ListNode prev;		/* Link to the previous node */
    Blt_ListNode next;		/* Link to the next node */
    ClientData clientData;	/* Pointer to the data object */
    Blt_List list;		/* List to eventually insert node */
    union {			/* Key has one of these forms: */
	CONST char *oneWordValue; /* One-word value for key. */
	int *words[1];		/* Multiple integer words for key.  The actual
				 * size will be as large as necessary for this
				 * table's keys. */
	char string[4];		/* String for key.  The actual size will be as
				 * large as needed to hold the key. */
    } key;			/* MUST BE LAST FIELD IN RECORD!! */
};

typedef int (Blt_ListCompareProc) _ANSI_ARGS_((Blt_ListNode *node1Ptr, 
	Blt_ListNode *node2Ptr));

/*
 * A Blt_List is a doubly chained list structure.
 */
struct Blt_ListStruct {
    Blt_ListNode head;		/* Pointer to first element in list */
    Blt_ListNode tail;		/* Pointer to last element in list */
    long nNodes;		/* Number of node currently in the list. */
    size_t type;		/* Type of keys in list. */
};

BLT_EXTERN void Blt_ListInit _ANSI_ARGS_((Blt_List list, size_t type));
BLT_EXTERN void Blt_ListReset _ANSI_ARGS_((Blt_List list));
BLT_EXTERN Blt_List Blt_ListCreate _ANSI_ARGS_((size_t type));
BLT_EXTERN void Blt_ListDestroy _ANSI_ARGS_((Blt_List list));
BLT_EXTERN Blt_ListNode Blt_ListCreateNode _ANSI_ARGS_((Blt_List list, 
	CONST char *key));
BLT_EXTERN void Blt_ListDeleteNode _ANSI_ARGS_((Blt_ListNode node));

BLT_EXTERN Blt_ListNode Blt_ListAppend _ANSI_ARGS_((Blt_List list, 
	CONST char *key, ClientData clientData));
BLT_EXTERN Blt_ListNode Blt_ListPrepend _ANSI_ARGS_((Blt_List list, 
	CONST char *key, ClientData clientData));
BLT_EXTERN void Blt_ListLinkAfter _ANSI_ARGS_((Blt_List list, 
	Blt_ListNode node, Blt_ListNode afterNode));
BLT_EXTERN void Blt_ListLinkBefore _ANSI_ARGS_((Blt_List list, 
	Blt_ListNode node, Blt_ListNode beforeNode));
BLT_EXTERN void Blt_ListUnlinkNode _ANSI_ARGS_((Blt_ListNode node));
BLT_EXTERN Blt_ListNode Blt_ListGetNode _ANSI_ARGS_((Blt_List list, 
	CONST char *key));
BLT_EXTERN void Blt_ListDeleteNodeByKey _ANSI_ARGS_((Blt_List list, 
	CONST char *key));
BLT_EXTERN Blt_ListNode Blt_ListGetNthNode _ANSI_ARGS_((Blt_List list,
	long position, int direction));
BLT_EXTERN void Blt_ListSort _ANSI_ARGS_((Blt_List list,
	Blt_ListCompareProc * proc));

#define Blt_ListGetLength(list) \
	(((list) == NULL) ? 0 : ((struct Blt_ListStruct *)list)->nNodes)
#define Blt_ListFirstNode(list) \
	(((list) == NULL) ? NULL : ((struct Blt_ListStruct *)list)->head)
#define Blt_ListLastNode(list)	\
	(((list) == NULL) ? NULL : ((struct Blt_ListStruct *)list)->tail)
#define Blt_ListPrevNode(node)	((node)->prev)
#define Blt_ListNextNode(node) 	((node)->next)
#define Blt_ListGetKey(node)	\
	(((node)->list->type == BLT_STRING_KEYS) \
		 ? (node)->key.string : (node)->key.oneWordValue)
#define Blt_ListGetValue(node)  	((node)->clientData)
#define Blt_ListSetValue(node, value) \
	((node)->clientData = (ClientData)(value))
#define Blt_ListAppendNode(list, node) \
	(Blt_ListLinkBefore((list), (node), (Blt_ListNode)NULL))
#define Blt_ListPrependNode(list, node) \
	(Blt_ListLinkAfter((list), (node), (Blt_ListNode)NULL))

#endif /* _BLT_LIST_H */
