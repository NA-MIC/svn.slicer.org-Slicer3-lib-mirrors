/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmLocalKdevelopGenerator.h,v $
  Language:  C++
  Date:      $Date: 2005/06/09 15:39:12 $
  Version:   $Revision: 1.7 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  Copyright (c) 2004 Alexander Neundorf, neundorf@kde.org. All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmLocalKdevelopGenerator_h
#define cmLocalKdevelopGenerator_h

#include "cmLocalUnixMakefileGenerator3.h"

class cmDependInformation;
class cmMakeDepend;
class cmTarget;
class cmSourceFile;

/** \class cmLocalKdevelopGenerator
 */
class cmLocalKdevelopGenerator : public cmLocalUnixMakefileGenerator3
{
public:
  ///! Set cache only and recurse to false by default.
  cmLocalKdevelopGenerator();

  virtual ~cmLocalKdevelopGenerator();

};

#endif
