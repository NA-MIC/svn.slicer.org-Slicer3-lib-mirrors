
/*
 *	Memory allocation/deallocation in BLT is performed via the
 *	global variables bltMallocPtr, bltFreePtr, and bltReallocPtr.
 *	By default, they point to the same routines that Tcl uses.
 *	The routine Blt_AllocInit allows you to specify your own
 *	memory allocation/deallocation routines for BLT on a
 *	library-wide basis.
 */

#ifndef _BLT_ALLOC_H
#define _BLT_ALLOC_H

#include <assert.h>

typedef void *(Blt_MallocProc) (size_t size);
typedef void *(Blt_ReallocProc) (void *ptr, size_t size);
typedef void (Blt_FreeProc) (CONST void *ptr);

BLT_EXTERN void Blt_AllocInit _ANSI_ARGS_((Blt_MallocProc *mallocProc, 
	Blt_ReallocProc *reallocProc, Blt_FreeProc *freeProc));

BLT_EXTERN void *Blt_Malloc _ANSI_ARGS_((size_t size));
BLT_EXTERN void *Blt_Realloc _ANSI_ARGS_((void *ptr, size_t size));
BLT_EXTERN void Blt_Free _ANSI_ARGS_((CONST void *ptr));
BLT_EXTERN void *Blt_Calloc (unsigned int nElem, size_t size);

BLT_EXTERN void *Blt_MallocAbortOnError _ANSI_ARGS_((size_t size, 
	const char *file, int line));

BLT_EXTERN void *Blt_CallocAbortOnError _ANSI_ARGS_((unsigned int nElem, 
	size_t size, const char *file, int line));
BLT_EXTERN char *Blt_StrdupAbortOnError _ANSI_ARGS_((CONST char *ptr,
	const char *file, int line));

#define Blt_CallocAssert(n,s) (Blt_CallocAbortOnError(n,s,__FILE__, __LINE__))
#define Blt_MallocAssert(s) (Blt_MallocAbortOnError(s,__FILE__, __LINE__))
#define Blt_StrdupAssert(s) (Blt_StrdupAbortOnError(s,__FILE__, __LINE__))

#endif /* _BLT_ALLOC_H */
