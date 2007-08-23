/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptGridTempDirectoryAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/01/28 18:30:49 $
  Version:   $Revision: 1.2 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptGridTempDirectoryAction.h"

namespace bm {

/** */
ScriptGridTempDirectoryAction::ScriptGridTempDirectoryAction()
: ScriptAction()
{
}

/** */
ScriptGridTempDirectoryAction::~ScriptGridTempDirectoryAction()
{
}

/** */
bool ScriptGridTempDirectoryAction::TestParam(ScriptError* error,int linenumber)
{
  if (m_Parameters.size() <1)
    {
    error->SetError(MString("No enough parameter for GridTempDirectory"),linenumber);
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
MString ScriptGridTempDirectoryAction::Help()
{
  return "GridTempDirectory(GridTempDirectory)";
}

/** */
void ScriptGridTempDirectoryAction::Execute()
{
  if(m_GridModule)
    {
    m_GridModule->SetGridTempDirectory(m_Parameters[0].toChar());
    }
}

} // end namespace bm
