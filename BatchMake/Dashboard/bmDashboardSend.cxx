/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmDashboardSend.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/13 16:31:35 $
  Version:   $Revision: 1.6 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/
#include "bmDashboardSend.h"

namespace bm {

/** Constructor */
DashboardSend::DashboardSend()
{
  this->Init();
}

/** Destructor */
DashboardSend::~DashboardSend()
{
}

/** Initialize the variables */
void DashboardSend::Init()
{
  m_User = "";
  m_Project = "";
  m_MethodName = "";
  m_ExperimentName = "";
  m_ServerOutput = "";
  m_Url = "";
  m_UserKey = "";
  m_HostKey = "";
  m_ValueList.clear(); 
  m_DataList.clear(); 
}
  
/** Clear the value list */
void DashboardSend::ClearValueList()
{
  m_ValueList.clear(); 
}

/** Clear the data list */
void DashboardSend::ClearDataList()
{
  m_DataList.clear();
}

/** Check the validity */
bool DashboardSend::CheckValidity()
{
  if(m_User.size() == 0)
    {
    std::cout << "Please set the username" << std::endl;
    return false;
    }

  if(m_Project.size() == 0)
    {
    std::cout << "Please set the project name" << std::endl;
    return false;
    }

  if(m_Url.size() == 0)
    {
    std::cout << "Please set the URL of the dashboard" << std::endl;
    return false;
    }
  return true;
}

/** Create an experiment.
 *  The actual experiment name is used */
bool DashboardSend::CreateExperiment(const char* description)
{
  if(!this->CheckValidity())
    {
    return false;
    }

  if(m_ExperimentName.size()==0)
    {
    std::cout << "You should set the experiment name before calling"
              << " CreateExperiment" << std::endl;
    }

  HttpRequest request;
  request.AddParam("user",m_User.c_str());
  request.AddParam("userkey",m_UserKey.c_str());
  request.AddParam("project",m_Project.c_str());
  request.AddParam("hostname",request.GetHostName().c_str());
  request.AddParam("hostip",request.GetHostIp().c_str());
  request.AddParam("hostkey",m_HostKey.c_str());
    
  request.AddParam("method","CreateExperiment");
  request.AddParam("name",m_ExperimentName.c_str());
  request.AddParam("description",description);
  
  return this->SendRequest(request);
}

/** Create a method.
 *  The actual experiment and method names are used */
bool DashboardSend::CreateMethod(const char* description)
{
  if(!this->CheckValidity())
    {
    return false;
    }

  if(m_ExperimentName.size()==0)
    {
    std::cout << "You should set the experiment name before calling"
              << " CreateMethod" << std::endl;
    }

  if(m_MethodName.size()==0)
    {
    std::cout << "You should set the method name before calling"
              << " CreateMethod" << std::endl;
    }

  HttpRequest request;
  request.AddParam("user",m_User.c_str());
  request.AddParam("userkey",m_UserKey.c_str());
  request.AddParam("project",m_Project.c_str());
  request.AddParam("hostname",request.GetHostName().c_str());
  request.AddParam("hostip",request.GetHostIp().c_str());
  request.AddParam("hostkey",m_HostKey.c_str());

  request.AddParam("method","CreateMethod");
  request.AddParam("name",m_MethodName.c_str());
  request.AddParam("experiment",m_ExperimentName.c_str());
  request.AddParam("description",description);
  
  return this->SendRequest(request);
}

/** Create a parameter for a method.
 *  The actual experiment and method names are used */
bool DashboardSend::CreateMethodParameter(const char* name,ParameterType paramType,
                                          const char* type, const char* unit)
{
  if(!this->CheckValidity())
    {
    return false;
    }

  if(m_ExperimentName.size()==0)
    {
    std::cout << "You should set the experiment name before calling"
              << " CreateMethod" << std::endl;
    }

  if(m_MethodName.size()==0)
    {
    std::cout << "You should set the method name before calling"
              << " CreateMethod" << std::endl;
    }

  HttpRequest request;
  request.AddParam("user",m_User.c_str());
  request.AddParam("userkey",m_UserKey.c_str());
  request.AddParam("project",m_Project.c_str());
  request.AddParam("hostname",request.GetHostName().c_str());
  request.AddParam("hostip",request.GetHostIp().c_str());
  request.AddParam("hostkey",m_HostKey.c_str());

  request.AddParam("method","CreateParameter");
  request.AddParam("name",name);
  request.AddParam("experiment",m_ExperimentName.c_str());
  request.AddParam("methodname",m_MethodName.c_str());
  switch(paramType)
    {
    case INPUT:
      request.AddParam("type","0");
      break;
    case OUTPUT:
      request.AddParam("type","1");
      break;
    case IDEALOUTPUT:
      request.AddParam("type","2");
      break;
    default:
      request.AddParam("type","0");
    }
  
  if(type)
    {
    request.AddParam("paramtype",type);
    }

  if(unit)
    {
    request.AddParam("paramunit",unit);
    }

  return this->SendRequest(request);
}

/** Add a value to the list of value or data */
void DashboardSend::AddValue(const char* name,const char* value,bool data)
{
  PairType p;
  p.first = name;
  p.second = value;
  if(data)
    {
    m_DataList.push_back(p);
    }
  else
    {
    m_ValueList.push_back(p);
    }
}

/** Send to the dashboard */
bool DashboardSend::Send()
{
  if(!this->CheckValidity())
    {
    return false;
    }

  HttpRequest request;
  request.AddParam("user",m_User.c_str());
  request.AddParam("userkey",m_UserKey.c_str());
  request.AddParam("project",m_Project.c_str());
  request.AddParam("hostname",request.GetHostName().c_str());
  request.AddParam("hostip",request.GetHostIp().c_str());
  request.AddParam("hostkey",m_HostKey.c_str());

  if(m_MethodName.size()==0)
    {
    std::cout << "Set a method name before sending data" << std::endl;
    return false;
    }

  if(m_ExperimentName.size()==0)
    {
    std::cout << "Set an experiment name before sending data" << std::endl;
    return false;
    }

  request.AddParam("method","AddData");
  request.AddParam("methodname",m_MethodName.c_str());
  request.AddParam("experiment",m_ExperimentName.c_str());

  ListType::const_iterator it = m_ValueList.begin();
  while(it != m_ValueList.end())
    {
    std::string name = (*it).first;
    std::string value = (*it).second;
    request.AddParam(name.c_str(),value.c_str());
    it++;
    }

  it = m_DataList.begin();
  while(it != m_DataList.end())
    {
    std::string name = (*it).first;
    std::string value = (*it).second;
    request.AddFile(name,value.c_str());
    }

  return this->SendRequest(request);
}

/** Send request to the dashboard */
bool DashboardSend::SendRequest(HttpRequest & request)
{
  // Send the data to the dashboard
  std::string url = m_Url;
  url += "/dashboard.php";
  m_ServerOutput = request.Send(url.c_str());

  //std::cout << m_ServerOutput.c_str() << std::endl;
  if (m_ServerOutput.length()>3)
    {
    std::cout << "Bad Host or connexion problem" << std::endl;
    return false;
    }
  else
    {
    if (atoi(m_ServerOutput.c_str()) == 0)
      {
      std::cout << "Data sent successfully." << std::endl;
      return true;
      }
    else
      {
      //std::cout << "Dashboard problem when sending data" << std::endl;
      switch(atoi(m_ServerOutput.c_str()))
        {
        case 1 :  std::cout << "Wrong user name or password" << std::endl; break;
        case 2 :  std::cout << "Wrong project name" << std::endl; break;
        case 3 :  std::cout << "User doesn't belong to project" << std::endl; break;
        case 4 :  std::cout << "Over quota: please use DbClear function first" << std::endl; break;
        //case 5 :  std::cout << "Host Database error" << std::endl; break;
        case -1 : std::cout << "Connexion problem" << std::endl; break;
        }
      return false;
      }
    }
  return false;
}

} // end namespace bm
