/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmIncludeExternalMSProjectCommand.cxx,v $
  Language:  C++
  Date:      $Date: 2006/05/11 20:05:57 $
  Version:   $Revision: 1.13.2.2 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmIncludeExternalMSProjectCommand.h"

// cmIncludeExternalMSProjectCommand
bool cmIncludeExternalMSProjectCommand
::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 2) 
  {
  this->SetError("INCLUDE_EXTERNAL_MSPROJECT called with incorrect "
                 "number of arguments");
    return false;
  }
// only compile this for win32 to avoid coverage errors
#ifdef _WIN32
  if(this->Makefile->GetDefinition("WIN32"))
    {
    std::string location = args[1];
    
    std::vector<std::string> depends;
    if (args.size() > 2)
      {
      for (unsigned int i=2; i<args.size(); ++i) 
        {
        depends.push_back(args[i]); 
        }
      }
    
    std::string utility_name("INCLUDE_EXTERNAL_MSPROJECT");
    utility_name += "_";
    utility_name += args[0];
    std::string path = args[1];
    cmSystemTools::ConvertToUnixSlashes(path);
    const char* no_output = 0;
    const char* no_working_directory = 0;
    this->Makefile->AddUtilityCommand(utility_name.c_str(), true,
                                  no_output, depends,
                                  no_working_directory,
                                  args[0].c_str(), path.c_str());
    
    }
#endif
  return true;
}
