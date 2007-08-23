/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptDashboardKeyAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/21 23:39:30 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptDashboardKeyAction.h"

namespace bm {

/** */
ScriptDashboardKeyAction::ScriptDashboardKeyAction()
: ScriptAction()
{
}

/** */
ScriptDashboardKeyAction::~ScriptDashboardKeyAction()
{
}

/** */
bool ScriptDashboardKeyAction::TestParam(ScriptError* error,int linenumber)
{
  if (m_Parameters.size() <1)
    {
    error->SetError(MString("No enough parameter for DashboardKey"),linenumber);
    return false;
    }

  m_Manager->SetTestVariable(m_Parameters[0]);

  for (unsigned int i=1;i<m_Parameters.size();i++)
    {
    m_Manager->TestConvert(m_Parameters[i],linenumber);
    }
  return true;
}

/** */
MString ScriptDashboardKeyAction::Help()
{
  return "DashboardKey('Password')";
}

/** */
void ScriptDashboardKeyAction::Execute()
{
  m_Manager->SetDashboardKey(m_Parameters[0].toChar());
}

} // end namespace bm
