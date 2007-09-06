/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptGridSetGroupingAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/09/06 14:09:48 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptGridSetGroupingAction.h"

namespace bm {

/** */
ScriptGridSetGroupingAction::ScriptGridSetGroupingAction()
: ScriptAction()
{
}

/** */
ScriptGridSetGroupingAction::~ScriptGridSetGroupingAction()
{
}

/** */
bool ScriptGridSetGroupingAction::TestParam(ScriptError* error,int linenumber)
{
  if (m_Parameters.size() <1)
    {
    error->SetError(MString("No enough parameter for GridSetGrouping"),linenumber);
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
MString ScriptGridSetGroupingAction::Help()
{
  return "GridSetGrouping(0 or 1)";
}

/** */
void ScriptGridSetGroupingAction::Execute()
{
  if(m_GridModule)
    {
    m_GridModule->SetGrouping(m_Parameters[0].toBool());
    }
}

} // end namespace bm
