/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptCreateExperimentActionGrid.cxx,v $
  Language:  C++
  Date:      $Date: 2007/09/06 14:09:48 $
  Version:   $Revision: 1.1 $
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
void ScriptCreateExperimentAction::GenerateGrid()
{
  // We create the bmGridSend application and send it to condor
  ApplicationWrapper app;
  MString appName = "bmGridSend";
  bool appFound = false;
  ScriptActionManager::ApplicationWrapperListType::iterator it = m_Manager->GetApplicationWrapperList()->begin();
  while (it != m_Manager->GetApplicationWrapperList()->end())
    {
    if(!strcmp((*it)->GetName().toChar(),appName.toChar()))
      {
      app = *(*it);
      appFound = true;
      break;
      }
    it++;
  }

  if(!appFound)
    {
    std::cout << "ScriptDashboardSendAction::GenerateCondor : Cannot find bmGridSend " 
              << appName.toChar() << std::endl;
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
  withslash += m_Parameters[1].toChar();
  withslash += "\"";
  app.SetParameterValue("project","",withslash);
  app.SetParameterValue("createExperiment","","1");
  withslash = "\"";
  withslash += m_Parameters[2].toChar();
  withslash += "\"";
  app.SetParameterValue("createExperiment.name","",withslash);
  if(m_Parameters.size()>3)
    {
    withslash = "\"";
    withslash += m_Parameters[3].toChar();
    withslash += "\"";
    app.SetParameterValue("createExperiment.description","",withslash);
    }
  m_GridModule->AddApplication(&app);
}

} // end namespace bm
