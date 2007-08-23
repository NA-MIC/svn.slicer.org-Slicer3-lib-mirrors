/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmApplicationWrapperGUIControls.h,v $
  Language:  C++
  Date:      $Date: 2007/03/07 21:35:32 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifndef __bmApplicationWrapperGUIControls_h_
#define __bmApplicationWrapperGUIControls_h_

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include "bmApplicationWrapperGUI.h"
#include "bmApplicationListGUIControls.h"
#include "ApplicationWrapper.h"
#include <vector>

namespace bm {

class ApplicationListGUIControls;

class ApplicationWrapperGUIControls : public ApplicationWrapperGUI
{
public:
  ApplicationWrapperGUIControls();
  ~ApplicationWrapperGUIControls();
  void Show();
  void OnReject();
  void OnSelectPath();
  void OnSelectType();
  void OnAddEnum();
  void OnSelectEnumList();
  void OnSelectParameters();
  void OnAddParameters();
  void OnSaveModule();
  void OnLoadModule();
  void OnDeleteParameters();
  void OnUpParam();
  void OnDownParam();
  void DisplayExample();
  void Refresh();
  void SetApplicationWrapper(ApplicationWrapper*);
  void SetWrappedApplicationsPath(MString applicationpath);
  void SetApplicationListGUIControls(ApplicationListGUIControls* applicationlistguicontrols);
  void AutomaticCommandLineParsing();

private:
  ApplicationWrapper* m_Applicationwrapper;
  MString m_Currentname;
  MString m_Currentfilename;
  MString m_WrappedApplicationsPath;
  ApplicationListGUIControls* m_Applicationlistguicontrols;
};

} // end namespace bm

#endif
