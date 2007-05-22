/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkCenteredVersorTransformInitializerTest.cxx,v $
  Language:  C++
  Date:      $Date: 2004/04/26 15:50:27 $
  Version:   $Revision: 1.4 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include "itkVersorRigid3DTransform.h"
#include "itkCenteredVersorTransformInitializer.h"
#include "itkImage.h"

#include "itkImageRegionIterator.h"


/** 
 *  This program tests the use of the CenteredVersorTransformInitializer class
 * 
 *  
 */ 

int itkCenteredVersorTransformInitializerTest(int , char* [] )
{

  bool pass = true;

  const unsigned int Dimension = 3;

  // Fixed Image Type
  typedef itk::Image<unsigned char, Dimension>      FixedImageType;

  // Moving Image Type
  typedef itk::Image<unsigned char, Dimension>       MovingImageType;

  // Size Type
  typedef FixedImageType::SizeType                 SizeType;
  typedef FixedImageType::SpacingType              SpacingType;
  typedef FixedImageType::PointType                PointType;
  typedef FixedImageType::IndexType                IndexType;
  typedef FixedImageType::RegionType               RegionType;


  // Transform Type
  typedef itk::VersorRigid3DTransform< double >     TransformType;
  typedef TransformType::ParametersType             ParametersType;

  SizeType size;
  size[0] = 100;
  size[1] = 100;
  size[2] = 150;
  
  PointType fixedOrigin;
  fixedOrigin[0] = 0.0;
  fixedOrigin[1] = 0.0;
  fixedOrigin[2] = 0.0;

  PointType movingOrigin;
  movingOrigin[0] = 29.0;
  movingOrigin[1] = 17.0;
  movingOrigin[2] = 13.0;

  SpacingType spacing;
  spacing[0] = 1.5;
  spacing[1] = 1.5;
  spacing[2] = 1.0;

  IndexType index;
  index[0] = 0;
  index[1] = 0;
  index[2] = 0;

  RegionType region;
  region.SetSize( size );
  region.SetIndex( index );


  FixedImageType::Pointer     fixedImage    = FixedImageType::New();
  MovingImageType::Pointer    movingImage   = MovingImageType::New();

  fixedImage->SetRegions( region );
  fixedImage->SetSpacing( spacing );
  fixedImage->SetOrigin(  fixedOrigin );
  fixedImage->Allocate();
  fixedImage->FillBuffer( 0 );

  movingImage->SetRegions( region );
  movingImage->SetSpacing( spacing );
  movingImage->SetOrigin(  movingOrigin );
  movingImage->Allocate();
  movingImage->FillBuffer( 0 );
  
  RegionType internalRegion;
  SizeType  internalSize;
  IndexType internalIndex;

  internalIndex[0] = index[0] + 20;
  internalIndex[1] = index[1] + 30;
  internalIndex[2] = index[2] + 10;
  
  internalSize[0]  = size[0] - 2 * 20;
  internalSize[1]  = size[1] - 2 * 30;
  internalSize[2]  = size[2] - 2 * 10;


  internalRegion.SetSize(  internalSize  );
  internalRegion.SetIndex( internalIndex );

  typedef itk::ImageRegionIterator< FixedImageType > FixedIterator;
  FixedIterator fi( fixedImage, internalRegion );

  fi.GoToBegin();
  while( !fi.IsAtEnd() )
    {
    fi.Set( 200 );
    ++fi;
    }
   

  internalIndex[0] = index[0] + 10;
  internalIndex[1] = index[1] + 20;
  internalIndex[2] = index[2] + 30;
  
  internalSize[0]  = size[0] - 2 * 10;
  internalSize[1]  = size[1] - 2 * 20;
  internalSize[2]  = size[2] - 2 * 30;


  internalRegion.SetSize(  internalSize  );
  internalRegion.SetIndex( internalIndex );


  typedef itk::ImageRegionIterator< MovingImageType > MovingIterator;
  MovingIterator mi( movingImage, internalRegion );

  mi.GoToBegin();
  while( !mi.IsAtEnd() )
    {
    mi.Set( 200 );
    ++mi;
    }
   


  
  TransformType::Pointer transform = TransformType::New();
  transform->SetIdentity();


  typedef itk::CenteredVersorTransformInitializer< 
                                  FixedImageType, 
                                  MovingImageType >
                                            InitializerType;

  InitializerType::Pointer initializer = InitializerType::New();

  initializer->SetFixedImage( fixedImage );
  initializer->SetMovingImage( movingImage );
  initializer->SetTransform( transform );
                                    
  initializer->InitializeTransform();

  TransformType::InputPointType   center2      = transform->GetCenter();
  TransformType::OutputVectorType translation2 = transform->GetTranslation();
  TransformType::OffsetType       offset2      = transform->GetOffset();

  { // Verfications 
  TransformType::InputPointType   fixedCenter;
  TransformType::InputPointType   movingCenter;

  for(unsigned int j=0; j < Dimension; j++ )
    {
    fixedCenter[j]  = fixedOrigin[j]  + size[j] * spacing[j] / 2.0 ;
    movingCenter[j] = movingOrigin[j] + size[j] * spacing[j] / 2.0 ;
    }
  
  TransformType::InputVectorType relativeCenter = movingCenter - fixedCenter;


  const double tolerance = 1e-3;

  for(unsigned int k=0; k < Dimension; k++ )
    {
    if( fabs( translation2[k] - relativeCenter[k] ) > tolerance )
      {
      std::cerr << "Translation differs from expected value" << std::endl;
      std::cerr << "It should be " << relativeCenter << std::endl;
      std::cerr << "but it is    " << translation2 << std::endl;
      pass = false;
      break;
      }
    if( fabs( offset2[k] - relativeCenter[k] ) > tolerance )
      {
      std::cerr << "Offset differs from expected value" << std::endl;
      std::cerr << "It should be " << relativeCenter << std::endl;
      std::cerr << "but it is    " << offset2 << std::endl;
      pass = false;
      break;
      }
    }
  }


  
  if( !pass )
    {
    std::cout << "Test FAILED." << std::endl;
    return EXIT_FAILURE;
    }

  std::cout << "Test PASSED." << std::endl;
  return EXIT_SUCCESS;


}

