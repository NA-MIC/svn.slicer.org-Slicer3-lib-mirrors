/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkImageRandomIteratorTest.cxx,v $
  Language:  C++
  Date:      $Date: 2005/10/06 19:35:15 $
  Version:   $Revision: 1.9 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <iostream>

#include "itkImage.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkImageRandomIteratorWithIndex.h"
#include "itkImageRandomConstIteratorWithIndex.h"




int itkImageRandomIteratorTest(int, char* [] )
{
  std::cout << "Creating an image of indices" << std::endl;

  const unsigned int ImageDimension = 3;

  typedef itk::Index< ImageDimension >             PixelType;

  typedef itk::Image< PixelType, ImageDimension >  ImageType;

  ImageType::Pointer myImage = ImageType::New();
  ImageType::ConstPointer myConstImage = myImage.GetPointer();
  
  ImageType::SizeType size;

  size[0] = 100;
  size[1] = 100;
  size[2] = 100;

  unsigned long numberOfSamples = 10;

  ImageType::IndexType start;
  start.Fill(0);

  ImageType::RegionType region;
  region.SetIndex( start );
  region.SetSize( size );

  myImage->SetLargestPossibleRegion( region );
  myImage->SetBufferedRegion( region );
  myImage->SetRequestedRegion( region );
  myImage->Allocate();

  typedef itk::ImageRegionIteratorWithIndex< ImageType >            IteratorType;

  typedef itk::ImageRandomIteratorWithIndex< ImageType >      RandomIteratorType;

  typedef itk::ImageRandomConstIteratorWithIndex< ImageType > RandomConstIteratorType;

  IteratorType it( myImage, region );

  it.GoToBegin();
  ImageType::IndexType index;
  
  // Fill an image with indices
  while( !it.IsAtEnd() )
  {
    index = it.GetIndex();
    it.Set( index );
    ++it;
  }

  
  // Sample the image 
  RandomIteratorType ot( myImage, region );
  ot.SetNumberOfSamples( numberOfSamples ); 
  ot.GoToBegin();

 
  std::cout << "Verifying non-const iterator... ";
  std::cout << "Random walk of the Iterator over the image " << std::endl;
  while( !ot.IsAtEnd() )
    {
    index = ot.GetIndex();
    if( ot.Get() != index )
      {
        std::cerr << "Values don't correspond to what was stored "
          << std::endl;
        std::cerr << "Test failed at index ";
        std::cerr << index << std::endl;
        return EXIT_FAILURE;
      }
    std::cout << index << std::endl;
    ++ot;
    }
  std::cout << "   Done ! " << std::endl;

  
  // Verification 
  RandomConstIteratorType cot( myConstImage, region );
  cot.SetNumberOfSamples( numberOfSamples );
  cot.GoToBegin();

 
  std::cout << "Verifying const iterator... ";
  std::cout << "Random walk of the Iterator over the image " << std::endl;

  while( !cot.IsAtEnd() )
  {
    index = cot.GetIndex();
    if( cot.Get() != index )
      {
      std::cerr << "Values don't correspond to what was stored "
        << std::endl;
      std::cerr << "Test failed at index ";
      std::cerr << index << " value is " << cot.Get() <<  std::endl;
      return EXIT_FAILURE;
      }
    std::cout << index << std::endl;
    ++cot;
  }
  std::cout << "   Done ! " << std::endl;



  // Verification 
  std::cout << "Verifying iterator in reverse direction... " << std::endl;
  std::cout << "Should be a random walk too (a different one)" << std::endl;

  RandomIteratorType ior( myImage, region );
  ior.SetNumberOfSamples( numberOfSamples );
  ior.GoToEnd();

  --ior;
 

  while( !ior.IsAtBegin() )
  {
    index = ior.GetIndex();
    if( ior.Get() != index )
    {
      std::cerr << "Values don't correspond to what was stored "
        << std::endl;
      std::cerr << "Test failed at index ";
      std::cerr << index << " value is " << ior.Get() <<  std::endl;
      return EXIT_FAILURE;
    }
    std::cout << index << std::endl;
    --ior;
  }
  std::cout << index << std::endl; // print the value at the beginning index
  std::cout << "   Done ! " << std::endl;



  // Verification 
  std::cout << "Verifying const iterator in reverse direction... ";

  RandomConstIteratorType cor( myImage, region );
  cor.SetNumberOfSamples( numberOfSamples ); // 0=x, 1=y, 2=z
  cor.GoToEnd();

  --cor; // start at the end position 

  while( !cor.IsAtBegin() )
    {
    index = cor.GetIndex();
    if( cor.Get() != index )
      {
      std::cerr << "Values don't correspond to what was stored "
        << std::endl;
      std::cerr << "Test failed at index ";
      std::cerr << index << " value is " << cor.Get() <<  std::endl;
      return EXIT_FAILURE;
      }
    std::cout << index << std::endl;
    --cor;
    }
  std::cout << index << std::endl; // print the value at the beginning index
  std::cout << "   Done ! " << std::endl;

 // Verification 
  std::cout << "Verifying const iterator in both directions... ";

  RandomConstIteratorType dor( myImage, region );
  dor.SetNumberOfSamples( numberOfSamples ); // 0=x, 1=y, 2=z
  dor.GoToEnd();

  --dor; // start at the last valid pixel position 

  for (unsigned int counter = 0; ! dor.IsAtEnd(); ++counter)
    {
      index = dor.GetIndex();
      if( dor.Get() != index )
        {
          std::cerr << "Values don't correspond to what was stored "
                    << std::endl;
          std::cerr << "Test failed at index ";
          std::cerr << index << " value is " << dor.Get() <<  std::endl;
          return EXIT_FAILURE;
        }
      std::cout << index << std::endl;
      if (counter < 6)  { --dor; }
      else { ++dor; }
    }
  std::cout << index << std::endl; // print the value at the beginning index
  std::cout << "   Done ! " << std::endl;
  

  // Verification of the Iterator in a subregion of the image
  {
    std::cout << "Verifying Iterator in a Region smaller than the whole image... "
              << std::endl;

    ImageType::IndexType start;
    start[0] = 10;
    start[1] = 12;
    start[2] = 14;
    
    ImageType::SizeType size;
    size[0] = 11;
    size[1] = 12;
    size[2] = 13;

    ImageType::RegionType region;
    region.SetIndex( start );
    region.SetSize( size );

    RandomIteratorType cbot( myImage, region );

    cbot.SetNumberOfSamples( numberOfSamples ); // 0=x, 1=y, 2=z
    cbot.GoToBegin();

    while( !cbot.IsAtEnd() )
      {
      ImageType::IndexType index =  cbot.GetIndex();
      ImageType::PixelType pixel =  cbot.Get();

      if( index != pixel )
        {
        std::cerr << "Iterator in region test failed" << std::endl;
        std::cerr << pixel << " should be" << index << std::endl;
        return EXIT_FAILURE;
        }

      if( !region.IsInside( index ) )
        {
        std::cerr << "Iterator in region test failed" << std::endl;
        std::cerr << index << " is outside the region " << region << std::endl;
        return EXIT_FAILURE;
        }
      std::cout << index << std::endl;
      ++cbot;
      }

    std::cout << "   Done ! " << std::endl;
  }



  // Verification of the Const Iterator in a subregion of the image
  {
    std::cout << "Verifying Const Iterator in a Region smaller than the whole image... "
              << std::endl;

    ImageType::IndexType start;
    start[0] = 10;
    start[1] = 12;
    start[2] = 14;
    
    ImageType::SizeType size;
    size[0] = 11;
    size[1] = 12;
    size[2] = 13;

    ImageType::RegionType region;
    region.SetIndex( start );
    region.SetSize( size );

    RandomConstIteratorType cbot( myImage, region );

    cbot.SetNumberOfSamples( numberOfSamples );
    cbot.GoToBegin();

    while( !cbot.IsAtEnd() )
      {
      ImageType::IndexType index =  cbot.GetIndex();
      ImageType::PixelType pixel =  cbot.Get();

      if( index != pixel )
        {
        std::cerr << "Iterator in region test failed" << std::endl;
        std::cerr << pixel << " should be" << index << std::endl;
        return EXIT_FAILURE;
        }
      if( !region.IsInside( index ) )
        {
        std::cerr << "Iterator in region test failed" << std::endl;
        std::cerr << index << " is outside the region " << region << std::endl;
        return EXIT_FAILURE;
        }
      std::cout << index << std::endl;

      ++cbot;
      }

    std::cout << "   Done ! " << std::endl;
  }


  std::cout << "Test passed" << std::endl;




    return EXIT_SUCCESS;

  }



