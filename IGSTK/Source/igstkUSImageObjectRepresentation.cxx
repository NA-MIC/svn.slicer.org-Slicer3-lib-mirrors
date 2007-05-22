/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    $RCSfile: igstkUSImageObjectRepresentation.cxx,v $
  Language:  C++
  Date:      $Date: 2007/02/09 23:29:44 $
  Version:   $Revision: 1.3 $

  Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "igstkUSImageObjectRepresentation.h"

namespace igstk
{ 

/** Constructor */
USImageObjectRepresentation
::USImageObjectRepresentation():m_StateMachine(this)
{

}

/** Destructor */
USImageObjectRepresentation
::~USImageObjectRepresentation()
{

}

/** Print Self function */
void USImageObjectRepresentation
::PrintSelf( std::ostream& os, itk::Indent indent ) const
{
  Superclass::PrintSelf(os, indent);
}


} // end namespace igstk
