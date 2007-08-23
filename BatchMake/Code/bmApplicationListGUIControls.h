/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmApplicationListGUIControls.h,v $
  Language:  C++
  Date:      $Date: 2007/03/07 21:35:32 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifndef __ApplicationListGUIControls_h_
#define __ApplicationListGUIControls_h_

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include "bmApplicationListGUI.h"
#include "ApplicationWrapper.h"
#include <vector>
#include <vector>
#include "XMLIniIO.h"

namespace bm {

class ScriptEditorGUIControls;

class ApplicationListGUIControls : public ApplicationListGUI
{

public:
  ApplicationListGUIControls();
  ~ApplicationListGUIControls();

  void SetWrappedApplicationPath(MString applicationpath);
  void SetApplicationList(std::vector<ApplicationWrapper*>*);
  void Show();
  void OnAccept();
  void OnEdit();
  void OnNew();
  void OnRemove();
  void OnSelect();
  void OnSelectDir();
  void OnChangePath();
  void Update(MString name);
  void UpdateNew(ApplicationWrapper* wrapper);

  /** Pass a pointer to the ini file so we can modify it */
  void SetEditorGUI(ScriptEditorGUIControls* editor) {m_EditorGUI = editor;}

private:
  std::vector<ApplicationWrapper*>* m_applicationlist;
  MString m_WrappedApplicationsPath;
  ScriptEditorGUIControls* m_EditorGUI;

};

} // end namepsace bm

#endif
