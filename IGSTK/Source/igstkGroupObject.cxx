/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    $RCSfile: igstkGroupObject.cxx,v $
  Language:  C++
  Date:      $Date: 2007/02/09 23:29:43 $
  Version:   $Revision: 1.5 $

  Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "igstkGroupObject.h"
#include "igstkEvents.h"

namespace igstk
{ 

/** Constructor */
GroupObject::GroupObject():m_StateMachine(this)
{
  // Create the Group Spatial Object
  m_GroupSpatialObject = GroupSpatialObjectType::New();
  this->RequestSetSpatialObject( m_GroupSpatialObject );
} 

/** Destructor */
GroupObject::~GroupObject()  
{
}

/** Return the number of objects in the group */
unsigned long GroupObject::GetNumberOfObjects() const
{
  return m_GroupSpatialObject->GetNumberOfChildren(9999);
}

/** Print object information */
void GroupObject::PrintSelf( std::ostream& os, itk::Indent indent ) const
{
  Superclass::PrintSelf(os, indent);

  if( this->m_GroupSpatialObject )
    {
    os << indent << this->m_GroupSpatialObject << std::endl;
    }
}


} // end namespace igstk
