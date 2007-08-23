/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptIfAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/01/28 18:30:48 $
  Version:   $Revision: 1.2 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptIfAction.h"
#include <fstream>

namespace bm {

ScriptIfAction::ScriptIfAction()
: ScriptAction()
{
  m_Mode = 0;
}

ScriptIfAction::~ScriptIfAction()
{
}

void ScriptIfAction::SetMode(int mode)
{
  m_Mode = mode;
}

bool ScriptIfAction::TestParam(ScriptError* error,int linenumber)
{
  if (m_Parameters.size() <3)
    {
    error->SetError(MString("if() takes at least 3 arguments"),linenumber);
    return false;
    }

  m_Manager->SetTestVariable(m_Parameters[0]);

  for (unsigned int i=1;i<m_Parameters.size();i++)
    {
    m_Manager->TestConvert(m_Parameters[i],linenumber);
    }
   return true;
}


void ScriptIfAction::AddAction(ScriptAction* action)
{
  if (m_Mode == 0)
    {
    m_ThenAction.push_back(action);
    }
  else
    {
    m_ElseAction.push_back(action);
    }
}

MString ScriptIfAction::Help()
{
  return "If(<$variable1> ==|<=|>=|!= <$variable2> || && <condition>) ... EndIf(<variable>)";
}

/** Return the status of the current condition*/
bool ScriptIfAction::TestCurrentCondition(unsigned int startingParameter,unsigned int conditionSize)
{
  MString left = m_Parameters[startingParameter];
  if(left[0] == '$')
    {
    if ((m_Manager->GetVariable(m_Parameters[startingParameter])).size())
      {
      left = m_Manager->GetVariable(m_Parameters[startingParameter])[0];
      }
    else
      {
      left = "";
      }
    }
  
  MString right = "";

  if(conditionSize>2)
    {
    right = m_Parameters[startingParameter+2];
    if(right[0] == '$')
      {
      right = m_Manager->GetVariable(m_Parameters[startingParameter+2])[0];
      }
    }

  // if we only have on parameter
  // something like if(${myvar}) we just test if the var exist
  if(conditionSize==1)
    {
    if (left != "")
      {
      return true;
      }
    }
  else if (m_Parameters[startingParameter+1].toLower() == "==")
    {
    if (left == right)
      {
      return true;
      }
    }
  else if (m_Parameters[startingParameter+1].toLower() == "!=")
    {
    if (left != right)
      {
      return true;
      }
    }
  else if (m_Parameters[startingParameter+1].toLower() == ">")
    {
    if (left.toFloat() > right.toFloat())
      {
      return true;
      }
    } 
  else if (m_Parameters[startingParameter+1].toLower() == "<")
    {
    if (left.toFloat() < right.toFloat())
      {
      return true;
      }
    }
  else if (m_Parameters[startingParameter+1].toLower() == ">=")
    {
    if (left.toFloat() >= right.toFloat())
      {
      return true;
      }
    }
  else if (m_Parameters[startingParameter+1].toLower() == "<=")
    {
    if (left.toFloat() <= right.toFloat())
      {
      return true;
      }
    }

  return false;

}

void ScriptIfAction::Execute()
{
  bool thenAction = false;

  unsigned int currentPosition = 0;
  // We loop through the conditions
  while(currentPosition < m_Parameters.size())
    {
    std::string nextCondition = "";
    unsigned int endPosition = m_Parameters.size();
    // Find the end of the condition
    for(unsigned int i=currentPosition;i<m_Parameters.size();i++)
      {
      if(m_Parameters[i] == "&&" || m_Parameters[i] == "||")
        {
        nextCondition = m_Parameters[i].toChar();
        endPosition = i;
        break;
        }
      }
   
    bool currentCondition = this->TestCurrentCondition(currentPosition,endPosition-currentPosition);

    // If we have already the condition we stop
    if((nextCondition.find("&&") != -1) && !currentCondition)
      {
      thenAction = false;
      break;
      }
    else if((nextCondition.find("||") != -1) && currentCondition)
      {
      thenAction = true;
      break;
      }
    else
      {
      thenAction = currentCondition;
      }

    currentPosition = endPosition+1;
    }

  if(thenAction)
    {
    for (unsigned int i=0;i<m_ThenAction.size();i++)
      {
      m_ThenAction[i]->Execute();
      }
    }
  else
    {
    for (unsigned int i=0;i<m_ElseAction.size();i++)
      {
      m_ElseAction[i]->Execute();
      }
    }
}

void ScriptIfAction::Delete()
{
  for (unsigned int i=0;i<m_ThenAction.size();i++)
    {
    m_ThenAction[i]->Delete();
    delete m_ThenAction[i];
    }

  for (unsigned int j=0;j<m_ElseAction.size();j++)
    {
    m_ElseAction[j]->Delete();
    delete m_ElseAction[j];
    }
}

} // end namespace bm
