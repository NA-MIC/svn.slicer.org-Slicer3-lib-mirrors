/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptDashboardUserAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/01/28 18:30:49 $
  Version:   $Revision: 1.2 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptDashboardUserAction.h"

namespace bm {

/** */
ScriptDashboardUserAction::ScriptDashboardUserAction()
: ScriptAction()
{
}

/** */
ScriptDashboardUserAction::~ScriptDashboardUserAction()
{
}

/** */
bool ScriptDashboardUserAction::TestParam(ScriptError* error,int linenumber)
{
  if (m_Parameters.size() <1)
    {
    error->SetError(MString("No enough parameter for DashboardUser"),linenumber);
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
MString ScriptDashboardUserAction::Help()
{
  return "DashboardUser('FirstName LastName')";
}

/** */
void ScriptDashboardUserAction::Execute()
{
  m_Manager->SetDashboardUser(m_Parameters[0].toChar());
}

} // end namespace bm
