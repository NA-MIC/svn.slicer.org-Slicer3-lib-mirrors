/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptGetCurrentDateTimeAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/03/15 18:53:58 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptGetCurrentDateTimeAction.h"
#include <itksys/SystemTools.hxx>

namespace bm {

ScriptGetCurrentDateTimeAction::ScriptGetCurrentDateTimeAction()
: ScriptAction()
{
}

ScriptGetCurrentDateTimeAction::~ScriptGetCurrentDateTimeAction()
{
}

bool ScriptGetCurrentDateTimeAction::TestParam(ScriptError* error,int linenumber)
{
   if (m_Parameters.size() < 2)
   {
     error->SetError(MString("No enough parameter for GetCurrentDateTime"),linenumber);
     return false;
   }

   if (m_Parameters.size() > 2)
   {
     error->SetError(MString("Too much parameters for GetCurrentDateTime"),linenumber);
     return false;
   }

   m_Manager->SetTestVariable(m_Parameters[0]);

  for (unsigned int i=1;i<m_Parameters.size();i++)
    {
    m_Manager->TestConvert(m_Parameters[i],linenumber);
    }

   return true;
}

MString ScriptGetCurrentDateTimeAction::Help()
{
return "GetCurrentDateTime(<variable> <format>) - Ex: GetCurrentDateTime(timeVar %H:%M:%S)";
}


void ScriptGetCurrentDateTimeAction::Execute()
{
  MString m_value = "'";
  m_value += itksys::SystemTools::GetCurrentDateTime(m_Parameters[1].toChar()).c_str();
  m_value += "'";
  m_Manager->SetVariable(m_Parameters[0], m_value);
}

} // end namespace bm
