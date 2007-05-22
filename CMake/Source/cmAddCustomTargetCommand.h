/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmAddCustomTargetCommand.h,v $
  Language:  C++
  Date:      $Date: 2006/02/08 15:58:34 $
  Version:   $Revision: 1.15 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmAddCustomTargetCommand_h
#define cmAddCustomTargetCommand_h

#include "cmCommand.h"

/** \class cmAddCustomTargetCommand
 * \brief Command that adds a target to the build system.
 *
 * cmAddCustomTargetCommand adds an extra target to the build system.
 * This is useful when you would like to add special
 * targets like "install,", "clean," and so on.
 */
class cmAddCustomTargetCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmAddCustomTargetCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);
  
  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() 
    {return "ADD_CUSTOM_TARGET";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Add a target with no output so it will always be built.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  ADD_CUSTOM_TARGET(Name [ALL] [command1 [args1...]]\n"
      "                    [COMMAND command2 [args2...] ...]\n"
      "                    [DEPENDS depend depend depend ... ])\n"
      "                    [WORKING_DIRECTORY dir]\n"
      "Adds a target with the given name that executes the given commands "
      "every time the target is built.  If the ALL option is specified "
      "it indicates that this target should be added to the default build "
      "target so that it will be run every time.  "
      "The command and arguments are optional.  If not specified, "
      "it will create an empty target.  The ADD_DEPENDENCIES command can be "
      "used in conjunction with this command to drive custom target "
      "generation.  The command cannot be called ALL. "
      "If WORKING_DIRECTORY is set, then the command will be run in that "
      "directory.";
    }
  
  cmTypeMacro(cmAddCustomTargetCommand, cmCommand);
};

#endif
