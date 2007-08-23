/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptEditorGUIBase.h,v $
  Language:  C++
  Date:      $Date: 2006/09/07 02:22:14 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifndef __BatchMakeScriptEditorGUIBase_h_
#define __BatchMakeScriptEditorGUIBase_h_
#include <FL/Fl_Pack.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Input.H>

#include <vector>

namespace bm {

class ScriptEditorGUIBase
{
public:
  virtual void OnOpenScript() = 0;
  virtual void OnSaveScript() = 0;
  virtual void OnCompile() = 0;
  virtual void OnExecute() = 0;
  virtual void OnApplicationWrapper() = 0;
  virtual void OnGenerateCondor() = 0;
  virtual void OnGenerateGAD() = 0;
  virtual void OnGenerateScripts() = 0;
  virtual void OnGenerateShell() = 0;
  virtual void hide() = 0;
};

} // end namespace bm

#endif
