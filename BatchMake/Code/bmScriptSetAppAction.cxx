/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptSetAppAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/21 23:39:13 $
  Version:   $Revision: 1.5 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptSetAppAction.h"

namespace bm {

ScriptSetAppAction::ScriptSetAppAction()
: ScriptAction()
{
}

ScriptSetAppAction::~ScriptSetAppAction()
{
}

bool ScriptSetAppAction::TestParam(ScriptError* error,int linenumber)
{  
  if (m_Parameters.size() < 2)
    {
    error->SetError(MString("SetApp() takes at least two arguments"),linenumber);
    return false;
    }

  m_Manager->SetTestVariable(m_Parameters[0]);

  for (unsigned int i=1;i<m_Parameters.size();i++)
    {
    m_Manager->TestConvert(m_Parameters[i],linenumber);
    }

  MString appName = m_Parameters[1];
  appName = appName.removeChar('@');

  bool appFound = false;

  if(!m_Manager->GetApplicationWrapperList())
    {
    error->SetError(MString("Calling SetApp() but no applications are defined"),linenumber);
    return false;
    }

  ScriptActionManager::ApplicationWrapperListType::iterator it = m_Manager->GetApplicationWrapperList()->begin();
  while (it != m_Manager->GetApplicationWrapperList()->end())
    {
    if(!strcmp((*it)->GetName().toChar(),appName.toChar()))
      {
      appFound = true;
      
      // Set all the variables of the application to off
      std::vector<ApplicationWrapperParam> & params = (*it)->GetParams();
      std::vector<ApplicationWrapperParam>::iterator itParams = params.begin();
        
      while(itParams != params.end())
        {
        (*itParams).SetValueDefined(false);
        itParams++;
        }

      (*it)->SetSequentialParsing(false);
      if(m_Parameters.size()==3)
        {
        if(m_Parameters[2].toInt())
          {
          (*it)->SetSequentialParsing(true);
          }
        }
      break;
      }
    it++;
    }

  if(!appFound)
    {
    error->SetError(MString("SetApp() cannot find the corresponding application"),linenumber);
    }

  return true;
}

MString ScriptSetAppAction::Help()
{
  return "SetApp(<variable> <application>)";
}


void ScriptSetAppAction::Execute()
{
  MString appName = m_Parameters[1];
  appName = appName.removeChar('@');
  bool appFound = false;
  ScriptActionManager::ApplicationWrapperListType::iterator it = m_Manager->GetApplicationWrapperList()->begin();
  while (it != m_Manager->GetApplicationWrapperList()->end())
    {
    if(!strcmp((*it)->GetName().toChar(),appName.toChar()))
      {
      m_Value = "'";
      std::string val = (*it)->GetApplicationPath().toChar();
      m_Value += val.c_str();
      m_Value += "'";
      appFound = true;
     
      
      // Set all the variables of the application to off
      std::vector<ApplicationWrapperParam> & params = (*it)->GetParams();
      std::vector<ApplicationWrapperParam>::iterator itParams = params.begin();
        
      while(itParams != params.end())
        {
        (*itParams).SetValueDefined(false);
        itParams++;
        }

      (*it)->SetSequentialParsing(false);
      if(m_Parameters.size()==3)
        {
        if(m_Parameters[2].toInt())
          {
          (*it)->SetSequentialParsing(true);
          }
        }
      break;
      }
    it++;
    }
  m_Manager->SetVariable(m_Parameters[0],m_Value);
}

} // end namespace bm
