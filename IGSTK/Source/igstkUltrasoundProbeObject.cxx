/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    $RCSfile: igstkUltrasoundProbeObject.cxx,v $
  Language:  C++
  Date:      $Date: 2007/02/09 23:29:44 $
  Version:   $Revision: 1.4 $

  Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "igstkUltrasoundProbeObject.h"

namespace igstk
{ 

/** Constructor */
UltrasoundProbeObject::UltrasoundProbeObject():m_StateMachine(this)
{
  m_UltrasoundProbeSpatialObject = UltrasoundProbeSpatialObjectType::New();
  this->RequestSetSpatialObject( m_UltrasoundProbeSpatialObject );
} 

/** Destructor */
UltrasoundProbeObject::~UltrasoundProbeObject()  
{
}

/** Print object information */
void UltrasoundProbeObject
::PrintSelf( std::ostream& os, itk::Indent indent ) const
{
  Superclass::PrintSelf(os, indent);
}


} // end namespace igstk
