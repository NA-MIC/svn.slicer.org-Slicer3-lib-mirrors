/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptIncludeAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/09/18 19:52:06 $
  Version:   $Revision: 1.3 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptIncludeAction.h"
#include "bmScriptParser.h"
#include <itksys/SystemTools.hxx>

namespace bm {

ScriptIncludeAction::ScriptIncludeAction()
: ScriptAction()
{
}

ScriptIncludeAction::~ScriptIncludeAction()
{
}

bool ScriptIncludeAction::TestParam(ScriptError* error,int linenumber)
{
  if (m_Parameters.size() <1)
   {
   error->SetError(MString("No enough parameter for Include"),linenumber);
   return false;
   }

  m_Manager->SetTestVariable(m_Parameters[0]);

  for (unsigned int i=1;i<m_Parameters.size();i++)
    {
    m_Manager->TestConvert(m_Parameters[i],linenumber);
    }

  MString filename = m_Manager->Convert(m_Parameters[0]);
  filename = filename.removeChar('\'');

  std::string fileToInclude = filename.toChar();
  std::string currentScriptPath = itksys::SystemTools::GetFilenamePath(static_cast<ScriptParser*>(m_Manager->GetParser())->GetCurrentFilename().c_str());

  // Test if the script exists
  if(!itksys::SystemTools::FileExists(fileToInclude.c_str()))
    {
    fileToInclude = currentScriptPath+"/"+fileToInclude;
    }

  long line = static_cast<ScriptParser*>(m_Manager->GetParser())->GetLineNumber();
  static_cast<ScriptParser*>(m_Manager->GetParser())->RemoveCodeLine(line-1);
  static_cast<ScriptParser*>(m_Manager->GetParser())->Compile(fileToInclude.c_str(),line-1,true); // don't parse

  return true;
}

MString ScriptIncludeAction::Help()
{
  return "Include(<myScriptFileName>)";
}

void ScriptIncludeAction::Execute()
{
}

} // end namespace bm
