#include <iostream>
#include <itksys/Process.h>
#include <itksys/SystemTools.hxx>
#include <itkMultiThreader.h>
#include "SystemInfo.h"
#include "metaCommand.h"

struct ThreadStruct
  {
  std::string program;
  float virtualMemUsed;
  float physicalMemUsed;
  float maxVirtualMemUsed;
  float maxPhysicalMemUsed;
  unsigned long iterations;
  bool done;
  };

ITK_THREAD_RETURN_TYPE RunProgramCallback(void * arg)
{
  unsigned int ThreadId = ((itk::MultiThreader::ThreadInfoStruct *)(arg))->ThreadID;
  ThreadStruct* str = (ThreadStruct *)(((itk::MultiThreader::ThreadInfoStruct *)(arg))->UserData);  
  //std::cout << "Running program" << str->program.c_str() << std::endl;

  // Extract the arguments from the command line
  // Warning: Only works for one space between arguments
  std::vector<const char*> args;
  std::vector<std::string> argstemp; // pointer holder
  long int currentpos = 0;
  long int pos = str->program.find(" ",currentpos);
  while(pos != -1)
    {
    std::string test = str->program.substr(currentpos,pos-currentpos);
    argstemp.push_back(test);
    args.push_back(argstemp[argstemp.size()-1].c_str());
    currentpos = pos+1;
    pos = str->program.find(" ",currentpos);
    }
  std::string test = str->program.substr(currentpos,pos-currentpos);
  argstemp.push_back(test);
  args.push_back(argstemp[argstemp.size()-1].c_str());
  args.push_back(0);

  // Run the application
  itksysProcess* gp = itksysProcess_New();
  itksysProcess_SetCommand(gp, &*args.begin());
  //itksysProcess_SetOption(gp,itksysProcess_Option_HideWindow,1);
  itksysProcess_Execute(gp);

  itksysProcess_WaitForExit(gp,0);

  int result = 1;
  switch(itksysProcess_GetState(gp))
    {
    case itksysProcess_State_Exited:
      {
      result = itksysProcess_GetExitValue(gp);
      } break;
    case itksysProcess_State_Error:
      {
      if(strlen(args[0])>0)
        {
        std::cerr << "Error: Could not run " << args[0] << ":\n";
        std::cerr << itksysProcess_GetErrorString(gp) << "\n";
        }
      } break;
    case itksysProcess_State_Exception:
      {
      std::cerr << "Error: " << args[0]
                << " terminated with an exception: "
                << itksysProcess_GetExceptionString(gp) << "\n";
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
  itksysProcess_Delete(gp);

  str->done = 1;
  return ITK_THREAD_RETURN_VALUE;
}

ITK_THREAD_RETURN_TYPE CheckMemoryCallback(void * arg)
{
  unsigned int ThreadId = ((itk::MultiThreader::ThreadInfoStruct *)(arg))->ThreadID;
  ThreadStruct* str = (ThreadStruct *)(((itk::MultiThreader::ThreadInfoStruct *)(arg))->UserData);
  SystemInfo info;

  unsigned long n = 0;
  float virtualMemUsed = 0;
  float physicalMemUsed = 0;
  float maxVirtualMemUsed = 0;
  float maxPhysicalMemUsed = 0;

  while(!str->done)
    {
    itksys::SystemTools::Delay(10);
    info.RunMemoryCheck();
    float v = info.GetTotalVirtualMemory()-info.GetAvailableVirtualMemory();
    float p = info.GetTotalPhysicalMemory()-info.GetAvailablePhysicalMemory();
    virtualMemUsed += v;
    physicalMemUsed += p;
    
    if(v>maxVirtualMemUsed)
      {
      maxVirtualMemUsed = v;
      }

    if(p>maxPhysicalMemUsed)
      {
      maxPhysicalMemUsed = p;
      }

    n++;
    }

  str->virtualMemUsed = virtualMemUsed/n;
  str->physicalMemUsed = physicalMemUsed/n;
  str->maxVirtualMemUsed = maxVirtualMemUsed;
  str->maxPhysicalMemUsed = maxPhysicalMemUsed;
  str->iterations = n;

  return ITK_THREAD_RETURN_VALUE;
}


int main(int argc, char * argv[]) 
{ 
  MetaCommand command;
  
  command.SetName("bmCheckMemory");
  command.SetVersion("1.0");
  command.SetAuthor("Kitware Inc");
  command.SetDescription("Check the mean memory usage of an executable");

  // Required field
  command.AddField("executable","Executable",MetaCommand::STRING);

  // Parsing
  if(!command.Parse(argc,argv))
    {
    return 1;
    }

  std::string executable = command.GetValueAsString("executable");

  itk::MultiThreader::Pointer multithreader = itk::MultiThreader::New();
  multithreader->SetNumberOfThreads(2);

  ThreadStruct str;
  str.done = 0;
  str.virtualMemUsed = 0;
  str.physicalMemUsed = 0;
  str.program = "";
   
  multithreader->SetMultipleMethod(1,RunProgramCallback, &str);
  multithreader->SetMultipleMethod(0,CheckMemoryCallback, &str);

  // Collect the memory info before running the program
  // We run the single shot first
  multithreader->MultipleMethodExecute();
  float maxVirtualMemUsedBeg = str.maxVirtualMemUsed;
  float maxPhysicalMemUsedBeg = str.maxPhysicalMemUsed;  

  // multithread the execution
  str.done = 0;
  str.program = executable;
  str.virtualMemUsed = 0;
  str.physicalMemUsed = 0; 
  multithreader->MultipleMethodExecute();

  //std::cout << "Mean Virtual Memory Used = " << str.virtualMemUsed-virtualMemUsedBeg  << "MB" << std::endl;
  //std::cout << "Mean Physical Memory Used = " << str.physicalMemUsed-physicalMemUsedBeg << "MB" << std::endl;
  std::cout << "Max Virtual Memory Used = " << str.maxVirtualMemUsed-maxVirtualMemUsedBeg  << "MB" << std::endl;
  std::cout << "Max Physical Memory Used = " << str.maxPhysicalMemUsed-maxPhysicalMemUsedBeg << "MB" << std::endl;
  
  if(str.iterations < 10)
    {
    std::cout << "Computed only over " << str.iterations << " iterations" << std::endl;
    }

  return 1; 
}
