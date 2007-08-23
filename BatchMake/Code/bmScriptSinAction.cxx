/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptSinAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/01/28 18:30:49 $
  Version:   $Revision: 1.2 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptSinAction.h"
#include <math.h>

namespace bm {

ScriptSinAction::ScriptSinAction()
: ScriptAction()
{
}

ScriptSinAction::~ScriptSinAction()
{
}

bool ScriptSinAction::TestParam(ScriptError* error,int linenumber)
{
  m_Manager->SetTestVariable(m_Parameters[0]);

  if (m_Parameters.size()<2)
    {
    error->SetError(MString("No enough parameter for Sin"),linenumber);
    return false;
    }

  return true;
} 

MString ScriptSinAction::Help()
{
  return "Sin(<outputvariale> <input>)";
}


void ScriptSinAction::Execute()
{
  MString m_value;
  char* m_text = new char[100];
  MString test = m_Manager->Convert(m_Parameters[1]);
  test = test.removeChar('\'');
  float m_X = test.toFloat();
  sprintf(m_text,"'%f'",(float)sin(m_X));
  m_value = m_text;
  m_Manager->SetVariable(m_Parameters[0],m_value);
}

} // end namespace bm
