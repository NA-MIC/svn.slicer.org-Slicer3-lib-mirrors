/*=========================================================================

Program:   Insight Segmentation & Registration Toolkit
Module:    $RCSfile: itkAnalyzeImageIOTest.cxx,v $
Language:  C++
Date:      $Date: 2005/07/19 21:30:57 $
Version:   $Revision: 1.26 $

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
#include "itkImageFileReader.h"
#include "itkImage.h"

#include <itksys/SystemTools.hxx>
#include "itkImageRegionIterator.h"
#include <iostream>
#include <fstream>

#include "itkImageFileWriter.h"
#include "itkImageIOFactory.h"
#include "itkAnalyzeImageIOFactory.h"
#include "itkAnalyzeImageIO.h"
#include <stdio.h>
#include "itkMetaDataObject.h"
#include "itkIOCommon.h"

#if defined(_WIN32) && (defined(_MSC_VER) || defined(__BORLANDC__))
#include <stdlib.h>
#define _unlink unlink
#else
#include <unistd.h>
#endif
static inline int Remove(const char *fname)
{
  return unlink(fname);
}

const unsigned char RPI=16;        /*Bit pattern 0 0 0  10000*/
const unsigned char LEFT=128;      /*Bit pattern 1 0 0  00000*/
const unsigned char ANTERIOR=64;   /*Bit pattern 0 1 0  00000*/
const unsigned char SUPERIOR=32;   /*Bit pattern 0 0 1  00000*/

int WriteTestFiles(void)
{
#include "LittleEndian_hdr.h"
#include "LittleEndian_img.h"
#include "BigEndian_hdr.h"
#include "BigEndian_img.h"
  std::ofstream little_hdr("LittleEndian.hdr", std::ios::binary | std::ios::out);
  if(!little_hdr.is_open())
    return EXIT_FAILURE;
  std::cout << "LittleEndian written" << std::endl;
  little_hdr.write(reinterpret_cast<const char *>(LittleEndian_hdr),sizeof(LittleEndian_hdr));
  little_hdr.close();
  std::ofstream little_img("LittleEndian.img", std::ios::binary | std::ios::out);
  if(!little_img.is_open())
    return EXIT_FAILURE;
  little_img.write(reinterpret_cast<const char *>(LittleEndian_img),sizeof(LittleEndian_img));
  little_img.close();
  std::ofstream big_hdr("BigEndian.hdr", std::ios::binary | std::ios::out);
  if(!big_hdr.is_open())
    return EXIT_FAILURE;
  big_hdr.write(reinterpret_cast<const char *>(BigEndian_hdr),sizeof(BigEndian_hdr));
  big_hdr.close();
  std::ofstream big_img("BigEndian.img", std::ios::binary | std::ios::out);
  if(!big_img.is_open())
    return EXIT_FAILURE;
  big_img.write(reinterpret_cast<const char *>(BigEndian_img),sizeof(BigEndian_img));
  big_img.close();
  return EXIT_SUCCESS;
}
void RemoveByteSwapTestFiles(void)
{
  Remove("LittleEndian.hdr");
  Remove("LittleEndian.img");
  Remove("BigEndian.hdr");
  Remove("BigEndian.img");
}

int TestByteSwap(void)
{
  int rval;
  typedef itk::Image<double, 3> ImageType ;
  typedef itk::ImageFileReader< ImageType > ImageReaderType ;
  if(WriteTestFiles() == -1)
    {
      return EXIT_FAILURE;
    }

  ImageType::Pointer little;
  ImageType::Pointer big;

  itk::ImageFileReader<ImageType>::Pointer imageReader =
    itk::ImageFileReader<ImageType>::New();
  try
  {
    imageReader->SetFileName("LittleEndian.hdr") ;
    imageReader->Update() ;
    little = imageReader->GetOutput() ;
    imageReader->SetFileName("BigEndian.hdr") ;
    imageReader->Update() ;
    big = imageReader->GetOutput();
    std::cout << "Printing Dictionary" << std::endl;
    big->GetMetaDataDictionary().Print(std::cout);
  }
  catch (itk::ExceptionObject e)
    {
      e.Print(std::cerr) ;
      RemoveByteSwapTestFiles();
      return EXIT_FAILURE;
    }
  rval = 0;
  try
    {
      itk::ImageRegionConstIterator<ImageType> littleIter(little,
                                                          little->GetLargestPossibleRegion());
      itk::ImageRegionConstIterator<ImageType> bigIter(big,
                                                       big->GetLargestPossibleRegion());
      while(!littleIter.IsAtEnd())
        {
          if(littleIter.Get() != bigIter.Get())
            break;
          ++littleIter;
          ++bigIter;
        }
      if(!littleIter.IsAtEnd() || !bigIter.IsAtEnd())
        rval = -1;
    }
  catch ( itk::ExceptionObject & ex )
    {
      std::cerr << "Error filling array" << ex.GetDescription() << std::endl;
      rval= -1;
    }

  RemoveByteSwapTestFiles();
  return rval;
}

template <typename T> int MakeImage(void)
{
  typedef itk::Image<T, 3> ImageType ;
  typedef itk::ImageFileReader< ImageType > ImageReaderType ;
  const char *filename = "test.hdr";
  //Allocate Images
  enum { ImageDimension = ImageType::ImageDimension };
  typename ImageType::Pointer img;
  const typename ImageType::SizeType size = {{10,10,10}};
  const typename ImageType::IndexType index = {{0,0,0}};
  typename ImageType::RegionType region;
  region.SetSize( size );
  region.SetIndex( index );

  img = ImageType::New();
  img->SetLargestPossibleRegion( region );
  img->SetBufferedRegion( region );
  img->SetRequestedRegion( region );
  img->Allocate();

  { //Fill in entire image
    itk::ImageRegionIterator<ImageType> ri(img,region);
    try
      {
        while(!ri.IsAtEnd())
          {
            ri.Set( RPI );
            ++ri;
          }
      }
    catch ( itk::ExceptionObject & ex )
      {
        std::cerr << "Error filling array" << ex.GetDescription() << std::endl;
        return EXIT_FAILURE;
      }
  }
  { //Fill in left half
    const typename ImageType::IndexType RPIindex = {{0,0,0}};
    const typename ImageType::SizeType RPIsize = {{5,10,10}};
    typename ImageType::RegionType RPIregion;
    RPIregion.SetSize( RPIsize );
    RPIregion.SetIndex( RPIindex );
    itk::ImageRegionIterator<ImageType > RPIiterator(img,RPIregion);
    while(!RPIiterator.IsAtEnd())
      {
        RPIiterator.Set( RPIiterator.Get() + LEFT );
        ++RPIiterator;
      }
  }
  { //Fill in anterior half
    const typename ImageType::IndexType RPIindex = {{0,5,0}};
    const typename ImageType::SizeType RPIsize = {{10,5,10}};
    typename ImageType::RegionType RPIregion;
    RPIregion.SetSize( RPIsize );
    RPIregion.SetIndex( RPIindex );
    itk::ImageRegionIterator<ImageType > RPIiterator(img,RPIregion);
    while(!RPIiterator.IsAtEnd())
      {
        RPIiterator.Set( RPIiterator.Get() + ANTERIOR );
        ++RPIiterator;
      }
  }
  { //Fill in superior half
    const typename ImageType::IndexType RPIindex = {{0,0,5}};
    const typename ImageType::SizeType RPIsize = {{10,10,5}};
    typename ImageType::RegionType RPIregion;
    RPIregion.SetSize( RPIsize );
    RPIregion.SetIndex( RPIindex );
    itk::ImageRegionIterator<ImageType > RPIiterator(img,RPIregion);
    while(!RPIiterator.IsAtEnd())
      {
        RPIiterator.Set( RPIiterator.Get() + SUPERIOR );
        ++RPIiterator;
      }
  }
  typedef itk::ImageFileWriter< ImageType >      ImageWriterType;
  typename ImageWriterType::Pointer ImageWriterPointer =
    ImageWriterType::New();

  //Set the output filename
  ImageWriterPointer->SetFileName(filename);

  //Attach input image to the writer.
  ImageWriterPointer->SetInput( img );
  //Determine file type and instantiate appropriate ImageIO class if not
  //explicitly stated with SetImageIO, then write to disk.
  try {
    ImageWriterPointer->Write();
  }
  catch ( itk::ExceptionObject & ex )
    {
      std::string message;
      message = "Problem found while writing image ";
      message += filename;
      message += "\n";
      message += ex.GetLocation();
      message += "\n";
      message += ex.GetDescription();
      std::cerr << message << std::endl;
      Remove(filename);
      return EXIT_FAILURE;
    }

  //typedef itk::ImageFileReader< ImageType > ImageReaderType ;
  typename ImageType::Pointer input;
  typename itk::ImageFileReader<ImageType>::Pointer imageReader =
    itk::ImageFileReader<ImageType>::New();
  try
    {
      imageReader->SetFileName(filename) ;
      imageReader->Update() ;
      input = imageReader->GetOutput() ;
    }
  catch (itk::ExceptionObject e)
    {
      e.Print(std::cerr) ;
      Remove(filename);
      return EXIT_FAILURE;
    }
  Remove(filename);
  return EXIT_SUCCESS;
}

//template int MakeImage<char>();

int itkAnalyzeImageIOTest(int ac, char* av[])
{
  int rval = 0;
  //
  // first argument is passing in the writable directory to do all testing
  if(ac > 1) {
    char *testdir = *++av;
    --ac;
    itksys::SystemTools::ChangeDirectory(testdir);
  }
  if(ac > 1) //This is a mechanism for reading unsigned char images for testing.
    {
      typedef itk::Image<unsigned char, 3> ImageType ;
      ImageType::Pointer input;
      itk::ImageFileReader<ImageType>::Pointer imageReader =
        itk::ImageFileReader<ImageType>::New();
      for(int imagenameindex=1; imagenameindex < ac; imagenameindex++)
        {
          //std::cout << "Attempting to read " << av[imagenameindex] << std::endl;
          try
            {
              imageReader->SetFileName(av[imagenameindex]) ;
              imageReader->Update() ;
              input=imageReader->GetOutput() ;
            }
          catch (itk::ExceptionObject e)
            {
              e.Print(std::cerr) ;
              rval = 0;
            }
        }
    }
  else //This is the mechanism for doing internal testing of all data types.
    {
      int cur_return;
      cur_return = MakeImage<char>();
      if(cur_return != 0)
        {
          std::cerr << "Error writing Analyze file type char" << std::endl;
        }
      else
        {
          rval += cur_return;
        }
      cur_return = MakeImage<unsigned char>();
      if(cur_return != 0)
        {
          std::cerr << "Error writing Analyze file type unsigned char" << std::endl;
        }
      else
        {
          rval += cur_return;
        }
      cur_return = MakeImage<short>();
      if(cur_return != 0)
        {
          std::cerr << "Error writing Analyze file type short" << std::endl;
        }
      else
        {
          rval += cur_return;
        }
      cur_return = MakeImage<unsigned short>();
      if(cur_return != 0)
        {
          std::cerr << "Error writing Analyze file type unsigned short" << std::endl;
        }
      else
        {
          rval += cur_return;
        }
      cur_return = MakeImage<int>();
      if(cur_return != 0)
        {
          std::cerr << "Error writing Analyze file type int" << std::endl;
        }
      else
        {
          rval += cur_return;
        }
      cur_return = MakeImage<float>();
      if(cur_return != 0)
        {
          std::cerr << "Error writing Analyze file type float" << std::endl;
        }
      else
        {
          rval += cur_return;
        }
      // awaiting a double precision byte swapper
      cur_return = MakeImage<double>();
      if(cur_return != 0)
        {
          std::cerr << "Error writing Analyze file type double" << std::endl;
        }
      else
        {
          rval += cur_return;
        }
      rval += TestByteSwap();
    }
  //Tests added to increase code coverage.
      {
      itk::AnalyzeImageIOFactory::Pointer MyFactoryTest=itk::AnalyzeImageIOFactory::New();
      //This was made a protected function.  MyFactoryTest->PrintSelf(std::cout,0);
      }
  return rval;
}

int itkAnalyzeImageIOTest2(int ac, char* av[])
{
  //
  // first argument is passing in the writable directory to do all testing
  if(ac > 1) {
    char *testdir = *++av;
    --ac;
    itksys::SystemTools::ChangeDirectory(testdir);
  }
  if(ac != 3)
    return EXIT_FAILURE;
  char *arg1 = av[1];
  char *arg2 = av[2];
  int test_success = 0;
  typedef itk::Image<signed short, 3> ImageType ;
  typedef ImageType::Pointer ImagePointer ;
  typedef itk::ImageFileReader< ImageType > ImageReaderType ;



  itk::AnalyzeImageIO::Pointer io = itk::AnalyzeImageIO::New();
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  ImagePointer input;
  try 
    {
      imageReader->SetImageIO(io);
      imageReader->SetFileName(arg2);
      imageReader->Update();
      input = imageReader->GetOutput();
    }
  catch (itk::ExceptionObject e)
    {
      test_success = 1;
    }
  


  if(strcmp(arg1, "true") == 0)
    {
      return test_success;
    }
  else
    {
      return !test_success;
    }

}
