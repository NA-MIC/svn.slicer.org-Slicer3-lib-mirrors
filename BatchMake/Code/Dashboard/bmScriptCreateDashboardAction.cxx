/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptCreateDashboardAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/09/06 14:09:48 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptCreateDashboardAction.h"
#include "HttpRequest.h"

namespace bm {

/** */
ScriptCreateDashboardAction::ScriptCreateDashboardAction()
: ScriptAction()
{
}

/** */
ScriptCreateDashboardAction::~ScriptCreateDashboardAction()
{
}

/** */
bool ScriptCreateDashboardAction::TestParam(ScriptError* error,int linenumber)
{
  if (m_Parameters.size()< 3)
    {
    error->SetError(MString("No enough parameter for CreateDashboard"),linenumber);
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
MString ScriptCreateDashboardAction::Help()
{
  return "CreateDashboard(var ExperimentVariable 'title' 'public=0|1')";
}

/** */
void ScriptCreateDashboardAction::Execute()
{
  // Add the experiment to the dashboard
  ScriptActionManager::BatchBoard* bboard = m_Manager->AddBatchBoard(m_Parameters[0].toChar(),
                                                m_Parameters[1].toChar(),
                                                m_Parameters[2].toChar(),
                                                m_Parameters[3].toChar());
 
/*#ifdef BM_GRID
  if(m_GridModule)
    {
    this->GenerateGrid();
    return;
    }
#endif*/

  // Create the experiment on the dashboard
  std::string url = m_Manager->GetDashboardURL();
  m_ProgressManager->AddAction("BMDashboard: Creating Dashboard");
  m_ProgressManager->IsRunning();

  HttpRequest m_request;
  m_request.AddParam("user",m_Manager->GetDashboardUser());
  m_request.AddParam("userkey",m_Manager->GetDashboardKey());

  // Get the project name
  const ScriptActionManager::Dashboard * dashboard = m_Manager->GetDashboard();
  const ScriptActionManager::DashboardExperiment* exp = NULL;
  std::vector<ScriptActionManager::DashboardExperiment>::const_iterator it = dashboard->experiments.begin();
  while(it != dashboard->experiments.end())
    {
    if(!strcmp((*it).variable.c_str(),m_Parameters[1].toChar()))
      {
      exp = &(*it);
      break;
      }
    it++;
    }

  if(!exp)
    {
    m_ProgressManager->AddError("BMDashboard: Cannot find experiment");
    return;
    }

  m_request.AddParam("project",exp->project.c_str());
  m_request.AddParam("method","CreateDashboard");
  m_request.AddParam("experiment",exp->name.c_str());
  m_request.AddParam("title",m_Parameters[2].toChar());
  m_request.AddParam("public",m_Parameters[3].toChar());

  m_request.AddParam("hostname",m_request.GetHostName().c_str());
  m_request.AddParam("hostip",m_request.GetHostIp().c_str());

  url += "/dashboard.php";
  std::string m_Output = m_request.Send(url.c_str());

  long pos = m_Output.find("Batchboardid=");
  if(pos!=-1)
    {
    long pos1 = m_Output.find("\n",pos);
    bboard->SQLid = m_Output.substr(pos+13,pos1-pos-13);
    }
}

} // end namespace bm
