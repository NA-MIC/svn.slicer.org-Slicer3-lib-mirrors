/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmGlobalWatcomWMakeGenerator.h,v $
  Language:  C++
  Date:      $Date: 2006/03/10 18:54:57 $
  Version:   $Revision: 1.2 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmGlobalWatcomWMakeGenerator_h
#define cmGlobalWatcomWMakeGenerator_h

#include "cmGlobalUNIXMakefileGenerator3.h"

/** \class cmGlobalWatcomWMakeGenerator
 * \brief Write a NMake makefiles.
 *
 * cmGlobalWatcomWMakeGenerator manages nmake build process for a tree
 */
class cmGlobalWatcomWMakeGenerator : public cmGlobalUnixMakefileGenerator3
{
public:
  cmGlobalWatcomWMakeGenerator();
  static cmGlobalGenerator* New() { return new cmGlobalWatcomWMakeGenerator; }
  ///! Get the name for the generator.
  virtual const char* GetName() const {
    return cmGlobalWatcomWMakeGenerator::GetActualName();}
  static const char* GetActualName() {return "Watcom WMake";}

  /** Get the documentation entry for this generator.  */
  virtual void GetDocumentation(cmDocumentationEntry& entry) const;
  
  ///! Create a local generator appropriate to this Global Generator
  virtual cmLocalGenerator *CreateLocalGenerator();

  /**
   * Try to determine system infomation such as shared library
   * extension, pthreads, byte order etc.  
   */
  virtual void EnableLanguage(std::vector<std::string>const& languages, 
                              cmMakefile *);
};

#endif
