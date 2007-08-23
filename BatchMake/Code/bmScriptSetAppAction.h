/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptSetAppAction.h,v $
  Language:  C++
  Date:      $Date: 2006/12/18 18:06:25 $
  Version:   $Revision: 1.2 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifndef __ScriptSetAppAction_h_
#define __ScriptSetAppAction_h_

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <vector>
#include <iostream>
#include "MString.h"
#include "bmScriptAction.h"

namespace bm {

class ScriptSetAppAction : public ScriptAction
{
public:
  ScriptSetAppAction();
  ~ScriptSetAppAction();
  void Execute();
  bool TestParam(ScriptError* error,int linenumber);
  MString Help();

private:

  std::string m_Value;

};

} // end namespace bm

#endif
