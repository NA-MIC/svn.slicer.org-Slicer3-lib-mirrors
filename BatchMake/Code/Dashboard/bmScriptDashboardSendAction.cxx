/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptDashboardSendAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/09/06 14:09:48 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptDashboardSendAction.h"
#include "HttpRequest.h"

namespace bm {

/** */
ScriptDashboardSendAction::ScriptDashboardSendAction()
: ScriptAction()
{
}

/** */
ScriptDashboardSendAction::~ScriptDashboardSendAction()
{
}

/** */
bool ScriptDashboardSendAction::TestParam(ScriptError* error,int linenumber)
{
  if (m_Parameters.size() < 1)
    {
    error->SetError(MString("No enough parameter for DashboardSend"),linenumber);
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
MString ScriptDashboardSendAction::Help()
{
  return "DashboardSend(method)";
}

/** */
void ScriptDashboardSendAction::Execute()
{

#ifdef BM_GRID
  if(m_GridModule)
    {
    this->GenerateGrid();
    return;
    }
#endif

  typedef ScriptActionManager::Dashboard DashboardType;
  typedef ScriptActionManager::DashboardExperiment DashboardExperimentType;
  typedef ScriptActionManager::DashboardMethod DashboardMethodType;
  typedef ScriptActionManager::DashboardMethodParameter DashboardMethodParameterType;
  
  // Create the experiment on the dashboard
  std::string url = m_Manager->GetDashboardURL();
  m_ProgressManager->AddAction("BMDashboard: Creating Parameter");
  m_ProgressManager->IsRunning();

  HttpRequest m_request;
  m_request.AddParam("user",m_Manager->GetDashboardUser());
  m_request.AddParam("userkey",m_Manager->GetDashboardKey());

  // Get the project name
  const DashboardType * dashboard = m_Manager->GetDashboard();
  const DashboardExperimentType* exp = NULL;
  const DashboardMethodType* meth = NULL;
  std::vector<DashboardExperimentType>::const_iterator it = dashboard->experiments.begin();
  while(it != dashboard->experiments.end())
    {
    std::vector<ScriptActionManager::DashboardMethod>::const_iterator itM = (*it).methods.begin();
    while(itM != (*it).methods.end())
      {
      if(!strcmp((*itM).variable.c_str(),m_Parameters[0].toChar()))
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
  m_request.AddParam("method","AddData");
  m_request.AddParam("methodname",meth->name.c_str());
  m_request.AddParam("experiment",exp->name.c_str());

  m_request.AddParam("hostname",m_request.GetHostName().c_str());
  m_request.AddParam("hostip",m_request.GetHostIp().c_str());

  url += "/dashboard.php";

  // Check that the experiment exist
  std::vector<DashboardExperimentType>::const_iterator itE 
                                              = dashboard->experiments.begin();
  while(itE != dashboard->experiments.end())
    {
    std::vector<DashboardMethodType>::const_iterator itMeth = (*itE).methods.begin();
    while(itMeth != (*itE).methods.end())
      {
      if((!strcmp((*itMeth).variable.c_str(),m_Parameters[0].toChar())))
        {
        std::vector<DashboardMethodParameterType>::const_iterator itParam 
                                                    = (*itMeth).parameters.begin();
        bool found = false;
        while(itParam != (*itMeth).parameters.end())
          {
          MString param = "${";
          param += (*itParam).variable.c_str(); 
          // if this is an ideal output
          if((*itParam).ideal == true)
            {
            param += "_ideal_output";
            }
          param += "}";

          std::string value = m_Manager->Convert(param).toChar();

          // if this is an image or a graph we load it and send the data
          if( !strcmp((*itParam).type.c_str(),"png")
            || !strcmp((*itParam).type.c_str(),"jpg")
            || !strcmp((*itParam).type.c_str(),"bmg")
            )
            {
            std::string imageFilename = m_Manager->Convert(param).toChar();
            long pos = imageFilename.find("'");
            while(pos != -1)
              {
              imageFilename = imageFilename.replace(pos,1,"");
              pos = imageFilename.find("'");
              }
            m_request.AddFile((*itParam).name.c_str(),imageFilename.c_str());
            }
          else
            {
            m_request.AddParam((*itParam).name.c_str(),value.c_str());
            }
           
          itParam++;
          }
        }
      itMeth++;
      }
    itE++;
    }
  

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
