/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptDashboardNotifyAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/09/06 14:09:48 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptDashboardNotifyAction.h"
#include "HttpRequest.h"

namespace bm {

/** */
ScriptDashboardNotifyAction::ScriptDashboardNotifyAction()
: ScriptAction()
{
}

/** */
ScriptDashboardNotifyAction::~ScriptDashboardNotifyAction()
{
}

/** */
bool ScriptDashboardNotifyAction::TestParam(ScriptError* error,int linenumber)
{
  if (m_Parameters.size() <1)
    {
    error->SetError(MString("No enough parameter for DashboardNotify"),linenumber);
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
MString ScriptDashboardNotifyAction::Help()
{
  return "DashboardNotify(myMessage)";
}

/** */
void ScriptDashboardNotifyAction::Execute()
{
  std::string url = m_Manager->GetDashboardURL();
  //const char* experiment = m_Manager->GetDashboardExperiment();
  m_ProgressManager->AddAction("Send notify to user");
  m_ProgressManager->IsRunning();
  HttpRequest m_request;

  //m_request.AddParam("experiment",experiment);
  m_request.AddParam("message",m_Parameters[0].toChar());
  m_request.AddParam("hostname",m_request.GetHostName().c_str());
  m_request.AddParam("hostip",m_request.GetHostIp().c_str());

  url += "/notify.php";
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
        case 1 :  m_ProgressManager->AddError("Bad account name"); break;
        case 2 :  m_ProgressManager->AddError("Bad password"); break;
        case 3 :  m_ProgressManager->AddError("Bad project name"); break;
        case 4 :  m_ProgressManager->AddError("Over quota: please use DbClear function first"); break;
        case 5 :  m_ProgressManager->AddError("Host Database error"); break;
        case -1 : m_ProgressManager->AddError("Connexion problem"); break;
        }  
      }
    }
}

} // end namespace bm
