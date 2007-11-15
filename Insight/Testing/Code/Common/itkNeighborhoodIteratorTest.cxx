/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkNeighborhoodIteratorTest.cxx,v $
  Language:  C++
  Date:      $Date: 2007/08/20 16:55:26 $
  Version:   $Revision: 1.21 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif
#include "itkNeighborhoodIteratorTestCommon.txx"
#include "itkNeighborhoodIterator.h"

int itkNeighborhoodIteratorTest(int, char* [] )
{
  TestImageType::Pointer img = GetTestImage(10, 10, 5, 3);
  itk::NeighborhoodIterator<TestImageType>::IndexType loc;
  loc[0] = 4; loc[1] = 4; loc[2] = 2; loc[3] = 1;

  itk::NeighborhoodIterator<TestImageType>::IndexType zeroIDX;
  zeroIDX.Fill(0);
  
  itk::NeighborhoodIterator<TestImageType>::RadiusType radius;
  radius[0] = radius[1] = radius[2] = radius[3] = 1;
  
  println("Creating NeighborhoodIterator");
  itk::NeighborhoodIterator<TestImageType>
    it(radius, img, img->GetRequestedRegion());

  println("Moving iterator using Superclass::SetLocation()");
  it.SetLocation(loc);
  it.Print(std::cout);

  println("Testing SetCenterPixel()");
  it.SetCenterPixel(zeroIDX);

  println("Testing SetPixel()");
  it.SetPixel(6,zeroIDX);

  println("Using Superclass::GetNeighborhood()");
  itk::NeighborhoodIterator<TestImageType>::NeighborhoodType n
    = it.GetNeighborhood();
  
  println("Testing SetNeighborhood()");
  it.SetNeighborhood(n);
  it.GetNeighborhood().Print(std::cout);

  println("Testing GetCenterPointer()");
  std::cout << it.GetCenterPointer() << " = "
            << *(it.GetCenterPointer()) << std::endl;

  println("Testing operator=");
  it = itk::NeighborhoodIterator<TestImageType>(radius, img,
                                                img->GetRequestedRegion());

  println("Testing copy constructor");
  itk::NeighborhoodIterator<TestImageType> it2(it);
  it.Print(std::cout);
  it2.Print(std::cout);


  println("Creating 5x5x1x1 neighborhood");
  radius[0] = 2; radius[1] = 2; radius[2] = 0; radius[3] = 0;
  itk::NeighborhoodIterator<TestImageType> it3(radius, img, img->GetRequestedRegion());

  println("Moving to location 2, 2, 0, 0");
  loc[0] = 2; loc[1] = 2; loc[2] = 0; loc[3] = 0;
  it3.SetLocation(loc);

  it3.Print(std::cout);
  unsigned x, y, i;
  for (y = 0, i=0; y < 5; y++)
    {
    for (x = 0; x < 5; x++, i++)
      {
      std::cout << it3.GetPixel(i) << " ";
      }
    std::cout << std::endl;
    }

  println("Testing SetNext(0, 2, [0,0,0,0])");
  itk::NeighborhoodIterator<TestImageType>::PixelType z;
  z[0] = 0;  z[1] = 0; z[2] = 0; z[3] = 0;
  it3.SetNext(0,2,z);

  for (y = 0, i=0; y < 5; y++)
    {
    for (x = 0; x < 5; x++, i++)
      {
      std::cout << it3.GetPixel(i) << " ";
      }
    std::cout << std::endl;
    }
  
  println("Testing SetNext(1, 2, [0,0,0,0])");
  it3.SetNext(1,2,z);
  for (y = 0, i=0; y < 5; y++)
    {
    for (x = 0; x < 5; x++, i++)
      {
      std::cout << it3.GetPixel(i) << " ";
      }
    std::cout << std::endl;
    }
  
  println("Testing SetNext(0, [0,0,0,0])");
  it3.SetNext(0, z);
  for (y = 0, i=0; y < 5; y++)
    {
    for (x = 0; x < 5; x++, i++)
      {
      std::cout << it3.GetPixel(i) << " ";
      }
    std::cout << std::endl;
    }

  println("Testing SetNext(1, [0,0,0,0])");
  it3.SetNext(1, z);
  for (y = 0, i=0; y < 5; y++)
    {
    for (x = 0; x < 5; x++, i++)
      {
      std::cout << it3.GetPixel(i) << " ";
      }
    std::cout << std::endl;
    }


  println("Testing SetPrevious(0, 2, [0,0,0,0])");
  it3.SetPrevious(0,2,z);

  for (y = 0, i=0; y < 5; y++)
    {
    for (x = 0; x < 5; x++, i++)
      {
      std::cout << it3.GetPixel(i) << " ";
      }
    std::cout << std::endl;
    }
  
  println("Testing SetPrevious(1, 2, [0,0,0,0])");
  it3.SetPrevious(1,2,z);
  for (y = 0, i=0; y < 5; y++)
    {
    for (x = 0; x < 5; x++, i++)
      {
      std::cout << it3.GetPixel(i) << " ";
      }
    std::cout << std::endl;
    }
  
  println("Testing SetPrevious(0, [0,0,0,0])");
  it3.SetPrevious(0, z);
  for (y = 0, i=0; y < 5; y++)
    {
    for (x = 0; x < 5; x++, i++)
      {
      std::cout << it3.GetPixel(i) << " ";
      }
    std::cout << std::endl;
    }

  println("Testing SetPrevious(1, [0,0,0,0])");
  it3.SetPrevious(1, z);
  for (y = 0, i=0; y < 5; y++)
    {
    for (x = 0; x < 5; x++, i++)
      {
      std::cout << it3.GetPixel(i) << " ";
      }
    std::cout << std::endl;
    }


  println("Testing SetPixel methods");
  bool raised = false;
  try
    {
    it3.GoToBegin();
    it3.SetPixel( it.Size()/2, z); // in bounds
    it3.SetPixel( 0, z); // out of bounds, should throw an exception
    }
  catch(itk::ExceptionObject &e)
    {
    std::cout << e << std::endl;
    raised = true;
    }
  if( !raised )
    {
    return EXIT_FAILURE;
    }
      
  

  
  return EXIT_SUCCESS;
}
