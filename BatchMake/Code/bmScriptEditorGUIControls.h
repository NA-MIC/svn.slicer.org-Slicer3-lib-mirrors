/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptEditorGUIControls.h,v $
  Language:  C++
  Date:      $Date: 2007/09/18 19:52:06 $
  Version:   $Revision: 1.5 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifndef __ScriptEditorGUIControls_h_
#define __ScriptEditorGUIControls_h_

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include "bmScriptEditorGUI.h"
#include "bmScriptErrorGUI.h"
#include "bmScriptParser.h"
#include "ApplicationWrapper.h"
#include <vector>
#include "XMLIniIO.h"

#ifdef BM_GRID
  #include "CondorWatcher.h"
#endif

namespace bm {

class ScriptEditorGUIControls : public ScriptEditorGUI
{
public:
  ScriptEditorGUIControls();
  ~ScriptEditorGUIControls();

  void SetBatchMakeBinaryPath(MString applicationpath);

  void Show();
 
  void OnOpenScript();
  void OnSaveScript();
  void OnCompile();
  void OnExecute();
  void OnApplicationWrapper();
  void OnGenerateCondor();
  void OnGenerateGAD();
  void OnGenerateShell();
  void OnGenerateScripts();
  void hide();

  static void Timer(void*);

  XMLIniIO* GetInitFile() {return m_InitFile;}
  void SetWrappedApplicationsPath(const char* app)
    {
    m_WrappedApplicationsPath = app;
    }

  ScriptParser* GetParser() {return m_Parser;}

  Fl_Text_Buffer* m_Errorbuffer;

protected:

  MString m_Filename;
  ScriptParser* m_Parser;
  ScriptErrorGUI* m_Errorgui;
  MString m_ApplicationPath;
  std::vector<ApplicationWrapper*> m_ApplicationList;
  MString m_Title;

#ifdef BM_GRID
  CondorWatcher* m_CondorWatcher;
#endif
  std::string m_SplashBuffer;

  XMLIniIO* m_InitFile;
  std::string  m_WrappedApplicationsPath;
};

} // end namespace bm

#endif
