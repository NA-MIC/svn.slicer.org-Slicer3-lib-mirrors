/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptAddMethodOutputAction.h,v $
  Language:  C++
  Date:      $Date: 2007/09/06 14:09:48 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifndef __ScriptAddMethodOutputAction_h_
#define __ScriptAddMethodOutputAction_h_

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <vector>
#include <iostream>
#include "MString.h"
#include "bmScriptAction.h"

namespace bm {

class ScriptAddMethodOutputAction : public ScriptAction
{
public:
  ScriptAddMethodOutputAction();
  ~ScriptAddMethodOutputAction();
  void Execute();
  bool TestParam(ScriptError* error,int linenumber);
  MString Help();

#ifdef BM_GRID
  void GenerateGrid();
#endif

};

} // end namespace bm

#endif
