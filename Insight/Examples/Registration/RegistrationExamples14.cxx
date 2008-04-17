/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: RegistrationExamples14.cxx,v $
  Language:  C++
  Date:      $Date: 2008-04-07 00:05:37 $
  Version:   $Revision: 1.1 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

// this file defines the RegistrationExamples for the test driver
// and all it expects is that you have a function called RegisterTests

#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

#include <iostream>
#include "itkTestMain.h" 


void RegisterTests()
{
  REGISTER_TEST(DeformableRegistration15Test);
}

#undef main
#define main  DeformableRegistration15Test
#undef CommandIterationUpdate
#define CommandIterationUpdate CommandIterationUpdate15
#include "DeformableRegistration15.cxx"

