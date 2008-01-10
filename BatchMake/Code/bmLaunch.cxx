/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmLaunch.cxx,v $
  Language:  C++
  Date:      $Date: 2007/11/22 17:57:01 $
  Version:   $Revision: 1.10 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmLaunch.h"
#include <fstream>

namespace bm {

Launch::Launch()
{
  m_ProgressManager = 0;
  m_Output = "";
  m_Error = "";
  m_ExecutionState = 0;
  m_Command = "";
  m_Process = 0;
  m_KillProcess = false;
}

Launch::~Launch()
{
}

/** Set the execution state */
void Launch::SetExecutionState(int state)
{
  m_ExecutionState = state;
}

/** Get the execution state */
int Launch::GetExecutionState()
{
  return m_ExecutionState;
}

/** Set the progress manager */
void Launch::SetProgressManager(ProgressManager* progressmanager)
{
  m_ProgressManager = progressmanager;
}

/** Get the progress manager */
ProgressManager* Launch::GetProgressManager()
{
  return m_ProgressManager;
}

/** Thread callback */
ITK_THREAD_RETURN_TYPE Launch::ThreaderCallback( void * arg )
{
  ThreadStruct *threadStruct;
  int threadId;

  threadId = ((itk::MultiThreader::ThreadInfoStruct *)(arg))->ThreadID;  
  threadStruct = (ThreadStruct *)(((itk::MultiThreader::ThreadInfoStruct *)(arg))->UserData);

  Launch* launcher = threadStruct->launcher;

  // Launch the process in the first thread
  launcher->SetExecutionState(1); // running
  launcher->RunCommand();
  launcher->SetExecutionState(2); // done
  return ITK_THREAD_RETURN_VALUE;
}

void Launch::RunCommand()
{
  // Extract the arguments from the command line
  // Warning: Only works for one space between arguments
  // Extract the arguments from the command line
  // Warning: Only works for one space between arguments
  std::vector<std::string> arglist; // keep the pointers in the list
  std::vector<const char*> args;
  std::string command = m_Command;
  long int start = -1;
  long int pos = command.find(' ',0);
  while(pos!=-1)
    {
    bool inQuotes = false;
    // Check if we are between quotes
    long int b0 = command.find('"',0);
    long int b1 = command.find('"',b0+1);
    while(b0 != -1 && b1 != -1 && b1>b0)
      {
      if(pos>b0 && pos<b1)
        {
        inQuotes = true;
        break;
        }
      b0 = command.find('"',b1+1);
      b1 = command.find('"',b0+1);
      }
    
    if(!inQuotes)
      {
      std::string arg = command.substr(start+1,pos-start-1);

      // Remove the quotes if any
      long int quotes = arg.find('"');
      while(quotes != -1)
        {
        arg.erase(quotes,1);
        quotes = arg.find('"');
        }
      arglist.push_back(arg);  
      start = pos;
      }
    pos = command.find(' ',pos+1);
    }
  std::string lastArg = command.substr(start+1,command.size()-start-1);
  arglist.push_back(lastArg.c_str());

  unsigned int i=0;
  for(i=0;i<arglist.size();i++)
    {
    if(arglist[i].size() >0 && arglist[i] != " ")
      {
      args.push_back(arglist[i].c_str());
      }
    }
  args.push_back(0);

  // Run the application
  m_Process = itksysProcess_New();
  //itksysProcess_SetPipeShared(gp, itksysProcess_Pipe_STDOUT, 1);
  //itksysProcess_SetPipeShared(gp, itksysProcess_Pipe_STDERR, 1);
  itksysProcess_SetCommand(m_Process, &*args.begin());
  itksysProcess_SetOption(m_Process,itksysProcess_Option_HideWindow,1);

  itksysProcess_Execute(m_Process);
  
  double timeout = 0.5;
  char* data = NULL;
  int length;

  while(int pipeid = itksysProcess_WaitForData(m_Process,&data,&length,&timeout))
    {
    if(pipeid == itksysProcess_Pipe_STDERR)
      {
      for(int i=0;i<length;i++)
        {
        m_Error += data[i];
        }
      }
    else
      {
      for(int i=0;i<length;i++)
        {
        if(data)
          {
          m_Output += data[i];
          }
        }
      }
    if(m_KillProcess)
      {
      itksysProcess_Kill(m_Process);
      break;
      }
    timeout = 0.5;
    }

  itksysProcess_WaitForExit(m_Process,0);
  int result = 1;
  switch(itksysProcess_GetState(m_Process))
    {
    case itksysProcess_State_Exited:
      {
      result = itksysProcess_GetExitValue(m_Process);
      } break;
    case itksysProcess_State_Error:
      {
      std::cerr << "Error: Could not run " << args[0] << ":\n";
      std::cerr << itksysProcess_GetErrorString(m_Process) << "\n";
      } break;
    case itksysProcess_State_Exception:
      {
      std::cerr << "Error: " << args[0]
                << " terminated with an exception: "
                << itksysProcess_GetExceptionString(m_Process) << "\n";
      } break;
    case itksysProcess_State_Starting:
    case itksysProcess_State_Executing:
    case itksysProcess_State_Expired:
    case itksysProcess_State_Killed:
      {
      // Should not get here.
      std::cerr << "Unexpected ending state after running " << args[0]
                << std::endl;
      } break;
    }
  itksysProcess_Delete(m_Process); 
}

void Launch::Execute(MString command)
{
  m_Output = "";
  m_Error = "";
  m_Command = command.toChar();
  m_KillProcess = false;

  itk::MultiThreader::Pointer threader = itk::MultiThreader::New();
  threader->SetNumberOfThreads(1);

  ThreadStruct str;
  str.launcher = this;
  int threadid = threader->SpawnThread(this->ThreaderCallback, &str);
  
  while(this->GetExecutionState() != 2)
    {
    if(m_ProgressManager)
       {
       m_ProgressManager->IsRunning(); // does the Fl::check();
       if (m_ProgressManager->IsStop())
         {
         m_KillProcess = true;
         break;
         }
       }
     itksys::SystemTools::Delay(300); //refresh rate
     }
  
  threader->TerminateThread(threadid);
  // Restore the states
  m_ExecutionState = 0;
}


MString Launch::GetOutput()
{
  return m_Output;
}


MString Launch::GetError()
{
  return m_Error;
}

} // end namespace bm
