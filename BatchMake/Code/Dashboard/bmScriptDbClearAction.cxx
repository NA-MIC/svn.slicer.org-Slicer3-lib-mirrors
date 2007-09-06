/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptDbClearAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/09/06 14:09:48 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptDbClearAction.h"
#include "HttpRequest.h"

namespace bm {

ScriptDbClearAction::ScriptDbClearAction()
: ScriptAction()
{
}

ScriptDbClearAction::~ScriptDbClearAction()
{
}

bool ScriptDbClearAction::TestParam(ScriptError* error,int linenumber)
{
  for (unsigned int i=0;i<m_Parameters.size();i++)
      m_Manager->TestConvert(m_Parameters[i],linenumber);
     
  if (m_Parameters.size() != 4)
  {
    error->SetError(MString("No enough parameter for DbClear"),linenumber);
    return false;
  }   

  return true;
}

MString ScriptDbClearAction::Help()
{
  return "DbSendValue(<host> <account> <password> <project>)";
}


void ScriptDbClearAction::Execute()
{
  m_ProgressManager->AddAction("Clear Dashboard");
  m_ProgressManager->IsRunning();
  HttpRequest m_request;

  MString m_host = m_Manager->Convert(m_Parameters[0]).removeChar('\'').latin1();
  MString m_account = m_Manager->Convert(m_Parameters[1]).removeChar('\'').latin1();
  MString m_password = m_Manager->Convert(m_Parameters[2]).removeChar('\'').latin1(); 
  MString m_project = m_Manager->Convert(m_Parameters[3]).removeChar('\'').latin1();

  m_request.AddParam("hostname",m_request.GetHostName().c_str());
  m_request.AddParam("hostip",m_request.GetHostIp().c_str());
  m_request.AddParam("project",m_project.toChar());
  m_request.AddParam("account",m_account.toChar());
  m_request.AddParam("password",m_password.toChar());  
  m_request.AddParam("clear","1");

  MString m_Output = m_request.Send(m_host.toChar());
  
  if (m_Output.length()>3)
    m_ProgressManager->AddError("Bad Host or connexion problem");
  else
  {
    if (m_Output.toInt() == 0)
      m_ProgressManager->FinishAction(MString("Data sent"));
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
