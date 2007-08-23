/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptIncAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/01/28 18:30:48 $
  Version:   $Revision: 1.2 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptIncAction.h"

namespace bm {

ScriptIncAction::ScriptIncAction()
: ScriptAction()
{
}

ScriptIncAction::~ScriptIncAction()
{
}

bool ScriptIncAction::TestParam(ScriptError* error,int linenumber)
{
   if (m_Parameters.size() <2)
   {
     error->SetError(MString("No enough parameter for Inc"),linenumber);
     return false;
   }

   m_Manager->SetTestVariable(m_Parameters[0]);

  for (unsigned int i=1;i<m_Parameters.size();i++)
      m_Manager->TestConvert(m_Parameters[i],linenumber);


   return true;
}

MString ScriptIncAction::Help()
{
  return "Inc(<myvar> <number>)";
}


void ScriptIncAction::Execute()
{
  MString m_value = m_Manager->Convert(m_Parameters[0]);

  // if we have the variable we want the value
  if(m_Parameters[0][0] != '$')
    {
    MString temp = "${";
    temp += m_Parameters[0];
    temp += "}";
    m_value = m_Manager->Convert(temp);
    }
  // We want the value of the variable
  MString m_inc = m_Parameters[1];

  // get the value without the ''
  MString test = (MString(m_value).rbegin("'"))+1;
  float val = test.toFloat();
  float inc = m_inc.toFloat();
  float sum = val + inc;
  MString newVal(sum);
  m_value ="'";
  m_value += newVal;
  m_value +="'";

  MString param = m_Parameters[0];
  // if we have the $ we want to set the var not the value
  if(m_Parameters[0][0] == '$')
    {
    param= m_Parameters[0].rbegin("}")+2;
    }

  m_Manager->SetVariable(param,m_value);
}

} // end namespace bm
