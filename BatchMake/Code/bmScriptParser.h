/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptParser.h,v $
  Language:  C++
  Date:      $Date: 2007/09/18 19:52:06 $
  Version:   $Revision: 1.7 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifndef __ScriptParser_h_
#define __ScriptParser_h_
#include "bmScriptAction.h"
#include "bmScriptError.h"
#include "bmProgressManager.h"

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <vector>
#include <iostream>
#include "MString.h"
#include "bmScriptActionManager.h"

#ifdef BM_GRID
  #include "bmGrid.h"
#endif

namespace bm {

class ScriptParser
{
public:

  /** Constructor */
  ScriptParser();

  /** Destructor */
  ~ScriptParser();
  
  void Load(MString filename);
  bool Parse(MString Line);
  void ParseBuffer(std::string buffer);

  /** Run the given batchmake script (as a buffer) on Condor */
  void RunCondor(std::string buffer,const char* outputDirectory=NULL);

  bool CheckOption(MString param);
  bool AddOption(MString option, MString param);
  ScriptAction::ParametersType GetParams(MString param);
  void AddCodeLine(MString line,unsigned long pos=0);
  bool Parse();
  void SetError(ScriptError* error);
  bool Execute();
  void SetBatchMakeBinaryPath(MString applicationpath);
  void SetScriptPath(MString scriptpath);
  void SetProgressManager(ProgressManager* progressmanager);
  bool Execute(MString filename,unsigned long pos=0);
  bool Compile(MString filename,unsigned long pos=0,bool isInclude = false);
  void LoadWrappedApplication(MString applicationpath);
  std::vector<ApplicationWrapper*>* GetApplicationList();
  void Reset();

#ifdef BM_GRID
  void SetGridModule(Grid* grid) {m_ScriptActionManager->SetGridModule(grid);}
#endif

  /** Return the ScriptActionManager */
  ScriptActionManager* GetScriptActionManager() {return m_ScriptActionManager;}

  std::vector<MString> & GetCode() {return m_Code;}
  void RemoveCodeLine(unsigned int line);

  unsigned long GetLineNumber() {return m_LineNumber;}

  /** Set the current script location. This is used by the include() command */
  std::string GetCurrentFilename() {return m_CurrentFilename;}
  void SetCurrentFilename(const char* filename)
    {
    m_CurrentFilename = filename;
    }

protected:
  
  int m_LineNumber;
  ScriptActionManager* m_ScriptActionManager;
  std::vector<MString> m_Code;
  ScriptError* m_Error;
  MString m_ApplicationPath;
  std::vector<ApplicationWrapper*>* m_ApplicationList;

  std::string m_CurrentFilename; // current path of the current script compiled
};

} // end namespace bm

#endif
