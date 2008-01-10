/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmLaunch.h,v $
  Language:  C++
  Date:      $Date: 2007/11/22 17:57:01 $
  Version:   $Revision: 1.5 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifndef __Launch_h_
#define __Launch_h_

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <iostream>
#include "MString.h"
#include "bmProgressManager.h"
#include <itkMultiThreader.h>
#include <itksys/SystemTools.hxx>
#include <itksys/Process.h>

namespace bm {

class Launch
{
public:
  Launch();
  ~Launch();
  void Execute(MString _command);

  /** Set/Get the progress manager */
  void SetProgressManager(ProgressManager* progressmanager);
  ProgressManager* GetProgressManager();

  MString GetOutput();
  MString GetError();

   /** Static function used as a "callback" by the MultiThreader.  The threading
   * library will call this routine for each thread, which will delegate the
   * control to ThreadedGetValue(). */
  static ITK_THREAD_RETURN_TYPE ThreaderCallback( void *arg );

  /** Internal structure used for passing image data into the threading library */
  struct ThreadStruct
    {
    Launch* launcher;
    };

  /** Set/Get the execution state */
  void SetExecutionState(int state);
  int GetExecutionState();

protected:

  void RunCommand();

  ProgressManager* m_ProgressManager;
  MString m_Output;
  MString m_Error;
  std::string m_Command;
  itksysProcess* m_Process;

  int m_ExecutionState;
  bool m_KillProcess;

};

} // end namespace bm

#endif
