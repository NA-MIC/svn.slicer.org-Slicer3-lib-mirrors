/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptGridDataHostAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/09/06 14:09:48 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptGridDataHostAction.h"

namespace bm {

/** */
ScriptGridDataHostAction::ScriptGridDataHostAction()
: ScriptAction()
{
}

/** */
ScriptGridDataHostAction::~ScriptGridDataHostAction()
{
}

/** */
bool ScriptGridDataHostAction::TestParam(ScriptError* error,int linenumber)
{
  if (m_Parameters.size() <1)
    {
    error->SetError(MString("No enough parameter for GridDataHost"),linenumber);
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
MString ScriptGridDataHostAction::Help()
{
  return "GridDataHost(myhostnamewiththedata.com)";
}

/** */
void ScriptGridDataHostAction::Execute()
{
  if(m_GridModule)
    {
    m_GridModule->SetDataHost(m_Parameters[0].toChar());
    }
}

} // end namespace bm
