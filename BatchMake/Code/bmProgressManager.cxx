/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmProgressManager.cxx,v $
  Language:  C++
  Date:      $Date: 2007/11/22 16:27:43 $
  Version:   $Revision: 1.3 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmProgressManager.h"

namespace bm {

ProgressManager::ProgressManager()
{
  m_Stop = false;
}

ProgressManager::~ProgressManager()
{
}


void ProgressManager::SetStatus(MString status)
{
  std::cout << "Status: " << status.toChar() << std::endl;
}


void ProgressManager::IsRunning()
{
}

void ProgressManager::AddAction(MString name)
{
}

void ProgressManager::FinishAction(MString output)
{
}

void ProgressManager::AddOutput(MString output)
{
}

void ProgressManager::AddError(MString output)
{
}

bool ProgressManager::IsStop()
{
  return m_Stop;
}

void ProgressManager::SetFinished(MString message)
{

}

void ProgressManager::DisplayOutput(MString message)
{
  std::cout << message.toChar();
}

void ProgressManager::DisplayError(MString message)
{
  std::cout << message.toChar();
}


} // end namespace bm
