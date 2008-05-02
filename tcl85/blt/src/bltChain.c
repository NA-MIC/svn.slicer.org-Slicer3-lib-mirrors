
/*
 * bltChain.c --
 *
 * The module implements a generic linked list package.
 *
 *	Copyright 1991-2004 George A Howlett.
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
#include "bltChain.h"

#ifndef ALIGN
#define ALIGN(a) \
	(((size_t)a + (sizeof(double) - 1)) & (~(sizeof(double) - 1)))
#endif /* ALIGN */

typedef struct Blt_ChainLinkStruct ChainLink;
typedef struct Blt_ChainStruct Chain;

/*
 *---------------------------------------------------------------------------
 *
 * Blt_ChainCreate --
 *
 *	Creates a new linked list (chain) structure and initializes 
 *	its pointers;
 *
 * Results:
 *	Returns a pointer to the newly created chain structure.
 *
 *---------------------------------------------------------------------------
 */
Blt_Chain
Blt_ChainCreate(void)
{
    Chain *chainPtr;

    chainPtr = Blt_Malloc(sizeof(Chain));
    if (chainPtr != NULL) {
	Blt_ChainInit(chainPtr);
    }
    return chainPtr;
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_ChainAllocLink --
 *
 *	Creates a new chain link.  Unlink Blt_ChainNewLink, this 
 *	routine also allocates extra memory in the node for data.
 *
 * Results:
 *	The return value is the pointer to the newly created entry.
 *
 *---------------------------------------------------------------------------
 */
Blt_ChainLink
Blt_ChainAllocLink(size_t extraSize)
{
    ChainLink *linkPtr;
    size_t linkSize;

    linkSize = ALIGN(sizeof(ChainLink));
    linkPtr = Blt_CallocAssert(1, linkSize + extraSize);
    if (extraSize > 0) {
	/* Point clientData at the memory beyond the normal structure. */
	linkPtr->clientData = (ClientData)((char *)linkPtr + linkSize);
    }
    return linkPtr;
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_ChainInitLink --
 *
 *	Initializes the new link.  This routine is for applications
 *	that use their own memory allocation procedures to allocate
 *	links.
 *
 * Results:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
void
Blt_ChainInitLink(ChainLink *linkPtr)
{
    linkPtr->clientData = NULL;
    linkPtr->next = linkPtr->prev = NULL;
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_ChainNewLink --
 *
 *	Creates a new link.
 *
 * Results:
 *	The return value is the pointer to the newly created link.
 *
 *---------------------------------------------------------------------------
 */
Blt_ChainLink
Blt_ChainNewLink(void)
{
    ChainLink *linkPtr;

    linkPtr = Blt_MallocAssert(sizeof(ChainLink));
    linkPtr->clientData = NULL;
    linkPtr->next = linkPtr->prev = NULL;
    return linkPtr;
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_ChainReset --
 *
 *	Removes all the links from the chain, freeing the memory for
 *	each link.  Memory pointed to by the link (clientData) is not
 *	freed.  It's the caller's responsibility to deallocate it.
 *
 * Results:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
void
Blt_ChainReset(Chain *chainPtr) /* Chain to clear */
{
    if (chainPtr != NULL) {
	ChainLink *oldPtr;
	ChainLink *linkPtr = chainPtr->head;

	while (linkPtr != NULL) {
	    oldPtr = linkPtr;
	    linkPtr = linkPtr->next;
	    Blt_Free(oldPtr);
	}
	Blt_ChainInit(chainPtr);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_ChainDestroy
 *
 *     Frees all the nodes from the chain and deallocates the memory
 *     allocated for the chain structure itself.  It's assumed that
 *     the chain was previous allocated by Blt_ChainCreate.
 *
 * Results:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
void
Blt_ChainDestroy(Chain *chainPtr)
{
    if (chainPtr != NULL) {
	Blt_ChainReset(chainPtr);
	Blt_Free(chainPtr);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_ChainInit --
 *
 *	Initializes a linked list.
 *
 * Results:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
void
Blt_ChainInit(Chain *chainPtr)
{
    chainPtr->nLinks = 0;
    chainPtr->head = chainPtr->tail = NULL;
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_ChainLinkAfter --
 *
 *	Inserts a link after another link.  If afterPtr is NULL, then
 *	the new link is prepended to the beginning of the chain.
 *
 * Results:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
void
Blt_ChainLinkAfter(Chain *chainPtr, ChainLink *linkPtr, ChainLink *afterPtr)
{
    if (chainPtr->head == NULL) {
	chainPtr->tail = chainPtr->head = linkPtr;
    } else {
	if (afterPtr == NULL) {
	    /* Prepend to the front of the chain */
	    linkPtr->next = chainPtr->head;
	    linkPtr->prev = NULL;
	    chainPtr->head->prev = linkPtr;
	    chainPtr->head = linkPtr;
	} else {
	    linkPtr->next = afterPtr->next;
	    linkPtr->prev = afterPtr;
	    if (afterPtr == chainPtr->tail) {
		chainPtr->tail = linkPtr;
	    } else {
		afterPtr->next->prev = linkPtr;
	    }
	    afterPtr->next = linkPtr;
	}
    }
    chainPtr->nLinks++;
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_ChainLinkBefore --
 *
 *	Inserts a new link preceding a given link in a chain.  If
 *	beforePtr is NULL, then the new link is placed at the
 *	beginning of the list.
 *
 * Results:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
void
Blt_ChainLinkBefore(
    Chain *chainPtr,		/* Chain to contain new entry */
    ChainLink *linkPtr,		/* New entry to be inserted */
    ChainLink *beforePtr)	/* Entry to link before */
{
    if (chainPtr->head == NULL) {
	chainPtr->tail = chainPtr->head = linkPtr;
    } else {
	if (beforePtr == NULL) {
	    /* Append to the end of the chain. */
	    linkPtr->next = NULL;
	    linkPtr->prev = chainPtr->tail;
	    chainPtr->tail->next = linkPtr;
	    chainPtr->tail = linkPtr;
	} else {
	    linkPtr->prev = beforePtr->prev;
	    linkPtr->next = beforePtr;
	    if (beforePtr == chainPtr->head) {
		chainPtr->head = linkPtr;
	    } else {
		beforePtr->prev->next = linkPtr;
	    }
	    beforePtr->prev = linkPtr;
	}
    }
    chainPtr->nLinks++;
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_ChainUnlinkLink --
 *
 *	Unlinks a link from the chain. The link is not deallocated, 
 *	but only removed from the chain.
 *
 * Results:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
void
Blt_ChainUnlinkLink(Chain *chainPtr, ChainLink *linkPtr)
{
    int unlinked;		/* Indicates if the link is actually
				 * removed from the chain. */

    unlinked = FALSE;
    if (chainPtr->head == linkPtr) {
	chainPtr->head = linkPtr->next;
	unlinked = TRUE;
    }
    if (chainPtr->tail == linkPtr) {
	chainPtr->tail = linkPtr->prev;
	unlinked = TRUE;
    }
    if (linkPtr->next != NULL) {
	linkPtr->next->prev = linkPtr->prev;
	unlinked = TRUE;
    }
    if (linkPtr->prev != NULL) {
	linkPtr->prev->next = linkPtr->next;
	unlinked = TRUE;
    }
    if (unlinked) {
	chainPtr->nLinks--;
    }
    linkPtr->prev = linkPtr->next = NULL;
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_ChainDeleteLink --
 *
 *	Unlinks and frees the given link from the chain.  It's assumed
 *	that the link belong to the chain. No error checking is
 *	performed to verify this.
 *
 * Results:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
void
Blt_ChainDeleteLink(Blt_Chain chain, Blt_ChainLink link)
{
    Blt_ChainUnlinkLink(chain, link);
    Blt_Free(link);
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_ChainAppend
 *
 *	Creates and new link with the given data and appends it to the
 *	end of the chain.
 *
 * Results:
 *	Returns a pointer to the link created.
 *
 *---------------------------------------------------------------------------
 */
Blt_ChainLink
Blt_ChainAppend(Blt_Chain chain, ClientData clientData)
{
    Blt_ChainLink link;

    link = Blt_ChainNewLink();
    Blt_ChainLinkBefore(chain, link, (Blt_ChainLink)NULL);
    Blt_ChainSetValue(link, clientData);
    return link;
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_ChainPrepend
 *
 *	Creates and new link with the given data and prepends it to
 *	beginning of the chain.
 *
 * Results:
 *	Returns a pointer to the link created.
 *
 *---------------------------------------------------------------------------
 */
Blt_ChainLink 
Blt_ChainPrepend(Blt_Chain chain, ClientData clientData)
{
    Blt_ChainLink link;

    link = Blt_ChainNewLink();
    Blt_ChainLinkAfter(chain, link, (Blt_ChainLink)NULL);
    Blt_ChainSetValue(link, clientData);
    return link;
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_ChainGetNthLink --
 *
 *	Find the link at the given position in the chain.
 *
 * Results:
 *	Returns the pointer to the link, if that numbered link
 *	exists. Otherwise NULL.
 *
 *---------------------------------------------------------------------------
 */
Blt_ChainLink
Blt_ChainGetNthLink(
    Chain *chainPtr,		/* Chain to traverse */
    long position)		/* Index of link to select from front
				 * or back of the chain. */
{
    ChainLink *linkPtr;

    if (chainPtr != NULL) {
	for (linkPtr = chainPtr->head; linkPtr != NULL;
	    linkPtr = linkPtr->next) {
	    if (position == 0) {
		return linkPtr;
	    }
	    position--;
	}
    }
    return NULL;
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_ChainSort --
 *
 *	Sorts the chain according to the given comparison routine.  
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The chain is reordered.
 *
 *---------------------------------------------------------------------------
 */
void
Blt_ChainSort(
    Chain *chainPtr,		/* Chain to traverse */
    Blt_ChainCompareProc *proc)
{
    ChainLink **linkArr;
    ChainLink *linkPtr;
    long i;

    if (chainPtr->nLinks < 2) {
	return;
    }
    linkArr = Blt_Malloc(sizeof(Blt_ChainLink) * (chainPtr->nLinks + 1));
    if (linkArr == NULL) {
	return;			/* Out of memory. */
    }
    i = 0;
    for (linkPtr = chainPtr->head; linkPtr != NULL; 
	 linkPtr = linkPtr->next) { 
	linkArr[i++] = linkPtr;
    }
    qsort((char *)linkArr, chainPtr->nLinks, sizeof(Blt_ChainLink),
	(QSortCompareProc *)proc);

    /* Rethread the chain. */
    linkPtr = linkArr[0];
    chainPtr->head = linkPtr;
    linkPtr->prev = NULL;
    for (i = 1; i < chainPtr->nLinks; i++) {
	linkPtr->next = linkArr[i];
	linkPtr->next->prev = linkPtr;
	linkPtr = linkPtr->next;
    }
    chainPtr->tail = linkPtr;
    linkPtr->next = NULL;
    Blt_Free(linkArr);
}

