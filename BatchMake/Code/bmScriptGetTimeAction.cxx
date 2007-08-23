/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptGetTimeAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/03/15 18:53:58 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptGetTimeAction.h"
#include <itksys/SystemTools.hxx>

namespace bm {

ScriptGetTimeAction::ScriptGetTimeAction()
: ScriptAction()
{
}

ScriptGetTimeAction::~ScriptGetTimeAction()
{
}

bool ScriptGetTimeAction::TestParam(ScriptError* error,int linenumber)
{
   if (m_Parameters.size() < 1)
   {
     error->SetError(MString("No enough parameter for GetTime"),linenumber);
     return false;
   }

   if (m_Parameters.size() > 1)
   {
     error->SetError(MString("Too much parameters for GetTime"),linenumber);
     return false;
   }

   m_Manager->SetTestVariable(m_Parameters[0]);

  for (unsigned int i=1;i<m_Parameters.size();i++)
    {
    m_Manager->TestConvert(m_Parameters[i],linenumber);
    }

   return true;
}

MString ScriptGetTimeAction::Help()
{
  return "GetTime(<variable>) - Ex: GetTime(timeVar)";
}


void ScriptGetTimeAction::Execute()
{
  MString m_value = "'";

  char* time = new char[255];
  sprintf(time,"%f",itksys::SystemTools::GetTime());
  m_value += time;
  m_value += "'";
  m_Manager->SetVariable(m_Parameters[0], m_value);
  delete [] time;
}

} // end namespace bm
