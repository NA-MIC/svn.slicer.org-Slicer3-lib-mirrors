
/*
 * bltVecMath.c --
 *
 * This module implements mathematical expressions with vector data
 * objects.
 *
 *	Copyright 1995-2004 George A Howlett.
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

#include "bltVecInt.h"
#include "bltNsUtil.h"
#include "bltParse.h"

/*
 * Three types of math functions:
 *
 *	ComponentProc		Function is applied in multiple calls to
 *				each component of the vector.
 *	VectorProc		Entire vector is passed, each component is
 *				modified.
 *	ScalarProc		Entire vector is passed, single scalar value
 *				is returned.
 */

typedef double (ComponentProc) _ANSI_ARGS_((double value));
typedef int (VectorProc) _ANSI_ARGS_((VectorObject *vPtr));
typedef double (ScalarProc) _ANSI_ARGS_((VectorObject *vPtr));

/*
 * Built-in math functions:
 */
typedef int (GenericMathProc) _ANSI_ARGS_(ANYARGS);

/*
 * MathFunction --
 *
 *	Contains information about math functions that can be called
 *	for vectors.  The table of math functions is global within the
 *	application.  So you can't define two different "sqrt"
 *	functions.
 */
typedef struct {
    const char *name;		/* Name of built-in math function.  If
				 * NULL, indicates that the function
				 * was user-defined and dynamically
				 * allocated.  Function names are
				 * global across all interpreters. */

    void *proc;			/* Procedure that implements this math
				 * function. */

    ClientData clientData;	/* Argument to pass when invoking the
				 * function. */

} MathFunction;


/*
 * Macros for testing floating-point values for certain special cases:
 *
 *	IS_NAN	Test for not-a-number by comparing a value against itself
 *	IF_INF	Test for infinity by comparing against the largest floating
 *		point value.
 */

#define IS_NAN(v) ((v) != (v))

#ifdef DBL_MAX
#   define IS_INF(v) (((v) > DBL_MAX) || ((v) < -DBL_MAX))
#else
#   define IS_INF(v) 0
#endif

/* The data structure below is used to describe an expression value,
 * which can be either a double-precision floating-point value, or a
 * string.  A given number has only one value at a time.  */

#define STATIC_STRING_SPACE 150

/*
 * Tokens --
 *
 *	The token types are defined below.  In addition, there is a
 *	table associating a precedence with each operator.  The order
 *	of types is important.  Consult the code before changing it.
 */
enum Tokens {
    VALUE, OPEN_PAREN, CLOSE_PAREN, COMMA, END, UNKNOWN,
    MULT = 8, DIVIDE, MOD, PLUS, MINUS,
    LEFT_SHIFT, RIGHT_SHIFT,
    LESS, GREATER, LEQ, GEQ, EQUAL, NEQ,
    OLD_BIT_AND, EXPONENT, OLD_BIT_OR, OLD_QUESTY, OLD_COLON,
    AND, OR, UNARY_MINUS, OLD_UNARY_PLUS, NOT, OLD_BIT_NOT
};

typedef struct {
    VectorObject *vPtr;
    char staticSpace[STATIC_STRING_SPACE];
    ParseValue pv;		/* Used to hold a string value, if any. */
} Value;

/*
 * ParseInfo --
 *
 *	The data structure below describes the state of parsing an
 *	expression.  It's passed among the routines in this module.
 */
typedef struct {
    const char *expr;		/* The entire right-hand side of the
				 * expression, as originally passed to
				 * Blt_ExprVector. */

    const char *nextPtr;	/* Position of the next character to
				 * be scanned from the expression
				 * string. */

    enum Tokens token;		/* Type of the last token to be parsed
				 * from nextPtr.  See below for
				 * definitions.  Corresponds to the
				 * characters just before nextPtr. */

} ParseInfo;

/*
 * Precedence table.  The values for non-operator token types are ignored.
 */
static int precTable[] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    12, 12, 12,			/* MULT, DIVIDE, MOD */
    11, 11,			/* PLUS, MINUS */
    10, 10,			/* LEFT_SHIFT, RIGHT_SHIFT */
    9, 9, 9, 9,			/* LESS, GREATER, LEQ, GEQ */
    8, 8,			/* EQUAL, NEQ */
    7,				/* OLD_BIT_AND */
    13,				/* EXPONENTIATION */
    5,				/* OLD_BIT_OR */
    4,				/* AND */
    3,				/* OR */
    2,				/* OLD_QUESTY */
    1,				/* OLD_COLON */
    14, 14, 14, 14		/* UNARY_MINUS, OLD_UNARY_PLUS, NOT,
				 * OLD_BIT_NOT */
};


/*
 * Forward declarations.
 */

static int NextValue _ANSI_ARGS_((Tcl_Interp *interp, ParseInfo *piPtr,
	int prec, Value * valuePtr));

#include <bltMath.h>

/*
 *---------------------------------------------------------------------------
 *
 * Sort --
 *
 *	A vector math function.  Sorts the values of the given 
 *	vector.
 *
 * Results:
 *	Always TCL_OK.
 *
 * Side Effects:
 *	The vector is sorted.
 *
 *---------------------------------------------------------------------------
 */
static int
Sort(VectorObject *vecObjPtr)
{
    int *indices;
    double *values;
    int i;

    indices = Blt_Vec_SortIndex(&vecObjPtr, 1);
    values = Blt_MallocAssert(sizeof(double) * vecObjPtr->length);
    for(i = vecObjPtr->first; i <= vecObjPtr->last; i++) {
	values[i] = vecObjPtr->valueArr[indices[i]];
    }
    Blt_Free(indices);
    for (i = vecObjPtr->first; i <= vecObjPtr->last; i++) {
	vecObjPtr->valueArr[i] = values[i];
    }
    Blt_Free(values);
    return TCL_OK;
}

static double
Length(Blt_Vector *vector)
{
    VectorObject *vecObjPtr = (VectorObject *)vector;

    return (double)(vecObjPtr->last - vecObjPtr->first + 1);
}

double
Blt_VecMax(Blt_Vector *vector)
{
    VectorObject *vecObjPtr = (VectorObject *)vector;

    return Blt_Vec_Max(vecObjPtr);
}

double
Blt_VecMin(Blt_Vector *vector)
{
    VectorObject *vecObjPtr = (VectorObject *)vector;

    return Blt_Vec_Min(vecObjPtr);
}


static double
Product(Blt_Vector *vector)
{
    VectorObject *vecObjPtr = (VectorObject *)vector;
    double prod;
    double *vp, *vend;

    prod = 1.0;
    for(vp = vecObjPtr->valueArr + vecObjPtr->first,
	    vend = vecObjPtr->valueArr + vecObjPtr->last; vp <= vend; vp++) {
	prod *= *vp;
    }
    return prod;
}

static double
Sum(Blt_Vector *vector)
{
    VectorObject *vecObjPtr = (VectorObject *)vector;
    double sum, c;
    double *vp, *vend;

    /* Kahan summation algorithm */

    vp = vecObjPtr->valueArr + vecObjPtr->first;
    sum = *vp++;
    c = 0.0;			/* A running compensation for lost
				 * low-order bits.*/
    for (vend = vecObjPtr->valueArr + vecObjPtr->last; vp <= vend; vp++) {
	double y, t;
	
        y = *vp - c;		/* So far, so good: c is zero.*/
        t = sum + y;		/* Alas, sum is big, y small, so
				 * low-order digits of y are lost.*/
        c = (t - sum) - y;	/* (t - sum) recovers the high-order
				 * part of y; subtracting y recovers
				 * -(low part of y) */
	sum = t;
    }
    return sum;
}

static double
Mean(Blt_Vector *vector)
{
    VectorObject *vecObjPtr = (VectorObject *)vector;
    double sum;
    int n;

    sum = Sum(vector);
    n = vecObjPtr->last - vecObjPtr->first + 1;
    return sum / (double)n;
}

/*
 *  var = 1/N Sum( (x[i] - mean)^2 )
 */
static double
Variance(Blt_Vector *vector)
{
    VectorObject *vecObjPtr = (VectorObject *)vector;
    double var, mean;
    double *vp, *vend;
    int count;

    mean = Mean(vector);
    var = 0.0;
    count = 0;
    for(vp = vecObjPtr->valueArr + vecObjPtr->first,
	vend = vecObjPtr->valueArr + vecObjPtr->last; vp <= vend; vp++) {
	double dx;

	dx = *vp - mean;
	var += dx * dx;
	count++;
    }
    if (count < 2) {
	return 0.0;
    }
    var /= (double)(count - 1);
    return var;
}

/*
 *  skew = Sum( (x[i] - mean)^3 ) / (var^3/2)
 */
static double
Skew(Blt_Vector *vector)
{
    VectorObject *vecObjPtr = (VectorObject *)vector;
    double diff, var, skew, mean, diffsq;
    double *vp, *vend;
    int count;

    mean = Mean(vector);
    var = skew = 0.0;
    count = 0;
    for(vp = vecObjPtr->valueArr + vecObjPtr->first,
	    vend = vecObjPtr->valueArr + vecObjPtr->last; vp <= vend; vp++) {
	diff = *vp - mean;
	diff = FABS(diff);
	diffsq = diff * diff;
	var += diffsq;
	skew += diffsq * diff;
	count++;
    }
    if (count < 2) {
	return 0.0;
    }
    var /= (double)(count - 1);
    skew /= count * var * sqrt(var);
    return skew;
}

static double
StdDeviation(Blt_Vector *vector)
{
    double var;

    var = Variance(vector);
    if (var > 0.0) {
	return sqrt(var);
    }
    return 0.0;
}


static double
AvgDeviation(Blt_Vector *vector)
{
    VectorObject *vecObjPtr = (VectorObject *)vector;
    double diff, avg, mean;
    double *vp, *vend;
    int count;

    mean = Mean(vector);
    avg = 0.0;
    count = 0;
    for(vp = vecObjPtr->valueArr + vecObjPtr->first,
	    vend = vecObjPtr->valueArr + vecObjPtr->last; vp <= vend; vp++) {
	diff = *vp - mean;
	avg += FABS(diff);
	count++;
    }
    if (count < 2) {
	return 0.0;
    }
    avg /= (double)count;
    return avg;
}


static double
Kurtosis(Blt_Vector *vector)
{
    VectorObject *vecObjPtr = (VectorObject *)vector;
    double diff, diffsq, kurt, var, mean;
    double *vp, *vend;
    int count;

    mean = Mean(vector);
    var = kurt = 0.0;
    count = 0;
    for(vp = vecObjPtr->valueArr + vecObjPtr->first,
	    vend = vecObjPtr->valueArr + vecObjPtr->last; vp <= vend; vp++) {
	diff = *vp - mean;
	diffsq = diff * diff;
	var += diffsq;
	kurt += diffsq * diffsq;
	count++;
    }
    if (count < 2) {
	return 0.0;
    }
    var /= (double)(count - 1);
    if (var == 0.0) {
	return 0.0;
    }
    kurt /= (count * var * var);
    return kurt - 3.0;		/* Fisher Kurtosis */
}


static double
Median(Blt_Vector *vector)
{
    VectorObject *vecObjPtr = (VectorObject *)vector;
    int *indices;
    double q2;
    int mid;

    if (vecObjPtr->length == 0) {
	return -DBL_MAX;
    }
    indices = Blt_Vec_SortIndex(&vecObjPtr, 1);
    mid = (vecObjPtr->length - 1) / 2;

    /*  
     * Determine Q2 by checking if the number of elements [0..n-1] is
     * odd or even.  If even, we must take the average of the two
     * middle values.  
     */
    if (vecObjPtr->length & 1) { /* Odd */
	q2 = vecObjPtr->valueArr[indices[mid]];
    } else {			/* Even */
	q2 = (vecObjPtr->valueArr[indices[mid]] + 
	      vecObjPtr->valueArr[indices[mid + 1]]) * 0.5;
    }
    Blt_Free(indices);
    return q2;
}

static double
Q1(Blt_Vector *vector)
{
    VectorObject *vecObjPtr = (VectorObject *)vector;
    double q1;
    int *indices;

    if (vecObjPtr->length == 0) {
	return -DBL_MAX;
    } 
    indices = Blt_Vec_SortIndex(&vecObjPtr, 1);

    if (vecObjPtr->length < 4) {
	q1 = vecObjPtr->valueArr[indices[0]];
    } else {
	int mid, q;

	mid = (vecObjPtr->length - 1) / 2;
	q = mid / 2;

	/* 
	 * Determine Q1 by checking if the number of elements in the
	 * bottom half [0..mid) is odd or even.   If even, we must
	 * take the average of the two middle values.
	 */
	if (mid & 1) {		/* Odd */
	    q1 = vecObjPtr->valueArr[indices[q]]; 
	} else {		/* Even */
	    q1 = (vecObjPtr->valueArr[indices[q]] + 
		  vecObjPtr->valueArr[indices[q + 1]]) * 0.5; 
	}
    }
    Blt_Free(indices);
    return q1;
}

static double
Q3(Blt_Vector *vector)
{
    VectorObject *vecObjPtr = (VectorObject *)vector;
    double q3;
    int *indices;

    if (vecObjPtr->length == 0) {
	return -DBL_MAX;
    } 

    indices = Blt_Vec_SortIndex(&vecObjPtr, 1);

    if (vecObjPtr->length < 4) {
	q3 = vecObjPtr->valueArr[indices[vecObjPtr->length - 1]];
    } else {
	int mid, q;

	mid = (vecObjPtr->length - 1) / 2;
	q = (vecObjPtr->length + mid) / 2;

	/* 
	 * Determine Q3 by checking if the number of elements in the
	 * upper half (mid..n-1] is odd or even.   If even, we must
	 * take the average of the two middle values.
	 */
	if (mid & 1) {		/* Odd */
	    q3 = vecObjPtr->valueArr[indices[q]];
	} else {		/* Even */
	    q3 = (vecObjPtr->valueArr[indices[q]] + 
		  vecObjPtr->valueArr[indices[q + 1]]) * 0.5; 
	}
    }
    Blt_Free(indices);
    return q3;
}


static int
Norm(Blt_Vector *vector)
{
    VectorObject *vecObjPtr = (VectorObject *)vector;
    double norm, range, min, max;
    int i;

    min = Blt_Vec_Min(vecObjPtr);
    max = Blt_Vec_Max(vecObjPtr);
    range = max - min;
    for (i = 0; i < vecObjPtr->length; i++) {
	norm = (vecObjPtr->valueArr[i] - min) / range;
	vecObjPtr->valueArr[i] = norm;
    }
    return TCL_OK;
}


static double
Nonzeros(Blt_Vector *vector)
{
    VectorObject *vecObjPtr = (VectorObject *)vector;
    int count;
    double *vp, *vend;

    count = 0;
    for(vp = vecObjPtr->valueArr + vecObjPtr->first,
	    vend = vecObjPtr->valueArr + vecObjPtr->last; vp <= vend; vp++) {
	if (*vp == 0.0) {
	    count++;
	}
    }
    return (double) count;
}

static double
Fabs(double value)
{
    if (value < 0.0) {
	return -value;
    }
    return value;
}

static double
Round(double value)
{
    if (value < 0.0) {
	return ceil(value - 0.5);
    } else {
	return floor(value + 0.5);
    }
}

static double
Fmod(double x, double y)
{
    if (y == 0.0) {
	return 0.0;
    }
    return x - (floor(x / y) * y);
}

/*
 *---------------------------------------------------------------------------
 *
 * MathError --
 *
 *	This procedure is called when an error occurs during a
 *	floating-point operation.  It reads errno and sets
 *	interp->result accordingly.
 *
 * Results:
 *	Interp->result is set to hold an error message.
 *
 * Side effects:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
static void
MathError(
    Tcl_Interp *interp,		/* Where to store error message. */
    double value)		/* Value returned after error; used to
				 * distinguish underflows from
				 * overflows. */
{
    if ((errno == EDOM) || (value != value)) {
	Tcl_AppendResult(interp, "domain error: argument not in valid range",
	    (char *)NULL);
	Tcl_SetErrorCode(interp, "ARITH", "DOMAIN", interp->result,
	    (char *)NULL);
    } else if ((errno == ERANGE) || IS_INF(value)) {
	if (value == 0.0) {
	    Tcl_AppendResult(interp, 
			     "floating-point value too small to represent",
		(char *)NULL);
	    Tcl_SetErrorCode(interp, "ARITH", "UNDERFLOW", interp->result,
		(char *)NULL);
	} else {
	    Tcl_AppendResult(interp, 
			     "floating-point value too large to represent",
		(char *)NULL);
	    Tcl_SetErrorCode(interp, "ARITH", "OVERFLOW", interp->result,
		(char *)NULL);
	}
    } else {
	Tcl_AppendResult(interp, "unknown floating-point error, ",
		"errno = ", Blt_Itoa(errno), (char *)NULL);
	Tcl_SetErrorCode(interp, "ARITH", "UNKNOWN", interp->result,
	    (char *)NULL);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * ParseString --
 *
 *	Given a string (such as one coming from command or variable
 *	substitution), make a Value based on the string.  The value
 *	will be a floating-point or integer, if possible, or else it
 *	will just be a copy of the string.
 *
 * Results:
 *	TCL_OK is returned under normal circumstances, and TCL_ERROR
 *	is returned if a floating-point overflow or underflow occurred
 *	while reading in a number.  The value at *valuePtr is modified
 *	to hold a number, if possible.
 *
 * Side effects:
 *	None.
 *
 *---------------------------------------------------------------------------
 */

static int
ParseString(
    Tcl_Interp *interp,		/* Where to store error message. */
    const char *string,		/* String to turn into value. */
    Value *valuePtr)		/* Where to store value information.
				 * Caller must have initialized pv field. */
{
    const char *endPtr;
    double value;

    errno = 0;

    /*   
     * The string can be either a number or a vector.  First try to
     * convert the string to a number.  If that fails then see if
     * we can find a vector by that name.
     */

    value = strtod(string, (char **)&endPtr);
    if ((endPtr != string) && (*endPtr == '\0')) {
	if (errno != 0) {
	    Tcl_ResetResult(interp);
	    MathError(interp, value);
	    return TCL_ERROR;
	}
	/* Numbers are stored as single element vectors. */
	if (Blt_Vec_ChangeLength(interp, valuePtr->vPtr, 1) != TCL_OK) {
	    return TCL_ERROR;
	}
	valuePtr->vPtr->valueArr[0] = value;
	return TCL_OK;
    } else {
	VectorObject *vecObjPtr;

	while (isspace(UCHAR(*string))) {
	    string++;		/* Skip spaces leading the vector name. */    
	}
	vecObjPtr = Blt_Vec_ParseElement(interp, valuePtr->vPtr->dataPtr, 
		string, &endPtr, NS_SEARCH_BOTH);
	if (vecObjPtr == NULL) {
	    return TCL_ERROR;
	}
	if (*endPtr != '\0') {
	    Tcl_AppendResult(interp, "extra characters after vector", 
			     (char *)NULL);
	    return TCL_ERROR;
	}
	/* Copy the designated vector to our temporary. */
	Blt_Vec_Duplicate(valuePtr->vPtr, vecObjPtr);
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ParseMathFunction --
 *
 *	This procedure is invoked to parse a math function from an
 *	expression string, carry out the function, and return the
 *	value computed.
 *
 * Results:
 *	TCL_OK is returned if all went well and the function's value
 *	was computed successfully.  If the name doesn't match any
 *	known math function, returns TCL_RETURN. And if a format error
 *	was found, TCL_ERROR is returned and an error message is left
 *	in interp->result.
 *
 *	After a successful return piPtr will be updated to point to
 *	the character just after the function call, the token is set
 *	to VALUE, and the value is stored in valuePtr.
 *
 * Side effects:
 *	Embedded commands could have arbitrary side-effects.
 *
 *---------------------------------------------------------------------------
 */
static int
ParseMathFunction(
    Tcl_Interp *interp,		/* Interpreter to use for error reporting. */
    const char *start,		/* Start of string to parse */
    ParseInfo *piPtr,		/* Describes the state of the parse.
				 * piPtr->nextPtr must point to the
				 * first character of the function's
				 * name. */
    Value *valuePtr)		/* Where to store value, if that is
				 * what's parsed from string.  Caller
				 * must have initialized pv field
				 * correctly. */
{
    Blt_HashEntry *hPtr;
    MathFunction *mathPtr;	/* Info about math function. */
    char *p;
    VectorInterpData *dataPtr;	/* Interpreter-specific data. */
    GenericMathProc *proc;

    /*
     * Find the end of the math function's name and lookup the
     * record for the function.
     */
    p = (char *)start;
    while (isspace(UCHAR(*p))) {
	p++;
    }
    piPtr->nextPtr = p;
    while (isalnum(UCHAR(*p)) || (*p == '_')) {
	p++;
    }
    if (*p != '(') {
	return TCL_RETURN;	/* Must start with open parenthesis */
    }
    dataPtr = valuePtr->vPtr->dataPtr;
    *p = '\0';
    hPtr = Blt_FindHashEntry(&dataPtr->mathProcTable, piPtr->nextPtr);
    *p = '(';
    if (hPtr == NULL) {
	return TCL_RETURN;	/* Name doesn't match any known function */
    }
    /* Pick up the single value as the argument to the function */
    piPtr->token = OPEN_PAREN;
    piPtr->nextPtr = p + 1;
    valuePtr->pv.next = valuePtr->pv.buffer;
    if (NextValue(interp, piPtr, -1, valuePtr) != TCL_OK) {
	return TCL_ERROR;	/* Parse error */
    }
    if (piPtr->token != CLOSE_PAREN) {
	Tcl_AppendResult(interp, "unmatched parentheses in expression \"",
	    piPtr->expr, "\"", (char *)NULL);
	return TCL_ERROR;	/* Missing right parenthesis */
    }
    mathPtr = (MathFunction *) Blt_GetHashValue(hPtr);
    proc = mathPtr->proc;
    if ((*proc) (mathPtr->clientData, interp, valuePtr->vPtr) != TCL_OK) {
	return TCL_ERROR;	/* Function invocation error */
    }
    piPtr->token = VALUE;
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * NextToken --
 *
 *	Lexical analyzer for expression parser:  parses a single value,
 *	operator, or other syntactic element from an expression string.
 *
 * Results:
 *	TCL_OK is returned unless an error occurred while doing lexical
 *	analysis or executing an embedded command.  In that case a
 *	standard Tcl error is returned, using interp->result to hold
 *	an error message.  In the event of a successful return, the token
 *	and field in piPtr is updated to refer to the next symbol in
 *	the expression string, and the expr field is advanced past that
 *	token;  if the token is a value, then the value is stored at
 *	valuePtr.
 *
 * Side effects:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
static int
NextToken(
    Tcl_Interp *interp,		/* Interpreter to use for error reporting. */
    ParseInfo *piPtr,		/* Describes the state of the parse. */
    Value *valuePtr)		/* Where to store value, if that is
				 * what's parsed from string.  Caller
				 * must have initialized pv field
				 * correctly. */
{
    const char *p;
    const char *endPtr;
    const char *var;
    int result;

    p = piPtr->nextPtr;
    while (isspace(UCHAR(*p))) {
	p++;
    }
    if (*p == '\0') {
	piPtr->token = END;
	piPtr->nextPtr = p;
	return TCL_OK;
    }
    /*
     * Try to parse the token as a floating-point number. But check
     * that the first character isn't a "-" or "+", which "strtod"
     * will happily accept as an unary operator.  Otherwise, we might
     * accidently treat a binary operator as unary by mistake, which
     * will eventually cause a syntax error.
     */
    if ((*p != '-') && (*p != '+')) {
	double value;

	errno = 0;
	value = strtod(p, (char **)&endPtr);
	if (endPtr != p) {
	    if (errno != 0) {
		MathError(interp, value);
		return TCL_ERROR;
	    }
	    piPtr->token = VALUE;
	    piPtr->nextPtr = endPtr;

	    /*
	     * Save the single floating-point value as an 1-component vector.
	     */
	    if (Blt_Vec_ChangeLength(interp, valuePtr->vPtr, 1) != TCL_OK) {
		return TCL_ERROR;
	    }
	    valuePtr->vPtr->valueArr[0] = value;
	    return TCL_OK;
	}
    }
    piPtr->nextPtr = p + 1;
    switch (*p) {
    case '$':
	piPtr->token = VALUE;
	var = Tcl_ParseVar(interp, p, &endPtr);
	if (var == NULL) {
	    return TCL_ERROR;
	}
	piPtr->nextPtr = endPtr;
	Tcl_ResetResult(interp);
	result = ParseString(interp, var, valuePtr);
	return result;

    case '[':
	piPtr->token = VALUE;
	result = Blt_ParseNestedCmd(interp, p + 1, 0, &endPtr, &(valuePtr->pv));
	if (result != TCL_OK) {
	    return result;
	}
	piPtr->nextPtr = endPtr;
	Tcl_ResetResult(interp);
	result = ParseString(interp, valuePtr->pv.buffer, valuePtr);
	return result;

    case '"':
	piPtr->token = VALUE;
	result = Blt_ParseQuotes(interp, p + 1, '"', 0, &endPtr, 
		&(valuePtr->pv));
	if (result != TCL_OK) {
	    return result;
	}
	piPtr->nextPtr = endPtr;
	Tcl_ResetResult(interp);
	result = ParseString(interp, valuePtr->pv.buffer, valuePtr);
	return result;

    case '{':
	piPtr->token = VALUE;
	result = Blt_ParseBraces(interp, p + 1, &endPtr, &valuePtr->pv);
	if (result != TCL_OK) {
	    return result;
	}
	piPtr->nextPtr = endPtr;
	Tcl_ResetResult(interp);
	result = ParseString(interp, valuePtr->pv.buffer, valuePtr);
	return result;

    case '(':
	piPtr->token = OPEN_PAREN;
	break;

    case ')':
	piPtr->token = CLOSE_PAREN;
	break;

    case ',':
	piPtr->token = COMMA;
	break;

    case '*':
	piPtr->token = MULT;
	break;

    case '/':
	piPtr->token = DIVIDE;
	break;

    case '%':
	piPtr->token = MOD;
	break;

    case '+':
	piPtr->token = PLUS;
	break;

    case '-':
	piPtr->token = MINUS;
	break;

    case '^':
	piPtr->token = EXPONENT;
	break;

    case '<':
	switch (*(p + 1)) {
	case '<':
	    piPtr->nextPtr = p + 2;
	    piPtr->token = LEFT_SHIFT;
	    break;
	case '=':
	    piPtr->nextPtr = p + 2;
	    piPtr->token = LEQ;
	    break;
	default:
	    piPtr->token = LESS;
	    break;
	}
	break;

    case '>':
	switch (*(p + 1)) {
	case '>':
	    piPtr->nextPtr = p + 2;
	    piPtr->token = RIGHT_SHIFT;
	    break;
	case '=':
	    piPtr->nextPtr = p + 2;
	    piPtr->token = GEQ;
	    break;
	default:
	    piPtr->token = GREATER;
	    break;
	}
	break;

    case '=':
	if (*(p + 1) == '=') {
	    piPtr->nextPtr = p + 2;
	    piPtr->token = EQUAL;
	} else {
	    piPtr->token = UNKNOWN;
	}
	break;

    case '&':
	if (*(p + 1) == '&') {
	    piPtr->nextPtr = p + 2;
	    piPtr->token = AND;
	} else {
	    piPtr->token = UNKNOWN;
	}
	break;

    case '|':
	if (*(p + 1) == '|') {
	    piPtr->nextPtr = p + 2;
	    piPtr->token = OR;
	} else {
	    piPtr->token = UNKNOWN;
	}
	break;

    case '!':
	if (*(p + 1) == '=') {
	    piPtr->nextPtr = p + 2;
	    piPtr->token = NEQ;
	} else {
	    piPtr->token = NOT;
	}
	break;

    default:
	piPtr->token = VALUE;
	result = ParseMathFunction(interp, p, piPtr, valuePtr);
	if ((result == TCL_OK) || (result == TCL_ERROR)) {
	    return result;
	} else {
	    VectorObject *vecObjPtr;

	    while (isspace(UCHAR(*p))) {
		p++;		/* Skip spaces leading the vector name. */    
	    }
	    vecObjPtr = Blt_Vec_ParseElement(interp, valuePtr->vPtr->dataPtr, 
			p, &endPtr, NS_SEARCH_BOTH);
	    if (vecObjPtr == NULL) {
		return TCL_ERROR;
	    }
	    Blt_Vec_Duplicate(valuePtr->vPtr, vecObjPtr);
	    piPtr->nextPtr = endPtr;
	}
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * NextValue --
 *
 *	Parse a "value" from the remainder of the expression in piPtr.
 *
 * Results:
 *	Normally TCL_OK is returned.  The value of the expression is
 *	returned in *valuePtr.  If an error occurred, then interp->result
 *	contains an error message and TCL_ERROR is returned.
 *	InfoPtr->token will be left pointing to the token AFTER the
 *	expression, and piPtr->nextPtr will point to the character just
 *	after the terminating token.
 *
 * Side effects:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
static int
NextValue(
    Tcl_Interp *interp,		/* Interpreter to use for error reporting. */
    ParseInfo *piPtr,	/* Describes the state of the parse
				 * just before the value (i.e. NextToken will
				 * be called to get first token of value). */
    int prec,			/* Treat any un-parenthesized operator
				 * with precedence <= this as the end
				 * of the expression. */
    Value *valuePtr)		/* Where to store the value of the expression.
				 * Caller must have initialized pv field. */
{
    Value value2;		/* Second operand for current operator.  */
    int operator;		/* Current operator (either unary or binary). */
    int gotOp;			/* Non-zero means already lexed the operator
				 * (while picking up value for unary operator).
				 * Don't lex again. */
    int result;
    VectorObject *vPtr, *v2Ptr;
    int i;

    /*
     * There are two phases to this procedure.  First, pick off an initial
     * value.  Then, parse (binary operator, value) pairs until done.
     */

    vPtr = valuePtr->vPtr;
    v2Ptr = Blt_Vec_New(vPtr->dataPtr);
    gotOp = FALSE;
    value2.vPtr = v2Ptr;
    value2.pv.buffer = value2.pv.next = value2.staticSpace;
    value2.pv.end = value2.pv.buffer + STATIC_STRING_SPACE - 1;
    value2.pv.expandProc = Blt_ExpandParseValue;
    value2.pv.clientData = NULL;

    result = NextToken(interp, piPtr, valuePtr);
    if (result != TCL_OK) {
	goto done;
    }
    if (piPtr->token == OPEN_PAREN) {

	/* Parenthesized sub-expression. */

	result = NextValue(interp, piPtr, -1, valuePtr);
	if (result != TCL_OK) {
	    goto done;
	}
	if (piPtr->token != CLOSE_PAREN) {
	    Tcl_AppendResult(interp, "unmatched parentheses in expression \"",
		piPtr->expr, "\"", (char *)NULL);
	    result = TCL_ERROR;
	    goto done;
	}
    } else {
	if (piPtr->token == MINUS) {
	    piPtr->token = UNARY_MINUS;
	}
	if (piPtr->token >= UNARY_MINUS) {
	    operator = piPtr->token;
	    result = NextValue(interp, piPtr, precTable[operator], valuePtr);
	    if (result != TCL_OK) {
		goto done;
	    }
	    gotOp = TRUE;
	    /* Process unary operators. */
	    switch (operator) {
	    case UNARY_MINUS:
		for(i = 0; i < vPtr->length; i++) {
		    vPtr->valueArr[i] = -(vPtr->valueArr[i]);
		}
		break;

	    case NOT:
		for(i = 0; i < vPtr->length; i++) {
		    vPtr->valueArr[i] = (double)(!vPtr->valueArr[i]);
		}
		break;
	    default:
		Tcl_AppendResult(interp, "unknown operator", (char *)NULL);
		goto error;
	    }
	} else if (piPtr->token != VALUE) {
	    Tcl_AppendResult(interp, "missing operand", (char *)NULL);
	    goto error;
	}
    }
    if (!gotOp) {
	result = NextToken(interp, piPtr, &value2);
	if (result != TCL_OK) {
	    goto done;
	}
    }
    /*
     * Got the first operand.  Now fetch (operator, operand) pairs.
     */
    for (;;) {
	operator = piPtr->token;

	value2.pv.next = value2.pv.buffer;
	if ((operator < MULT) || (operator >= UNARY_MINUS)) {
	    if ((operator == END) || (operator == CLOSE_PAREN) || 
		(operator == COMMA)) {
		result = TCL_OK;
		goto done;
	    } else {
		Tcl_AppendResult(interp, "bad operator", (char *)NULL);
		goto error;
	    }
	}
	if (precTable[operator] <= prec) {
	    result = TCL_OK;
	    goto done;
	}
	result = NextValue(interp, piPtr, precTable[operator], &value2);
	if (result != TCL_OK) {
	    goto done;
	}
	if ((piPtr->token < MULT) && (piPtr->token != VALUE) &&
	    (piPtr->token != END) && (piPtr->token != CLOSE_PAREN) &&
	    (piPtr->token != COMMA)) {
	    Tcl_AppendResult(interp, "unexpected token in expression",
		(char *)NULL);
	    goto error;
	}
	/*
	 * At this point we have two vectors and an operator.
	 */

	if (v2Ptr->length == 1) {
	    double *opnd;
	    double scalar;

	    /*
	     * 2nd operand is a scalar.
	     */
	    scalar = v2Ptr->valueArr[0];
	    opnd = vPtr->valueArr;
	    switch (operator) {
	    case MULT:
		for(i = 0; i < vPtr->length; i++) {
		    opnd[i] *= scalar;
		}
		break;

	    case DIVIDE:
		if (scalar == 0.0) {
		    Tcl_AppendResult(interp, "divide by zero", (char *)NULL);
		    goto error;
		}
		for(i = 0; i < vPtr->length; i++) {
		    opnd[i] /= scalar;
		}
		break;

	    case PLUS:
		for(i = 0; i < vPtr->length; i++) {
		    opnd[i] += scalar;
		}
		break;

	    case MINUS:
		for(i = 0; i < vPtr->length; i++) {
		    opnd[i] -= scalar;
		}
		break;

	    case EXPONENT:
		for(i = 0; i < vPtr->length; i++) {
		    opnd[i] = pow(opnd[i], scalar);
		}
		break;

	    case MOD:
		for(i = 0; i < vPtr->length; i++) {
		    opnd[i] = Fmod(opnd[i], scalar);
		}
		break;

	    case LESS:
		for(i = 0; i < vPtr->length; i++) {
		    opnd[i] = (double)(opnd[i] < scalar);
		}
		break;

	    case GREATER:
		for(i = 0; i < vPtr->length; i++) {
		    opnd[i] = (double)(opnd[i] > scalar);
		}
		break;

	    case LEQ:
		for(i = 0; i < vPtr->length; i++) {
		    opnd[i] = (double)(opnd[i] <= scalar);
		}
		break;

	    case GEQ:
		for(i = 0; i < vPtr->length; i++) {
		    opnd[i] = (double)(opnd[i] >= scalar);
		}
		break;

	    case EQUAL:
		for(i = 0; i < vPtr->length; i++) {
		    opnd[i] = (double)(opnd[i] == scalar);
		}
		break;

	    case NEQ:
		for(i = 0; i < vPtr->length; i++) {
		    opnd[i] = (double)(opnd[i] != scalar);
		}
		break;

	    case AND:
		for(i = 0; i < vPtr->length; i++) {
		    opnd[i] = (double)(opnd[i] && scalar);
		}
		break;

	    case OR:
		for(i = 0; i < vPtr->length; i++) {
		    opnd[i] = (double)(opnd[i] || scalar);
		}
		break;

	    case LEFT_SHIFT:
		{
		    int offset;

		    offset = (int)scalar % vPtr->length;
		    if (offset > 0) {
			double *hold;
			int j;

			hold = Blt_MallocAssert(sizeof(double) * offset);
			for (i = 0; i < offset; i++) {
			    hold[i] = opnd[i];
			}
			for (i = offset, j = 0; i < vPtr->length; i++, j++) {
			    opnd[j] = opnd[i];
			}
			for (i = 0, j = vPtr->length - offset;
			     j < vPtr->length; i++, j++) {
			    opnd[j] = hold[i];
			}
			Blt_Free(hold);
		    }
		}
		break;

	    case RIGHT_SHIFT:
		{
		    int offset;

		    offset = (int)scalar % vPtr->length;
		    if (offset > 0) {
			double *hold;
			int j;
			
			hold = Blt_MallocAssert(sizeof(double) * offset);
			for (i = vPtr->length - offset, j = 0; 
			     i < vPtr->length; i++, j++) {
			    hold[j] = opnd[i];
			}
			for (i = vPtr->length - offset - 1, 
				 j = vPtr->length - 1; i >= 0; i--, j--) {
			    opnd[j] = opnd[i];
			}
			for (i = 0; i < offset; i++) {
			    opnd[i] = hold[i];
			}
			Blt_Free(hold);
		    }
		}
		break;

	    default:
		Tcl_AppendResult(interp, "unknown operator in expression",
		    (char *)NULL);
		goto error;
	    }

	} else if (vPtr->length == 1) {
	    double *opnd;
	    double scalar;

	    /*
	     * 1st operand is a scalar.
	     */
	    scalar = vPtr->valueArr[0];
	    Blt_Vec_Duplicate(vPtr, v2Ptr);
	    opnd = vPtr->valueArr;
	    switch (operator) {
	    case MULT:
		for(i = 0; i < vPtr->length; i++) {
		    opnd[i] *= scalar;
		}
		break;

	    case PLUS:
		for(i = 0; i < vPtr->length; i++) {
		    opnd[i] += scalar;
		}
		break;

	    case DIVIDE:
		for(i = 0; i < vPtr->length; i++) {
		    if (opnd[i] == 0.0) {
			Tcl_AppendResult(interp, "divide by zero",
			    (char *)NULL);
			goto error;
		    }
		    opnd[i] = (scalar / opnd[i]);
		}
		break;

	    case MINUS:
		for(i = 0; i < vPtr->length; i++) {
		    opnd[i] = scalar - opnd[i];
		}
		break;

	    case EXPONENT:
		for(i = 0; i < vPtr->length; i++) {
		    opnd[i] = pow(scalar, opnd[i]);
		}
		break;

	    case MOD:
		for(i = 0; i < vPtr->length; i++) {
		    opnd[i] = Fmod(scalar, opnd[i]);
		}
		break;

	    case LESS:
		for(i = 0; i < vPtr->length; i++) {
		    opnd[i] = (double)(scalar < opnd[i]);
		}
		break;

	    case GREATER:
		for(i = 0; i < vPtr->length; i++) {
		    opnd[i] = (double)(scalar > opnd[i]);
		}
		break;

	    case LEQ:
		for(i = 0; i < vPtr->length; i++) {
		    opnd[i] = (double)(scalar >= opnd[i]);
		}
		break;

	    case GEQ:
		for(i = 0; i < vPtr->length; i++) {
		    opnd[i] = (double)(scalar <= opnd[i]);
		}
		break;

	    case EQUAL:
		for(i = 0; i < vPtr->length; i++) {
		    opnd[i] = (double)(opnd[i] == scalar);
		}
		break;

	    case NEQ:
		for(i = 0; i < vPtr->length; i++) {
		    opnd[i] = (double)(opnd[i] != scalar);
		}
		break;

	    case AND:
		for(i = 0; i < vPtr->length; i++) {
		    opnd[i] = (double)(opnd[i] && scalar);
		}
		break;

	    case OR:
		for(i = 0; i < vPtr->length; i++) {
		    opnd[i] = (double)(opnd[i] || scalar);
		}
		break;

	    case LEFT_SHIFT:
	    case RIGHT_SHIFT:
		Tcl_AppendResult(interp, "second shift operand must be scalar",
		    (char *)NULL);
		goto error;

	    default:
		Tcl_AppendResult(interp, "unknown operator in expression",
		    (char *)NULL);
		goto error;
	    }
	} else {
	    double *opnd1, *opnd2;
	    /*
	     * Carry out the function of the specified operator.
	     */
	    if (vPtr->length != v2Ptr->length) {
		Tcl_AppendResult(interp, "vectors are different lengths",
		    (char *)NULL);
		goto error;
	    }
	    opnd1 = vPtr->valueArr, opnd2 = v2Ptr->valueArr;
	    switch (operator) {
	    case MULT:
		for (i = 0; i < vPtr->length; i++) {
		    opnd1[i] *= opnd2[i];
		}
		break;

	    case DIVIDE:
		for (i = 0; i < vPtr->length; i++) {
		    if (opnd2[i] == 0.0) {
			Tcl_AppendResult(interp,
			    "can't divide by 0.0 vector component",
			    (char *)NULL);
			goto error;
		    }
		    opnd1[i] /= opnd2[i];
		}
		break;

	    case PLUS:
		for (i = 0; i < vPtr->length; i++) {
		    opnd1[i] += opnd2[i];
		}
		break;

	    case MINUS:
		for (i = 0; i < vPtr->length; i++) {
		    opnd1[i] -= opnd2[i];
		}
		break;

	    case MOD:
		for (i = 0; i < vPtr->length; i++) {
		    opnd1[i] = Fmod(opnd1[i], opnd2[i]);
		}
		break;

	    case EXPONENT:
		for (i = 0; i < vPtr->length; i++) {
		    opnd1[i] = pow(opnd1[i], opnd2[i]);
		}
		break;

	    case LESS:
		for (i = 0; i < vPtr->length; i++) {
		    opnd1[i] = (double)(opnd1[i] < opnd2[i]);
		}
		break;

	    case GREATER:
		for (i = 0; i < vPtr->length; i++) {
		    opnd1[i] = (double)(opnd1[i] > opnd2[i]);
		}
		break;

	    case LEQ:
		for (i = 0; i < vPtr->length; i++) {
		    opnd1[i] = (double)(opnd1[i] <= opnd2[i]);
		}
		break;

	    case GEQ:
		for (i = 0; i < vPtr->length; i++) {
		    opnd1[i] = (double)(opnd1[i] >= opnd2[i]);
		}
		break;

	    case EQUAL:
		for (i = 0; i < vPtr->length; i++) {
		    opnd1[i] = (double)(opnd1[i] == opnd2[i]);
		}
		break;

	    case NEQ:
		for (i = 0; i < vPtr->length; i++) {
		    opnd1[i] = (double)(opnd1[i] != opnd2[i]);
		}
		break;

	    case AND:
		for (i = 0; i < vPtr->length; i++) {
		    opnd1[i] = (double)(opnd1[i] && opnd2[i]);
		}
		break;

	    case OR:
		for (i = 0; i < vPtr->length; i++) {
		    opnd1[i] = (double)(opnd1[i] || opnd2[i]);
		}
		break;

	    case LEFT_SHIFT:
	    case RIGHT_SHIFT:
		Tcl_AppendResult(interp, "second shift operand must be scalar",
		    (char *)NULL);
		goto error;

	    default:
		Tcl_AppendResult(interp, "unknown operator in expression",
		    (char *)NULL);
		goto error;
	    }
	}
    }
  done:
    if (value2.pv.buffer != value2.staticSpace) {
	Blt_Free(value2.pv.buffer);
    }
    Blt_Vec_Free(v2Ptr);
    return result;

  error:
    if (value2.pv.buffer != value2.staticSpace) {
	Blt_Free(value2.pv.buffer);
    }
    Blt_Vec_Free(v2Ptr);
    return TCL_ERROR;
}

/*
 *---------------------------------------------------------------------------
 *
 * EvaluateExpression --
 *
 *	This procedure provides top-level functionality shared by
 *	procedures like Tcl_ExprInt, Tcl_ExprDouble, etc.
 *
 * Results:
 *	The result is a standard Tcl return value.  If an error
 *	occurs then an error message is left in interp->result.
 *	The value of the expression is returned in *valuePtr, in
 *	whatever form it ends up in (could be string or integer
 *	or double).  Caller may need to convert result.  Caller
 *	is also responsible for freeing string memory in *valuePtr,
 *	if any was allocated.
 *
 * Side effects:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
static int
EvaluateExpression(
    Tcl_Interp *interp,		/* Context in which to evaluate the
				 * expression. */
    char *string,		/* Expression to evaluate. */
    Value *valuePtr)		/* Where to store result.  Should
				 * not be initialized by caller. */
{
    ParseInfo info;
    int result;
    VectorObject *vPtr;
    double *vp, *vend;

    info.expr = info.nextPtr = string;
    valuePtr->pv.buffer = valuePtr->pv.next = valuePtr->staticSpace;
    valuePtr->pv.end = valuePtr->pv.buffer + STATIC_STRING_SPACE - 1;
    valuePtr->pv.expandProc = Blt_ExpandParseValue;
    valuePtr->pv.clientData = NULL;

    result = NextValue(interp, &info, -1, valuePtr);
    if (result != TCL_OK) {
	return result;
    }
    if (info.token != END) {
	Tcl_AppendResult(interp, ": syntax error in expression \"",
	    string, "\"", (char *)NULL);
	return TCL_ERROR;
    }
    vPtr = valuePtr->vPtr;

    /* Check for NaN's and overflows. */
    for (vp = vPtr->valueArr, vend = vp + vPtr->length; vp < vend; vp++) {
	if (!FINITE(*vp)) {
	    /*
	     * IEEE floating-point error.
	     */
	    MathError(interp, *vp);
	    return TCL_ERROR;
	}
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * Math Functions --
 *
 *	This page contains the procedures that implement all of the
 *	built-in math functions for expressions.
 *
 * Results:
 *	Each procedure returns TCL_OK if it succeeds and places result
 *	information at *resultPtr.  If it fails it returns TCL_ERROR
 *	and leaves an error message in interp->result.
 *
 * Side effects:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
static int
ComponentFunc(
    ClientData clientData,	/* Contains address of procedure that
				 * takes one double argument and
				 * returns a double result. */
    Tcl_Interp *interp,
    VectorObject *vecObjPtr)
{
    ComponentProc *procPtr = (ComponentProc *) clientData;
    double *vp, *vend;

    errno = 0;
    for(vp = vecObjPtr->valueArr + vecObjPtr->first, 
	    vend = vecObjPtr->valueArr + vecObjPtr->last; vp <= vend; vp++) {
	*vp = (*procPtr) (*vp);
	if (errno != 0) {
	    MathError(interp, *vp);
	    return TCL_ERROR;
	}
	if (!FINITE(*vp)) {
	    /*
	     * IEEE floating-point error.
	     */
	    MathError(interp, *vp);
	    return TCL_ERROR;
	}
    }
    return TCL_OK;
}

static int
ScalarFunc(ClientData clientData, Tcl_Interp *interp, VectorObject *vecObjPtr)
{
    double value;
    ScalarProc *procPtr = (ScalarProc *) clientData;

    errno = 0;
    value = (*procPtr) (vecObjPtr);
    if (errno != 0) {
	MathError(interp, value);
	return TCL_ERROR;
    }
    if (Blt_Vec_ChangeLength(interp, vecObjPtr, 1) != TCL_OK) {
	return TCL_ERROR;
    }
    vecObjPtr->valueArr[0] = value;
    return TCL_OK;
}

/*ARGSUSED*/
static int
VectorFunc(ClientData clientData, Tcl_Interp *interp, VectorObject *vecObjPtr)
{
    VectorProc *procPtr = (VectorProc *) clientData;

    return (*procPtr) (vecObjPtr);
}


static MathFunction mathFunctions[] =
{
    {"abs",     ComponentFunc, Fabs},
    {"acos",	ComponentFunc, acos},
    {"asin",	ComponentFunc, asin},
    {"atan",	ComponentFunc, atan},
    {"adev",	ScalarFunc,    AvgDeviation},
    {"ceil",	ComponentFunc, ceil},
    {"cos",	ComponentFunc, cos},
    {"cosh",	ComponentFunc, cosh},
    {"exp",	ComponentFunc, exp},
    {"floor",	ComponentFunc, floor},
    {"kurtosis",ScalarFunc,    Kurtosis},
    {"length",	ScalarFunc,    Length},
    {"log",	ComponentFunc, log},
    {"log10",	ComponentFunc, log10},
    {"max",	ScalarFunc,    Blt_VecMax},
    {"mean",	ScalarFunc,    Mean},
    {"median",	ScalarFunc,    Median},
    {"min",	ScalarFunc,    Blt_VecMin},
    {"norm",	VectorFunc,    Norm},
    {"nz",	ScalarFunc,    Nonzeros},
    {"q1",	ScalarFunc,    Q1},
    {"q3",	ScalarFunc,    Q3},
    {"prod",	ScalarFunc,    Product},
#ifdef HAVE_DRAND48
    {"random",	ComponentFunc, drand48},
#endif
    {"round",	ComponentFunc, Round},
    {"sdev",	ScalarFunc,    StdDeviation},
    {"sin",	ComponentFunc, sin},
    {"sinh",	ComponentFunc, sinh},
    {"skew",	ScalarFunc,    Skew},
    {"sort",	VectorFunc,    Sort},
    {"sqrt",	ComponentFunc, sqrt},
    {"sum",	ScalarFunc,    Sum},
    {"tan",	ComponentFunc, tan},
    {"tanh",	ComponentFunc, tanh},
    {"var",	ScalarFunc,    Variance},
    {(char *)NULL,},
};

void
Blt_Vec_InstallMathFunctions(Blt_HashTable *tablePtr)
{
    MathFunction *mathPtr;

    for (mathPtr = mathFunctions; mathPtr->name != NULL; mathPtr++) {
	Blt_HashEntry *hPtr;
	int isNew;

	hPtr = Blt_CreateHashEntry(tablePtr, mathPtr->name, &isNew);
	Blt_SetHashValue(hPtr, (ClientData)mathPtr);
    }
}

void
Blt_Vec_UninstallMathFunctions(Blt_HashTable *tablePtr)
{
    Blt_HashEntry *hPtr;
    Blt_HashSearch cursor;

    for (hPtr = Blt_FirstHashEntry(tablePtr, &cursor); hPtr != NULL; 
	hPtr = Blt_NextHashEntry(&cursor)) {
	MathFunction *mathPtr;

	mathPtr = (MathFunction *) Blt_GetHashValue(hPtr);
	if (mathPtr->name == NULL) {
	    Blt_Free(mathPtr);
	}
    }
}


static void
InstallIndexProc(
    Blt_HashTable *tablePtr,
    const char *string,
    Blt_VectorIndexProc *procPtr) /* Pointer to function to be called
				   * when the vector finds the named index.
				   * If NULL, this indicates to remove
				   * the index from the table.
				   */
{
    Blt_HashEntry *hPtr;
    int dummy;

    hPtr = Blt_CreateHashEntry(tablePtr, string, &dummy);
    if (procPtr == NULL) {
	Blt_DeleteHashEntry(tablePtr, hPtr);
    } else {
	Blt_SetHashValue(hPtr, (ClientData)procPtr);
    }
}

void
Blt_Vec_InstallSpecialIndices(Blt_HashTable *tablePtr)
{
    InstallIndexProc(tablePtr, "min",  Blt_VecMin);
    InstallIndexProc(tablePtr, "max",  Blt_VecMax);
    InstallIndexProc(tablePtr, "mean", Mean);
    InstallIndexProc(tablePtr, "sum",  Sum);
    InstallIndexProc(tablePtr, "prod", Product);
}


/*
 *---------------------------------------------------------------------------
 *
 * Blt_ExprVector --
 *
 *	Evaluates an vector expression and returns its value(s).
 *
 * Results:
 *	Each of the procedures below returns a standard Tcl result.
 *	If an error occurs then an error message is left in
 *	interp->result.  Otherwise the value of the expression,
 *	in the appropriate form, is stored at *resultPtr.  If
 *	the expression had a result that was incompatible with the
 *	desired form then an error is returned.
 *
 * Side effects:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
int
Blt_ExprVector(
    Tcl_Interp *interp,		/* Context in which to evaluate the
				 * expression. */
    char *string,		/* Expression to evaluate. */
    Blt_Vector *vector)		/* Where to store result. */
{
    VectorInterpData *dataPtr;	/* Interpreter-specific data. */
    VectorObject *vecObjPtr = (VectorObject *)vector;
    Value value;

    dataPtr = (vector != NULL) 
	? vecObjPtr->dataPtr : Blt_Vec_GetInterpData(interp);
    value.vPtr = Blt_Vec_New(dataPtr);
    if (EvaluateExpression(interp, string, &value) != TCL_OK) {
	Blt_Vec_Free(value.vPtr);
	return TCL_ERROR;
    }
    if (vecObjPtr != NULL) {
	Blt_Vec_Duplicate(vecObjPtr, value.vPtr);
    } else {
	Tcl_Obj *listObjPtr;
	double *vp, *vend;

	/* No result vector.  Put values in interp->result.  */
	listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **) NULL);
	for (vp = value.vPtr->valueArr, vend = vp + value.vPtr->length; 
	     vp < vend; vp++) {
	    Tcl_ListObjAppendElement(interp, listObjPtr, Tcl_NewDoubleObj(*vp));
	}
	Tcl_SetObjResult(interp, listObjPtr);
    }
    Blt_Vec_Free(value.vPtr);
    return TCL_OK;
}
