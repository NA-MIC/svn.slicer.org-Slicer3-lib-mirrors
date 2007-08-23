/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptForNFoldAction.h,v $
  Language:  C++
  Date:      $Date: 2007/02/15 15:39:13 $
  Version:   $Revision: 1.2 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifndef __ScriptForNFoldAction_h_
#define __ScriptForNFoldAction_h_

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <vector>
#include <iostream>
#include "MString.h"
#include "bmScriptAction.h"

namespace bm {

class ScriptForNFoldAction : public ScriptAction
{
public:
  ScriptForNFoldAction();
  ~ScriptForNFoldAction();
  void AddAction(ScriptAction* action);
  void Execute();
  bool TestParam(ScriptError* error, int linenumber);
  void Delete();
  MString Help();

protected:

  void CreateLoop();

  std::vector<ScriptAction*> m_Action;
  typedef std::vector<MString> MStringVectorType;
  std::vector<MStringVectorType> m_Folds;

};

} // end namespace bm

#endif
