
/*
 * bltChain.h --
 *
 *	Copyright 1993-2004 George A Howlett.
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
#ifndef _BLT_CHAIN_H
#define _BLT_CHAIN_H

typedef struct Blt_ChainStruct *Blt_Chain;
typedef struct Blt_ChainLinkStruct *Blt_ChainLink;

/*
 * A Blt_ChainLink is the container structure for the Blt_Chain.
 */

struct Blt_ChainLinkStruct {
    Blt_ChainLink prev;		/* Link to the previous link */
    Blt_ChainLink next;		/* Link to the next link */
    ClientData clientData;	/* Pointer to the data object */
};

typedef int (Blt_ChainCompareProc) _ANSI_ARGS_((Blt_ChainLink *l1Ptr, 
	Blt_ChainLink *l2Ptr));

/*
 * A Blt_Chain is a doubly chained list structure.
 */
struct Blt_ChainStruct {
    Blt_ChainLink head;		/* Pointer to first element in chain */
    Blt_ChainLink tail;		/* Pointer to last element in chain */
    long nLinks;		/* Number of elements in chain */
};

BLT_EXTERN void Blt_ChainInit _ANSI_ARGS_((Blt_Chain chain));
BLT_EXTERN Blt_Chain Blt_ChainCreate _ANSI_ARGS_((void));
BLT_EXTERN void Blt_ChainDestroy _ANSI_ARGS_((Blt_Chain chain));
BLT_EXTERN Blt_ChainLink Blt_ChainNewLink _ANSI_ARGS_((void));
BLT_EXTERN Blt_ChainLink Blt_ChainAllocLink _ANSI_ARGS_((size_t size));
BLT_EXTERN Blt_ChainLink Blt_ChainAppend _ANSI_ARGS_((Blt_Chain chain,
	ClientData clientData));
BLT_EXTERN Blt_ChainLink Blt_ChainPrepend _ANSI_ARGS_((Blt_Chain chain,
	ClientData clientData));
BLT_EXTERN void Blt_ChainReset _ANSI_ARGS_((Blt_Chain chain));
BLT_EXTERN void Blt_ChainInitLink _ANSI_ARGS_((Blt_ChainLink link));
BLT_EXTERN void Blt_ChainLinkAfter _ANSI_ARGS_((Blt_Chain chain,
	Blt_ChainLink link, Blt_ChainLink after));
BLT_EXTERN void Blt_ChainLinkBefore _ANSI_ARGS_((Blt_Chain chain,
	Blt_ChainLink link, Blt_ChainLink before));
BLT_EXTERN void Blt_ChainUnlinkLink _ANSI_ARGS_((Blt_Chain chain,
	Blt_ChainLink link));
BLT_EXTERN void Blt_ChainDeleteLink _ANSI_ARGS_((Blt_Chain chain,
	Blt_ChainLink link));
BLT_EXTERN Blt_ChainLink Blt_ChainGetNthLink _ANSI_ARGS_((Blt_Chain chain, 
	long position));
BLT_EXTERN void Blt_ChainSort _ANSI_ARGS_((Blt_Chain chain,
	Blt_ChainCompareProc *proc));

#define Blt_ChainGetLength(c)	(((c) == NULL) ? 0 : (c)->nLinks)
#define Blt_ChainFirstLink(c)	(((c) == NULL) ? NULL : (c)->head)
#define Blt_ChainLastLink(c)	(((c) == NULL) ? NULL : (c)->tail)
#define Blt_ChainPrevLink(l)	((l)->prev)
#define Blt_ChainNextLink(l) 	((l)->next)
#define Blt_ChainGetValue(l)  	((l)->clientData)
#define Blt_ChainSetValue(l, value) ((l)->clientData = (ClientData)(value))
#define Blt_ChainAppendLink(c, l) \
	(Blt_ChainLinkBefore((c), (l), (Blt_ChainLink)NULL))
#define Blt_ChainPrependLink(c, l) \
	(Blt_ChainLinkAfter((c), (l), (Blt_ChainLink)NULL))

#endif /* _BLT_CHAIN_H */
