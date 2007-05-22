/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    $RCSfile: igstkImageSpatialObjectTest.cxx,v $
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
#pragma warning ( disable : 4786 )
#endif

#include "igstkImageSpatialObject.h"


int igstkImageSpatialObjectTest( int , char* [] )
{

  igstk::RealTimeClock::Initialize();

 
  typedef signed short    PixelType;
  const unsigned int      Dimension = 3;

  typedef igstk::ImageSpatialObject< PixelType, Dimension > 
                                                       ImageSpatialObjectType;

  ImageSpatialObjectType::Pointer 
                           imageSpatialObject = ImageSpatialObjectType::New();

  imageSpatialObject->Print( std::cout );

  return EXIT_SUCCESS;
}
