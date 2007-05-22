/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    $RCSfile: igstkMouseTracker.h,v $
  Language:  C++
  Date:      $Date: 2007/02/09 23:29:44 $
  Version:   $Revision: 1.6 $

  Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __igstkMouseTracker_h
#define __igstkMouseTracker_h

#include "igstkTracker.h"

namespace igstk
{

/** \class MouseTracker
 *   \brief Implementation of the Mouse TrackerTool class.
 *
 *   The purpose of this class is to provide a minimal 
 *   implementation of a Tracker. This class is intended 
 *   only for minimal demos and for debugging applications.
 *   It is not intended to be used in a real application to
 *   be deployed in a surgery room. 
 *
 *   \ingroup Trackers
 */
class MouseTracker : public Tracker
{
public:

  /** Macro with standard traits declarations. */
  igstkStandardClassTraitsMacro( MouseTracker, Tracker )

  typedef Superclass::TransformType           TransformType;

  /** Get the transform */ 
  void GetTransform(TransformType & transform);

  /** Scaling factor */
  igstkSetMacro( ScaleFactor, double );
  igstkGetMacro( ScaleFactor, double );

protected:

  MouseTracker();

  virtual ~MouseTracker();

  typedef Tracker::ResultType                 ResultType;

  virtual ResultType InternalOpen( void );

  virtual ResultType InternalActivateTools( void );

  virtual ResultType InternalStartTracking( void );

  virtual ResultType InternalUpdateStatus( void );

  virtual ResultType InternalReset( void );

  virtual ResultType InternalStopTracking( void );

  virtual ResultType InternalDeactivateTools( void );

  virtual ResultType InternalClose( void );

  /** Print object information */
  virtual void PrintSelf( std::ostream& os, itk::Indent indent ) const; 

private:

  MouseTracker(const Self&);  //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  typedef TrackerTool                 TrackerToolType;
  typedef TrackerPort                 TrackerPortType;

  typedef Transform::TimePeriodType   TimePeriodType;

  TimePeriodType                      m_ValidityTime;

  TrackerToolType::Pointer            m_Tool;

  TrackerPortType::Pointer            m_Port;

  double                              m_ScaleFactor;
};

}

#endif //__igstk_MouseTracker_h_
