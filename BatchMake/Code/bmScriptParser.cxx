/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptParser.cxx,v $
  Language:  C++
  Date:      $Date: 2007/06/21 12:54:17 $
  Version:   $Revision: 1.17 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptParser.h"
#include <fstream>
#include <itksys/Directory.hxx>
#include <itksys/SystemTools.hxx>
#include <itksys/Process.h>

#ifdef WIN32
  #include <windows.h>
#else
  #include <dirent.h>
#endif

namespace bm {

ScriptParser::ScriptParser()
{
  m_ScriptActionManager = new ScriptActionManager();
  m_ScriptActionManager->SetParser(this);
  m_Error = 0;
  m_ApplicationList = 0;
}

ScriptParser::~ScriptParser()
{
  delete m_ScriptActionManager;

  if(m_ApplicationList)
    {
    std::vector<ApplicationWrapper*>::iterator it = m_ApplicationList->begin();
    while(it != m_ApplicationList->end())
      {
      ApplicationWrapper* app = (*it);
      it = m_ApplicationList->erase(it);
      delete app;
      }
    delete m_ApplicationList;
    }
  delete m_Error;
}

void ScriptParser::SetBatchMakeBinaryPath(MString applicationpath)
{
 m_ApplicationPath = applicationpath;
 m_ScriptActionManager->SetApplicationPath(m_ApplicationPath);
}

void ScriptParser::SetScriptPath(MString scriptpath)
{
  m_ScriptActionManager->SetScriptPath(scriptpath.rbegin("/"));
  m_ScriptActionManager->SetScriptFullPath(scriptpath.toChar());
}

void ScriptParser::LoadWrappedApplication(MString applicationpath) 
{
  if(m_ApplicationList)
    {
    delete m_ApplicationList;
    }
  m_ApplicationList = new std::vector<ApplicationWrapper*>;

  itksys::Directory directory;
  std::string dirpath = applicationpath.toChar();
  dirpath += "/";

  if(directory.Load(dirpath.c_str()))
    {
    for(unsigned int i=0;i<directory.GetNumberOfFiles();i++)
      {
      std::string extension = itksys::SystemTools::GetFilenameExtension( directory.GetFile(i) );
      if(extension == ".bmm")
        {
        std::string file = directory.GetFile(i);
        ApplicationWrapper* m_newapplication = new ApplicationWrapper();  
        m_newapplication->Load(applicationpath + "/" + file.c_str());
        m_ApplicationList->push_back(m_newapplication);
        }
      }
    }
  else
    {
    std::cout << "Applications directory: " << dirpath.c_str() << " doesn't exist" << std::endl;
    }

 m_ScriptActionManager->SetApplicationWrapperList(m_ApplicationList);
}

std::vector<ApplicationWrapper*>* ScriptParser::GetApplicationList()
{
  return m_ApplicationList;
}

void ScriptParser::SetProgressManager(ProgressManager* progressmanager)
{
  m_ScriptActionManager->SetProgressManager(progressmanager);
}

void ScriptParser::SetError(ScriptError* error)
{
  m_Error = error;
  m_ScriptActionManager->SetError(error);
}

void  ScriptParser::RemoveCodeLine(unsigned int line)
{
  std::vector<MString>::iterator it = m_Code.begin();
  for(unsigned int i=0;i<line;i++)
    {
    it++;
    }   
  m_Code.erase(it);
}


bool  ScriptParser::Compile(MString filename,unsigned long pos,bool isInclude)
{
  std::ifstream m_file;
  m_file.open(filename.toChar(),std::ifstream::binary);
  if(!m_file.good())
    {
    std::cout << "Cannot open file: " << filename.toChar() << std::endl;
    return false;
    }
  unsigned long position = pos;
  char* data = (char*)malloc(1000);
  strcpy(data,"");
  MString m_currentline;
  while(!m_file.eof())
    {
    m_file.getline(data,1000);
    if (data[strlen(data)-1] == '\r')
      {
      data[strlen(data)-1] = '\0';
      }
    m_currentline = data;
    if(isInclude && position == 0)
      {
      std::vector<MString>::iterator it = m_Code.begin();
      m_Code.insert(it,m_currentline);
      }
    else
      {
      this->AddCodeLine(m_currentline,position);
      }
    position++;
    }

  delete [] data;
  m_file.close();

  if(!isInclude)
    {
    this->Parse();
    m_ScriptActionManager->GetError()->DisplaySummary();
    }

  return true;
}

bool ScriptParser::Execute(MString filename,unsigned long pos)
{ 
  std::ifstream m_file;
  m_file.open(filename.toChar(),std::ifstream::binary);
  if(!m_file.good())
    {
    std::cout << "Cannot open file: " << filename.toChar() << std::endl;
    return false;
    }
  char* data = new char[1000];
  strcpy(data,"");
  MString m_currentline;
  unsigned long position = pos;
  while(!m_file.eof())
    {
    m_file.getline(data,1000);
   if (strlen(data)>0 && data[strlen(data)-1] == '\r')
      {
      data[strlen(data)-1] = '\0';
      }
   m_currentline = data;
   this->AddCodeLine(m_currentline,position);

   position++;
   }
  m_file.close();

  delete [] data;
 
  if (this->Parse())
    {
    m_ScriptActionManager->Execute();
    return true;
    }
  else
    {
    return false;
    }
}


/** Run the given batchmake script (as a buffer) on Condor */
void ScriptParser::RunCondor(std::string buffer,const char* outputDirectory)
{
  bool inComment = false;
  long int startingLine = 0;
  long int posLine = buffer.find("\n");
  while(posLine != -1)
    {
    MString currentline = buffer.substr(startingLine,posLine+1-startingLine).c_str();
    startingLine = posLine+1;
    this->AddCodeLine(currentline,0);
    posLine = buffer.find("\n",startingLine);
    }

  std::string scriptfile = outputDirectory;
  scriptfile += "/bmcondor.bmc.tmp";
  
#ifdef BM_GRID
  bm::Grid grid;
  if(outputDirectory)
    {
    grid.SetOutputDirectory(outputDirectory);
    }
  grid.SetFileName(scriptfile.c_str());
  this->SetGridModule(&grid);
#endif

  if(this->Parse())
    {
    m_ScriptActionManager->Execute();

#ifdef BM_GRID
    grid.WriteCondor();
#endif

    // Submit the script to condor
    std::vector<const char*> args;
    args.push_back("condor_submit_dag");
    args.push_back("-f");
    args.push_back(scriptfile.c_str());
    args.push_back(0);

    // Run the application
    itksysProcess* gp = itksysProcess_New();
    itksysProcess_SetCommand(gp, &*args.begin());
    itksysProcess_Execute(gp); 

    std::string output = "";

    char* data = NULL;
    int length;
    double timeout = 255;

    while(itksysProcess_WaitForData(gp,&data,&length,&timeout)) // wait for 1s
      {
      for(int i=0;i<length;i++)
        {
        output += data[i];
        }
      }
    itksysProcess_WaitForExit(gp, 0);

    std::cout << "Condor submit output: " << output << std::endl;

    int result = 1;
    switch(itksysProcess_GetState(gp))
      {
      case itksysProcess_State_Exited:
        {
        result = itksysProcess_GetExitValue(gp);
        } break;
      case itksysProcess_State_Error:
        {
        std::cerr << "Error: Could not run " << args[0] << ":\n";
        std::cerr << itksysProcess_GetErrorString(gp) << "\n";
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
    itksys::SystemTools::RemoveFile("bmcondor.bmc.tmp");
    }
  else
    {
    std::cout << "Cannot parse script!" << std::endl;
    }
}


/** Parse and Execute a buffer */
void ScriptParser::ParseBuffer(std::string buffer)
{
  bool inComment = false;

  long int startingLine = 0;
  long int posLine = buffer.find("\n");
  while(posLine != -1)
    {
    MString currentline = buffer.substr(startingLine,posLine+1-startingLine).c_str();
    startingLine = posLine+1;
    this->AddCodeLine(currentline,0);
    posLine = buffer.find("\n",startingLine);
    }

  if(this->Parse())
    {
    m_ScriptActionManager->Execute();
    }
  else
    {
    std::cout << "Cannot parse script!" << std::endl;
    }
}


/** Load and execute a file */
void ScriptParser::Load(MString filename)
{
  this->SetScriptPath(filename);
  m_LineNumber = 0;
  std::ifstream m_file;
  m_file.open(filename.toChar(),std::ifstream::binary);
  char* data = (char*)malloc(1000);
  strcpy(data,"");
  MString m_currentline;
  MString m_line;
  bool inComment = false;
  while(!m_file.eof())
    {
    m_file.getline(data,1000);
    m_LineNumber++;

    m_currentline = data;
    int pos = m_currentline.find("/*");
    if(!m_currentline.isInBetweenChar('\'',pos))
      {
      inComment = true;
      }
    pos = m_currentline.find("*/");
    if(!m_currentline.isInBetweenChar('\'',pos))
      {
      inComment = false;
      }

    if (m_currentline.startWith('#'))
      {
      //Comment
      }
    else if(!inComment)
      {
      if (m_currentline.find("(") != -1)
        {
        m_line = m_currentline;
        }
      else
        {
        m_line += m_currentline;
        }
     if(m_currentline.find(")") != -1)
        {
        Parse(m_line);
        }
      }
    }
  m_file.close();

  m_ScriptActionManager->Execute();
}



bool ScriptParser::Parse(MString line)
{
  MString m_option = line.begin("(").removeChar(' ').removeChar('\t').toLower();
  if (m_option == "")
    {
    return true;
    }
  return AddOption(m_option,line.end("("));
}


bool ScriptParser::AddOption(MString option, MString param)
{
  if (CheckOption(param))
    {
    m_ScriptActionManager->SetLineNumber(m_LineNumber);
    m_ScriptActionManager->AddAction(option,GetParams(param));
    return true;
    }
  return false;
}

bool ScriptParser::CheckOption(MString param)
{
  if (param.count('(') != param.count(')'))
  {
     if (m_Error)
        m_Error->SetError(MString("Missing '(' or ')'"),m_LineNumber);
     return false;
  }
 
  return true;
}

ScriptAction::ParametersType ScriptParser::GetParams(MString param)
{
  ScriptAction::ParametersType m_params;
  MString m_param = param.removeChar('(');
  m_param = m_param.begin(")");
  m_param = m_param.removeChar(')');
  MString m_value = "";
  bool m_isquote = false;

  for (int i=0;i<m_param.length();i++)
  {
      if (m_param[i] == '\'')
      {
        m_isquote=!m_isquote;
      }
    
      if ((m_isquote) && (m_param[i] != '\''))
        m_value += m_param[i];
      
      if ((!m_isquote) && (m_param[i] != '\'') && (m_param[i] != ' '))
        m_value += m_param[i];

      if (((m_param[i] == ' ') && (m_value != "") && (!m_isquote)) || (i==m_param.length()-1))
      {
       // m_value = MString("'") + m_value + "'";
        m_params.push_back(m_value);
        m_value = "";
      }
   }
  return m_params;
}


void ScriptParser::Reset()
{
  m_ScriptActionManager->Reset();
  m_Code.clear();
}


void ScriptParser::AddCodeLine(MString line,unsigned long pos)
{
  if(pos==0)
    {
    m_Code.push_back(line);
    return;
    }
  std::vector<MString>::iterator it = m_Code.begin();
  for(unsigned long i=0;i<pos;i++)
    {
    it++;
    }
  m_Code.insert(it,line);
}


bool ScriptParser::Parse()
{
  m_ScriptActionManager->Reset();
  m_LineNumber = 0;
  MString m_currentline;
  MString m_line;
  bool inComment = false;

  for (unsigned int i=0;i<m_Code.size();i++)
    {
    m_LineNumber++;
    m_currentline = m_Code[i];
      
    MString optionName = m_currentline.begin("(").removeChar(' ').removeChar('\t').toLower();

    int pos = m_currentline.find("/*");
    if(!m_currentline.isInBetweenChar('\'',pos))
      {
      inComment = true;
      }
    pos = m_currentline.find("*/");
    if(!m_currentline.isInBetweenChar('\'',pos))
      {
      inComment = false;
      }

    if (m_currentline.startWith('#'))
      {
      //Comments
      }
    else if (optionName == "include")
      {
      this->Parse(m_currentline);
      this->Parse();
      return true;
      }
    else if(!inComment)
      {
      m_line += " ";
      m_line += m_currentline;

      if ((m_currentline.find(")") != -1) || (i==m_Code.size()-1))
        {
        if (this->Parse(m_line) == false)
          {
          return false;
          }
        m_line = "";
        }
      }
    }

  if (m_ScriptActionManager->GetError()->GetError() != 0)
    {
    return false;
    }
  return true;
}


bool ScriptParser::Execute()
{
  m_ScriptActionManager->Execute();
  return true;
}

} // end namespace bm
