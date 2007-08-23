/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmProgressGUIControls.h,v $
  Language:  C++
  Date:      $Date: 2007/01/28 18:30:48 $
  Version:   $Revision: 1.3 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifndef __ProgressGUIControls_h_
#define __ProgressGUIControls_h_

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include "bmProgressGUI.h"
#include "bmProgressManager.h"
#include "bmProgressManagerGUI.h"

namespace bm {

class ProgressManagerGUI;

class ProgressGUIControls : public ProgressGUI
{
public:
  ProgressGUIControls();
  ~ProgressGUIControls();
  void Show();
  void OnCancel();
  void SetProgressManagerGUI(ProgressManagerGUI* progressmanagergui);
  Fl_Text_Buffer* m_Buffer;

protected:
  ProgressManagerGUI* m_ProgressManagergui;
};

}

#endif
