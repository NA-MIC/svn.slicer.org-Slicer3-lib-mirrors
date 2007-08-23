/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptGetFilenameAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/15 13:28:18 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptGetFilenameAction.h"
#include <itksys/SystemTools.hxx>

namespace bm {

bool ScriptGetFilenameAction::TestParam(ScriptError* error,int linenumber)
{
  m_Manager->SetTestVariable(m_Parameters[0]);

  if (m_Parameters.size()<3)
    {
    error->SetError(MString("No enough parameter for GetFilename"),linenumber);
    return false;
    }

  return true;
} 

MString ScriptGetFilenameAction::Help()
{
  return "GetFilename(<outputvariale> <input> [NAME PATH PARENT_PATH EXTENSION NAME_WITHOUT_EXTENSION])";
}


void ScriptGetFilenameAction::Execute()
{
  MString input = m_Manager->Convert(m_Parameters[1]);
  MString option = m_Manager->Convert(m_Parameters[2]).toLower();

  MString value = "'";
  if(!strcmp(option.toChar(),"'name'"))
    {
    value += itksys::SystemTools::GetFilenameName(input.toChar());
    }
  else if(!strcmp(option.toChar(),"'path'"))
    {
    value += itksys::SystemTools::GetFilenamePath(input.toChar());
    }
  else if(!strcmp(option.toChar(),"'name_without_extension'"))
    {
    value += itksys::SystemTools::GetFilenameWithoutExtension(input.toChar());
    }
  else if(!strcmp(option.toChar(),"'parent_path'"))
    {
    std::string currentPath = itksys::SystemTools::GetFilenamePath(input.toChar());
    value += itksys::SystemTools::GetFilenamePath(currentPath);
    }
  else if(!strcmp(option.toChar(),"'extension'"))
    {
    value += itksys::SystemTools::GetFilenameExtension(input.toChar());
    }
  else
    {
    std::cout << "ScriptGetFilenameAction: " << option.toChar() << " not defined!" << std::endl;
    }
  value += "'";
  m_Manager->SetVariable(m_Parameters[0],value);
}

} // end namespace bm
