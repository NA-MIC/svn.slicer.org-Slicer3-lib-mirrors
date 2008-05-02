
/*
 * bltWinFont.c --
 *
 * This module implements rotated fonts for the BLT toolkit.
 *
 *	Copyright 2005 George A Howlett.
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
#include "tkDisplay.h"
#include "tkFont.h"
#include "bltFont.h"

#define DEBUG_FONT_SELECTION 0

enum FontTypes { FONT_UNKNOWN, FONT_TK, FONT_WIN };

static Blt_NameOfFontProc tkNameOfFont;
static Blt_GetFontMetricsProc tkGetFontMetrics;
static Blt_FontIdProc tkFontId;
static Blt_MeasureCharsProc tkMeasureChars;
static Blt_TextWidthProc tkTextWidth;
static Blt_FreeFontProc  tkFreeFont;
static Blt_DrawCharsProc tkDrawChars;
static Blt_PostscriptFontNameProc tkPostscriptFontName;
static Blt_FamilyOfFontProc tkFamilyOfFont;
static Blt_CanRotateFontProc tkCanRotateFont;
static Blt_UnderlineCharsProc  tkUnderlineChars;

BLT_EXTERN int TkFontGetPixels(Tk_Window tkwin, int size);	

static Blt_FontClass tkFontClass = {
    FONT_TK,
    tkNameOfFont,		/* Blt_NameOfFontProc */
    tkFamilyOfFont,		/* Blt_FamilyOfFontProc */
    tkFontId,			/* Blt_FontIdProc */
    tkGetFontMetrics,		/* Blt_GetFontMetricsProc */
    tkMeasureChars,		/* Blt_MeasureCharsProc */
    tkTextWidth,		/* Blt_TextWidthProc */
    tkCanRotateFont,		/* Blt_CanRotateFontProc */
    tkDrawChars,		/* Blt_DrawCharsProc */
    tkPostscriptFontName,	/* Blt_PostscriptFontNameProc */
    tkFreeFont,			/* Blt_FreeFontProc */
    tkUnderlineChars,		/* Blt_UnderlineCharsProc */
};

static char *
tkNameOfFont(struct Blt_FontStruct *fontPtr) 
{
    return Tk_NameOfFont(fontPtr->fontData);
}

static const char *
tkFamilyOfFont(struct Blt_FontStruct *fontPtr) 
{
    return ((TkFont *)fontPtr->fontData)->fa.family;
}

static Font
tkFontId(struct Blt_FontStruct *fontPtr) 
{
    return Tk_FontId(fontPtr->fontData);
}

static void
tkGetFontMetrics(struct Blt_FontStruct *fontPtr, Blt_FontMetrics *metricsPtr)
{
    Tk_GetFontMetrics(fontPtr->fontData, metricsPtr);
}

static int
tkMeasureChars(struct Blt_FontStruct *fontPtr, const char *text, int nBytes,
    int maxLength, int flags, int *lengthPtr)
{
    return Tk_MeasureChars(fontPtr->fontData, text, nBytes, maxLength, flags, 
	lengthPtr);
}

static int
tkTextWidth(struct Blt_FontStruct *fontPtr, const char *string, int nBytes)
{
    return Tk_TextWidth(fontPtr->fontData, string, nBytes);
}    

static void
tkDrawChars(
    Tk_Window tkwin,		/* Display on which to draw. */
    Drawable drawable,		/* Window or pixmap in which to draw. */
    GC gc,			/* Graphics context for drawing characters. */
    struct Blt_FontStruct *fontPtr, /* Font in which characters will be drawn;
				 * must be the same as font used in GC. */
    int depth,			/* Not used. */
    float angle,		/* Not used. */
    const char *text,		/* UTF-8 string to be displayed.  Need not be
				 * '\0' terminated.  All Tk meta-characters
				 * (tabs, control characters, and newlines)
				 * should be stripped out of the string that
				 * is passed to this function.  If they are
				 * not stripped out, they will be displayed as
				 * regular printing characters. */
    int nBytes,			/* Number of bytes in string. */
    int x, int y)		/* Coordinates at which to place origin of
				 * string when drawing. */
{
    Tk_DrawChars(Tk_Display(tkwin), drawable, gc, fontPtr->fontData, text, 
	nBytes, x, y);
}

static int
tkPostscriptFontName(struct Blt_FontStruct *fontPtr, Tcl_DString *resultPtr) 
{
    return Tk_PostscriptFontName(fontPtr->fontData, resultPtr);
}

static int
tkCanRotateFont(struct Blt_FontStruct *fontPtr, float angle) 
{
    return FALSE;
}

static void
tkFreeFont(struct Blt_FontStruct *fontPtr) 
{
    Tk_FreeFont(fontPtr->fontData);
    Blt_Free(fontPtr);
}

/*
 *---------------------------------------------------------------------------
 *
 * tkUnderlineChars --
 *
 *	This procedure draws an underline for a given range of characters
 *	in a given string.  It doesn't draw the characters (which are
 *	assumed to have been displayed previously); it just draws the
 *	underline.  This procedure would mainly be used to quickly
 *	underline a few characters without having to construct an
 *	underlined font.  To produce properly underlined text, the
 *	appropriate underlined font should be constructed and used. 
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Information gets displayed in "drawable".
 *
 *---------------------------------------------------------------------------
 */

static void
tkUnderlineChars(
    Display *display,		/* Display on which to draw. */
    Drawable drawable,		/* Window or pixmap in which to draw. */
    GC gc,			/* Graphics context for actually drawing
				 * line. */
    struct Blt_FontStruct *fontPtr, /* Font used in GC; must have been
				 * allocated by Tk_GetFont().  Used
				 * for character dimensions, etc. */
    const char *string,		/* String containing characters to be
				 * underlined or overstruck. */
    int x, int y,		/* Coordinates at which first character of
				 * string is drawn. */
    int first,			/* Byte offset of the first character. */
    int last)			/* Byte offset after the last character. */
{
    Tk_UnderlineChars(display, drawable, gc, fontPtr->fontData, string, x, y, 
	first, last);
}

#ifdef notdef
/*
 *---------------------------------------------------------------------------
 *
 * CreateRotatedFont --
 *
 *	Creates a rotated copy of the given font.  This only works 
 *	for TrueType fonts.
 *
 * Results:
 *	Returns the newly create font or NULL if the font could not
 *	be created.
 *
 *---------------------------------------------------------------------------
 */

static HFONT
winCreateRotatedFont(
    unsigned long fontId,	/* Font identifier (actually a Tk_Font) */
    long angle10)
{				/* Number of degrees to rotate font */
    TkFontAttributes *faPtr;	/* Set of attributes to match. */
    TkFont *fontPtr;
    HFONT hFont;
    LOGFONTW lf;

    fontPtr = (TkFont *) fontId;
    faPtr = &fontPtr->fa;
    ZeroMemory(&lf, sizeof(LOGFONT));
    lf.lfHeight = -faPtr->size;
    if (lf.lfHeight < 0) {
	HDC dc;

	dc = GetDC(NULL);
	lf.lfHeight = -MulDiv(faPtr->size, GetDeviceCaps(dc, LOGPIXELSY), 72);
	ReleaseDC(NULL, dc);
    }
    lf.lfWidth = 0;
    lf.lfEscapement = lf.lfOrientation = angle10;
#define TK_FW_NORMAL	0
    lf.lfWeight = (faPtr->weight == TK_FW_NORMAL) ? FW_NORMAL : FW_BOLD;
    lf.lfItalic = faPtr->slant;
    lf.lfUnderline = faPtr->underline;
    lf.lfStrikeOut = faPtr->overstrike;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfOutPrecision = OUT_TT_ONLY_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = DEFAULT_QUALITY;
    lf.lfQuality = ANTIALIASED_QUALITY;
    lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

    hFont = NULL;
    if (faPtr->family == NULL) {
	lf.lfFaceName[0] = '\0';
    } else {
#if (_TCL_VERSION >= _VERSION(8,1,0)) 
	Tcl_DString dString;

	Tcl_UtfToExternalDString(systemEncoding, faPtr->family, -1, &dString);

	if (Blt_GetPlatformId() == VER_PLATFORM_WIN32_NT) {
	    Tcl_UniChar *src, *dst;
	    
	    /*
	     * We can only store up to LF_FACESIZE wide characters
	     */
	    if (Tcl_DStringLength(&dString) >= (LF_FACESIZE * sizeof(WCHAR))) {
		Tcl_DStringSetLength(&dString, LF_FACESIZE);
	    }
	    src = (Tcl_UniChar *)Tcl_DStringValue(&dString);
	    dst = (Tcl_UniChar *)lf.lfFaceName;
	    while (*src != '\0') {
		*dst++ = *src++;
	    }
	    *dst = '\0';
	    hFont = CreateFontIndirectW((LOGFONTW *)&lf);
	} else {
	    /*
	     * We can only store up to LF_FACESIZE characters
	     */
	    if (Tcl_DStringLength(&dString) >= LF_FACESIZE) {
		Tcl_DStringSetLength(&dString, LF_FACESIZE);
	    }
	    strcpy((char *)lf.lfFaceName, Tcl_DStringValue(&dString));
	    hFont = CreateFontIndirectA((LOGFONTA *)&lf);
	}
	Tcl_DStringFree(&dString);
#else
	strncpy((char *)lf.lfFaceName, faPtr->family, LF_FACESIZE - 1);
	lf.lfFaceName[LF_FACESIZE] = '\0';
#endif /* _TCL_VERSION >= 8.1.0 */
    }

    if (hFont == NULL) {
#if WINDEBUG
	PurifyPrintf("can't create font: %s\n", Blt_LastError());
#endif
    } else { 
	HFONT oldFont;
	TEXTMETRIC tm;
	HDC hRefDC;
	int result;

	/* Check if the rotated font is really a TrueType font. */

	hRefDC = GetDC(NULL);		/* Get the desktop device context */
	oldFont = SelectFont(hRefDC, hFont);
	result = ((GetTextMetrics(hRefDC, &tm)) && 
		  (tm.tmPitchAndFamily & TMPF_TRUETYPE));
	SelectFont(hRefDC, oldFont);
	ReleaseDC(NULL, hRefDC);
	if (!result) {
#if WINDEBUG
	    PurifyPrintf("not a true type font\n");
#endif
	    DeleteFont(hFont);
	    return NULL;
	}
    }
    return hFont;
}

static char *
winNameOfFont(struct Blt_FontStruct *fontPtr) 
{
    return Tk_NameOfFont(fontPtr->fontData);
}

static const char *
winFamilyOfFont(struct Blt_FontStruct *fontPtr) 
{
    return ((TkFont *)fontPtr->fontData)->fa.family;
}

static Font
winFontId(struct Blt_FontStruct *fontPtr) 
{
    return Tk_FontId(fontPtr->fontData);
}

static void
winGetFontMetrics(struct Blt_FontStruct *fontPtr, Blt_FontMetrics *metricsPtr)
{
    Tk_GetFontMetrics(fontPtr->fontData, metricsPtr);
}

static int
winMeasureChars(struct Blt_FontStruct *fontPtr, const char *text, int nBytes,
    int maxLength, int flags, int *lengthPtr)
{
    return Tk_MeasureChars(fontPtr->fontData, text, nBytes, maxLength, flags, 
	lengthPtr);
}

static int
winTextWidth(struct Blt_FontStruct *fontPtr, const char *string, int nBytes)
{
    return Tk_TextWidth(fontPtr->fontData, string, nBytes);
}    

static void
winDrawChars(
    Tk_Window tkwin,		/* Display on which to draw. */
    Drawable drawable,		/* Window or pixmap in which to draw. */
    GC gc,			/* Graphics context for drawing characters. */
    struct Blt_FontStruct *fontPtr, /* Font in which characters will be drawn;
				 * must be the same as font used in GC. */
    int depth,			/* Not used. */
    float angle,		/* Not used. */
    const char *text,		/* UTF-8 string to be displayed.  Need not be
				 * '\0' terminated.  All Tk meta-characters
				 * (tabs, control characters, and newlines)
				 * should be stripped out of the string that
				 * is passed to this function.  If they are
				 * not stripped out, they will be displayed as
				 * regular printing characters. */
    int nBytes,			/* Number of bytes in string. */
    int x, int y)		/* Coordinates at which to place origin of
				 * string when drawing. */
{
    HFONT hFont;
    WinFontData *dataPtr = fontPtr->fontData;
    Blt_HashEntry *hPtr;
    long angle10;

    angle10 = (long)(angle * 10.0);
    hPtr = Blt_FindHashEntry(&dataPtr->fontTable, (char *)angle10);
    if (hPtr == NULL) {
	fprintf(stderr, "can't find font %s at %g rotated\n", 
		dataPtr->name, angle);
	return;			/* Can't find instance at requested angle. */
    }
    hFont = Blt_GetHashValue(hPtr);

    Tk_DrawChars(Tk_Display(tkwin), drawable, gc, fontPtr->fontData, text, 
	nBytes, x, y);
}

static int
winPostscriptFontName(struct Blt_FontStruct *fontPtr, Tcl_DString *resultPtr) 
{
    return Tk_PostscriptFontName(fontPtr->fontData, resultPtr);
}

static int
winCanRotateFont(struct Blt_FontStruct *fontPtr, float angle) 
{
    winFontData *dataPtr = fontPtr->fontData;
    HFONT hFont;
    long angle10;

    angle10 = (int)((double)angle * 10.0);
    if (Blt_FindHashEntry(&dataPtr->fontTable, (char *)angle10) != NULL) {
	return TRUE;		/* Rotated font already exists. */
    }
    hFont = winCreateRotateFont(Blt_FontId(fontPtr), angle10);
    if (hFont == NULL) {
	return FALSE;
    }
    Blt_SetHashValue(hPtr, hFont);
    return TRUE;
}

static void
winFreeFont(struct Blt_FontStruct *fontPtr) 
{
    Tk_FreeFont(fontPtr->fontData);
    Blt_Free(fontPtr);
}

#ifdef notdef
/*
 *---------------------------------------------------------------------------
 *
 * Tk_UnderlineChars --
 *
 *	This procedure draws an underline for a given range of characters
 *	in a given string.  It doesn't draw the characters (which are
 *	assumed to have been displayed previously); it just draws the
 *	underline.  This procedure would mainly be used to quickly
 *	underline a few characters without having to construct an
 *	underlined font.  To produce properly underlined text, the
 *	appropriate underlined font should be constructed and used. 
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Information gets displayed in "drawable".
 *
 *---------------------------------------------------------------------------
 */

static void
winUnderlineChars(
    Display *display,		/* Display on which to draw. */
    Drawable drawable,		/* Window or pixmap in which to draw. */
    GC gc,			/* Graphics context for actually drawing
				 * line. */
    Blt_Font font,		/* Font used in GC;  must have been allocated
				 * by Tk_GetFont().  Used for character
				 * dimensions, etc. */
    const char *string,		/* String containing characters to be
				 * underlined or overstruck. */
    int x, int y,		/* Coordinates at which first character of
				 * string is drawn. */
    int firstByte,		/* Index of first byte of first character. */
    int lastByte)		/* Index of first byte after the last
				 * character. */
{
    TkFont *fontPtr;
    int startX, endX;

    Blt_MeasureChars(font, string, firstByte, -1, 0, &startX);
    Blt_MeasureChars(font, string, lastByte, -1, 0, &endX);

    underlinePos = descent / 2;
    underlineHeight = TkFontGetPixels(tkwin, fontPtr->fa.size) / 10;

    fontPtr = (TkFont *) tkfont;
    XFillRectangle(display, drawable, gc, x + startX,
	    y + fontPtr->underlinePos, (unsigned int) (endX - startX),
	    (unsigned int) fontPtr->underlineHeight);
}
#endif
#endif


/*
 *---------------------------------------------------------------------------
 *
 * Blt_GetFontFromObj -- 
 *
 *	Given a string description of a font, map the description to a
 *	corresponding Tk_Font that represents the font.
 *
 * Results:
 *	The return value is token for the font, or NULL if an error
 *	prevented the font from being created.  If NULL is returned, an
 *	error message will be left in the interp's result.
 *
 * Side effects:
 *	The font is added to an internal database with a reference
 *	count.  For each call to this procedure, there should eventually
 *	be a call to Tk_FreeFont() or Tk_FreeFontFromObj() so that the
 *	database is cleaned up when fonts aren't in use anymore.
 *
 *---------------------------------------------------------------------------
 */
Blt_Font
Blt_GetFontFromObj(
    Tcl_Interp *interp,		/* Interp for database and error return. */
    Tk_Window tkwin,		/* For display on which font will be used. */
    Tcl_Obj *objPtr)		/* String describing font, as: named font,
				 * native format, or parseable string. */
{
    struct Blt_FontStruct *fontPtr; 
    
    fontPtr = Blt_Calloc(1, sizeof(struct Blt_FontStruct));
    if (fontPtr == NULL) {
	return NULL;		/* Out of memory. */
    }
    fontPtr->fontData = Tk_GetFont(interp, tkwin, Tcl_GetString(objPtr));
    if (fontPtr->fontData == NULL) {
	Blt_Free(fontPtr);
#if DEBUG_FONT_SELECTION
    fprintf(stderr, "FAILED to find Tk font \"%s\"\n", Tcl_GetString(objPtr));
#endif
	return NULL;		/* Failed to find Tk font. */
    }
#if DEBUG_FONT_SELECTION
    fprintf(stderr, "SUCCESS: Found Tk font \"%s\"\n", Tcl_GetString(objPtr));
#endif
    fontPtr->classPtr = &tkFontClass;
    return fontPtr;		/* Found Tk font. */
}

Blt_Font
Blt_AllocFontFromObj(
    Tcl_Interp *interp,		/* Interp for database and error return. */
    Tk_Window tkwin,		/* For screen on which font will be used. */
    Tcl_Obj *objPtr)		/* Object describing font, as: named font,
				 * native format, or parseable string. */
{
    return Blt_GetFontFromObj(interp, tkwin, objPtr);
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_GetFont -- 
 *
 *	Given a string description of a font, map the description to a
 *	corresponding Tk_Font that represents the font.
 *
 * Results:
 *	The return value is token for the font, or NULL if an error
 *	prevented the font from being created.  If NULL is returned, an
 *	error message will be left in interp's result object.
 *
 * Side effects:
 * 	The font is added to an internal database with a reference
 * 	count.  For each call to this procedure, there should
 * 	eventually be a call to Blt_FreeFont so that the database is
 * 	cleaned up when fonts aren't in use anymore.
 *
 *---------------------------------------------------------------------------
 */

Blt_Font
Blt_GetFont(
    Tcl_Interp *interp,		/* Interp for database and error return. */
    Tk_Window tkwin,		/* For screen on which font will be used. */
    const char *string)		/* Object describing font, as: named font,
				 * native format, or parseable string. */
{
    Blt_Font font;
    Tcl_Obj *objPtr;

    objPtr = Tcl_NewStringObj(string, strlen(string));
    font = Blt_GetFontFromObj(interp, tkwin, objPtr);
    Tcl_DecrRefCount(objPtr);
    return font;
}
