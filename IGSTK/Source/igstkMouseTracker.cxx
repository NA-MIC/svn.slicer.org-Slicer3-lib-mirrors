/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    $RCSfile: igstkMouseTracker.cxx,v $
  Language:  C++
  Date:      $Date: 2007/02/09 23:29:44 $
  Version:   $Revision: 1.5 $

  Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#if defined(_MSC_VER)
// Warning about: identifier was truncated to '255' characters in 
// the debug information (MVC6.0 Debug)
#pragma warning( disable : 4786 )
#endif

#include <FL/Fl.H>
#include "igstkMouseTracker.h"

namespace igstk
{

MouseTracker::MouseTracker():m_StateMachine(this)
{
  m_ScaleFactor = 1.0;
}

MouseTracker::~MouseTracker()
{
}

MouseTracker::ResultType MouseTracker::InternalOpen( void )
{
  return SUCCESS;
}

MouseTracker::ResultType MouseTracker::InternalStartTracking( void )
{
  return SUCCESS;
}

MouseTracker::ResultType MouseTracker::InternalReset( void )
{
  return SUCCESS;
}

MouseTracker::ResultType MouseTracker::InternalStopTracking( void )
{
  return SUCCESS;
}

MouseTracker::ResultType MouseTracker::InternalDeactivateTools( void )
{
  return SUCCESS;
}

MouseTracker::ResultType MouseTracker::InternalClose( void )
{
  return SUCCESS;
}

MouseTracker::ResultType MouseTracker::InternalActivateTools( void )
{
  igstkLogMacro( DEBUG, "MouseTracker::InternalActivateTools called ...\n");
  m_ValidityTime = 100.0; // 100.0 milliseconds
  m_Port = TrackerPortType::New();
  m_Tool = TrackerToolType::New();
  m_Port->AddTool( m_Tool );
  this->AddPort( m_Port );
  return SUCCESS;
}
    
MouseTracker::ResultType MouseTracker::InternalUpdateStatus( void )
{
  igstkLogMacro( DEBUG, "MouseTracker::InternalUpdateStatus called ...\n");

  typedef igstk::Transform   TransformType;
  TransformType transform;
  transform.SetToIdentity( m_ValidityTime );
  
  typedef TransformType::VectorType PositionType;
  PositionType  position;
  position[0] = Fl::event_x() / m_ScaleFactor;
  position[1] = Fl::event_y() / m_ScaleFactor;
  position[2] = 0;

  typedef TransformType::ErrorType  ErrorType;
  ErrorType errorValue = 0.5; // +/- half Pixel Uncertainty

  transform.SetTranslation( position, errorValue, m_ValidityTime );
  this->SetToolTransform( 0, 0, transform );

  return SUCCESS;
}
 
void MouseTracker::GetTransform(TransformType & transform)
{
  this->GetToolTransform(0, 0, transform);
}

/** Print Self function */
void MouseTracker::PrintSelf( std::ostream& os, itk::Indent indent ) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Validity Time: " << m_ValidityTime << std::endl;
  
  if( this->m_Tool )
    {
    os << indent << *m_Tool << std::endl;
    }
  
  if( this->m_Port )
    {
    os << indent << *m_Port << std::endl;
    }

  os << indent << "Scale Factor: " << m_ScaleFactor << std::endl;
}

}
