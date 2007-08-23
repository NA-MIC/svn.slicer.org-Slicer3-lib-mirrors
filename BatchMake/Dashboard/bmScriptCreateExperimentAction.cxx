/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptCreateExperimentAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/21 23:39:46 $
  Version:   $Revision: 1.6 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptCreateExperimentAction.h"
#include "HttpRequest.h"

namespace bm {

/** */
ScriptCreateExperimentAction::ScriptCreateExperimentAction()
: ScriptAction()
{
}

/** */
ScriptCreateExperimentAction::~ScriptCreateExperimentAction()
{
}

/** */
bool ScriptCreateExperimentAction::TestParam(ScriptError* error,int linenumber)
{
  if (m_Parameters.size()< 3)
    {
    error->SetError(MString("No enough parameter for CreateExperiment"),linenumber);
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
MString ScriptCreateExperimentAction::Help()
{
  return "CreateExperiment(var 'Project Name' 'Experiment Name' [description])";
}

/** */
void ScriptCreateExperimentAction::Execute()
{
  // Add the experiment to the dashboard
  m_Manager->AddDashboardExperiment(m_Parameters[0].toChar(),
                                    m_Parameters[1].toChar(),
                                    m_Parameters[2].toChar());
 
#ifdef BM_GRID
  if(m_GridModule)
    {
    this->GenerateGrid();
    return;
    }
#endif

  // Create the experiment on the dashboard
  std::string url = m_Manager->GetDashboardURL();
  m_ProgressManager->AddAction("BMDashboard: Creating Experiment");
  m_ProgressManager->IsRunning();

  HttpRequest m_request;
  m_request.AddParam("user",m_Manager->GetDashboardUser());
  m_request.AddParam("userkey",m_Manager->GetDashboardKey());
  m_request.AddParam("project",m_Parameters[1].toChar());
  m_request.AddParam("method","CreateExperiment");
  m_request.AddParam("name",m_Parameters[2].toChar());
  if(m_Parameters.size()>3)
    {
    m_request.AddParam("description",m_Parameters[3].toChar());
    }
  m_request.AddParam("hostname",m_request.GetHostName().c_str());
  m_request.AddParam("hostip",m_request.GetHostIp().c_str());
  m_request.AddFile("script",m_Manager->GetScriptFullPath());

  url += "/dashboard.php";
  std::string m_Output = m_request.Send(url.c_str());

/* 
  if (m_Output.length()>3)
    {
    m_ProgressManager->AddError("Bad Host or connexion problem");
    }
  else
    {
    if (m_Output.toInt() == 0)
      {
      m_ProgressManager->FinishAction(MString("Data sent"));
      }
    else
      {
      m_ProgressManager->FinishAction(MString("Dashboard problem when sending data"));
      switch(m_Output.toInt())
        {
        case 1 :  m_ProgressManager->AddError("Bad user name"); break;
        case 2 :  m_ProgressManager->AddError("Bad project name"); break;
        case 3 :  m_ProgressManager->AddError("User doesn't belong to project"); break;
        case 4 :  m_ProgressManager->AddError("Over quota: please use DbClear function first"); break;
        case 5 :  m_ProgressManager->AddError("Host Database error"); break;
        case -1 : m_ProgressManager->AddError("Connexion problem"); break;
        }  
      }
    }
*/
}

} // end namespace bm
