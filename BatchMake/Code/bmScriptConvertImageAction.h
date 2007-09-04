/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptConvertImageAction.h,v $
  Language:  C++
  Date:      $Date: 2007/08/28 14:15:01 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifndef __ScriptConvertImageAction_h_
#define __ScriptConvertImageAction_h_

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <vector>
#include <iostream>
#include "MString.h"
#include "bmScriptAction.h"

namespace bm {

class ScriptConvertImageAction : public ScriptAction
{
public:
  ScriptConvertImageAction();
  ~ScriptConvertImageAction();
  void Execute();
  MString Help();
  bool TestParam(ScriptError* error,int linenumber);

#ifdef BM_GRID
  void GenerateGrid();
#endif

protected:

};

} // end namespace bm

#endif
