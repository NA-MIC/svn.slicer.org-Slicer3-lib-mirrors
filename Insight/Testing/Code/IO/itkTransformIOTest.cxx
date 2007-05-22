/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkTransformIOTest.cxx,v $
  Language:  C++
  Date:      $Date: 2005/04/05 15:41:45 $
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

#include "itkTransformFileWriter.h"
#include "itkTransformFileReader.h"
#include "itkAffineTransform.h"
#include "itkTransformFactory.h"
#include "itkSimilarity2DTransform.h"
#include "itkBSplineDeformableTransform.h"

int itkTransformIOTest(int itkNotUsed(ac), char* itkNotUsed(av)[])
{
  unsigned int i;
  // Create an odd type of transform, and register it
  typedef itk::AffineTransform<double,4> AffineTransformType;
  typedef itk::AffineTransform<double,6> AffineTransformTypeNotRegistered;
  itk::TransformFactory<AffineTransformType>::RegisterTransform();
  AffineTransformType::Pointer affine = AffineTransformType::New();
  AffineTransformType::InputPointType cor;

  // Set it's parameters
  AffineTransformType::ParametersType p = affine->GetParameters();
  for ( i = 0; i < p.GetSize(); i++ )
    {
    p[i] = i;
    }
  affine->SetParameters ( p );
  p = affine->GetFixedParameters ();
  for ( i = 0; i < p.GetSize(); i++ )
    {
    p[i] = i;
    }
  affine->SetFixedParameters ( p );
  itk::TransformFileWriter::Pointer writer;
  itk::TransformFileReader::Pointer reader;
  reader = itk::TransformFileReader::New();
  writer = itk::TransformFileWriter::New();
  writer->AddTransform(affine);

  writer->SetFileName( "Transforms.txt" );
  reader->SetFileName( "Transforms.txt" );
 
  // Testing writing
  std::cout << "Testing write : ";
  affine->Print ( std::cout );
  try
   {
   writer->Update();
   std::cout << std::endl;
   std::cout << "Testing read : " << std::endl;
   reader->Update();
   }
  catch( itk::ExceptionObject & excp )
   {
   std::cerr << "Error while saving the transforms" << std::endl;
   std::cerr << excp << std::endl;
   std::cout << "[FAILED]" << std::endl;
   return EXIT_FAILURE;
   }


  try
    {
    itk::TransformFileReader::TransformListType *list;
    list = reader->GetTransformList();
    itk::TransformFileReader::TransformListType::iterator i = list->begin();
    while ( i != list->end() )
      {
      (*i)->Print ( std::cout );
      i++;
      }
    }
  catch( itk::ExceptionObject & excp )
    {
    std::cerr << "Error while saving the transforms" << std::endl;
    std::cerr << excp << std::endl;
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
    }


  std::cout << "Creating bad writer" << std::endl;
  AffineTransformTypeNotRegistered::Pointer Bogus = AffineTransformTypeNotRegistered::New();

  // Set it's parameters
  p = Bogus->GetParameters();
  for ( i = 0; i < p.GetSize(); i++ )
    {
    p[i] = i;
    }
  Bogus->SetParameters ( p );
  p = Bogus->GetFixedParameters ();
  for ( i = 0; i < p.GetSize(); i++ )
    {
    p[i] = i;
    }
  Bogus->SetFixedParameters ( p );


  
  itk::TransformFileWriter::Pointer badwriter;
  itk::TransformFileReader::Pointer badreader;
  badreader = itk::TransformFileReader::New();
  badwriter = itk::TransformFileWriter::New();
  badwriter->AddTransform(Bogus);
  badwriter->SetFileName( "TransformsBad.txt" );
  badreader->SetFileName( "TransformsBad.txt" );
 
  // Testing writing
  std::cout << "Testing write of non register transform : " << std::endl;
  std::cout << std::flush;
  try
   {
   badwriter->Update();
   }
  catch( itk::ExceptionObject & excp )
   {
   std::cerr << "Error while saving the transforms" << std::endl;
   std::cerr << excp << std::endl;
   std::cout << "[FAILED]" << std::endl;
   return EXIT_FAILURE;
   }
  
  // Testing writing
  
  std::cout << "Testing read of non register transform : " << std::endl;
  std::cout << std::flush;
  bool caught = 0;
  try
    {
    badreader->Update();
    }
  catch( itk::ExceptionObject & excp )
   {
   caught = 1;
   std::cout << "Caught exception as expected" << std::endl;
   std::cout << excp << std::endl;
   }
  if ( !caught )
    {
    std::cerr << "Did not catch non registered transform" << std::endl;
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "[PASSED]" << std::endl;

  return EXIT_SUCCESS;
}
