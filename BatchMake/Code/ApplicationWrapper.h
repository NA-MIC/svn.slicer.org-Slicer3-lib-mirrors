/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: ApplicationWrapper.h,v $
  Language:  C++
  Date:      $Date: 2007/06/01 13:02:19 $
  Version:   $Revision: 1.4 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifndef __ApplicationWrapper_h_
#define __ApplicationWrapper_h_

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <vector>
#include <iostream>
#include "ApplicationWrapperParam.h"
#include "MString.h"
#include "XMLReader.h"


class ApplicationWrapper
{
public:
  ApplicationWrapper();
  ~ApplicationWrapper();

  void SetApplicationPath(MString applicationpath);
  MString GetApplicationPath();
  void SetName(MString name);
  MString GetName();
  void SetVersion(MString version);
  MString GetVersion();
  void AddParam(ApplicationWrapperParam param);
  ApplicationWrapperParam* GetParam(MString name);
  void Save(MString filename);
  void Load(MString filename);
  MString GetExampleLine();
  void ReadModule(XMLReader& m_reader);
  void ReadParam(XMLReader& m_reader);
  std::vector<ApplicationWrapperParam> & GetParams();
  void DisplayParam(MString& m_line,int offset);
  void DeleteParam(MString name);
  void UpParam(MString name);
  void DownParam(MString name);
  void AutomaticCommandLineParsing(const char * _path);

  /** Return the current command line arguments */
  std::string GetCurrentCommandLineArguments(bool relativePath=true);

  /** Set the parameter value */
  void SetParameterValue(std::string first, std::string second, std::string value);

  /** Return true if a parameter exists */
  bool ParameterExists(std::string first);

  /** Set/Get if the application uses sequential arguments */
  void SetSequentialParsing(bool val)
    {
    m_Sequential = val; 
    m_SequentialParams.clear();
    }
  bool GetSequentialParsing() {return m_Sequential;}

  void SetDataHost(const char* hostname) {m_DataHost = hostname;}
  const char* GetDataHost() {return m_DataHost.c_str();}
  
  void SetOutputHost(const char* hostname) {m_OutputHost = hostname;}
  const char* GetOutputHost() {return m_OutputHost.c_str();}
 
  void SetOutputDirectory(const char* directory) {m_OutputDirectory = directory;}
  const char* GetOutputDirectory() {return m_OutputDirectory.c_str();}
  void SetDataDirectory(const char* directory) {m_DataDirectory = directory;}
  const char* GetDataDirectory() {return m_DataDirectory.c_str();}

  void SetDependsOn(int val) {m_DependsOn = val;}
  int  GetDependsOn() {return m_DependsOn;}

  void SetAfterEndBarrier(bool val) {m_AfterEndBarrier = val;}
  bool  GetAfterEndBarrier() {return m_AfterEndBarrier;}

  void SetExecutionBlockNumber(int val) {m_ExecutionBlockNumber = val;}
  unsigned long GetExecutionBlockNumber() {return m_ExecutionBlockNumber;}

  void SetDependencyTag(const char* tag) {m_DependencyTag = tag;}
  const char* GetDependencyTag() {return m_DependencyTag.c_str();}

  void SetSingleNode(int single) {m_SingleNode = single;}
  int GetSingleNode() {return m_SingleNode;}

  std::vector<std::string> & GetInputDataToTransfer()
    {
    return m_InputDataToTransfer;
    }

  void AddInputDataToTransfer(const char* filename)
    {
    m_InputDataToTransfer.push_back(filename);
    }

  void ClearParameterValues();

private:
  MString m_name;
  MString m_applicationpath;
  MString m_version;
  bool    m_Sequential; // Set if we should use the sequential application
  std::vector<ApplicationWrapperParam> m_params;
  std::vector<ApplicationWrapperParam> m_SequentialParams;
  std::string m_DataHost;
  std::string m_OutputHost;
  std::string m_OutputDirectory;
  std::string m_DataDirectory;
  int         m_DependsOn;
  std::string m_DependencyTag;
  int         m_SingleNode;
  bool        m_AfterEndBarrier;
  unsigned long m_ExecutionBlockNumber;
  std::vector<std::string> m_InputDataToTransfer;
};

#endif
