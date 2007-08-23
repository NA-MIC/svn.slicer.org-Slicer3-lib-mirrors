/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptExtractStringAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/01/28 18:30:48 $
  Version:   $Revision: 1.3 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptExtractStringAction.h"

namespace bm {

ScriptExtractStringAction::ScriptExtractStringAction()
: ScriptAction()
{
}

ScriptExtractStringAction::~ScriptExtractStringAction()
{
}

bool ScriptExtractStringAction::TestParam(ScriptError* error,int linenumber)
{
   if (m_Parameters.size() <2)
   {
     error->SetError(MString("No enough parameter for ExtractString"),linenumber);
     return false;
   }

   m_Manager->SetTestVariable(m_Parameters[0]);

  for (unsigned int i=1;i<m_Parameters.size();i++)
      m_Manager->TestConvert(m_Parameters[i],linenumber);


   return true;
}

MString ScriptExtractStringAction::Help()
{
  return "ExtractString(<output> <input> <lenght> [FROMEND] [KEEPEND])";
}


void ScriptExtractStringAction::Execute()
{
  MString m_value="";
  m_value = "'";
  unsigned int size = m_Parameters[2].toInt();

  if(m_Parameters.size() > 3)
    {
    if(!strcmp(m_Parameters[3].toChar(),"FROMEND"))
      {
      if(m_Parameters.size() > 4 && !strcmp(m_Parameters[4].toChar(),"KEEPEND"))
        {
        unsigned int lenght = strlen(m_Manager->Convert(m_Parameters[1]).toChar());
        for(unsigned int i=lenght-size-1;i<lenght;i++)
          {
          char v = m_Manager->Convert(m_Parameters[1])[i];
          m_value+=v;
          }
        }
      else
        {
        unsigned int lenght = strlen(m_Manager->Convert(m_Parameters[1]).toChar());
        for(unsigned int i=1;i<lenght-size-1;i++)
          {
          char v = m_Manager->Convert(m_Parameters[1])[i];
          m_value+=v;
          }
        }
      }
    if(!strcmp(m_Parameters[3].toChar(),"KEEPEND"))
      {
      unsigned int lenght = strlen(m_Manager->Convert(m_Parameters[1]).toChar());
      for(unsigned int i=size+1;i<lenght;i++)
        {
        char v = m_Manager->Convert(m_Parameters[1])[i];
        m_value+=v;
        }
      }
    }
  else
    {
    for(unsigned int i=1;i<size+1;i++)
      {
      char v = m_Manager->Convert(m_Parameters[1])[i];
      m_value+=v;
      }
    }
  m_value+="'";
  m_Manager->SetVariable(m_Parameters[0],m_value);
}

} // end namespace bm
