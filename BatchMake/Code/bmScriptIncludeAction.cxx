/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptIncludeAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/01/28 18:30:48 $
  Version:   $Revision: 1.2 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptIncludeAction.h"
#include "bmScriptParser.h"

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
  long line = static_cast<ScriptParser*>(m_Manager->GetParser())->GetLineNumber();
  static_cast<ScriptParser*>(m_Manager->GetParser())->RemoveCodeLine(line-1);
  static_cast<ScriptParser*>(m_Manager->GetParser())->Compile(filename,line-1,true); // don't parse

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
