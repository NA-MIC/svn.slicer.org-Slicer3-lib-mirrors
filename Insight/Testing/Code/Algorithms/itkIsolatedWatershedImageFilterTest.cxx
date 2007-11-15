/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkIsolatedWatershedImageFilterTest.cxx,v $
  Language:  C++
  Date:      $Date: 2007/08/20 12:47:12 $
  Version:   $Revision: 1.2 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <fstream>
#include "itkIsolatedWatershedImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIterator.h"
#include "itkNumericTraits.h"

int itkIsolatedWatershedImageFilterTest(int ac, char* av[] )
{
  if(ac < 7)
    {
    std::cerr << "Usage: " << av[0] << " InputImage OutputImage seed1_x seed1_y seed2_x seed2_y\n";
    return EXIT_FAILURE;
    }

  typedef unsigned char PixelType;
  typedef itk::Image<PixelType, 2> myImage;
  itk::ImageFileReader<myImage>::Pointer input 
    = itk::ImageFileReader<myImage>::New();
  input->SetFileName(av[1]);
  
  // Create a filter
  typedef itk::IsolatedWatershedImageFilter<myImage,myImage> FilterType;

  FilterType::Pointer filter = FilterType::New();

  filter->SetInput(input->GetOutput());
  
  FilterType::IndexType seed1;
  
  seed1[0] = atoi(av[3]); seed1[1] = atoi(av[4]);
  filter->SetSeed1(seed1);
  
  seed1[0] = atoi(av[5]); seed1[1] = atoi(av[6]);
  filter->SetSeed2(seed1);
  
  filter->SetThreshold(0.001);
  filter->SetReplaceValue1(255);
  filter->SetReplaceValue2(127);
  filter->SetUpperValueLimit(1);
    
  // Test SetMacro
  filter->SetIsolatedValueTolerance(.0001);
  
  // Test GetMacros
  double threshold = filter->GetThreshold();
  std::cout << "filter->GetThreshold(): "
            << threshold
            << std::endl;
  double isolatedValueTolerance = filter->GetIsolatedValueTolerance();
  std::cout << "filter->GetIsolatedValueTolerance(): " 
            << isolatedValueTolerance
            << std::endl;
  double upperValueLimit = filter->GetUpperValueLimit();
  std::cout << "filter->GetUpperValueLimit(): "
            << upperValueLimit
            << std::endl;
  PixelType replaceValue1 = filter->GetReplaceValue1();
  std::cout << "filter->GetReplaceValue1(): "
            << static_cast<itk::NumericTraits<PixelType>::PrintType>(replaceValue1)
            << std::endl;
  PixelType replaceValue2 = filter->GetReplaceValue2();
  std::cout << "filter->GetReplaceValue2(): "
            << static_cast<itk::NumericTraits<PixelType>::PrintType>(replaceValue2)
            << std::endl;
  

  try
    {
    input->Update();
    filter->Update();
    double isolatedValue = filter->GetIsolatedValue();
    std::cout << "filter->GetIsolatedValue(): " 
              << isolatedValue
              << std::endl;
    }
  catch (itk::ExceptionObject& e)
    {
    std::cerr << "Exception detected: "  << e.GetDescription();
    return EXIT_FAILURE;
    }

  // Generate test image
  itk::ImageFileWriter<myImage>::Pointer writer;
    writer = itk::ImageFileWriter<myImage>::New();
    writer->SetInput( filter->GetOutput() );
    writer->SetFileName( av[2] );
    writer->Update();

  return EXIT_SUCCESS;
}
