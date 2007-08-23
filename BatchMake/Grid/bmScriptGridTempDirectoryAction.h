/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptGridTempDirectoryAction.h,v $
  Language:  C++
  Date:      $Date: 2006/10/11 13:13:19 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifndef __ScriptGridTempDirectoryAction_h_
#define __ScriptGridTempDirectoryAction_h_

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <vector>
#include <iostream>
#include "MString.h"
#include "bmScriptAction.h"

namespace bm {

class ScriptGridTempDirectoryAction : public ScriptAction
{
public:
  ScriptGridTempDirectoryAction();
  ~ScriptGridTempDirectoryAction();
  void Execute();
  bool TestParam(ScriptError* error,int linenumber);
  MString Help();

};

} // end namespace bm

#endif
