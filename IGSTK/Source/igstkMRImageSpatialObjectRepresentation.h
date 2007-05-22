/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    $RCSfile: igstkMRImageSpatialObjectRepresentation.h,v $
  Language:  C++
  Date:      $Date: 2007/02/09 23:29:44 $
  Version:   $Revision: 1.6 $

  Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __igstkMRImageSpatialObjectRepresentation_h
#define __igstkMRImageSpatialObjectRepresentation_h

#include "igstkImageSpatialObjectRepresentation.h"
#include "igstkMRImageSpatialObject.h"

namespace igstk
{

/** \class MRImageSpatialObjectRepresentation
 *  \brief This class implements a spatial object represenation 
 *  for MRI modality.
 *
 * \ingroup Spatial object representation
 */

class MRImageSpatialObjectRepresentation : 
         public ImageSpatialObjectRepresentation< MRImageSpatialObject >
{

public:

  /** Macro with standard traits declarations. */
  igstkStandardClassTraitsMacro( MRImageSpatialObjectRepresentation, \
                    ImageSpatialObjectRepresentation< MRImageSpatialObject > )

protected:

  MRImageSpatialObjectRepresentation( void );
  virtual ~MRImageSpatialObjectRepresentation( void ) {};

  /** Print the object information in a stream. */
  void PrintSelf( std::ostream& os, itk::Indent indent ) const; 

private:

};

} // end namespace igstk

#endif // __igstkMRImageSpatialObjectRepresentation_h
