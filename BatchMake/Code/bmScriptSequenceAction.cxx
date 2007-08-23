/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptSequenceAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/01/28 18:30:48 $
  Version:   $Revision: 1.3 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptSequenceAction.h"

namespace bm {

ScriptSequenceAction::ScriptSequenceAction()
: ScriptAction()
{
}

ScriptSequenceAction::~ScriptSequenceAction()
{
}

bool ScriptSequenceAction::TestParam(ScriptError* error,int linenumber)
{
   if (m_Parameters.size() < 4)
   {
     error->SetError(MString("No enough parameter for Sequence"),linenumber);
     return false;
   }

   if (m_Parameters.size() > 4)
   {
     error->SetError(MString("Too much parameters for Sequence"),linenumber);
     return false;
   }


   m_Manager->SetTestVariable(m_Parameters[0]);

  for (unsigned int j=1;j<m_Parameters.size();j++)
    {
    if (!m_Parameters[j].isFloat())
      {
      error->SetError(MString("Parameter %1 should be a float for Sequence ").
                      arg(j) + m_Parameters[j], linenumber);
      return false;
      }
    }

  for (unsigned int i=1;i<m_Parameters.size();i++)
    {
    m_Manager->TestConvert(m_Parameters[i],linenumber);
    }

   return true;
}

MString ScriptSequenceAction::Help()
{
  return "Sequence(<variable> <From> <To> <Step>) - Ex: Sequence(seq 1 10 0.2)";
}


void ScriptSequenceAction::Execute()
{
  MString m_value;
  for (float i=m_Parameters[1].toFloat();
       i<=m_Parameters[2].toFloat();
       i+=m_Parameters[3].toFloat())
    {
    int j=(int)i;
    if(j-i == 0) // if the given number is an int then we write an int
      {  
      m_value+= MString("'%1' ").arg(j);
      }
    else
      {
      m_value+= MString("'%1' ").arg(i);
      }
    }
  m_Manager->SetVariable(m_Parameters[0], m_value);
}

} // end namespace bm
