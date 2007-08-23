/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmProgressGUIBase.h,v $
  Language:  C++
  Date:      $Date: 2006/09/07 02:22:14 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifndef __BatchMakeProgressGUIBase_h_
#define __BatchMakeProgressGUIBase_h_
#include <FL/Fl_Pack.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Input.H>

namespace bm {

class ProgressGUIBase
{
public:
  virtual void OnCancel() = 0;

};

} // end namespace bm

#endif
