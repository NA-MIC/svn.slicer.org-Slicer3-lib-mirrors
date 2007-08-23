/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmGridSend.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/25 18:50:03 $
  Version:   $Revision: 1.10 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/
#include "metaCommand.h"
#include "HttpRequest.h"
#include "MString.h"

int main(int argc, char* argv[])
{
  for(unsigned int i=0;i<argc;i++)
    {
    std::cout << i << " : " << argv[i] << std::endl;
    }
  
  MetaCommand command;
  
  command.SetName("bmGridSend");
  command.SetVersion("1.0");
  command.SetAuthor("Kitware Inc");
  command.SetDescription("Send batchmake validation results to the dashboard");

  // Required field
  command.AddField("hostname","Hostname of the dashboard",MetaCommand::STRING);
  command.AddField("user","Username to connect to the dashboard",MetaCommand::STRING);
  command.AddField("project","Name of the project",MetaCommand::STRING);
  
  // User key if any
  command.SetOption("key","key",false,"User key");
  command.AddOptionField("key","key",MetaCommand::STRING,true);

  // Create an experiment
  command.SetOption("createExperiment","ce",false,"Create an Experiment");
  command.AddOptionField("createExperiment","name",MetaCommand::STRING,true);
  command.AddOptionField("createExperiment","description",MetaCommand::STRING,false);
  
  // Create a method
  command.SetOption("createMethod","cm",false,"Create an Method for an experiment");
  command.AddOptionField("createMethod","experimentName",MetaCommand::STRING,true);
  command.AddOptionField("createMethod","name",MetaCommand::STRING,true);
  command.AddOptionField("createMethod","description",MetaCommand::STRING,false);

  // Create a method input
  command.SetOption("createMethodParameter","cp",false,"Create a parameter for a method");
  command.AddOptionField("createMethodParameter","experiment",MetaCommand::STRING,true);
  command.AddOptionField("createMethodParameter","method",MetaCommand::STRING,true);
  command.AddOptionField("createMethodParameter","name",MetaCommand::STRING,true);
  command.AddOptionField("createMethodParameter","type",MetaCommand::STRING,true);
  command.AddOptionField("createMethodParameter","paramtype",MetaCommand::STRING,false);

  // Send the data
  command.SetOption("send","send",false,"Send data to the dashboard");
  command.AddOptionField("send","experiment",MetaCommand::STRING,true);
  command.AddOptionField("send","method",MetaCommand::STRING,true);

  // Send the data
  command.SetOption("data","data",false,"Send data to the dashboard",MetaCommand::LIST);
  command.SetOption("imagedata","imagedata",false,"Send image data to the dashboard",MetaCommand::LIST);

  // Parse an input file to find the values
  command.SetOption("inputFile","i",false,
    "Parse input file to find the missing values",MetaCommand::STRING,"",MetaCommand::DATA_IN);

  // Parsing
  if(!command.Parse(argc,argv))
    {
    return 1;
    }

  std::string hostname = command.GetValueAsString("hostname");
  std::string user = command.GetValueAsString("user");
  std::string project = command.GetValueAsString("project");
  std::string key = command.GetValueAsString("key","key");

  std::string createExperimentName = command.GetValueAsString("createExperiment","name");
  std::string createExperimentDescription = command.GetValueAsString("createExperiment","description");
    
  std::string createMethodName = command.GetValueAsString("createMethod","name");
  std::string createMethodDescription = command.GetValueAsString("createMethod","description");
  std::string createMethodExperiment = command.GetValueAsString("createMethod","experimentName");

  std::string createParameterName = command.GetValueAsString("createMethodParameter","name");
  std::string createParameterExperiment = command.GetValueAsString("createMethodParameter","experiment");
  std::string createParameterMethod = command.GetValueAsString("createMethodParameter","method");
  std::string createParameterType = command.GetValueAsString("createMethodParameter","type");
  std::string createParameterParamType = command.GetValueAsString("createMethodParameter","paramtype");

  std::string sendDataExperiment = command.GetValueAsString("send","experiment");
  std::string sendDataMethod = command.GetValueAsString("send","method");

  std::string url = hostname;

  HttpRequest m_request;
  m_request.AddParam("user",user.c_str());
  m_request.AddParam("userkey",key.c_str());
  m_request.AddParam("project",project.c_str());
  m_request.AddParam("hostname",m_request.GetHostName().c_str());
  m_request.AddParam("hostip",m_request.GetHostIp().c_str());

  if(command.GetOptionWasSet("createExperiment"))
    {
    m_request.AddParam("method","CreateExperiment");
    m_request.AddParam("name",createExperimentName.c_str());
    m_request.AddParam("description",createExperimentDescription.c_str());
    }
  else if(command.GetOptionWasSet("createMethod"))
    {
    m_request.AddParam("method","CreateMethod");
    m_request.AddParam("name",createMethodName.c_str());
    m_request.AddParam("experiment",createMethodExperiment.c_str());
    m_request.AddParam("description",createMethodDescription.c_str());
    }
 else if(command.GetOptionWasSet("createMethodParameter"))
    {
    m_request.AddParam("method","CreateParameter");
    m_request.AddParam("name",createParameterName.c_str());
    m_request.AddParam("experiment",createParameterExperiment.c_str());
    m_request.AddParam("methodname",createParameterMethod.c_str());
    m_request.AddParam("type",createParameterType.c_str()); // 0 = input | 1 = output | 2 = idealoutput

    if(createParameterParamType.size()>0)
      {
      m_request.AddParam("paramtype",createParameterParamType.c_str());
      }
    }
 else if(command.GetOptionWasSet("send"))
    {
    m_request.AddParam("method","AddData");
    m_request.AddParam("methodname",sendDataMethod.c_str());
    m_request.AddParam("experiment",sendDataExperiment.c_str());

    if(!command.GetOptionWasSet("data"))
      {
      std::cout << "Please use the -data option to set the data fields" << std::endl;
      return 0;
      }

    std::string buffer = "";

    if(command.GetOptionWasSet("inputFile"))
      {
      std::string inputFile = command.GetValueAsString("inputFile");
      // Read the file
      std::ifstream file;
      file.open(inputFile.c_str(), std::ios::binary | std::ios::in);
      if(!file.is_open())
        {
        std::cout << "Cannot open file: " << inputFile.c_str() << std::endl;
        return 0;
        }
      file.seekg(0,std::ios::end);
      unsigned long fileSize = file.tellg();
      file.seekg(0,std::ios::beg);

      char* buf = new char[fileSize+1];
      file.read(buf,fileSize);
      buf[fileSize] = 0; 
      buffer = buf;
      buffer.resize(fileSize);
      delete [] buf;
      }

    std::list<std::string> values = command.GetValueAsList("data");
    std::list<std::string>::const_iterator it = values.begin();
    while(it!=values.end())
      {
      std::string name = *it;
      it++;
      std::string value = *it;
      it++;
     

      std::string val = value+":";
      int pos = buffer.find(val);
      if(pos != -1)
        {
        unsigned int size = buffer.find("\n",pos);
        value = buffer.substr(pos+val.size(),size-pos-val.size());
        }
      m_request.AddParam(name,value.c_str());
      //std::cout << name.c_str() << " : " << value.c_str() << std::endl;
      }

    std::list<std::string> imagevalues = command.GetValueAsList("imagedata");
    it = imagevalues.begin();
    while(it!=imagevalues.end())
      {
      std::string name = *it;
      it++;
      std::string value = *it;
      it++;

      std::string val = value+":";
      int pos = buffer.find(val);
      if(pos != -1)
        {
        unsigned int size = buffer.find("\n",pos);
        value = buffer.substr(pos+val.size(),size-pos-val.size());
        }
      m_request.AddFile(name,value.c_str());
      }
    }

  // Send the data to the dashboard
  url += "/dashboard.php";
  MString m_Output = m_request.Send(url.c_str());

  if (m_Output.length()>3)
    {
    std::cout << "Bad Host or connexion problem" << std::endl;
    }
  else
    {
    if (m_Output.toInt() == 0)
      {
      std::cout << "Command successful." << std::endl;
      }
    else
      {
      std::cout << "Dashboard problem when sending data" << std::endl;
      switch(m_Output.toInt())
        {
        case 1 :  std::cout << "Bad user name" << std::endl; break;
        case 2 :  std::cout << "Bad project name" << std::endl; break;
        case 3 :  std::cout << "User doesn't belong to project" << std::endl; break;
        case 4 :  std::cout << "Over quota: please use DbClear function first" << std::endl; break;
        case 5 :  std::cout << "Host Database error" << std::endl; break;
        case -1 : std::cout << "Connexion problem" << std::endl; break;
        }  
      }
    }

  return 0;
}
