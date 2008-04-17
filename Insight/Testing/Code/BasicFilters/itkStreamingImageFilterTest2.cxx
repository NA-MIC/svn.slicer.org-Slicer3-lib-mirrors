/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkStreamingImageFilterTest2.cxx,v $
  Language:  C++
  Date:      $Date: 2003-09-10 14:30:08 $
  Version:   $Revision: 1.7 $

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
#include "itkImageRegionIterator.h"
#include "itkShrinkImageFilter.h"
#include "itkStreamingImageFilter.h"
#include "itkImageRegionMultidimensionalSplitter.h"
#include "itkXMLFileOutputWindow.h"

int itkStreamingImageFilterTest2(int, char* [] )
{
  itk::XMLFileOutputWindow::Pointer logger = itk::XMLFileOutputWindow::New();
  logger->SetInstance(logger);
  
  // typedefs to simplify the syntax
  typedef itk::Image<short, 2>   ShortImage;

  // Test the creation of an image with native type
  ShortImage::Pointer if2 = ShortImage::New();

  // fill in an image
  ShortImage::IndexType  index = {{0, 0}};
  ShortImage::SizeType   size = {{42, 63}};
  ShortImage::RegionType region;
  region.SetSize( size );
  region.SetIndex( index );
  if2->SetLargestPossibleRegion( region );
  if2->SetBufferedRegion( region );
  if2->Allocate();

  itk::ImageRegionIterator<ShortImage> iterator(if2, region);

  short i=0;
  short scalar;
  for (; !iterator.IsAtEnd(); ++iterator, ++i)
    {
    scalar = i;
    iterator.Set( scalar );
    }
  
  // Create a filter
  itk::ShrinkImageFilter< ShortImage, ShortImage >::Pointer shrink;
  shrink = itk::ShrinkImageFilter< ShortImage, ShortImage >::New();
  shrink->SetInput( if2 );
  
  unsigned int factors[2] = { 2, 3 };
  shrink->SetShrinkFactors(factors);
  shrink->DebugOn();

  itk::ImageRegionMultidimensionalSplitter<2>::Pointer splitter;
  splitter = itk::ImageRegionMultidimensionalSplitter<2>::New();
  splitter->DebugOn();
  
  itk::StreamingImageFilter<ShortImage, ShortImage>::Pointer streamer;
  streamer = itk::StreamingImageFilter<ShortImage, ShortImage>::New();
  streamer->SetInput( shrink->GetOutput() );
  streamer->SetNumberOfStreamDivisions( 25 );
  streamer->SetRegionSplitter( splitter );
  streamer->Update();

  std::cout << "Input spacing: " << if2->GetSpacing()[0] << ", "
            << if2->GetSpacing()[1] << std::endl;
  std::cout << "Output spacing: " << streamer->GetOutput()->GetSpacing()[0]
            << ", "
            << streamer->GetOutput()->GetSpacing()[1] << std::endl;


  //
  // The rest of this code determines whether the shrink code produced
  // the image we expected.
  //
  ShortImage::RegionType requestedRegion;
  requestedRegion = streamer->GetOutput()->GetRequestedRegion();
  
  itk::ImageRegionIterator<ShortImage>
    iterator2(streamer->GetOutput(), requestedRegion);

  bool passed = true;
  for (; !iterator2.IsAtEnd(); ++iterator2)
    {
    short trueValue = (short) (shrink->GetShrinkFactors()[0] * iterator2.GetIndex()[0])
              + (region.GetSize()[0]
                * shrink->GetShrinkFactors()[1] * iterator2.GetIndex()[1]);

    if ( iterator2.Get() != trueValue )
      {
      passed = false;
      std::cout << "Pixel " << iterator2.GetIndex() << " is incorrect" << std::endl;
      }
    }

  if (passed)
    {
    std::cout << "ImageStreamingFilter test passed." << std::endl;
    try
      {
      return EXIT_SUCCESS;
      }
    catch (...)
      {
      std::cout << "Caught an exception on exit" << std::endl;
      return EXIT_FAILURE;
      }
    }
  else
    {
    std::cout << "ImageStreaming Filter test failed." << std::endl;
    return EXIT_FAILURE;
    }

}
