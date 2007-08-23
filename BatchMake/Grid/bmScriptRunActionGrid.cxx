/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptRunActionGrid.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/21 23:40:27 $
  Version:   $Revision: 1.4 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptRunAction.h"

namespace bm {


/** Generate the condor script */
void ScriptRunAction::GenerateGrid(const char* appname)
{
  // we look for the current application
  // first we remove any ${}
  MString appvar(appname);
  appvar = appvar.removeChar('$');
  appvar = appvar.removeChar('{');
  appvar = appvar.removeChar('}');

  if(m_Manager->GetVariable(appvar).size()==0)
    {
    std::cout << "ScriptRunAction::GenerateGrid():" 
              << "Applications should be wrapped to be used on the grid"
              << std::endl;
    return;
    }

  // second we try to find the current variable in the list of applications
  ApplicationWrapper * application = NULL;
  MString appName = m_Manager->GetVariable(appvar)[0];
  bool appFound = false;
  ScriptActionManager::ApplicationWrapperListType::iterator it = m_Manager->GetApplicationWrapperList()->begin();
  while (it != m_Manager->GetApplicationWrapperList()->end())
    {
    if(!strcmp((*it)->GetApplicationPath().toChar(),appName.toChar()))
      {
      application = *it;
      appFound = true;
      break;
      }
    it++;
  }

  if(!application)
    {
    std::cout << "ScriptRunAction::GenerateCondor : Cannot find the application corresponding to " << appvar.toChar() << std::endl;
    return;
    }

  if(!application->ParameterExists("GenerateXMLFile"))
    {
    m_GridModule->AddApplication(application);
    }
  else
    {
    // If the application support it. we force the application to write the output out as XML
    std::string xmloutputname = application->GetName().toChar();
    xmloutputname += ".";
    char* num = new char[10];
    sprintf(num,"%d",m_GridModule->GetNumberOfApplications());
    xmloutputname += num;
    xmloutputname += ".out.xml";
    application->SetParameterValue("GenerateXMLFile","","1");
    application->SetParameterValue("GenerateXMLFile.GenerateXMLFile","",xmloutputname);
    delete [] num;

    m_GridModule->AddApplication(application);

    // We create the bmGridStore application and send it to condor
    ApplicationWrapper app;
    MString gsName = "bmGridStore";
    bool gsFound = false;
    ScriptActionManager::ApplicationWrapperListType::iterator itApp = m_Manager->GetApplicationWrapperList()->begin();
    while (itApp != m_Manager->GetApplicationWrapperList()->end())
      {
      if(!strcmp((*itApp)->GetName().toChar(),gsName.toChar()))
        {
        app = *(*itApp);
        gsFound = true;
        break;
        }
      itApp++;
      }

    if(!gsFound)
      {
      std::cout << "ScriptDashboardSendAction::GenerateCondor : Cannot find bmGridStore " 
                << appName.toChar() << std::endl;
      return;
      }

    app.SetParameterValue("filename","",m_GridModule->GetCurrentScopeFile());
    app.SetParameterValue("name","","\" \"");
    app.SetParameterValue("value","","\" \"");
    //std::string parse = "-p ";
    //parse += xmloutputname;
    app.SetParameterValue("parse","","1");
    app.SetParameterValue("parse.parse","",xmloutputname.c_str());
    //std::string appvarname = "-a ";
    MString appn = m_Parameters[1];
    appn = appn.removeChar('$');
    appn = appn.removeChar('{');
    appn = appn.removeChar('}');
    //appvarname += appn.toChar();
    app.SetParameterValue("appname","","1");
    app.SetParameterValue("appname.appname","",appn.toChar());
    m_GridModule->AddApplication(&app);
    }
  
}

} // end namespace bm
