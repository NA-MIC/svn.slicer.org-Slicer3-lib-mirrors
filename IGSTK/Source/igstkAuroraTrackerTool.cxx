/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    $RCSfile: igstkAuroraTrackerTool.cxx,v $
  Language:  C++
  Date:      $Date: 2007/02/09 23:29:43 $
  Version:   $Revision: 1.5 $

  Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#if defined(_MSC_VER)
//  Warning about: identifier was truncated to '255' characters 
//  in the debug information (MVC6.0 Debug)
#pragma warning( disable : 4786 )
#endif

#include "igstkAuroraTrackerTool.h"

namespace igstk
{

/** Constructor (initializes Aurora-specific tool values) */
AuroraTrackerTool::AuroraTrackerTool():m_StateMachine(this)
{
}

/** Destructor */
AuroraTrackerTool::~AuroraTrackerTool()
{
}

/** Print Self function */
void AuroraTrackerTool::PrintSelf( std::ostream& os, itk::Indent indent ) const
{
  Superclass::PrintSelf(os, indent);
}


}
