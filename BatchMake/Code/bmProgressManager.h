/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmProgressManager.h,v $
  Language:  C++
  Date:      $Date: 2007/01/28 18:30:48 $
  Version:   $Revision: 1.2 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifndef __ProgressManager_h_
#define __ProgressManager_h_

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <iostream>
#include "MString.h"

namespace bm {

class ProgressManager
{
public:
  ProgressManager();
  ~ProgressManager();
  virtual void SetStatus(MString status);
  virtual void IsRunning();
  virtual void AddAction(MString name);
  virtual void FinishAction(MString output);
  virtual void AddOutput(MString output);
  virtual void AddError(MString output);
  virtual bool IsStop();
  virtual void SetFinished(MString message);
  virtual void DisplayOutput(MString message);
  virtual void DisplayError(MString message);

protected:
  bool m_Stop;


};

} // end namespace bm

#endif
