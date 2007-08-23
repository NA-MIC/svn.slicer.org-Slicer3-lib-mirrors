/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmProgressManagerGUI.h,v $
  Language:  C++
  Date:      $Date: 2007/01/28 18:30:48 $
  Version:   $Revision: 1.3 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifndef __ProgressManagerGUI_h_
#define __ProgressManagerGUI_h_

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <iostream>
#include "MString.h"
#include "bmProgressGUIControls.h"
#include "bmProgressManager.h"
#include "FLU/Flu_Tree_Browser.h"
#include "FL/Fl_Progress.h"

namespace bm {

class ProgressGUIControls;

class ProgressManagerGUI : public ProgressManager
{
public:
  ProgressManagerGUI();
  ~ProgressManagerGUI();
  void SetStatus(MString status);
  void IsRunning();
  void SetProgressGUI(ProgressGUIControls* ProgressGUI);
  void AddAction(MString name);
  void FinishAction(MString output);
  void AddOutput(MString output);
  void AddError(MString output);
  void Stop();
  void SetFinished(MString message);
  void DisplayOutput(MString message);
  void DisplayError(MString message);
  void DisplayInfo(MString message);

protected:
  ProgressGUIControls* m_ProgressGUI;
  Flu_Tree_Browser::Node* m_CurrentNode;
  Flu_Tree_Browser::Node* m_Outputnode;
  Flu_Tree_Browser::Node* m_Errornode;
  Fl_Progress* m_Progress;
  int m_Offset;


};


} // end namespace bm

#endif
