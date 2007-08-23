/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptIfAction.h,v $
  Language:  C++
  Date:      $Date: 2007/01/28 18:30:48 $
  Version:   $Revision: 1.2 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifndef __ScriptIfAction_h_
#define __ScriptIFAction_h_

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <vector>
#include <iostream>
#include "MString.h"
#include "bmScriptAction.h"

namespace bm {

class ScriptIfAction : public ScriptAction
{
public:

  ScriptIfAction();
  ~ScriptIfAction();
  void AddAction(ScriptAction* action);
  void Execute();
  void SetMode(int mode);
  void Delete();
  MString Help();
  bool TestParam(ScriptError* error,int linenumber);

protected:

  std::vector<ScriptAction*> m_ThenAction;
  std::vector<ScriptAction*> m_ElseAction;
  int m_Mode;
  bool TestCurrentCondition(unsigned int startingParameter,unsigned int conditionSize);
};

} // end namespace bm

#endif
