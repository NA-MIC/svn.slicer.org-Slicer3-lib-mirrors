/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: main.cxx,v $
  Language:  C++
  Date:      $Date: 2007/09/26 14:12:36 $
  Version:   $Revision: 1.2 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "metaCommand.h"
#include "metaOutput.h"
#include "HttpRequest.h"
#include "XMLBufferReader.h"

class BatchMakeStream : public MetaOutputStream
{
public:
  BatchMakeStream(MetaCommand* metaCommand) 
    {
    // Define the command line arguments
    m_MetaCommand = metaCommand;
    m_MetaCommand->SetOption("SendDashboard","sendDashboard",
                             false,"Send output to Batchmake Validation Dashboard");
    m_MetaCommand->AddOptionField("SendDashboard","hostname",MetaCommand::STRING,true);
    m_MetaCommand->AddOptionField("SendDashboard","experimentid",MetaCommand::STRING,true);
    m_MetaCommand->AddOptionField("SendDashboard","methodid",MetaCommand::STRING,true);
    };

  ~BatchMakeStream() {};

  bool Open()
    {
    m_ExperimentID = m_MetaCommand->GetValueAsString("SendDashboard","experimentid");
    m_RemoteHostname = m_MetaCommand->GetValueAsString("SendDashboard","hostname");
    m_MethodID = m_MetaCommand->GetValueAsString("SendDashboard","methodid");
    m_Request.AddParam("experiment",m_ExperimentID.c_str());
    m_Request.AddParam("methodid",m_MethodID.c_str());
    if(m_MetaOutput)
      {
      m_Request.AddParam("hostname",static_cast<MetaOutput*>(m_MetaOutput)->GetHostname().c_str());
      m_Request.AddParam("hostip",static_cast<MetaOutput*>(m_MetaOutput)->GetHostip().c_str());
      }
    return true;
    }

  bool Write(const char* buffer)
    {
    typedef itk::XMLBufferReader XMLParserType;
    XMLParserType::Pointer xmlReader = XMLParserType::New();
    std::string Buffer = buffer;
    xmlReader->Parse(Buffer.c_str(),Buffer.size());
   
    XMLParserType::TagVectorType tags = xmlReader->GetTags();

    XMLParserType::TagVectorType::const_iterator it = tags.begin();
    while(it != tags.end())
      {
      if(((*it).name == "Input") || ((*it).name == "Output"))
        {
        std::string name = "";
        std::string value = "";
        std::vector<XMLParserType::PairType> values = (*it).values;
        std::vector<XMLParserType::PairType>::const_iterator itValues = values.begin();
        while(itValues != values.end())
          {
          if((*itValues).first == "name")
            {
            name = (*itValues).second;
            }
          else if((*itValues).first == "value")
            {
            value = (*itValues).second;
            }
          itValues++;
          }
        m_Request.AddParam(name,value.c_str());
        }
      
      it++;
      }
    return true;
    }

  // This function actually send the data over the network
  bool Close()
    {
    std::string url = m_RemoteHostname;
    url += "/upload.php";
    std::string output = m_Request.Send(url.c_str());
 
    if (output.length()>3)
      {
      std::cout << "Bad Host or connexion problem" << std::endl;
      }
    else if (output[0] == '0')
      {
      std::cout << "Data sent successfully!" << std::endl;
      }
    else
      {
      std::cout << "Dashboard problem when sending data" << std::endl;
      switch(output[0])
        {
        case '1' :  std::cout << "Bad account name" << std::endl; break;
        case '2' :  std::cout << "Bad password" << std::endl; break;
        case '3' :  std::cout << "Bad project name" << std::endl; break;
        case '4' :  std::cout << "Over quota: please use DbClear function first" << std::endl; break;
        case '5' :  std::cout << "Host Database error" << std::endl; break;
        default:
          std::cout << "Connexion problem" << std::endl; break;
        }  
      }
    return true;
    }

private:

  MetaCommand* m_MetaCommand;
  std::string  m_ExperimentID;
  std::string  m_RemoteHostname;
  std::string  m_LocalHostname;
  std::string  m_LocalIP;
  std::string  m_MethodID;
  HttpRequest  m_Request;
};


int main(int argc, char* argv[])
{
  MetaCommand command;
  
  command.SetName("Calculator");
  command.SetVersion("1.0");
  command.SetAuthor("Kitware Inc");
  command.SetDescription("Perform number calculation");

  command.AddField("firstNumber", "first number", MetaCommand::FLOAT);
  command.AddField("operand", "operand", MetaCommand::STRING);
  command.AddField("secondNumber", "second number", MetaCommand::FLOAT);

  // This should be put before the parsing
  MetaOutput output;
  output.SetMetaCommand(&command);
  BatchMakeStream bmstream(&command);

  // Parsing
  if(!command.Parse(argc,argv))
    {
    return 1;
    }

  float first = command.GetValueAsFloat("firstNumber");
  float second = command.GetValueAsFloat("secondNumber");
  std::string operand = command.GetValueAsString("operand");

  float result = 0;
  if(operand == "+")
    {
    result = first + second;
    }
  else if(operand == "-")
    {
    result = first - second;
    }
  else if(operand == "/")
    {
    result = first / second;
    }
  else if(operand == "*")
    {
    result = first * second;
    }

  output.AddFloatField("Result","Computation Result",result);
  //output.AddStreamFile("OutputFile","c:/Julien/metaOutput.txt");
  //output.AddStream("StandardOutput",std::cout);
  //output.DisableStream("StandardOutput");
  //output.AddStream("DataBaseStream",&bmstream);
  output.Write();

  return 0;
}
