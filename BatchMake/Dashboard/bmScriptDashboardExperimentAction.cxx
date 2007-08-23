/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptDashboardExperimentAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/01/28 18:30:49 $
  Version:   $Revision: 1.2 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptDashboardExperimentAction.h"

namespace bm {

/** */
ScriptDashboardExperimentAction::ScriptDashboardExperimentAction()
: ScriptAction()
{
}

/** */
ScriptDashboardExperimentAction::~ScriptDashboardExperimentAction()
{
}

/** */
bool ScriptDashboardExperimentAction::TestParam(ScriptError* error,int linenumber)
{
  if (m_Parameters.size() <1)
    {
    error->SetError(MString("No enough parameter for DashboardExperiment"),linenumber);
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
MString ScriptDashboardExperimentAction::Help()
{
  return "DashboardExperiment(encryptedExperimentId)";
}

/** */
void ScriptDashboardExperimentAction::Execute()
{
  m_Manager->SetDashboardExperiment(m_Parameters[0].toChar());
}

} // end namespace bm
