/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    $RCSfile: igstkView3D.h,v $
  Language:  C++
  Date:      $Date: 2007/02/09 23:29:44 $
  Version:   $Revision: 1.10 $

  Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __igstkView3D_h
#define __igstkView3D_h

#include "igstkView.h"

namespace igstk {

/** \class View3D
 *
 * \brief View3D provies the functionality of rendering a scene in a 3D
 * window. This class derives from the View class, and represents the
 * abstraction of a window in a GUI in which 3D objects will be displayed but
 * from a point of view in which the camera can change its orientation and
 * position.
 *
 *
 *  \image html  igstkView3D.png  "View3D State Machine Diagram"
 *  \image latex igstkView3D.eps  "View3D State Machine Diagram" 
 *
 */
class View3D : public View 
{
public:
  typedef View3D    Self;
  typedef View      Superclass;

  igstkTypeMacro( View3D, View );
   
  /** Constructor. The parameters of this constructor are related to the FLTK
   * box class. They include the screen coordinates of the upper left
   * coordinate, its width and height, and a string associated to the label */
  View3D( int x, int y, int w, int h, const char *l="");

  /** Destructor */
  ~View3D( void );

  /** Print the object information in a stream. */
  void PrintSelf( std::ostream& os, ::itk::Indent indent ) const; 

protected:

  /** This method implements the user interactions with the view. It is an
   * overload of a virtual medthod defined for FLTK classes. */
  int  handle( int event );

};

} // end namespace igstk

#endif 
