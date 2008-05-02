
/*
 * bltText.h --
 *
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

#ifndef _BLT_TEXT_H
#define _BLT_TEXT_H

#define DEF_TEXT_FLAGS (TK_PARTIAL_OK | TK_IGNORE_NEWLINES)
#define UPDATE_GC	1

/*
 *---------------------------------------------------------------------------
 *
 * TextFragment --
 *
 *---------------------------------------------------------------------------
 */
typedef struct {
    const char *text;		/* Text to be displayed */

    short int x, y;		/* X-Y offset of the baseline from the
				 * upper-left corner of the bbox. */
    short int sx, sy;		/* See bltWinUtil.c */
    unsigned short int count;	/* Number of bytes in text. The actual
				 * character count may differ because of
				 * multi-byte UTF encodings. */

    unsigned short int width;	/* Width of segment in pixels. This
				 * information is used to draw PostScript
				 * strings the same width as X. */
} TextFragment;

/*
 *---------------------------------------------------------------------------
 *
 * TextLayout --
 *
 *---------------------------------------------------------------------------
 */
typedef struct {
    TextFragment *underlinePtr;
    short int underline;
    short int width, height;	/* Dimensions of text bounding box */
    short int nFrags;		/* # fragments of text */
    TextFragment fragments[1]; /* Information about each fragment of text */
} TextLayout;

/*
 *---------------------------------------------------------------------------
 *
 * TextStyle --
 *
 * 	A somewhat convenient structure to hold text attributes that determine
 * 	how a text string is to be displayed on the screen or drawn with
 * 	PostScript commands.  The alternative is to pass lots of parameters to
 * 	the drawing and printing routines. This seems like a more efficient
 * 	and less cumbersome way of passing parameters.
 *
 *---------------------------------------------------------------------------
 */
typedef struct {
    unsigned int state;		/* If non-zero, indicates to draw text
				 * in the active color */
    XColor *color;		/* Color to draw the text. */
    Blt_Font font;		/* Font to use to draw text */
    Tk_3DBorder border;		/* Background color of text.  This is also
				 * used for drawing disabled text. */
    float angle;		/* Rotation of text in degrees. */
    Tk_Justify justify;		/* Justification of the text string. This
				 * only matters if the text is composed
				 * of multiple lines. */
    Tk_Anchor anchor;		/* Indicates how the text is anchored around
				 * its x,y coordinates. */
    Blt_Pad padX, padY;		/* # pixels padding of around text region. */
    short int leader;		/* # pixels spacing between lines of text. */
    short int underline;
    /* Private fields. */
    unsigned short flags;
    GC gc;			/* GC used to draw the text */
} TextStyle;


BLT_EXTERN TextLayout *Blt_Ts_CreateLayout(const char *string, int length, 
	TextStyle *tsPtr);

BLT_EXTERN void Blt_Ts_DrawLayout(Tk_Window tkwin, Drawable drawable, 
	TextLayout *textPtr, TextStyle *tsPtr, int x, int y, int xMax);

BLT_EXTERN void Blt_Ts_GetExtents(TextStyle *tsPtr, const char *text, 
	int *widthPtr, int *heightPtr);

BLT_EXTERN void Blt_Ts_ResetStyle(Tk_Window tkwin, TextStyle *tsPtr);

BLT_EXTERN void Blt_Ts_FreeStyle(Display *display, TextStyle *tsPtr);

BLT_EXTERN void Blt_Ts_SetDrawStyle (TextStyle *tsPtr, Blt_Font font, GC gc, 
	XColor *fgColor, float angle, Tk_Anchor anchor, Tk_Justify justify, 
	int leader);

BLT_EXTERN void Blt_Ts_SetPrintStyle(TextStyle *tsPtr, Blt_Font font, 
	XColor *fgColor, XColor *bgColor, float angle, Tk_Anchor anchor, 
	Tk_Justify justify, int leader);

BLT_EXTERN void Blt_DrawText(Tk_Window tkwin, Drawable drawable, 
	const char *string, TextStyle *tsPtr, int x, int y);

BLT_EXTERN void Blt_DrawText2(Tk_Window tkwin, Drawable drawable, 
	const char *string, TextStyle *tsPtr, int x, int y, Dim2D * dimPtr);

BLT_EXTERN Pixmap Blt_Ts_Bitmap(Tk_Window tkwin, TextLayout *textPtr, 
	TextStyle *tsPtr, int *widthPtr, int *heightPtr, int xMax);

BLT_EXTERN int Blt_DrawTextWithRotatedFont(Tk_Window tkwin, Drawable drawable, 
	float angle, TextStyle *tsPtr, TextLayout *textPtr, int x, int y, 
	int xMax);

BLT_EXTERN void Blt_DrawLayout(Tk_Window tkwin, Drawable drawable, GC gc, 
	Blt_Font font, int depth, float angle, int x, int y, 
	TextLayout *layoutPtr, int xMax);

BLT_EXTERN void Blt_GetTextExtents(Blt_Font font, int leader, const char *text, 
	int textLen, int *widthPtr, int *heightPtr);

BLT_EXTERN void Blt_RotateTextStartingPositions(TextLayout *textPtr, 
	float angle);

BLT_EXTERN Tk_TextLayout Blt_ComputeTextLayout(Blt_Font font, 
	const char *string, int numChars, int wrapLength, Tk_Justify justify, 
	int flags, int *widthPtr, int *heightPtr);

BLT_EXTERN void Blt_DrawTextLayout(Display *display, Drawable drawable, GC gc, 
	Tk_TextLayout layout, int x, int y, int firstChar, int lastChar);

BLT_EXTERN int Blt_CharBbox(Tk_TextLayout layout, int index, int *xPtr, 
	int *yPtr, int *widthPtr, int *heightPtr);

BLT_EXTERN void Blt_UnderlineTextLayout(Display *display, Drawable drawable,
	GC gc, Tk_TextLayout layout, int x, int y, int underline);

#define Blt_Ts_GetAnchor(ts)		((ts).anchor)
#define Blt_Ts_GetAngle(ts)		((ts).angle)
#define Blt_Ts_GetBorder(ts)		((ts).border)
#define Blt_Ts_GetFont(ts)		((ts).font)
#define Blt_Ts_GetForeground(ts)	((ts).color)
#define Blt_Ts_GetJustify(ts)		((ts).justify)
#define Blt_Ts_GetLeader(ts)		((ts).leader)

#define Blt_Ts_SetAnchor(ts, a)	((ts).anchor = (a))
#define Blt_Ts_SetAngle(ts, r)		((ts).angle = (float)(r))
#define Blt_Ts_SetBorder(ts, b)	((ts).border = (b))
#define Blt_Ts_SetFont(ts, f)		\
	(((ts).font != (f)) ? ((ts).font = (f), (ts).flags |= UPDATE_GC) : 0)
#define Blt_Ts_SetForeground(ts, c)    \
	(((ts).color != (c)) ? ((ts).color = (c), (ts).flags |= UPDATE_GC) : 0)
#define Blt_Ts_SetGC(ts, g)	((ts).gc = (g))
#define Blt_Ts_SetJustify(ts, j)	((ts).justify = (j))
#define Blt_Ts_SetLeader(ts, l)	((ts).leader = (l))
#define Blt_Ts_SetPadding(ts, l, r, t, b)    \
	((ts).padX.side1 = (l), \
	(ts).padX.side2 = (r),  \
	(ts).padY.side1 = (t),  \
	(ts).padY.side2 = (b))
#define Blt_Ts_SetState(ts, s)		((ts).state = (s))
#define Blt_Ts_SetUnderline(ts, ul)	((ts).underline = (ul))

#define Blt_Ts_InitStyle(ts)		\
	((ts).anchor = TK_ANCHOR_NW, \
	(ts).color = (XColor *)NULL, \
	(ts).font = NULL, \
	(ts).justify = TK_JUSTIFY_LEFT, \
	(ts).leader = 0, \
	(ts).underline = -1, \
	(ts).padX.side1 = (ts).padX.side2 = 0, \
	(ts).padY.side1 = (ts).padY.side2 = 0, \
	(ts).state = 0, \
	(ts).flags = 0, \
        (ts).gc = NULL, \
	(ts).angle = 0.0)

#endif /* _BLT_TEXT_H */
