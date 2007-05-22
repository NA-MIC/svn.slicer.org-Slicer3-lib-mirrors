/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmAddCustomTargetCommand.cxx,v $
  Language:  C++
  Date:      $Date: 2006/03/15 16:01:58 $
  Version:   $Revision: 1.21 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmAddCustomTargetCommand.h"

// cmAddCustomTargetCommand
bool cmAddCustomTargetCommand::InitialPass(
  std::vector<std::string> const& args)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // Accumulate one command line at a time.
  cmCustomCommandLine currentLine;

  // Save all command lines.
  cmCustomCommandLines commandLines;

  // Accumulate dependencies.
  std::vector<std::string> depends;
  std::string working_directory;

  // Keep track of parser state.
  enum tdoing {
    doing_command,
    doing_depends,
    doing_working_directory
  };
  tdoing doing = doing_command;

  // Look for the ALL option.
  bool all = false;
  unsigned int start = 1;
  if(args.size() > 1)
    {
    if(args[1] == "ALL")
      {
      all = true;
      start = 2;
      }
    }

  // Parse the rest of the arguments.
  for(unsigned int j = start; j < args.size(); ++j)
    {
    std::string const& copy = args[j];

    if(copy == "DEPENDS")
      {
      doing = doing_depends;
      }
    else if(copy == "WORKING_DIRECTORY")
      {
      doing = doing_working_directory;
      }
    else if(copy == "COMMAND")
      {
      doing = doing_command;

      // Save the current command before starting the next command.
      if(!currentLine.empty())
        {
        commandLines.push_back(currentLine);
        currentLine.clear();
        }
      }
    else
      {
      switch (doing)
        {
        case doing_working_directory:
          working_directory = copy;
          break;
        case doing_command:
          currentLine.push_back(copy);
          break;
        case doing_depends:
          depends.push_back(copy);
          break;
        default:
          this->SetError("Wrong syntax. Unknown type of argument.");
          return false;
        }
      }
    }

  std::string::size_type pos = args[0].find_first_of("#<>");
  if(pos != args[0].npos)
    {
    cmOStringStream msg;
    msg << "called with target name containing a \"" << args[0][pos]
        << "\".  This character is not allowed.";
    this->SetError(msg.str().c_str());
    return false;
    }

  // Store the last command line finished.
  if(!currentLine.empty())
    {
    commandLines.push_back(currentLine);
    currentLine.clear();
    }

  // Add the utility target to the makefile.
  const char* no_output = 0;
  this->Makefile->AddUtilityCommand(args[0].c_str(), all, no_output,
                                    working_directory.c_str(), depends,
                                    commandLines);

  return true;
}
