/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptAddMethodOutputAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/09/06 14:09:48 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptAddMethodOutputAction.h"
#include "HttpRequest.h"

namespace bm {

/** */
ScriptAddMethodOutputAction::ScriptAddMethodOutputAction()
: ScriptAction()
{
}

/** */
ScriptAddMethodOutputAction::~ScriptAddMethodOutputAction()
{
}

/** */
bool ScriptAddMethodOutputAction::TestParam(ScriptError* error,int linenumber)
{
  if (m_Parameters.size() < 3)
    {
    error->SetError(MString("No enough parameter for AddMethodOutput"),linenumber);
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
MString ScriptAddMethodOutputAction::Help()
{
  return "AddMethodOutput(methodVariable ExperimentVariable 'MethodName')";
}

/** */
void ScriptAddMethodOutputAction::Execute()
{
  std::string parameterType = "";
  if(m_Parameters.size() > 3)
    {
    parameterType = m_Parameters[3].toChar();
    }

  m_Manager->AddDashboardMethodParameter(m_Parameters[0].toChar(),
                               m_Parameters[1].toChar(),
                               m_Parameters[2].toChar(),
                               true,false,
                               parameterType.c_str()
                               );
  
#ifdef BM_GRID
  if(m_GridModule)
    {
    this->GenerateGrid();
    return;
    }
#endif

  // Create the experiment on the dashboard
  std::string url = m_Manager->GetDashboardURL();
  m_ProgressManager->AddAction("BMDashboard: Creating Parameter");
  m_ProgressManager->IsRunning();

  HttpRequest m_request;
  m_request.AddParam("user",m_Manager->GetDashboardUser());
  m_request.AddParam("userkey",m_Manager->GetDashboardKey());

  // Get the project name
  const ScriptActionManager::Dashboard * dashboard = m_Manager->GetDashboard();
  const ScriptActionManager::DashboardExperiment* exp = NULL;
  const ScriptActionManager::DashboardMethod* meth = NULL;
  std::vector<ScriptActionManager::DashboardExperiment>::const_iterator it = dashboard->experiments.begin();
  while(it != dashboard->experiments.end())
    {
    std::vector<ScriptActionManager::DashboardMethod>::const_iterator itM = (*it).methods.begin();
    while(itM != (*it).methods.end())
      {
      if(!strcmp((*itM).variable.c_str(),m_Parameters[1].toChar()))
        {
        exp = &(*it);
        meth = &(*itM);
        break;
        }
      itM++;
      }
    it++;
    }

   if(!meth)
    {
    m_ProgressManager->AddError("BMDashboard: Cannot find method");
    return;
    }

  if(!exp)
    {
    m_ProgressManager->AddError("BMDashboard: Cannot find experiment");
    return;
    }

  m_request.AddParam("project",exp->project.c_str());
  m_request.AddParam("method","CreateParameter");
  m_request.AddParam("name",m_Parameters[2].toChar());
  m_request.AddParam("experiment",exp->name.c_str());

  m_request.AddParam("methodname",meth->name.c_str());
  m_request.AddParam("type","1"); //output
  m_request.AddParam("paramtype",parameterType.c_str()); // type of the parameter

  m_request.AddParam("hostname",m_request.GetHostName().c_str());
  m_request.AddParam("hostip",m_request.GetHostIp().c_str());

  url += "/dashboard.php";
  MString m_Output = m_request.Send(url.c_str());

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
