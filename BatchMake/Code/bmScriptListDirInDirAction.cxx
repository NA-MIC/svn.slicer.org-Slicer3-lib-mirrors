/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptListDirInDirAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/11 17:56:10 $
  Version:   $Revision: 1.10 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptListDirInDirAction.h"
#include <itksys/Directory.hxx>
#include <itksys/SystemTools.hxx>
#include <itksys/RegularExpression.hxx>

namespace bm {

ScriptListDirInDirAction::ScriptListDirInDirAction()
: ScriptAction()
{
}

ScriptListDirInDirAction::~ScriptListDirInDirAction()
{
}

bool ScriptListDirInDirAction::TestParam(ScriptError* error,int linenumber)
{
  if (m_Parameters.size() <2)
    {
    error->SetError(MString("No enough parameter for ListDirInDir function !"),linenumber);
    return false;
    }

  m_Manager->SetTestVariable(m_Parameters[0]);

  for (unsigned int i=1;i<m_Parameters.size();i++)
    {
    m_Manager->TestConvert(m_Parameters[i],linenumber);
    }
  return true;
}

MString ScriptListDirInDirAction::Help()
{
  return "ListDirInDir(<dirlist> 'directory' *.* [NOOVERWRITE])";
}


void ScriptListDirInDirAction::Execute()
{
  MString m_initdir = m_Manager->Convert(m_Parameters[1]);
  if (m_initdir.startWith('\''))
    m_initdir = m_initdir.rbegin("'") + 1;

  MString m_filter = "*";
  if (m_Parameters.size() == 3 && strcmp(m_Parameters[2].toChar(),"NOOVERWRITE"))
    {
    m_filter = m_Manager->Convert(m_Parameters[2]);
    if (m_filter.startWith('\''))
      m_filter = m_filter.rbegin("'") + 1;
    }

  // By default the value is the current value of the first variable
  MString value = "";

  bool checkOverwrite = false;
 
  // We do not overwrite the variable if specified
  if( (m_Parameters.size() == 4 && !strcmp(m_Parameters[3].toChar(),"NOOVERWRITE"))
    || (m_Parameters.size() == 3 && !strcmp(m_Parameters[2].toChar(),"NOOVERWRITE"))
    )
    {
    checkOverwrite = true;
    std::vector<MString> values = m_Manager->GetVariable(m_Parameters[0]);
    for(unsigned int i=0;i<values.size();i++)
      {
      if(i>0)
        {
        value += " ";
        }
      value += "'";
      value += values[i].toChar();
      value += "'";
      }
    }
 
  std::string dir = m_initdir.toChar();

  if( (dir[dir.length()-1] != '/') && (dir[dir.length()-1] != '\\') )
    {
    dir += '/';
    }
 
  itksys::Directory directory;

  if(!directory.Load(dir.c_str()))
    {
    m_ProgressManager->AddAction("Action: ListDirInDir()");
    std::string error = dir;
    error += " is not a valid directory";
    m_ProgressManager->AddError(error.c_str());
    return;
    }

  std::string regexFromWildcard = MString::ConvertWildcardToRegEx(m_filter.toChar());
  itksys::RegularExpression regex(regexFromWildcard.c_str());

  for(unsigned int i=0;i<directory.GetNumberOfFiles();i++)
    {    
    std::string dname = directory.GetFile(i);

    // Check if this is actually a directory
    std::string completePath = dir+"/"+dname;
    if(!itksys::SystemTools::FileIsDirectory(completePath.c_str()))
      {
      continue;
      }

    std::string filename = itksys::SystemTools::GetFilenameName(dname);

    if(regex.find(dname.c_str())
      && filename != "."
      && filename != "..")
      {
      // Check that the value doesn't exists already
      bool exists = false;
      if(checkOverwrite)
        {
        std::vector<MString> values = m_Manager->GetVariable(m_Parameters[0]);
        for(unsigned int i=0;i<values.size();i++)
          {
          if(!strcmp(values[i].toChar(),dname.c_str()))
            {
            exists = true;
            break;
            }
          }
        }
      if(!exists)
        {
        if (value != "")
          {
          value += " ";
          }
        value += MString("'") + MString(dname.c_str()) + MString("'");
        }
      }
    }
   m_Manager->SetVariable(m_Parameters[0],value);
}

} // end namespace bm
