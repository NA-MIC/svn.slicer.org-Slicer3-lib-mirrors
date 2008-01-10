/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptGetListSizeAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/11/22 15:06:28 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptGetListSizeAction.h"

namespace bm {

ScriptGetListSizeAction::ScriptGetListSizeAction()
: ScriptAction()
{
}

ScriptGetListSizeAction::~ScriptGetListSizeAction()
{
}

bool ScriptGetListSizeAction::TestParam(ScriptError* error,int linenumber)
{
   if (m_Parameters.size() < 2)
     {
     error->SetError(MString("No enough parameter for GetListSize"),linenumber);
     return false;
     }

  m_Manager->SetTestVariable(m_Parameters[0]);

  for (unsigned int i=1;i<m_Parameters.size();i++)
    {
    m_Manager->TestConvert(m_Parameters[i],linenumber);
    }

   return true;
}

MString ScriptGetListSizeAction::Help()
{
  return "GetListSize(<new variable> <$variable>)";
}

void ScriptGetListSizeAction::Execute()
{
  MString value;
  std::vector<MString> list = m_Manager->GetParamsFromVariable(m_Manager->Convert(m_Parameters[1]));

  char* size = new char[10];
  sprintf(size,"'%d'",list.size());
  value = size;
  delete [] size;

  m_Manager->SetVariable(m_Parameters[0],value);
}

} // end namespace bm
