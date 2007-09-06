/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptDataDirectoryAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/09/06 14:09:48 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptDataDirectoryAction.h"

namespace bm {

/** */
ScriptDataDirectoryAction::ScriptDataDirectoryAction()
: ScriptAction()
{
}

/** */
ScriptDataDirectoryAction::~ScriptDataDirectoryAction()
{
}

/** */
bool ScriptDataDirectoryAction::TestParam(ScriptError* error,int linenumber)
{
  if (m_Parameters.size() <1)
    {
    error->SetError(MString("No enough parameter for DataDirectory"),linenumber);
    return false;
    }

  m_Manager->SetTestVariable(m_Parameters[0]);

  for (unsigned int i=1;i<m_Parameters.size();i++)
    {
    m_Manager->TestConvert(m_Parameters[i],linenumber);
    }
  return true;
}

/** */
MString ScriptDataDirectoryAction::Help()
{
  return "DataDirectory(myhostnamewiththedata.com)";
}

/** */
void ScriptDataDirectoryAction::Execute()
{
  if(m_GridModule)
    {
    m_GridModule->SetDataDirectory(m_Parameters[0].toChar());
    }
}

} // end namespace bm
