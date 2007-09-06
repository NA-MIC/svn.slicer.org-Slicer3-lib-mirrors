/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmDashboardSend.h,v $
  Language:  C++
  Date:      $Date: 2007/09/06 14:09:48 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifndef __DashboardSend_h_
#define __DashboardSend_h_

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <list>
#include <iostream>
#include "HttpRequest.h"

namespace bm {

class DashboardSend
{
public:

  DashboardSend();
  ~DashboardSend();

  typedef std::pair<std::string,std::string> PairType;
  typedef std::list<PairType>                ListType;

  typedef enum
    {
    INPUT = 0,
    OUTPUT = 1,
    IDEALOUTPUT =2
    } ParameterType;

  /** Send the values to the dashboard */
  bool Send();

  /** Set the username */
  void SetUserName(const char* name)
    {
    m_User = name;
    }

  /** Set the username */
  void SetUserKey(const char* key)
    {
    m_UserKey = key;
    }

  /** Set the project name */
  void SetProjectName(const char* name)
    {
    m_Project = name;
    }

  /** Set the method name */
  void SetMethodName(const char* name)
    {
    m_MethodName = name;
    }

  /** Set the experiment name */
  void SetExperimentName(const char* name)
    {
    m_ExperimentName = name;
    }

  /** Set the URL */
  void SetUrl(const char* url)
    {
    m_Url = url;
    }

  /** Set the hostkey */
  void SetHostKey(const char* hostKey)
    {
    m_HostKey = hostKey;
    }

  /** Create an experiment.
   *  The actual experiment name is used */
  bool CreateExperiment(const char* description);

  /** Create a method.
   *  The actual experiment and method names are used */
  bool CreateMethod(const char* description);

  /** Create a parameter for a method.
   *  The actual experiment and method names are used */
  bool CreateMethodParameter(const char* name,ParameterType paramType=INPUT,
                             const char* type=NULL, const char* unit=NULL);

  /** Add a value to the list of value or data */
  void AddValue(const char* name,const char* value,bool data=false);

  /** Initialize the variables */
  void Init();
  void ClearValueList();
  void ClearDataList();

protected:

  /** Check if the user and project have been set */
  bool CheckValidity();

  /** Send request to the dashboard */
  bool SendRequest(HttpRequest & request);

  /** Variables which should be set by the user before
   *  calling Send() */
  std::string m_User;
  std::string m_Project;
  std::string m_MethodName;
  std::string m_ExperimentName;
  std::string m_Url;
  std::string m_UserKey;
  std::string m_HostKey;

  std::string m_ServerOutput; // string returned by the server

  ListType m_ValueList;
  ListType m_DataList;

};

} // end namespace bm

#endif
