/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    $RCSfile: igstkView3D.cxx,v $
  Language:  C++
  Date:      $Date: 2007/02/09 23:29:44 $
  Version:   $Revision: 1.8 $

  Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "igstkView3D.h"

#include "vtkInteractorStyleTrackballCamera.h"

namespace igstk {

/** Constructor */
View3D::View3D( int x, int y, int w, int h, const char *l ) : View(x,y,w,h,l)
{
  vtkInteractorStyleTrackballCamera * interactorStyle = 
                                     vtkInteractorStyleTrackballCamera::New();
  this->SetInteractorStyle( interactorStyle );
  interactorStyle->Delete();
}

/** Destructor */
View3D::~View3D()
{
  this->SetInteractorStyle( NULL );
}

/** Main FLTK event handler */
int View3D::handle( int event ) 
{
  return View::handle( event );
}


/** Print object information */
void View3D::PrintSelf( std::ostream& os, ::itk::Indent indent ) const
{
  this->Superclass::PrintSelf(os,indent);
}


} // end namespace igstk
