/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptDashboardSendActionGrid.cxx,v $
  Language:  C++
  Date:      $Date: 2007/06/01 13:02:09 $
  Version:   $Revision: 1.5 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptDashboardSendAction.h"
#include "itksys/SystemTools.hxx"

namespace bm {

/** */
void ScriptDashboardSendAction::GenerateGrid()
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

  typedef ScriptActionManager::Dashboard DashboardType;
  typedef ScriptActionManager::DashboardExperiment DashboardExperimentType;
  typedef ScriptActionManager::DashboardMethod DashboardMethodType;
  typedef ScriptActionManager::DashboardMethodParameter DashboardMethodParameterType;

  // Get the project name
  const DashboardType * dashboard = m_Manager->GetDashboard();
  const DashboardExperimentType* exp = NULL;
  const DashboardMethodType* meth = NULL;
  std::vector<DashboardExperimentType>::const_iterator it2 = dashboard->experiments.begin();
  while(it2 != dashboard->experiments.end())
    {
    std::vector<ScriptActionManager::DashboardMethod>::const_iterator itM = (*it2).methods.begin();
    while(itM != (*it2).methods.end())
      {
      if(!strcmp((*itM).variable.c_str(),m_Parameters[0].toChar()))
        {
        exp = &(*it2);
        meth = &(*itM);
        break;
        }
      itM++;
      }
    it2++;
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
  app.SetParameterValue("send","","1");
  withslash = "\"";
  withslash += exp->name;
  withslash += "\"";
  app.SetParameterValue("send.experiment","",withslash);
  withslash = "\"";
  withslash += meth->name;
  withslash += "\"";
  app.SetParameterValue("send.method","",withslash);
  app.SetParameterValue("data","","1");
 
  // Check that the experiment exist
  std::string data = "";
  unsigned int num = 0;
  std::string imagedata = "";
  unsigned int imagenum = 0;

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
          if( !strcmp((*itParam).type.c_str(),"png")
            || !strcmp((*itParam).type.c_str(),"jpg"))
            {
            MString param = "${";
            param += (*itParam).variable.c_str();
            param += "}";
            imagedata += "\"";
            imagedata += (*itParam).name.c_str();
            imagedata += "\"";
            imagedata += " ";
            imagedata += "\"";
            MString value = m_Manager->Convert(param).toChar();
            value = value.removeChar('\'');          
            std::string imagefile = itksys::SystemTools::GetFilenameName(value.toChar());     
            imagedata += imagefile.c_str();
            imagedata += "\" ";
            app.AddInputDataToTransfer(imagefile.c_str());
            imagenum++;
            }
          else
            {
            MString param = "${";
            param += (*itParam).variable.c_str();
            param += "}";
            data += "\"";
            data += (*itParam).name.c_str();
            data += "\"";
            data += " ";
            data += "\"";
            MString value = m_Manager->Convert(param).toChar();
            value = value.removeChar('\'');
            data += value.toChar();
            data += "\" ";
            num++;
            }
          itParam++;
          }
        }
      itMeth++;
      }
    itE++;
    }

  char* numParams = new char[10];
  sprintf(numParams,"%d",num*2);
  std::string numParam = numParams;
  numParam += " ";
  numParam += data;
  delete [] numParams;
  app.SetParameterValue("data.NumberOfValues","",numParam);

  numParams = new char[10];
  sprintf(numParams,"%d",imagenum*2);
  numParam = numParams;
  numParam += " ";
  numParam += imagedata;
  delete [] numParams;
  app.SetParameterValue("imagedata.NumberOfValues","",numParam);

  if(imagenum>0)
    {
    app.SetParameterValue("imagedata","","1");
    }

  app.SetParameterValue("inputFile","","1");
  app.SetParameterValue("inputFile.inputFile","",m_GridModule->GetCurrentScopeFile());

  m_GridModule->AddApplication(&app,m_GridModule->GetOutputDirectory());
}

} // end namespace bm
