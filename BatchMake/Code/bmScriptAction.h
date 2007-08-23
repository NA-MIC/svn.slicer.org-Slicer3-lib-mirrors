/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptAction.h,v $
  Language:  C++
  Date:      $Date: 2007/01/28 18:30:48 $
  Version:   $Revision: 1.4 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifndef __ScriptAction_h_
#define __ScriptAction_h_

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <vector>
#include <iostream>
#include "MString.h"
#include "bmScriptActionManager.h"
#include "bmProgressManager.h"
#include "bmScriptError.h"

#ifdef BM_GRID
  #include "bmGrid.h"
#endif

namespace bm {

class ScriptActionManager;

class ScriptAction
{
public:
  ScriptAction();
  virtual ~ScriptAction();
  
  typedef std::vector<MString> ParametersType;
  typedef char* (*Help_Func)(void);

  virtual void SetName(MString name);
  virtual MString GetName();
  virtual void SetParameters(ParametersType parameters);
  virtual void Execute();
  virtual void SetParent(ScriptAction* parent);
  virtual ScriptAction* GetParent();
  virtual void AddAction(ScriptAction* action);
  virtual void SetManager(ScriptActionManager* manager);
  virtual void SetProgressManager(ProgressManager* progressmanager);
  virtual bool TestParam(ScriptError* error,int linenumber=-1);
  virtual void Delete();
  virtual MString Help();

#ifdef BM_GRID
  void SetGridModule(Grid* grid) {m_GridModule = grid;}
#endif

protected:
  ParametersType m_Parameters;
  MString m_Name;
  ScriptAction* m_Parent;
  ScriptActionManager* m_Manager;
  ProgressManager* m_ProgressManager;

#ifdef BM_GRID
  Grid* m_GridModule;
#endif

};

} // end namespace bm

#endif
