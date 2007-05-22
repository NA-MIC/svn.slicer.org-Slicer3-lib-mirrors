/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: RegistrationExamples3.cxx,v $
  Language:  C++
  Date:      $Date: 2006/05/14 12:16:23 $
  Version:   $Revision: 1.20 $

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
#include "vnl/vnl_sample.h"
#include "itkTestMain.h" 


void RegisterTests()
{
  vnl_sample_reseed(8775070);
  REGISTER_TEST(ImageRegistration8Test);
  REGISTER_TEST(ImageRegistration9Test);
}

#undef main
#define main  ImageRegistration8Test
#undef CommandIterationUpdate
#define CommandIterationUpdate CommandIterationUpdate10
#include "ImageRegistration8.cxx"

#undef main
#define main  ImageRegistration9Test
#undef CommandIterationUpdate
#define CommandIterationUpdate CommandIterationUpdate11
#include "ImageRegistration9.cxx"


