/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkNaryAddImageFilterTest.cxx,v $
  Language:  C++
  Date:      $Date: 2007/08/10 14:34:02 $
  Version:   $Revision: 1.13 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <itkImage.h>
#include <itkNaryAddImageFilter.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <iostream>



// Define the dimension of the images
const unsigned int myDimension = 3;

// Declare the types of the images
typedef itk::Image<float, myDimension>  myInputImageType;
typedef itk::Image<float, myDimension>  myOutputImageType;

// Declare the type of the index to access images
typedef itk::Index<myDimension>         myIndexType;

// Declare the type of the size 
typedef itk::Size<myDimension>          mySizeType;

// Declare the type of the Region
typedef itk::ImageRegion<myDimension>        myRegionType;

// Declare the type of the Region
typedef itk::ImageRegionIteratorWithIndex<myInputImageType>  myImageIteratorType;

// Declare the type for the ADD filter
typedef itk::NaryAddImageFilter<
                              myInputImageType,
                              myOutputImageType  >  myFilterType;
 



// Function for image initialization
void InitializeImage( myInputImageType * image, double value   )
{

  myInputImageType::Pointer inputImage( image );

  // Define their size, and start index
  mySizeType size;
  size[0] = 2;
  size[1] = 2;
  size[2] = 2;

  myIndexType start;
  start.Fill(0);

  myRegionType region;
  region.SetIndex( start );
  region.SetSize( size );
  
  inputImage->SetLargestPossibleRegion( region );
  inputImage->SetBufferedRegion( region );
  inputImage->SetRequestedRegion( region );
  inputImage->Allocate();

  myImageIteratorType it( inputImage, 
                     inputImage->GetRequestedRegion() );
  
  it.GoToBegin();
  while( !it.IsAtEnd() ) 
    {
    it.Set( value );
    ++it;
    }


}



// Function for image printing
void PrintImage( myInputImageType * image, const char *)
{
  // Create an iterator for going through the image
  myImageIteratorType it( image, 
                          image->GetRequestedRegion() );
  
  it.GoToBegin();
  //  Print the content of the image
  //std::cout << text << std::endl;
  while( !it.IsAtEnd() ) 
  {
    std::cout << it.Get() << std::endl;
    ++it;
  }

}






int itkNaryAddImageFilterTest(int, char* [] ) 
{

  // Create two images
  myInputImageType::Pointer inputImageA  = myInputImageType::New();
  myInputImageType::Pointer inputImageB  = myInputImageType::New();
  
  InitializeImage( inputImageA, 12 );
  InitializeImage( inputImageB, 17 );

  PrintImage( inputImageA, "Input image A" ); 
  PrintImage( inputImageB, "Input image B" ); 

  // Create an ADD Filter                                
  myFilterType::Pointer filter = myFilterType::New();


  // Connect the input images
  filter->SetInput( 0, inputImageA ); 
  filter->SetInput( 1, inputImageB );

  // Get the Smart Pointer to the Filter Output 
  myOutputImageType::Pointer outputImage = filter->GetOutput();

  
  // Execute the filter
  filter->Update();
  filter->SetFunctor(filter->GetFunctor());

  PrintImage( outputImage, "Resulting image" ); 

  // Testing with vector Images
  typedef itk::Image< itk::Vector<double,2>, 2> VectorImageType;
  typedef itk::NaryAddImageFilter< VectorImageType, VectorImageType > NaryAdderType;
  NaryAdderType::Pointer adder = NaryAdderType::New();

  // All objects should be automatically destroyed at this point
  return EXIT_SUCCESS;

}




