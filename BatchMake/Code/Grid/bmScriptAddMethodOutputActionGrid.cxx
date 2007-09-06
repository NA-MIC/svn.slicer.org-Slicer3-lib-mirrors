/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptAddMethodOutputActionGrid.cxx,v $
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

namespace bm {

/** */
void ScriptAddMethodOutputAction::GenerateGrid()
{
 // We create the bmGridSend application and send it to condor
  ApplicationWrapper app;
  MString appName = "bmGridSend";
  bool appFound = false;
  ScriptActionManager::ApplicationWrapperListType::iterator itApp = m_Manager->GetApplicationWrapperList()->begin();
  while (itApp != m_Manager->GetApplicationWrapperList()->end())
    {
    if(!strcmp((*itApp)->GetName().toChar(),appName.toChar()))
      {
      app = *(*itApp);
      appFound = true;
      break;
      }
    itApp++;
  }

  if(!appFound)
    {
    std::cout << "ScriptDashboardSendAction::GenerateCondor : Cannot find bmGridSend " 
              << appName.toChar() << std::endl;
    return;
    }
 
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
    std::cout << "BMDashboard: Cannot find method" << std::endl;
    return;
    }

  if(!exp)
    {
    std::cout << "BMDashboard: Cannot find experiment" << std::endl;
    return;
    }

  app.ClearParameterValues();

  std::string withslash = "\"";
  withslash += m_Manager->GetDashboardUser();
  withslash += "\"";
  app.SetParameterValue("hostname","",m_Manager->GetDashboardURL());
  app.SetParameterValue("user","",withslash);
  app.SetParameterValue("key","","1");
  withslash = "\"";
  withslash += m_Manager->GetDashboardKey();
  withslash += "\"";
  app.SetParameterValue("key.key","",withslash);

  withslash = "\"";
  withslash += exp->project;
  withslash += "\"";
  app.SetParameterValue("project","",withslash);
  app.SetParameterValue("createMethodParameter","","1");
  withslash = "\"";
  withslash += exp->name;
  withslash += "\"";
  app.SetParameterValue("createMethodParameter.experiment","",withslash);
  withslash = "\"";
  withslash += meth->name;
  withslash += "\"";
  app.SetParameterValue("createMethodParameter.method","",withslash);
  withslash = "\"";
  withslash += m_Parameters[2].toChar();
  withslash += "\"";
  app.SetParameterValue("createMethodParameter.name","",withslash);
  app.SetParameterValue("createMethodParameter.type","","1");

  if(m_Parameters.size() > 3)
    {
    app.SetParameterValue("createMethodParameter.paramtype","",m_Parameters[3].toChar());
    }
  else
    {
    app.SetParameterValue("createMethodParameter.paramtype","","string");
    }
  m_GridModule->AddApplication(&app);
}

} // end namespace bm
