/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkIOCommonTest.cxx,v $
  Language:  C++
  Date:      $Date: 2008-07-08 20:00:43 $
  Version:   $Revision: 1.3 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include "itkIOCommon.h"
#include <cstring>
#include <string>
#include <itksys/SystemTools.hxx>

bool CheckFileNameParsing(const std::string fileName,
                          const std::string correctNameOnly,
                          const std::string correctExtension,
                          const std::string correctPath)
{
#if !defined(ITK_LEGACY_REMOVE)
  // the old, depricated way...
  std::cout << "(itk, depricated) Extracting...file name..."; std::flush(std::cout);  
  char* nameOnly  = itk::IOCommon::ExtractFileName(fileName.c_str());
  std::cout << "extension..."; std::flush(std::cout);  
  char* extension = itk::IOCommon::ExtractFileExtension(fileName.c_str());
  std::cout << "path..."; std::flush(std::cout);  
  char* path      = itk::IOCommon::ExtractFilePath(fileName.c_str());
  std::cout << "DONE" << std::endl;
#else
  // the current kwsys way...
  std::cout << "(kwsys) Extracting...file name..."; std::flush(std::cout);  
  std::string fileNameString = 
    itksys::SystemTools::GetFilenameWithoutLastExtension
    (itksys::SystemTools::GetFilenameName(fileName));
  char* nameOnly = new char[fileNameString.size() + 1];
  strcpy(nameOnly, fileNameString.c_str());
  std::cout << "extension..."; std::flush(std::cout);  
  std::string extensionString =
    itksys::SystemTools::GetFilenameLastExtension(fileName);
  // NB: remove the period (kwsys leaves it on, ITK precedent was to
  // remove it)            
  char* extension = new char[extensionString.size()];
  strcpy(extension, extensionString.c_str() + 1);
  std::cout << "path..."; std::flush(std::cout);  
  std::string pathString = itksys::SystemTools::GetFilenamePath(fileName);
  // NB: add trailing slash iff the result is non-empty (kwsys always
  // removes it, ITK precedent was to keep it) 
  if (pathString.size() > 1)
  {
#if defined(_WIN32)
    pathString = pathString + "\\";
#else
    pathString = pathString + "/";
#endif
  }
  char* path = new char[pathString.size() + 1];
  strcpy(path, pathString.c_str());
  std::cout << "DONE" << std::endl;
#endif

  std::cout << "Comparing...file name..."; std::flush(std::cout);  
  bool nameMatches;
  if (nameOnly == NULL)
    {
    nameMatches = correctNameOnly.size() == 0;
    }
  else
    {
    nameMatches = correctNameOnly.compare(nameOnly) == 0;
    }

  std::cout << "extension..."; std::flush(std::cout);  
  bool extensionMatches;
  if (extension == NULL)
    {
    extensionMatches = correctExtension.size() == 0;
    }
  else
    {
    extensionMatches = correctExtension.compare(extension) == 0;
    }

  std::cout << "path..."; std::flush(std::cout);  
  bool pathMatches;
  if (path == NULL)
    {
    pathMatches = correctPath.size() == 0;
    }
  else
    {
    pathMatches = correctPath.compare(path) == 0;
    }
  std::cout << "DONE" << std::endl;

  std::cout << "FullFileName: \"" << fileName << "\"" << std::endl;
  std::cout << "FileName: (expected) \"" << correctNameOnly 
            << "\" (actual) \"" << (nameOnly != NULL ? nameOnly : "(null)") 
            << "\"" 
            << " (correct) " << nameMatches << std::endl;
  std::cout << "Extension: (expected) \"" << correctExtension 
            << "\" (actual) \"" << (extension != NULL ? extension : "(null)") 
            << "\"" 
            << " (correct) " << extensionMatches << std::endl;
  std::cout << "Path: (expected) \"" << correctPath 
            << "\" (actual) \"" << (path != NULL ? path : "(null)") 
            << "\"" 
            << " (correct) " << pathMatches << std::endl;            

  bool correctParse = nameMatches && extensionMatches && pathMatches;
  std::cout << "Parsing is " << (correctParse ? "correct" : "incorrect") 
            << std::endl;

  // clean up
  std::cout << "Cleaning up...";
  if (nameOnly != NULL)
    {
    delete [] nameOnly;
    }
  if (extension != NULL)
    {
    delete [] extension;
    }
  if (path != NULL)
    {
    delete [] path;
    }
  std::cout << "DONE" << std::endl;

  return correctParse;
}

int itkIOCommonTest(int , char* [])
{
  bool success = true;

  //
  // reasonable cases
  //

#if defined(_WIN32)
  success = success && 
    CheckFileNameParsing("c:\\dir1\\dir2\\myfile.tar.gz",
                         "myfile.tar",
                         "gz",
                         "c:\\dir1\\dir2\\");
  success = success && 
    CheckFileNameParsing("c:/dir1/dir2/myfile.tar.gz",
                         "myfile.tar",
                         "gz",
                         "c:/dir1/dir2/");
  success = success && 
    CheckFileNameParsing("\\\\sambaserver\\dir1\\dir2\\myfile.tar.gz",
                         "myfile.tar",
                         "gz",
                         "\\\\sambaserver\\dir1\\dir2\\");
#endif
  success = success && 
    CheckFileNameParsing("/dir1/dir2/myfile.tar.gz",
                         "myfile.tar",
                         "gz",
                         "/dir1/dir2/");

  //
  // less reasonable cases
  //
  success = success && 
    CheckFileNameParsing(".", "", "", "");

  success = success && 
    CheckFileNameParsing("/", "", "", "/");

#if defined(_WIN32)
  success = success && 
    CheckFileNameParsing("\\", "", "", "\\");
#endif

  success = success && 
    CheckFileNameParsing("/.tar.gz", ".tar", "gz", "/");

#if defined(_WIN32)
  success = success && 
    CheckFileNameParsing("\\.tar.gz", ".tar", "gz", "\\");
#endif

  success = success && 
    CheckFileNameParsing(".tar.gz", ".tar", "gz", "");

  success = success && 
    CheckFileNameParsing("myfile", "myfile", "", "");

  success = success && 
    CheckFileNameParsing("/myfile", "myfile", "", "/");

#if defined(_WIN32)
  success = success && 
    CheckFileNameParsing("\\myfile", "myfile", "", "\\");
#endif

  success = success && 
    CheckFileNameParsing("", "", "", "");

  return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
