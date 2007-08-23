/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptRandomizeAction.h,v $
  Language:  C++
  Date:      $Date: 2006/09/07 02:22:15 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifndef __ScriptRandomizeAction_h_
#define __ScriptRandomizeAction_h_

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <vector>
#include <iostream>
#include "MString.h"
#include "bmScriptAction.h"

namespace bm {

class ScriptRandomizeAction : public ScriptAction
{
public:
  ScriptRandomizeAction();
  ~ScriptRandomizeAction();
  void Execute();
  bool TestParam(ScriptError* error,int linenumber);
  MString Help();

};

} // end namespace bm

#endif
