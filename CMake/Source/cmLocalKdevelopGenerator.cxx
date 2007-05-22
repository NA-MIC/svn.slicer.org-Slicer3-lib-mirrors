/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmLocalKdevelopGenerator.cxx,v $
  Language:  C++
  Date:      $Date: 2006/04/08 18:15:06 $
  Version:   $Revision: 1.19 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  Copyright (c) 2004 Alexander Neundorf, neundorf@kde.org. All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmGlobalGenerator.h"
#include "cmLocalKdevelopGenerator.h"
#include "cmMakefile.h"
#include "cmSystemTools.h"
#include "cmSourceFile.h"
#include "cmCacheManager.h"
#include "cmGeneratedFileStream.h"
#include "cmake.h"
#include <cmsys/RegularExpression.hxx>


cmLocalKdevelopGenerator::cmLocalKdevelopGenerator()
  :cmLocalUnixMakefileGenerator3()
{
   // KDevelop can itself shorten the output, so it should
   // always get the full output, otherwise the "full output"
   // option in kdevelop doesn't make much sense, Alex
   this->ForceVerboseMakefiles=true;
}

cmLocalKdevelopGenerator::~cmLocalKdevelopGenerator()
{
}

