/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkHMaximaMinimaImageFilterTest.cxx,v $
  Language:  C++
  Date:      $Date: 2007/08/10 14:34:01 $
  Version:   $Revision: 1.6 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

//  

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkFilterWatcher.h"
#include "itkHMaximaImageFilter.h"
#include "itkHMinimaImageFilter.h"

int itkHMaximaMinimaImageFilterTest( int argc, char * argv[] )
{
  if( argc < 4 )
    {
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << "  inputImageFile  ";
    std::cerr << " outputImageFile  height" << std::endl;
    return EXIT_FAILURE;
    }


  //
  //  The following code defines the input and output pixel types and their
  //  associated image types.
  //
  const unsigned int Dimension = 2;
  
  typedef unsigned char   InputPixelType;
  typedef unsigned char   OutputPixelType;
  typedef unsigned char    WritePixelType;

  typedef itk::Image< InputPixelType,  Dimension >   InputImageType;
  typedef itk::Image< OutputPixelType, Dimension >   OutputImageType;
  typedef itk::Image< WritePixelType, Dimension >    WriteImageType;


  // readers/writers
  typedef itk::ImageFileReader< InputImageType  >  ReaderType;
  typedef itk::ImageFileWriter< WriteImageType >  WriterType;
  typedef itk::RescaleIntensityImageFilter<OutputImageType, WriteImageType>
    RescaleType;

  // define the hmaxima filter
  typedef itk::HMaximaImageFilter<
                            InputImageType, 
                            OutputImageType >  HmaximaFilterType;
  // define the hminima filter
  typedef itk::HMinimaImageFilter<
                            InputImageType, 
                            OutputImageType >  HminimaFilterType;


  // Creation of Reader and Writer filters
  ReaderType::Pointer reader = ReaderType::New();
  WriterType::Pointer writer  = WriterType::New();
  RescaleType::Pointer rescaler = RescaleType::New();
  
  // Create the filters
  HmaximaFilterType::Pointer  hmaxima = HmaximaFilterType::New();
  FilterWatcher watchHmaxima(hmaxima,"hmaxima");
  HminimaFilterType::Pointer  hminima = HminimaFilterType::New();

  // Setup the input and output files
  reader->SetFileName( argv[1] );
  writer->SetFileName(  argv[2] );
  
  // Setup the hmaxima method
  hmaxima->SetInput(  reader->GetOutput() );
  hmaxima->SetHeight( static_cast<InputPixelType>(atof(argv[3])) );

  // Setup the hminima method
  hminima->SetInput(  hmaxima->GetOutput() );
  hminima->SetHeight( static_cast<InputPixelType>(atof(argv[3])) );

  // Run the filter
//   rescaler->SetInput( hmaxima->GetOutput() );
//   rescaler->SetOutputMinimum(   0 );
//   rescaler->SetOutputMaximum( 255 );
  writer->SetInput( hminima->GetOutput() );
  writer->Update();

  // Output the number of iterations used
  std::cout << "Hmaxima took " << hmaxima->GetNumberOfIterationsUsed() << " iterations." << std::endl;
  std::cout << "Hminima took " << hminima->GetNumberOfIterationsUsed() << " iterations." << std::endl;

  std::cout << "<DartMeasurement name=\"HMaximaNumberOfIterations\" type=\"numeric/integer\">" << hmaxima->GetNumberOfIterationsUsed() << "</DartMeasurement>" << std::endl;
  std::cout << "<DartMeasurement name=\"HMinimaNumberOfIterations\" type=\"numeric/integer\">" << hminima->GetNumberOfIterationsUsed() << "</DartMeasurement>" << std::endl;


  return EXIT_SUCCESS;

}

