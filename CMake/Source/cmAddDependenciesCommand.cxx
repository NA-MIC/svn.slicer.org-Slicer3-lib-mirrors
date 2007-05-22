/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmAddDependenciesCommand.cxx,v $
  Language:  C++
  Date:      $Date: 2006/03/15 16:01:58 $
  Version:   $Revision: 1.13 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmAddDependenciesCommand.h"

// cmDependenciesCommand
bool cmAddDependenciesCommand::InitialPass(
  std::vector<std::string> const& args)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  std::string target_name = args[0];

  cmTargets &tgts = this->Makefile->GetTargets();
  if (tgts.find(target_name) != tgts.end())
    {
    std::vector<std::string>::const_iterator s = args.begin();
    ++s;
    for (; s != args.end(); ++s)
      {
      tgts[target_name].AddUtility(s->c_str());
      }
    }
  else
    {
    std::string error = "Adding dependency to non-existent target: ";
    error += target_name;
    this->SetError(error.c_str());
    return false;
    }


  return true;
}

