/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptAddDashboardLabelAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/07/26 21:49:15 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptAddDashboardLabelAction.h"
#include "HttpRequest.h"

namespace bm {

/** */
ScriptAddDashboardLabelAction::ScriptAddDashboardLabelAction()
: ScriptAction()
{
}

/** */
ScriptAddDashboardLabelAction::~ScriptAddDashboardLabelAction()
{
}

/** */
bool ScriptAddDashboardLabelAction::TestParam(ScriptError* error,int linenumber)
{
  if (m_Parameters.size() < 3)
    {
    error->SetError(MString("No enough parameter for AddDashboardLabel"),linenumber);
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
MString ScriptAddDashboardLabelAction::Help()
{
  return "AddDashboardLabel(dashboardVariable methodVariable parameterName [title])";
}

/** */
void ScriptAddDashboardLabelAction::Execute()
{
  std::string parameterType = "";
/*
#ifdef BM_GRID
  if(m_GridModule)
    {
    this->GenerateGrid();
    return;
    }
#endif
*/

  // Create the experiment on the dashboard
  std::string url = m_Manager->GetDashboardURL();
  m_ProgressManager->AddAction("BMDashboard: Creating dashboard label");
  m_ProgressManager->IsRunning();

  HttpRequest m_request;
  m_request.AddParam("user",m_Manager->GetDashboardUser());
  m_request.AddParam("userkey",m_Manager->GetDashboardKey());

  // Get the project name
  const ScriptActionManager::Dashboard * dashboard = m_Manager->GetDashboard();
  const ScriptActionManager::BatchBoard* batchboard = NULL;
  const ScriptActionManager::DashboardMethod* method = NULL;
  const ScriptActionManager::DashboardExperiment* exp = NULL;

  // Loop through the list of experiment to find the method as well as the batchboard
  std::vector<ScriptActionManager::DashboardExperiment>::const_iterator itExp = dashboard->experiments.begin();
  while(itExp != dashboard->experiments.end())
    {
    // Method
    std::vector<ScriptActionManager::DashboardMethod>::const_iterator itM = (*itExp).methods.begin();
    while(itM != (*itExp).methods.end())
      {
      if(!strcmp((*itM).variable.c_str(),m_Parameters[1].toChar()))
        {
        exp = &(*itExp);
        method = &(*itM);
        break;
        }
      itM++;
      }

    // Batchboard
    std::vector<ScriptActionManager::BatchBoard>::const_iterator itB = (*itExp).batchboards.begin();
    while(itB != (*itExp).batchboards.end())
      {
      if(!strcmp((*itB).variable.c_str(),m_Parameters[0].toChar()))
        {
        batchboard = &(*itB);
        break;
        }
      itB++;
      }
    itExp++;
    }

   if(!method)
    {
    m_ProgressManager->AddError("BMDashboard: Cannot find method");
    return;
    }

   if(!batchboard)
    {
    m_ProgressManager->AddError("BMDashboard: Cannot find batchboard");
    return;
    }

  // Find the parameter name
  const ScriptActionManager::DashboardMethodParameter* parameter = NULL;

  std::vector<ScriptActionManager::DashboardMethodParameter>::const_iterator itP = method->parameters.begin();
  while(itP != method->parameters.end())
    {
    if(!strcmp((*itP).name.c_str(),  m_Parameters[2].toChar()))
      {
      parameter = &(*itP);
      break;
      }
    itP++;
    }
   
  if(!parameter)
    {
    m_ProgressManager->AddError("BMDashboard: Cannot find parameter");
    return;
    }

  m_request.AddParam("project",exp->project.c_str());
  m_request.AddParam("method","CreateDashboardLabel");
  m_request.AddParam("experiment",exp->name.c_str());
  m_request.AddParam("methodname",method->name.c_str());
  m_request.AddParam("dashboardname",batchboard->title.c_str());
  m_request.AddParam("labelparameter",parameter->name.c_str());

  if(m_Parameters.size()>3)
    {
    m_request.AddParam("labeltitle",m_Parameters[3].toChar());
    }

  m_request.AddParam("hostname",m_request.GetHostName().c_str());
  m_request.AddParam("hostip",m_request.GetHostIp().c_str());

  url += "/dashboard.php";
  MString m_Output = m_request.Send(url.c_str());

  std::cout << m_Output.toChar() << std::endl;

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
}

} // end namespace bm
