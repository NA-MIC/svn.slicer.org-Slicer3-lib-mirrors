
/*
 * bltUnixMain.c --
 *
 * Provides a default version of the Tcl_AppInit procedure for
 * use in wish and similar Tk-based applications.
 *
 *	Copyright 1998-2004 George A Howlett.
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
 *
 * This file was adapted from the Tk distribution.
 *
 *	Copyright (c) 1993 The Regents of the University of
 *	California. All rights reserved.
 *
 *	Permission is hereby granted, without written agreement and
 *	without license or royalty fees, to use, copy, modify, and
 *	distribute this software and its documentation for any
 *	purpose, provided that the above copyright notice and the
 *	following two paragraphs appear in all copies of this
 *	software.
 *
 *	IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO
 *	ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 *	CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS
 *	SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
 *	CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *	THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY
 *	WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *	PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS"
 *	BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO *
 *	PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
 *	MODIFICATIONS.
 *
 */

#include <blt.h>
#include <tcl.h>
#ifndef TCL_ONLY
#include <tk.h>
#endif
#include "config.h"
/*
 * The following variable is a special hack that is needed in order for
 * Sun shared libraries to be used for Tcl.
 */

#ifdef NEED_MATHERR
BLT_EXTERN int matherr();
int *tclDummyMathPtr = (int *)matherr;
#endif

BLT_EXTERN Tcl_AppInitProc Bltcore_Init;
BLT_EXTERN Tcl_AppInitProc Bltcore_SafeInit;
#ifndef TCL_ONLY
BLT_EXTERN Tcl_AppInitProc Bltx_Init;
BLT_EXTERN Tcl_AppInitProc Bltx_SafeInit;
#endif
#ifdef STATIC_PKGS
#ifndef TCL_ONLY
BLT_EXTERN Tcl_AppInitProc Blt_PictureBmpInit;
BLT_EXTERN Tcl_AppInitProc Blt_PictureGifInit;
BLT_EXTERN Tcl_AppInitProc Blt_PictureJpgInit;
BLT_EXTERN Tcl_AppInitProc Blt_PicturePbmInit;
BLT_EXTERN Tcl_AppInitProc Blt_PicturePhotoInit;
BLT_EXTERN Tcl_AppInitProc Blt_PicturePngInit;
BLT_EXTERN Tcl_AppInitProc Blt_PicturePsInit;
BLT_EXTERN Tcl_AppInitProc Blt_PictureTifInit;
BLT_EXTERN Tcl_AppInitProc Blt_PictureXbmInit;
BLT_EXTERN Tcl_AppInitProc Blt_PictureXpmInit;
#endif /* TCL_ONLY */
BLT_EXTERN Tcl_AppInitProc Blt_Dt_CsvInit;
BLT_EXTERN Tcl_AppInitProc Blt_Dt_VectorInit;
#ifdef HAVE_LIBEXPAT
BLT_EXTERN Tcl_AppInitProc Blt_Dt_XmlInit;
BLT_EXTERN Tcl_AppInitProc Blt_TreeXmlInit;
#endif
BLT_EXTERN Tcl_AppInitProc Blt_Dt_TreeInit;
#ifdef HAVE_LIBMYSQL
BLT_EXTERN Tcl_AppInitProc Blt_Dt_MysqlInit;
#endif	/* HAVE_LIBMYSQL */
#endif /* STATIC_PKGS */
/*
 *---------------------------------------------------------------------------
 *
 * Initialize --
 *
 *	This procedure performs application-specific initialization.
 *	Most applications, especially those that incorporate additional
 *	packages, will have their own version of this procedure.
 *
 * Results:
 *	Returns a standard Tcl completion code, and leaves an error
 *	message in interp->result if an error occurs.
 *
 * Side effects:
 *	Depends on the startup script.
 *
 *---------------------------------------------------------------------------
 */

static int
Initialize(Tcl_Interp *interp)	/* Interpreter for application. */
{
#ifdef TCLLIBPATH
    /* 
     * It seems that some distributions of Tcl don't compile-in a
     * default location of the library.  This causes Tcl_Init to fail
     * if bltwish and bltsh are moved to another directory. The
     * workaround is to set the magic variable "tclDefaultLibrary".
     */
    Tcl_SetVar(interp, "tclDefaultLibrary", TCLLIBPATH, TCL_GLOBAL_ONLY);
#endif /* TCLLIBPATH */
    if (Tcl_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }
    /*
     * Call the init procedures for included packages.  Each call should
     * look like this:
     *
     * if (Mod_Init(interp) == TCL_ERROR) {
     *     return TCL_ERROR;
     * }
     *
     * where "Mod" is the name of the module.
     */
    if (Bltcore_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }
    Tcl_StaticPackage(interp, "bltcore", Bltcore_Init, Bltcore_SafeInit);

#ifdef STATIC_PKGS
    if (Blt_Dt_CsvInit(interp) != TCL_OK) {
	return TCL_ERROR;
    }
    Tcl_StaticPackage(interp, "bltdatatablecsv", Blt_Dt_CsvInit, 
	Blt_Dt_CsvInit);
#ifdef HAVE_LIBMYSQL
    if (Blt_Dt_MysqlInit(interp) != TCL_OK) {
	return TCL_ERROR;
    }
    Tcl_StaticPackage(interp, "bltdatatablemysql", Blt_Dt_MysqlInit, 
	Blt_Dt_MysqlInit);
#endif	/* HAVE_LIBMYSQL */
    if (Blt_Dt_VectorInit(interp) != TCL_OK) {
	return TCL_ERROR;
    }
    Tcl_StaticPackage(interp, "bltdatatablevector", Blt_Dt_VectorInit,
	Blt_Dt_VectorInit);
   if (Blt_Dt_TreeInit(interp) != TCL_OK) {
	return TCL_ERROR;
    }
    Tcl_StaticPackage(interp, "bltdatatabletree", Blt_Dt_TreeInit, 
	Blt_Dt_TreeInit);
#ifdef HAVE_LIBEXPAT
    if (Blt_Dt_XmlInit(interp) != TCL_OK) {
	return TCL_ERROR;
    }
    Tcl_StaticPackage(interp, "bltdatatablexml", Blt_Dt_XmlInit, 
	Blt_Dt_XmlInit);

    if (Blt_TreeXmlInit(interp) != TCL_OK) {
	return TCL_ERROR;
    }
    Tcl_StaticPackage(interp, "blttreexml", Blt_TreeXmlInit, Blt_TreeXmlInit);
#endif	/* HAVE_LIBEXPAT */
#endif /* STATIC_PKGS */

#ifndef TCL_ONLY
    if (Tk_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }
    if (Bltx_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }
    Tcl_StaticPackage(interp, "bltextra", Bltx_Init, Bltx_SafeInit);

#ifdef STATIC_PKGS
#ifdef HAVE_LIBJPG
    if (Blt_PictureJpgInit(interp) != TCL_OK) {
	return TCL_ERROR;
    }
    Tcl_StaticPackage(interp, "bltpicturejpg", Blt_PictureJpgInit, 
		      Blt_PictureJpgInit);
#endif /*HAVE_LIBJPG*/
#ifdef HAVE_LIBTIF
    if (Blt_PictureTifInit(interp) != TCL_OK) {
	return TCL_ERROR;
    }
    Tcl_StaticPackage(interp, "bltpicturetif", Blt_PictureTifInit, 
		      Blt_PictureTifInit);
#endif /*HAVE_LIBTIF*/
#ifdef HAVE_LIBPNG
    if (Blt_PicturePngInit(interp) != TCL_OK) {
	return TCL_ERROR;
    }
    Tcl_StaticPackage(interp, "bltpicturepng", Blt_PicturePngInit, 
	Blt_PicturePngInit);
#endif /*HAVE_LIBPNG*/
#ifdef HAVE_LIBXPM
    if (Blt_PictureXpmInit(interp) != TCL_OK) {
	return TCL_ERROR;
    }
    Tcl_StaticPackage(interp, "bltpicturexpm", Blt_PictureXpmInit, 
	Blt_PictureXpmInit);
#endif /*HAVE_LIBXPM*/
    if (Blt_PictureGifInit(interp) != TCL_OK) {
	return TCL_ERROR;
    }
    Tcl_StaticPackage(interp, "bltpicturegif", Blt_PictureGifInit, 
	Blt_PictureGifInit);

    if (Blt_PictureXbmInit(interp) != TCL_OK) {
	return TCL_ERROR;
    }
    Tcl_StaticPackage(interp, "bltpicturexbm", Blt_PictureXbmInit, 
	Blt_PictureXbmInit);

    if (Blt_PictureBmpInit(interp) != TCL_OK) {
	return TCL_ERROR;
    }
    Tcl_StaticPackage(interp, "bltpicturebmp", Blt_PictureBmpInit, 
	Blt_PictureBmpInit);

    if (Blt_PicturePhotoInit(interp) != TCL_OK) {
	return TCL_ERROR;
    }
    Tcl_StaticPackage(interp, "bltpicturephoto", Blt_PicturePhotoInit, 
	Blt_PicturePhotoInit);

    if (Blt_PicturePbmInit(interp) != TCL_OK) {
	return TCL_ERROR;
    }
    Tcl_StaticPackage(interp, "bltpicturepbm", Blt_PicturePbmInit, 
	Blt_PicturePbmInit);

    if (Blt_PicturePsInit(interp) != TCL_OK) {
	return TCL_ERROR;
    }
    Tcl_StaticPackage(interp, "bltpictureps", Blt_PicturePsInit, 
	Blt_PicturePsInit);
#endif /* STATIC_PKGS */
#endif /*TCL_ONLY*/
    /*
     * Specify a user-specific startup file to invoke if the application
     * is run interactively.  Typically the startup file is "~/.apprc"
     * where "app" is the name of the application.  If this line is deleted
     * then no user-specific startup file will be run under any conditions.
     */

    Tcl_SetVar(interp, "tcl_rcFileName", "~/.wishrc", TCL_GLOBAL_ONLY);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * main --
 *
 *	This is the main program for the application.
 *
 * Results:
 *	None: Tk_Main never returns here, so this procedure never
 *	returns either.
 *
 * Side effects:
 *	Whatever the application does.
 *
 *---------------------------------------------------------------------------
 */
int
main(int argc, char **argv)
{
#ifdef TCL_ONLY
    Tcl_Main(argc, argv, Initialize);
#else 
    Tk_Main(argc, argv, Initialize);
#endif
    return 0;			/* Needed only to prevent compiler warning. */
}

