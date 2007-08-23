/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptGlobAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/21 19:09:31 $
  Version:   $Revision: 1.2 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptGlobAction.h"
#include <itksys/Glob.hxx>
#include <itksys/SystemTools.hxx>

#include <stdio.h>

namespace bm {

bool ScriptGlobAction::TestParam(ScriptError* error,int linenumber)
{
  if (m_Parameters.size() < 2)
    {
    error->SetError(MString("No enough parameter for Glob function !"),linenumber);
    return false;
    }

  m_Manager->SetTestVariable(m_Parameters[0]);

  for (unsigned int i=1;i<m_Parameters.size();i++)
    {
    m_Manager->TestConvert(m_Parameters[i],linenumber);
    }
  return true;
}

 
MString ScriptGlobAction::Help()
{
  return "Glob(<var> 'globexpression' '[RECURSIVE]')";
}

void ScriptGlobAction::Execute()
{
  // Read the glob expression
  MString globexpression = m_Manager->Convert(m_Parameters[1]);
  if (globexpression.startWith('\''))
    {
    globexpression = globexpression.rbegin("'") + 1;
    }

  itksys::Glob glob;

  // Are we recursive
  if (m_Parameters.size() == 3)
    {
    MString recurse = m_Manager->Convert(m_Parameters[2]);
    if(recurse.find("RECURSIVE")!= -1 || recurse.find("recursive")!= -1)
      {
      glob.RecurseOn();
      }
    }

  if(globexpression.endWith('\\') || globexpression.endWith('/'))
    {
    globexpression += "*.*";
    }

  MString value="";
 
  std::string convertedGlobExpression = globexpression.toChar();
  itksys::SystemTools::ConvertToUnixSlashes(convertedGlobExpression);

  if(glob.FindFiles(convertedGlobExpression))
    {
    std::vector<std::string> filenames = glob.GetFiles();

    for(unsigned int i=0;i<filenames.size();i++)
      {
      if (value != "")
        {
        value += " ";
        }
      value += MString("'") + MString(filenames[i]) + MString("'");
      }
    }

  m_Manager->SetVariable(m_Parameters[0],value);
}

} // end namespace bm
