/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptSetAppOptionAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/01/28 18:30:48 $
  Version:   $Revision: 1.4 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptSetAppOptionAction.h"

namespace bm {

ScriptSetAppOptionAction::ScriptSetAppOptionAction()
: ScriptAction()
{
}

ScriptSetAppOptionAction::~ScriptSetAppOptionAction()
{
}

bool ScriptSetAppOptionAction::TestParam(ScriptError* error,int linenumber)
{  
  if (m_Parameters.size() < 2)
    {
     error->SetError(MString("SetAppOption() takes at least two arguments"),linenumber);
     return false;
    }

  m_Manager->SetTestVariable(m_Parameters[0]);

  for (unsigned int i=1;i<m_Parameters.size();i++)
    {
    m_Manager->TestConvert(m_Parameters[i],linenumber);
    }

   return true;
}

MString ScriptSetAppOptionAction::Help()
{
  return "SetAppOption(<application.option> <value>)";
}


void ScriptSetAppOptionAction::Execute()
{
  MString m_value;
  m_value=m_Manager->Convert(m_Parameters[1]);

  unsigned int i; 
  for (i=2;i<m_Parameters.size();i++)
    {
    if (m_value != "")
      {
      m_value+= " ";
      }
    m_value+=m_Manager->Convert(m_Parameters[i]);
    }


  // First we search the name of the variable
  long pos = m_Parameters[0].find(".",0);
  std::string first = m_Parameters[0].mid(0,pos).toChar();

  long pos1 = m_Parameters[0].find(".",pos+1);

  std::string second = "";
  std::string third = "";  
  if(pos1 !=-1)
    {
    second = m_Parameters[0].mid(pos+1,pos1-pos-1).toChar();
    }
  else if(pos>-1)
    {
    second = m_Parameters[0].mid(pos+1,m_Parameters.size()-pos-1).toChar();
    }

  if(pos1 > -1)
    {
    third = m_Parameters[0].mid(pos1+1,m_Parameters.size()-pos1-1).toChar();
    }

  // Copy the values
  std::string value = "";
  for(i=1;i<m_Parameters.size();i++)
    {
    std::string param = m_Parameters[i].toChar();
    long int currentpos = 0;
    long int posvar = param.find("${");

    while(posvar != -1) // if the second parameters has been defined as a variable
      {
      value +=  param.substr(currentpos,posvar-currentpos);

      long int curly = param.find("}",posvar); 
      if(curly!=-1)
        {
        currentpos = curly+1;
        std::string var = param.substr(posvar,curly-posvar+1);
        value += m_Manager->GetVariable(var)[0].toChar();
        }
       posvar =param.find("$",posvar+1);
      }
    if(param.size()-currentpos>0)
      {
      value +=  param.substr(currentpos,param.size()-currentpos);
      }
    value += " ";
    }
  ApplicationWrapper * app = NULL;
  MString appName = m_Manager->GetVariable(first.c_str())[0];
  bool appFound = false;
  ScriptActionManager::ApplicationWrapperListType::iterator it = m_Manager->GetApplicationWrapperList()->begin();
  while (it != m_Manager->GetApplicationWrapperList()->end())
    {
    if(!strcmp((*it)->GetApplicationPath().toChar(),appName.toChar()))
      {
      (*it)->SetParameterValue(second,third,value);
      app = *it;
      appFound = true;
      break;
      }
    it++;
  }

  if(!appFound) 
    {
    std::cout << "Cannot find application : "  << appName.toChar() << std::endl;
    return;
    }

  m_value = "'";
  m_value += app->GetApplicationPath();
  m_value += "' '";
  m_value += app->GetCurrentCommandLineArguments(false);
  m_value += "'";

  m_Manager->SetVariable(first.c_str(),m_value);
}

} // end namespace bm
