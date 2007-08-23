/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptExtractSliceAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/01/28 18:30:48 $
  Version:   $Revision: 1.4 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptExtractSliceAction.h"
#include "SliceExtractor.h"

namespace bm {

ScriptExtractSliceAction::ScriptExtractSliceAction()
: ScriptAction()
{
}

ScriptExtractSliceAction::~ScriptExtractSliceAction()
{
}

bool ScriptExtractSliceAction::TestParam(ScriptError* error,int linenumber)
{
  for (unsigned int i=0;i<m_Parameters.size();i++)
      m_Manager->TestConvert(m_Parameters[i],linenumber);
    
  if (m_Parameters.size() < 2)
  {
    error->SetError(MString("No enough parameter for ExtractSlice"),linenumber);
    return false;
  }   

  return true;
}

MString ScriptExtractSliceAction::Help()
{
  return "ExtractSlice(<3D image filename> <2D image filename> [orientation] [slice] [FROM_MIDDLE])";
}

void ScriptExtractSliceAction::Execute()
{
#ifdef BM_GRID
  if(m_GridModule)
    {
    this->GenerateGrid();
    return;
    }
#endif

  MString m_input = m_Manager->Convert(m_Parameters[0]).removeChar('\'').latin1();
  MString m_Output = m_Manager->Convert(m_Parameters[1]).removeChar('\'').latin1();
   
  SliceExtractor m_sliceextractor;
 
  if(m_Parameters.size()>2)
    {
    MString m_orientation = m_Manager->Convert(m_Parameters[2]).removeChar('\'').latin1();
    m_sliceextractor.SetOrientation(m_orientation.toInt());
    }
  
  if(m_Parameters.size()>3)
    {
    MString m_slice = m_Manager->Convert(m_Parameters[3]).removeChar('\'').latin1();
    m_sliceextractor.SetSlice(m_slice.toInt());
    }

  if(m_Parameters.size()>4 && !strcmp(m_Parameters[4].toChar(),"FROM_MIDDLE"))
    {
    m_sliceextractor.SetFromMiddleSlice(true);
    }

  m_sliceextractor.Extract(m_input.toChar(),m_Output.toChar());
}

} // end namespace bm
