
/*
 * bltVector.h --
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

#ifndef _BLT_VECTOR_H
#define _BLT_VECTOR_H

typedef enum {
    BLT_VECTOR_NOTIFY_UPDATE = 1, /* The vector's values has been updated */
    BLT_VECTOR_NOTIFY_DESTROY	/* The vector has been destroyed and the client
				 * should no longer use its data (calling
				 * Blt_FreeVectorId) */
} Blt_VectorNotify;

typedef struct Blt_VectorIdStruct *Blt_VectorId;

typedef void (Blt_VectorChangedProc) _ANSI_ARGS_((Tcl_Interp *interp,
	ClientData clientData, Blt_VectorNotify notify));

typedef struct {
    double *valueArr;		/* Array of values (possibly malloc-ed) */
    int numValues;		/* Number of values in the array */
    int arraySize;		/* Size of the allocated space */
    double min, max;		/* Minimum and maximum values in the vector */
    int dirty;			/* Indicates if the vector has been updated */
    int reserved;		/* Reserved for future use */

} Blt_Vector;

typedef double (Blt_VectorIndexProc) _ANSI_ARGS_((Blt_Vector * vecPtr));

typedef enum {
    BLT_MATH_FUNC_SCALAR = 1,	/* The function returns a single double
				 * precision value. */
    BLT_MATH_FUNC_VECTOR	/* The function processes the entire vector. */
} Blt_MathFuncType;

/*
 * To be safe, use the macros below, rather than the fields of the
 * structure directly.
 *
 * The Blt_Vector is basically an opaque type.  But it's also the
 * actual memory address of the vector itself.  I wanted to make the
 * API as unobtrusive as possible.  So instead of giving you a copy of
 * the vector, providing various functions to access and update the
 * vector, you get your hands on the actual memory (array of doubles)
 * shared by all the vector's clients.
 *
 * The trade-off for speed and convenience is safety.  You can easily
 * break things by writing into the vector when other clients are
 * using it.  Use Blt_ResetVector to get around this.  At least the
 * macros are a reminder it isn't really safe to reset the data
 * fields, except by the API routines.  
 */
#define Blt_VecData(v)		((v)->valueArr)
#define Blt_VecLength(v)	((v)->numValues)
#define Blt_VecSize(v)		((v)->arraySize)
#define Blt_VecDirty(v)		((v)->dirty)

BLT_EXTERN double Blt_VecMin _ANSI_ARGS_((Blt_Vector *vPtr));
BLT_EXTERN double Blt_VecMax _ANSI_ARGS_((Blt_Vector *vPtr));

BLT_EXTERN Blt_VectorId Blt_AllocVectorId _ANSI_ARGS_((Tcl_Interp *interp,
	char *vecName));

BLT_EXTERN void Blt_SetVectorChangedProc _ANSI_ARGS_((Blt_VectorId clientId,
	Blt_VectorChangedProc * proc, ClientData clientData));

BLT_EXTERN void Blt_FreeVectorId _ANSI_ARGS_((Blt_VectorId clientId));

BLT_EXTERN int Blt_GetVectorById _ANSI_ARGS_((Tcl_Interp *interp,
	Blt_VectorId clientId, Blt_Vector **vecPtrPtr));

BLT_EXTERN CONST char *Blt_NameOfVectorId _ANSI_ARGS_((Blt_VectorId clientId));

BLT_EXTERN CONST char *Blt_NameOfVector _ANSI_ARGS_((Blt_Vector *vecPtr));

BLT_EXTERN int Blt_VectorNotifyPending _ANSI_ARGS_((Blt_VectorId clientId));

BLT_EXTERN int Blt_CreateVector _ANSI_ARGS_((Tcl_Interp *interp, 
	CONST char *vecName, int size, Blt_Vector ** vecPtrPtr));

BLT_EXTERN int Blt_CreateVector2 _ANSI_ARGS_((Tcl_Interp *interp, 
	CONST char *vecName, CONST char *cmdName, CONST char *varName,
	int initialSize, Blt_Vector **vecPtrPtr));

BLT_EXTERN int Blt_GetVector _ANSI_ARGS_((Tcl_Interp *interp, 
	CONST char *vecName, Blt_Vector **vecPtrPtr));

BLT_EXTERN int Blt_GetVectorFromObj _ANSI_ARGS_((Tcl_Interp *interp, 
	Tcl_Obj *objPtr, Blt_Vector **vecPtrPtr));

BLT_EXTERN int Blt_VectorExists _ANSI_ARGS_((Tcl_Interp *interp, 
	CONST char *vecName));

BLT_EXTERN int Blt_ResetVector _ANSI_ARGS_((Blt_Vector *vecPtr, double *dataArr,
	int n, int arraySize, Tcl_FreeProc *freeProc));

BLT_EXTERN int Blt_ResizeVector _ANSI_ARGS_((Blt_Vector *vecPtr, int n));

BLT_EXTERN int Blt_DeleteVectorByName _ANSI_ARGS_((Tcl_Interp *interp,
	CONST char *vecName));

BLT_EXTERN int Blt_DeleteVector _ANSI_ARGS_((Blt_Vector *vecPtr));

BLT_EXTERN int Blt_ExprVector _ANSI_ARGS_((Tcl_Interp *interp, char *expr,
	Blt_Vector *vecPtr));

BLT_EXTERN void Blt_InstallIndexProc _ANSI_ARGS_((Tcl_Interp *interp, 
	CONST char *indexName, Blt_VectorIndexProc * procPtr));

BLT_EXTERN int Blt_VectorExists2 _ANSI_ARGS_((Tcl_Interp *interp, 
	CONST char *vecName));

#endif /* _BLT_VECTOR_H */
