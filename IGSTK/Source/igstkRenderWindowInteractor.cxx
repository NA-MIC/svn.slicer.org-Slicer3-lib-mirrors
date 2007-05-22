/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    $RCSfile: igstkRenderWindowInteractor.cxx,v $
  Language:  C++
  Date:      $Date: 2007/02/09 23:29:44 $
  Version:   $Revision: 1.3 $

  Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
// Disabling warning C4355: 'this' : used in base member initializer list
#if defined(_MSC_VER)
#pragma warning ( disable : 4355 )
#endif


#include "igstkRenderWindowInteractor.h"
#include "vtkObjectFactory.h"


namespace igstk {

vtkCxxRevisionMacro (RenderWindowInteractor, "$Revision: 1.3 $");
vtkStandardNewMacro (RenderWindowInteractor);


RenderWindowInteractor
::RenderWindowInteractor()
{
}

RenderWindowInteractor
::~RenderWindowInteractor()
{
}


void
RenderWindowInteractor
::Initialize()
{
  this->Initialized = 1;
  this->Enable();
}

void
RenderWindowInteractor
::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf( os, indent );
}


} // end of IGSTK namespace
