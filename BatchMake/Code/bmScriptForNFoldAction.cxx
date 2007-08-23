/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptForNFoldAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/01/28 18:30:48 $
  Version:   $Revision: 1.2 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/
#include "bmScriptForNFoldAction.h"

#include <fstream>
#include <string>

namespace bm {

ScriptForNFoldAction::ScriptForNFoldAction()
: ScriptAction()
{
  m_Folds.clear();
  m_Action.clear();
}

ScriptForNFoldAction::~ScriptForNFoldAction()
{
}


void ScriptForNFoldAction::AddAction(ScriptAction* action)
{
  m_Action.push_back(action);
}

bool ScriptForNFoldAction::TestParam(ScriptError* error, int linenumber)
{
  if (m_Parameters.size() < 5)
    {
    error->SetError(
           MString("Not enough parameters - ForNFold requires 4 parameters."),
           linenumber);
    return false;
    }

  // Ensure all variables used are defined
  for (unsigned int k=0;k<m_Parameters.size();k++)
    {
    m_Manager->TestConvert(m_Parameters[k], linenumber);
    }

  // Define first arg as the variable that will contain iteration count
  m_Manager->SetTestVariable(m_Parameters[0]);

  // Define second arg as the variable that will contain training data
  m_Manager->SetTestVariable(m_Parameters[1]);

  // Define third arg as the variable that will contain testing data
  m_Manager->SetTestVariable(m_Parameters[2]);

  // I have no idea what Julien is trying to do here
  for (unsigned int i=0;i<m_Action.size();i++)
    {
    // TestParam always returns true...
    if (!m_Action[i]->TestParam(error))
      {
      return false;
      }
    }

  return true;
}


MString ScriptForNFoldAction::Help()
{
  return "ForNFold(<iterCountVar> <iterTrainListVar> <iterTestListVar>"
         " <NFold> <$data>|'value1 value2 ...')"
         " ... EndForNFold(<iterCountVar>)";
}

void ScriptForNFoldAction::CreateLoop()
{ 
  unsigned int numberOfFolds = m_Parameters[3].toInt();

  std::vector<MString> forDataList;
  MString forDataString = m_Manager->Convert(m_Parameters[4]);
  MString forDataItem = "";

  while ((forDataString != "") && (forDataString != forDataItem))
    {
    forDataString = forDataString.removeChar(' ', true);

    if (forDataString.startWith('\''))
      {
      forDataString = forDataString.removeChar('\'', true);
      forDataItem = forDataString.begin("\'");
      forDataString = forDataString.end("\'");

      if (forDataString.length() != 0)
        {
        forDataString = forDataString + 1;
        }
      }
    else
      {
      forDataItem = forDataString.begin(" ");
      forDataString = forDataString.end(" ");
      }
      
    if (forDataItem.length() != 0)
      {
      forDataList.push_back(MString("'") + forDataItem + "'");
      }
    }

  m_Folds.clear();
  m_Folds.resize(numberOfFolds);

  std::vector<unsigned int> foldSize(numberOfFolds);
  unsigned int fs = (int)(forDataList.size()/numberOfFolds);
  unsigned int res = forDataList.size() - (fs * numberOfFolds);
  unsigned int i;
  for(i=0; i<numberOfFolds; i++)
    {
    foldSize[i] = fs;
    if(i<res)
      {
      foldSize[i] += 1;
      }
    }
  
  unsigned int c = 0;
  unsigned int j;
  for(i=0; i<numberOfFolds; i++)
    {
    for(j=0; j<foldSize[i]; j++)
      {
      m_Folds[i].push_back(forDataList[c++]);
      }
    }
}

void ScriptForNFoldAction::Execute()
{

#ifdef BM_GRID
  // We put a grid barrier
  if(m_GridModule)
    {
    m_GridModule->SetGridBarrier();
    }
#endif

  this->CreateLoop();

  MString loopCount;
  MString loopTrain;
  MString loopTest;
  unsigned int i, loop;
  for (loop=0; loop<m_Folds.size(); loop++)
    {
#ifdef BM_GRID
    if(m_GridModule)
      {
      m_GridModule->SetDistributed(true);
      }
#endif

    loopCount = MString((int)loop);
    loopTest = "";
    loopTrain = "";
    for(i=0; i<m_Folds.size(); i++)
      {
      if(i == m_Folds.size()-loop-1)
        {
        loopTest = m_Manager->GetVariableFromParams(m_Folds[i]);
        }
      else
        {
        loopTrain += m_Manager->GetVariableFromParams(m_Folds[i]);
        }
      }
    m_Manager->SetVariable(m_Parameters[0], loopCount);
    m_Manager->SetVariable(m_Parameters[1], loopTrain);
    m_Manager->SetVariable(m_Parameters[2], loopTest);
    for (i=0; i<m_Action.size(); i++)
      {
      if (!m_ProgressManager->IsStop())
        {
        m_Action[i]->Execute();
        }
      }
    }
#ifdef BM_GRID
  if(m_GridModule)
    {
    m_GridModule->SetDistributed(false);
    // Remove Grid barrier
    m_GridModule->RemoveGridBarrier();
    }
#endif
}

void ScriptForNFoldAction::Delete()
{
  for (unsigned int i=0;i<m_Action.size();i++)
    {
    m_Action[i]->Delete();
    delete m_Action[i];
    }
}

} // end namespace bm
