/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: CondorWatcher.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/25 18:50:42 $
  Version:   $Revision: 1.2 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "CondorWatcher.h"
#include "FLU/flu_pixmaps.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>


#ifdef WIN32
  #include <Windows.h>
  #include <fcntl.h>
  #include <errno.h>
  #include <io.h>
  #include <stdio.h>
  #include <process.h>
#else
  #include <unistd.h>
  #include <sys/wait.h>
  #include <fcntl.h>
  #include <errno.h>
#endif


//CondorWatcher* watcher;

void timer_callback(void* watcher)
{
  static_cast<CondorWatcher*>(watcher)->Update();
  Fl::repeat_timeout(10,timer_callback, watcher);
}

/** Constructor */
CondorWatcher::CondorWatcher()
{
  Jobs->insertion_mode(FLU_INSERT_BACK);
}

/** Destructor */
CondorWatcher::~CondorWatcher()
{
}

std::string CondorWatcher::RemoveExtraChar(std::string & buffer)
{
  // First we remove unused char
  long int pos = buffer.find("&gt;");
  if(pos != -1)
    {
    buffer.erase(pos,4);
    }
  pos = buffer.find("&lt;");
  if(pos != -1)
    {
    buffer.erase(pos,4);
    }

  pos = buffer.find("<b v=\"");
  if(pos != -1)
    {
    buffer.replace(pos,6,"");
    }

  pos = buffer.find("\"/>");
  if(pos != -1)
    {
    buffer.replace(pos,3,"");
    }

  // Then we remove the XML tag
  std::vector<std::string> charList;
  charList.push_back("s");
  charList.push_back("i");
  charList.push_back("r");
  charList.push_back("e");

  std::vector<std::string>::const_iterator it = charList.begin();

  while(it != charList.end())
    {
    std::string word = "<";
    word += (*it).c_str();
    word += ">";
    long int pos = buffer.find(word.c_str());
    if(pos != -1)
      {
      std::string word2 = "</";
      word2 += (*it).c_str();
      word2 += ">";
      long pos2 = buffer.find(word2.c_str(),pos);
      if(pos2 != -1)
        {
        std::string buf = buffer.substr(pos+word.size(),pos2-pos-word.size());
        // Replace all '/' by '\'
        long posslash = buf.find("/");
        while(posslash != -1)
          {
          buf.replace(posslash,1,"\\");
          posslash = buf.find("/",posslash+1);
          }
        
        return buf;
        }
      return buffer;
      }
    it++;
    }

  return buffer;
}

/** Parse the condor_status xml result */
void CondorWatcher::ParseStatus(std::string & buffer)
{
  // Each machine is between <c> </c>
  long pos = buffer.find("<c>");
  if(pos == -1) 
    {
    return;
    }
  long pos1 = buffer.find("</c>",pos);
  if(pos1 == -1) 
    {
    return;
    }
  
  // We loop to all the machines in the pool
  while(pos1 != -1)
    {
    machine m;
    m.StatusList.push_back(PairType("MyType",""));
    m.StatusList.push_back(PairType("TargetType",""));
    m.StatusList.push_back(PairType("Name",""));
    m.StatusList.push_back(PairType("Machine",""));
    m.StatusList.push_back(PairType("Rank",""));
    m.StatusList.push_back(PairType("CpuBusy",""));
    m.StatusList.push_back(PairType("COLLECTOR_HOST_STRING",""));
    m.StatusList.push_back(PairType("CondorVersion",""));
    m.StatusList.push_back(PairType("CondorPlatform",""));
    m.StatusList.push_back(PairType("VirtualMachineID",""));
    m.StatusList.push_back(PairType("VirtualMemory",""));
    m.StatusList.push_back(PairType("Disk",""));
    m.StatusList.push_back(PairType("CondorLoadAvg",""));
    m.StatusList.push_back(PairType("LoadAvg",""));
    m.StatusList.push_back(PairType("KeyboardIdle",""));
    m.StatusList.push_back(PairType("ConsoleIdle",""));
    m.StatusList.push_back(PairType("Memory",""));
    m.StatusList.push_back(PairType("Cpus",""));
    m.StatusList.push_back(PairType("StartdIpAddr",""));
    m.StatusList.push_back(PairType("Arch",""));
    m.StatusList.push_back(PairType("OpSys",""));
    m.StatusList.push_back(PairType("UidDomain",""));
    m.StatusList.push_back(PairType("FileSystemDomain",""));
    m.StatusList.push_back(PairType("Subnet",""));
    m.StatusList.push_back(PairType("HasIOProxy",""));
    m.StatusList.push_back(PairType("TotalVirtualMemory",""));
    m.StatusList.push_back(PairType("TotalDisk",""));
    m.StatusList.push_back(PairType("KFlops",""));
    m.StatusList.push_back(PairType("Mips",""));
    m.StatusList.push_back(PairType("LastBenchmark",""));
    m.StatusList.push_back(PairType("TotalLoadAvg",""));
    m.StatusList.push_back(PairType("TotalCondorLoadAvg",""));
    m.StatusList.push_back(PairType("ClockMin",""));
    m.StatusList.push_back(PairType("ClockDay",""));
    m.StatusList.push_back(PairType("TotalVirtualMachines",""));
    m.StatusList.push_back(PairType("HasFileTransfer",""));
    m.StatusList.push_back(PairType("HasMPI",""));
    m.StatusList.push_back(PairType("HasJICLocalConfig",""));
    m.StatusList.push_back(PairType("HasJICLocalStdin",""));
    m.StatusList.push_back(PairType("StarterAbilityList",""));
    m.StatusList.push_back(PairType("CpuBusyTime",""));
    m.StatusList.push_back(PairType("CpuIsBusy",""));
    m.StatusList.push_back(PairType("State",""));
    m.StatusList.push_back(PairType("EnteredCurrentState",""));
    m.StatusList.push_back(PairType("Activity",""));
    m.StatusList.push_back(PairType("EnteredCurrentActivity",""));
    m.StatusList.push_back(PairType("Start",""));
    m.StatusList.push_back(PairType("Requirements",""));
    m.StatusList.push_back(PairType("CurrentRank",""));
    m.StatusList.push_back(PairType("DaemonStartTime",""));
    m.StatusList.push_back(PairType("UpdateSequenceNumber",""));
    m.StatusList.push_back(PairType("MyAddress",""));
    m.StatusList.push_back(PairType("LastHeardFrom",""));
    m.StatusList.push_back(PairType("UpdatesTotal",""));
    m.StatusList.push_back(PairType("UpdatesSequenced",""));
    m.StatusList.push_back(PairType("UpdatesLost",""));
    m.StatusList.push_back(PairType("UpdatesHistory",""));

    std::string mach = buffer.substr(pos,pos1-pos);
    long p = 0;
    unsigned int i;
    for(i=0;i<m.StatusList.size();i++)
      {
      std::string word = "<a n=\"";
      word += m.StatusList[i].first;
      word += "\">";
      p = mach.find(word.c_str(),p+1);
    
      if(p != -1)
        {
        long p1 = mach.find("</a>",p+1);
        if(p1 != -1)
          {
          std::string tmps = mach.substr(p+word.size(),p1-p-word.size());
          m.StatusList[i].second = RemoveExtraChar(tmps);
          p = p1;
          }
        }
      }

     m_MachineVector.push_back(m);


    /*
    for(i=0;i<m_StatusList.size();i++)
      {
      std::cout << m_StatusList[i].first.c_str() << " = " << m_StatusList[i].second.c_str() << std::endl;
      }*/

    pos = buffer.find("<c>",pos1+1);
    if(pos == -1) 
      {
      return;
      }
    pos1 = buffer.find("</c>",pos+1);
    }

}


/** Update the list of clients in the condor pool */
void CondorWatcher::ParseQueue(std::string & buffer)
{
  // Each job is between <c> </c>
  long pos = buffer.find("<c>");
  if(pos == -1) 
    {
    return;
    }
  long pos1 = buffer.find("</c>",pos);
  if(pos1 == -1) 
    {
    return;
    }
  
  // We loop to all the machines in the pool
  while(pos1 != -1)
    {
    job m;
    m.StatusList.push_back(PairType("MyType",""));
    m.StatusList.push_back(PairType("TargetType",""));
    m.StatusList.push_back(PairType("ClusterId",""));
    m.StatusList.push_back(PairType("QDate",""));
    m.StatusList.push_back(PairType("CompletionDate",""));
    m.StatusList.push_back(PairType("Owner",""));
    m.StatusList.push_back(PairType("NTDomain",""));
    m.StatusList.push_back(PairType("RemoteWallClockTime",""));
    m.StatusList.push_back(PairType("LocalUserCpu",""));
    m.StatusList.push_back(PairType("LocalSysCpu",""));
    m.StatusList.push_back(PairType("RemoteUserCpu",""));
    m.StatusList.push_back(PairType("RemoteSysCpu",""));
    m.StatusList.push_back(PairType("ExitStatus",""));
    m.StatusList.push_back(PairType("NumCkpts",""));
    m.StatusList.push_back(PairType("NumRestarts",""));
    m.StatusList.push_back(PairType("NumSystemHolds",""));
    m.StatusList.push_back(PairType("CommittedTime",""));
    m.StatusList.push_back(PairType("TotalSuspensions",""));
    m.StatusList.push_back(PairType("LastSuspensionTime",""));
    m.StatusList.push_back(PairType("CumulativeSuspensionTime",""));
    m.StatusList.push_back(PairType("ExitBySignal",""));
    m.StatusList.push_back(PairType("CondorVersion",""));
    m.StatusList.push_back(PairType("CondorPlatform",""));
    m.StatusList.push_back(PairType("Iwd",""));
    m.StatusList.push_back(PairType("JobUniverse",""));
    m.StatusList.push_back(PairType("Cmd",""));
    m.StatusList.push_back(PairType("MinHosts",""));
    m.StatusList.push_back(PairType("MaxHosts",""));
    m.StatusList.push_back(PairType("CurrentHosts",""));
    m.StatusList.push_back(PairType("WantRemoteSyscalls",""));
    m.StatusList.push_back(PairType("WantCheckpoint",""));
    m.StatusList.push_back(PairType("JobStatus",""));
    m.StatusList.push_back(PairType("EnteredCurrentStatus",""));
    m.StatusList.push_back(PairType("JobPrio",""));
    m.StatusList.push_back(PairType("User",""));
    m.StatusList.push_back(PairType("NiceUser",""));
    m.StatusList.push_back(PairType("Env",""));
    m.StatusList.push_back(PairType("JobNotification",""));
    m.StatusList.push_back(PairType("UserLog",""));
    m.StatusList.push_back(PairType("CoreSize",""));
    m.StatusList.push_back(PairType("Rank",""));
    m.StatusList.push_back(PairType("In",""));
    m.StatusList.push_back(PairType("TransferIn",""));
    m.StatusList.push_back(PairType("Out",""));
    m.StatusList.push_back(PairType("Err",""));
    m.StatusList.push_back(PairType("BufferSize",""));
    m.StatusList.push_back(PairType("BufferBlockSize",""));
    m.StatusList.push_back(PairType("ShouldTransferFiles",""));
    m.StatusList.push_back(PairType("WhenToTransferOutput",""));
    m.StatusList.push_back(PairType("TransferFiles",""));
    m.StatusList.push_back(PairType("ImageSize",""));
    m.StatusList.push_back(PairType("ExecutableSize",""));
    m.StatusList.push_back(PairType("DiskUsage",""));
    m.StatusList.push_back(PairType("Requirements",""));
    m.StatusList.push_back(PairType("PeriodicHold",""));
    m.StatusList.push_back(PairType("PeriodicRelease",""));
    m.StatusList.push_back(PairType("PeriodicRemove",""));
    m.StatusList.push_back(PairType("OnExitHold",""));
    m.StatusList.push_back(PairType("OnExitRemove",""));
    m.StatusList.push_back(PairType("LeaveJobInQueue",""));
    m.StatusList.push_back(PairType("Args",""));
    m.StatusList.push_back(PairType("ProcId",""));
    m.StatusList.push_back(PairType("ServerTime",""));
    
    std::string job = buffer.substr(pos,pos1-pos);
    long p = 0;
    unsigned int i;
    for(i=0;i<m.StatusList.size();i++)
      {
      std::string word = "<a n=\"";
      word += m.StatusList[i].first;
      word += "\">";
      p = job.find(word.c_str(),p+1);
    
      if(p != -1)
        {
        long p1 = job.find("</a>",p+1);
        if(p1 != -1)
          {
          std::string tmps = job.substr(p+word.size(),p1-p-word.size());
          m.StatusList[i].second = RemoveExtraChar(tmps);
          p = p1;
          }
        }
      }

     m_JobVector.push_back(m);

    pos = buffer.find("<c>",pos1+1);
    if(pos == -1) 
      {
      return;
      }
    pos1 = buffer.find("</c>",pos+1);
    }

}


/** Update the list of clients in the condor pool */
void CondorWatcher::UpdateClients()
{
  std::string clients = this->Run("condor_status -xml");
  this->ParseStatus(clients);
  std::string jobs = this->Run("condor_q -xml");
  this->ParseQueue(jobs);
}

/** Update the current display */
void CondorWatcher::Update()
{
  if(m_MachineVector.size()>0)
    {
    m_MachineVector.clear();
    }

  if(m_JobVector.size()>0)
    {
    m_JobVector.clear();
    }

  this->UpdateClients();
  this->Clients->clear();
  this->Clients->always_open(true);
  Flu_Tree_Browser::Node* n = this->Clients->set_root("Clients");
  n->branch_icon(new Fl_Pixmap( (char*const*)network_drive_xpm ));
  n->label_size(11);

  // Update the list of Clients
  std::vector<machine>::const_iterator it = m_MachineVector.begin();

  while(it != m_MachineVector.end())
    { 
    std::string name = (*it).StatusList[2].second;
    long pos = name.find("@");
    if(pos != -1)
      {
      name.replace(pos,1,"_");
      }

    name += " (";
    name += (*it).StatusList[42].second;
    name += "-";
    name += (*it).StatusList[44].second;
    name += "-";

    char* loadAvg = new char[20];
    sprintf(loadAvg,"%1.4f",atof((*it).StatusList[13].second.c_str()));
    name += loadAvg;
    delete loadAvg;
    name += ")";
 
    Flu_Tree_Browser::Node* c = this->Clients->add_branch(name.c_str());
    c->branch_icon(new Fl_Pixmap( (char*const*)computer_xpm ));
    c->label_size(11);

    std::vector<PairType>::const_iterator it2 = (*it).StatusList.begin();
    while(it2 != (*it).StatusList.end())
      {
      std::string desc = (*it2).first;
      desc += " = ";
      desc += (*it2).second;
      Flu_Tree_Browser::Node* l = this->Clients->add_leaf(c,desc.c_str());
      l->label_size(11);
      it2++;
      }
    
    it++;
    }

  // Update the list of Jobs
  std::vector<job>::const_iterator it_job = m_JobVector.begin();
  
  this->Jobs->clear();
  this->Jobs->always_open(true);
  n = this->Jobs->set_root("Jobs");
  n->branch_icon(new Fl_Pixmap( (char*const*)home_xpm ));
  n->label_size(11);

  while(it_job != m_JobVector.end())
    { 
    std::string name = (*it_job).StatusList[2].second;
    name += " : ";
    
    long poslast = (*it_job).StatusList[25].second.find_last_of("\\");
      
    if(poslast != -1)
      {
      std::string cmd = (*it_job).StatusList[25].second;
      name += cmd.substr(poslast+1,cmd.size()-poslast-1);
      }
    else
      {
      name += (*it_job).StatusList[25].second;
      }

    int working = atoi((*it_job).StatusList[31].second.c_str());
    name += " (";
    if(working == 1)
      {
      name += "idle";
      }
    else if(working == 2)
      {
      name += "busy";
      }
    
    name += ")";


    Flu_Tree_Browser::Node* c = this->Jobs->add_branch(name.c_str());
    if(working == 1)
      {
      c->branch_icon(new Fl_Pixmap( (char*const*)reddot_xpm));
      }
    else if(working == 2)
      {
      c->branch_icon(new Fl_Pixmap( (char*const*)greendot_xpm));
      }
    c->label_size(11);

    std::vector<PairType>::const_iterator it2 = (*it_job).StatusList.begin();
    while(it2 != (*it_job).StatusList.end())
      {
      std::string desc = (*it2).first;
      desc += " = ";
      desc += (*it2).second;
      Flu_Tree_Browser::Node* l = this->Jobs->add_leaf(c,desc.c_str());
      l->label_size(11);
      it2++;
      }
    
    it_job++;
    }
/*static int i = 0;
  
  if(i%2 == 0)
    {
    Flu_Tree_Browser::Node* n = this->Clients->get_root();
    n->branch_icon(new Fl_Pixmap( (char*const*)computer_xpm ));
    }
  else
    {
    Flu_Tree_Browser::Node* n = this->Clients->get_root();
    n->branch_icon(new Fl_Pixmap( (char*const*)reddot_xpm ));
    }
    i++;
  */
  this->Clients->redraw();
  this->Jobs->redraw();

  Fl::check();
  
}


/** Watching loop */
void CondorWatcher::Watch()
{
  // Init the browsers
  this->Clients->clear();
  this->Clients->always_open(true);
  Flu_Tree_Browser::Node* n = this->Clients->set_root("Clients");
  n->branch_icon(new Fl_Pixmap( (char*const*)network_drive_xpm ));
  n->label_size(11);

  this->Jobs->clear();
  this->Jobs->always_open(true);
  n = this->Jobs->set_root("Jobs");
  n->branch_icon(new Fl_Pixmap( (char*const*)home_xpm ));
  n->label_size(11);

  Fl::add_timeout(1.0, timer_callback, this);
  //Fl::run();
}


/** Launch an application and get the result in a buffer */
std::string CondorWatcher::Run(const char* program)
{
  //std::cout << "Running = " << program << std::endl;
  std::string m_output = "";
//  std::string m_error = "";

#ifdef WIN32

  char buffer[BUFSIZ+1];
//  char buffer_err[BUFSIZ+1];

  STARTUPINFO si;
  PROCESS_INFORMATION pi;
 
  SECURITY_ATTRIBUTES tmpSec;
  ZeroMemory( &tmpSec, sizeof(tmpSec) );
  tmpSec.nLength = sizeof(tmpSec);
  tmpSec.bInheritHandle = true;
  HANDLE hReadPipe;
  HANDLE hWritePipe;
//  HANDLE hReadErrorPipe;
//  HANDLE hWriteErrorPipe;

  SECURITY_ATTRIBUTES  sa;
  ZeroMemory( &sa, sizeof(sa) );
  sa.nLength = sizeof(sa);
  sa.bInheritHandle = true;

  SECURITY_ATTRIBUTES  sa2;
  ZeroMemory( &sa2, sizeof(sa2) );
  sa2.nLength = sizeof(sa);
  sa2.bInheritHandle = true;

  CreatePipe(&hReadPipe,&hWritePipe,&sa,0);
//  CreatePipe(&hReadErrorPipe,&hWriteErrorPipe,&sa2,0);


  ZeroMemory( &si, sizeof(si) );
  si.cb = sizeof(si);
  si.dwFlags = STARTF_USESTDHANDLES;
  si.hStdOutput = hWritePipe; //output;
//  si.hStdError = hWriteErrorPipe; //error;
  ZeroMemory( &pi, sizeof(pi) );

  memset(buffer,'\0',sizeof(buffer)); 
//  memset(buffer_err,'\0',sizeof(buffer_err)); 
  
  // Start the child process. 
  if( !CreateProcess( NULL,       // No module name (use command line). 
      (char*)program,  // Command line. 
      NULL,                       // Process handle not inheritable. 
      NULL,                       // Thread handle not inheritable. 
      TRUE,                       // Set handle inheritance to FALSE. 
      0,                          //CREATE_NEW_PROCESS_GROUP,  // No creation flags. 
      NULL,                       // Use parent's environment block. 
      NULL,                       // Use parent's starting directory. 
      &si,                        // Pointer to STARTUPINFO structure.
      &pi )                       // Pointer to PROCESS_INFORMATION structure.
  ) 
  {
    std::cout << "CondorWatcher - CreateProcess failed!" << std::endl;
//    m_error = "CondorWatcher - CreateProcess failed!";
    return "";
  }

  CloseHandle(hWritePipe);
//  CloseHandle(hWriteErrorPipe);

   // Wait until child process exits.
  bool m_run = true;

  unsigned int i=0;
  while(m_run)
  {
  unsigned long m_nbread = 0;
  unsigned long m_nberrorread = 0;
  unsigned long m_nbtoread = 512;
  unsigned long m_nbtoreaderror = 512;
  int m_read = 0;
  int m_readerror = 0;
  unsigned long m_nbreaded = 0;
  unsigned long m_nberrorreaded = 0;

  if (WaitForSingleObject( pi.hProcess, 500 )  == 0)
    {
    m_run = false;
    }

  PeekNamedPipe(hReadPipe,buffer,sizeof(buffer),&m_nbtoread,&m_nbread,NULL); 
//  PeekNamedPipe(hReadErrorPipe,buffer_err,sizeof(buffer_err),&m_nbtoreaderror,&m_nberrorread,NULL);

  int val = ReadFile(hReadPipe,buffer,512,&m_nbreaded,NULL); 
  while (m_nbread > 0 && val)
    {
    for (unsigned int k=0;k<m_nbreaded;k++)
      {
      if (buffer[k] != '\r')
        {
        m_output += buffer[k];
        }
      }
    memset(buffer,'\0',sizeof(buffer));
    val = ReadFile(hReadPipe,buffer,512,&m_nbreaded,NULL);
    }

//   if (m_nberrorread != 0)
//       ReadFile(hReadErrorPipe, buffer_err,512,&m_nberrorreaded,NULL); 
  

    /*if (m_nberrorreaded != 0)
    {
    
      for (unsigned int k=0;k<strlen(buffer_err);k++)
      {
        if (buffer[k] != '\r') 
          m_error += buffer_err[k];
      }

      memset(buffer_err,'\0',sizeof(buffer_err)); 
    }*/

  } 

  //Terminate Process
  TerminateProcess(pi.hProcess,0);

  CloseHandle(hReadPipe);
  //CloseHandle(hReadErrorPipe);

  CloseHandle( pi.hProcess );
  CloseHandle( pi.hThread );

  return m_output;

#else  
  int stdin_pipe[2];
  int stdout_pipe[2];
//  int stderr_pipe[2];
  char buffer[BUFSIZ+1];
//  char buffer_err[BUFSIZ+1];
  int fork_result;
  int data_processed;
//  int data_processed_err;
  int nchars = 0;
int status = 0;

  memset(buffer,'\0',sizeof(buffer));
 //memset(buffer_err,'\0',sizeof(buffer)); 

   if ( (pipe(stdin_pipe)==0)   
        && (pipe(stdout_pipe)==0)
  //      && (pipe(stderr_pipe)==0)
      )
   {
     fork_result = fork();
     if (fork_result == -1)
     {
       std::cerr << "Create Process failed (Pipe error) ! " << std::endl;   
     //  m_error = "Create Process failed (Pipe error) ! "; 
      exit(EXIT_FAILURE);
     }  
     else if (fork_result == 0)
     { 
       // This is the child
       close(0);
      dup(stdin_pipe[0]);
      close(stdin_pipe[0]);
      close(stdin_pipe[1]);
      close(1);      
      dup(stdout_pipe[1]);     
      close(stdout_pipe[0]); 
      close(stdout_pipe[1]);      
      close(2);
      //dup(stderr_pipe[1]);     
      //close(stderr_pipe[0]);      
      //close(stderr_pipe[1]);   
      //MString m_prog = m_command.begin(" ");
      //MString m_param = m_command.end(" ");   
   

      fcntl(stdout_pipe[1], F_SETFL, O_NONBLOCK);
   //   fcntl(stderr_pipe[1], F_SETFL, O_NONBLOCK);

    /* if (m_param == m_prog)
       m_param = "";

     if (m_param.length() != 0)
        m_param = m_param + 1;
*/

    if (execlp(program,program,"",NULL) == -1)
    {         
  if (errno == 2)
  {
      //std::cerr << (MString("Program (") + m_prog + ") not found!").toChar() << std::endl;      
      std::cout << "Program not found : " << program  << std::endl;
   }
    }

      exit(EXIT_FAILURE);
    } 
    else   
    { 
      // This is the parent
      close(stdin_pipe[0]);
      close(stdin_pipe[1]);
     // close(stderr_pipe[1]);
      close(stdout_pipe[1]);  

      fcntl(stdout_pipe[0], F_SETFL, O_NONBLOCK);
     // fcntl(stderr_pipe[0], F_SETFL, O_NONBLOCK);

      while(1)   
      {       
       /* data_processed_err = read(stderr_pipe[0],buffer_err,BUFSIZ);
        if (data_processed_err != -1)
        {
          for (unsigned int k=0;k<strlen(buffer_err);k++)
            m_error += buffer_err[k];
       
          if (m_progressmanager)
            m_progressmanager->DisplayError(MString(buffer_err));
       
    memset(buffer_err,'\0',sizeof(buffer));
        }
*/
 
       data_processed = read(stdout_pipe[0],buffer,BUFSIZ);
       if (data_processed != -1)
       {
   for (unsigned int k=0;k<strlen(buffer);k++)
           m_output += buffer[k];
       

         memset(buffer,'\0',sizeof(buffer));
       }
      

       if ((data_processed == 0) ) break;
     }

//     close(stderr_pipe[0]);
     close(stdout_pipe[0]);
    }
  }
#endif

}
