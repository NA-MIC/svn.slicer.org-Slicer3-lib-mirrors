/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptSetActionGrid.cxx,v $
  Language:  C++
  Date:      $Date: 2007/12/15 01:06:08 $
  Version:   $Revision: 1.2 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/
#include "bmScriptSetAction.h"

namespace bm {

/** Generate the condor script */
void ScriptSetAction::GenerateGrid(std::string name,std::string value)
{
  // We create the bmGridStore application and send it to condor
  ApplicationWrapper app;
  MString appName = "bmGridStore";
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

  if(m_GridModule->HasCurrentScopeFile())
    {
    app.SetParameterValue("inputfilename","",m_GridModule->GetCurrentScopeFile());
    }
  app.SetParameterValue("filename","",m_GridModule->GetCurrentScopeFile());
  app.SetParameterValue("name","",name);
  app.SetParameterValue("value","",value);

  m_GridModule->AddApplication(&app);
}

} // end namespace bm
