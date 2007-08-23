/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptError.h,v $
  Language:  C++
  Date:      $Date: 2006/09/07 02:22:15 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifndef __ScriptError_h_
#define __ScriptError_h_

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <iostream>
#include "MString.h"

namespace bm {

class ScriptError
{

public:
  
  ScriptError();
  ~ScriptError();
  virtual void SetError(MString error,int line=-1);
  virtual void SetWarning(MString warning,int line=-1);
  virtual void SetStatus(MString status);
  virtual void DisplaySummary();
  virtual int GetError();
  virtual int GetWarning();
  virtual void Reset();

protected:

  int nb_error;
  int nb_warning;

};

} // end namespace bm

#endif
