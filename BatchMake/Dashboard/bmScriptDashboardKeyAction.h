/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptDashboardKeyAction.h,v $
  Language:  C++
  Date:      $Date: 2007/05/21 23:39:30 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifndef __ScriptDashboardKeyAction_h_
#define __ScriptDashboardKeyAction_h_

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <vector>
#include <iostream>
#include "MString.h"
#include "bmScriptAction.h"

namespace bm {

class ScriptDashboardKeyAction : public ScriptAction
{
public:
  ScriptDashboardKeyAction();
  ~ScriptDashboardKeyAction();
  void Execute();
  bool TestParam(ScriptError* error,int linenumber);
  MString Help();

};

} // end namespace bm

#endif
