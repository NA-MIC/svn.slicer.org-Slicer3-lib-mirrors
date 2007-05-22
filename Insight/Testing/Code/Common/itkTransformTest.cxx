/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkTransformTest.cxx,v $
  Language:  C++
  Date:      $Date: 2003/09/10 14:30:10 $
  Version:   $Revision: 1.10 $

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

#include "itkTransform.h"
#include "itkMatrix.h"

   

int itkTransformTest(int, char* [] )
{

  typedef  itk::Transform<double,3,3>      TransformType;
  TransformType::Pointer transform = TransformType::New();

  TransformType::InputPointType pnt;
  transform->TransformPoint(pnt);

  TransformType::InputVectorType  vec;
  transform->TransformVector(vec);

  TransformType::InputVnlVectorType   vec_vnl;
  transform->TransformVector(vec_vnl);

  TransformType::InputCovariantVectorType    covec;
  transform->TransformCovariantVector(covec);

  TransformType::ParametersType parameters(6);
  try
    {
    transform->SetParameters(parameters);
    }
  catch( itk::ExceptionObject & e )
    {
    std::cerr << e << std::endl;
    }

  try
    {
    transform->GetParameters();
    }
  catch( itk::ExceptionObject & e )
    {
    std::cerr << e << std::endl;
    }

  try
    {
    transform->GetJacobian(pnt);
    }
  catch( itk::ExceptionObject & e )
    {
    std::cerr << e << std::endl;
    }

  std::cout << "[ PASSED ]" << std::endl;
  return EXIT_SUCCESS;

}
