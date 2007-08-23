/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptAppendFileAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/01/28 18:30:48 $
  Version:   $Revision: 1.2 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptAppendFileAction.h"

namespace bm {

ScriptAppendFileAction::ScriptAppendFileAction()
: ScriptAction()
{
}

ScriptAppendFileAction::~ScriptAppendFileAction()
{
}

bool ScriptAppendFileAction::TestParam(ScriptError* error,int linenumber)
{
  for (unsigned int i=0;i<m_Parameters.size();i++)
    m_Manager->TestConvert(m_Parameters[i],linenumber);
    
  if (m_Parameters.size() < 2)
  {
    error->SetError(MString("No enough parameter for AppendFile"),linenumber);
    return false;
  }   

  return true;
}

MString ScriptAppendFileAction::Help()
{
  return "AppendFile(<value1> [value2] ...)";
}


void ScriptAppendFileAction::Execute()
{
  std::ofstream m_file;
   m_file.open((m_Manager->Convert(m_Parameters[0]).removeChar('\'')).latin1(),std::ofstream::binary | ios_base::app);


  MString m_value;
  for (unsigned int i=1;i<m_Parameters.size();i++)
  {
    MString m_text = m_Manager->Convert(m_Parameters[i]).removeChar('\'').latin1();
    bool m_insert;
    for (int l=0;l<m_text.length();l++)
    {
      m_insert = true;
      if ((m_text[l] == '\\') && (l!=m_text.length()-1))
        if (m_text[l+1] == 'n')
        {
           m_file << std::endl;
           l++;
           m_insert = false;
        }        
     
       if (m_insert)
         m_file << m_text[l];
    } 
  }

    m_file.close();  
}

} // end namespace bm
