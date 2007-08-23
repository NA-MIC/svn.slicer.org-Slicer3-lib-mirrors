/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptSetAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/01/28 18:30:48 $
  Version:   $Revision: 1.2 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptSetAction.h"

namespace bm {

ScriptSetAction::ScriptSetAction()
: ScriptAction()
{
}

ScriptSetAction::~ScriptSetAction()
{
}

bool ScriptSetAction::TestParam(ScriptError* error,int linenumber)
{
  if (m_Parameters.size() <2)
    {
    error->SetError(MString("No enough parameter for Set"),linenumber);
    return false;
    }

  m_Manager->SetTestVariable(m_Parameters[0]);

  for (unsigned int i=1;i<m_Parameters.size();i++)
    {
    m_Manager->TestConvert(m_Parameters[i],linenumber);
    }
  return true;
}

MString ScriptSetAction::Help()
{
  return "Set(<variable> <name1> <name2> ...)";
}

void ScriptSetAction::Execute()
{
  MString m_value;
  MString param;
  // We check if the parameter 1 is a defined variable
  // This is used by the output application
  std::vector<MString> vars = m_Manager->GetVariable(m_Parameters[1]);
  if(vars.size()>0 && m_Parameters[1][0] != '$' && m_Parameters[1][1] != '{')
    {
    param = "${";
    param += m_Parameters[1].toChar();
    param += "}\0";
    }
  else
    {
    param = m_Parameters[1];
    }

  m_value = m_Manager->Convert(param);

  for (unsigned int i=2;i<m_Parameters.size();i++)
    {
    if (m_value != "")
      {
      m_value+= " ";
      }
    m_value+=m_Manager->Convert(m_Parameters[i]);
    }

  m_Manager->SetVariable(m_Parameters[0],m_value);

#ifdef BM_GRID
  // If we are on the grid we use the bmGridStore to store the variable
  if(m_GridModule)
    {
    this->GenerateGrid(m_Parameters[0].toChar(),m_value.toChar());
    return;
    }
#endif

}

} // end namespace bm
