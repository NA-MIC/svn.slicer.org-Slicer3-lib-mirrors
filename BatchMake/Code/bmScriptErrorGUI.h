/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptErrorGUI.h,v $
  Language:  C++
  Date:      $Date: 2007/01/28 18:30:48 $
  Version:   $Revision: 1.2 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifndef __ScriptErrorGUI_h_
#define __ScriptErrorGUI_h_

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <bmScriptError.h>
#include <iostream>
#include "MString.h"
#include <FL/Fl_Text_Display.H>

namespace bm {

class ScriptErrorGUI : public ScriptError
{
public:
  ScriptErrorGUI();
  ~ScriptErrorGUI();

  void SetTextDisplay(Fl_Text_Display* textdisplay);
  void SetError(MString error,int line=-1);
  void SetWarning(MString warning,int line=-1);
  void SetStatus(MString status);
  void DisplaySummary();

protected:
  Fl_Text_Display* m_TextDisplay;
};

} // end namespace bm


#endif
