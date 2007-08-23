/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmGridStore.cxx,v $
  Language:  C++
  Date:      $Date: 2006/09/28 16:50:10 $
  Version:   $Revision: 1.4 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/
#include "metaCommand.h"
#include "MString.h"
#include "XMLBufferReader.h"

void AddField(std::string filename, std::string name, std::string value)
{
  std::string outbuffer = "";
  bool updated = false;

  std::ifstream infile;
  infile.open(filename.c_str(), std::ios::binary | std::ios::in);
  if(infile.is_open())
    { 
    while(!infile.eof())
      {
      char str[2000];
      infile.getline(str,2000);
      std::string line = str;
      if(line.size()>0)
        {
        std::string n = line.substr(0,line.find(":"));
        if(n == name)
          {
          outbuffer += name;
          outbuffer += ":";
          outbuffer += value;
          outbuffer += "\n";
          updated = true;
          }
        else
          {
          outbuffer += line;
          outbuffer += "\n";
          }
        }
      }
    }
  infile.close();

  if(!updated)
    {
    outbuffer += name;
    outbuffer += ":";
    outbuffer += value;
    outbuffer += "\n";
    }

  // Write out the file
  std::ofstream outfile;
  outfile.open(filename.c_str(), std::ios::binary | std::ios::out);
  if(!outfile.is_open())
    {
    std::cout << "Cannot open file for writing: " <<  std::endl;
    return;
    }

  outfile << outbuffer;
  outfile.close();
}

/** Parse the XML output and set some variables */
void ParseXMLOutput(const char* filename, 
                    const char* output,const char* appname)
{
  typedef itk::XMLBufferReader XMLParserType;
  XMLParserType::Pointer xmlReader = XMLParserType::New();
  std::string Buffer = output;
  try
    {
    xmlReader->Parse(Buffer.c_str(),Buffer.size());
    }
  catch(...)
    {
    //std::cout << "Cannot parse output" << std::endl;
    return;
    }
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
          value = "'"+(*itValues).second+"'";
          }
        itValues++;
        }
      // We only add the output values
      if((*it).name == "Output")
        {
        MString app = appname;
        app = app.removeChar('$');
        app = app.removeChar('{');
        app = app.removeChar('}');
        std::string fullname = app.toChar();
        fullname += "."+name;
        AddField(filename,fullname,value);
        }
      }
    it++;
    }
}

int main(int argc, char* argv[])
{
  MetaCommand command;
  
  command.SetName("bmGridStore");
  command.SetVersion("1.0");
  command.SetAuthor("Kitware Inc");
  command.SetDescription("Store variable in a file");

  // Required field
  command.AddField("filename","Name of the file to store the variables",MetaCommand::STRING,MetaCommand::DATA_OUT);
  command.AddField("name","Name of the variable",MetaCommand::STRING);
  command.AddField("value","Value to store",MetaCommand::STRING);

  // Add option to parse the output of an executable
  command.SetOption("parse","p",false,
                    "Parse a file and add the output to the current filename",
                    MetaCommand::STRING,"",MetaCommand::DATA_IN);

  // Add option to set the app variable name
  command.SetOption("appname","a",false,
                    "Set the application variable name",
                    MetaCommand::STRING);

  // Parsing
  if(!command.Parse(argc,argv))
    {
    return 1;
    }

  std::string filename = command.GetValueAsString("filename");
  std::string name = command.GetValueAsString("name");
  std::string value = command.GetValueAsString("value");

  if(command.GetOptionWasSet("parse"))
    {
    std::string filen = command.GetValueAsString("parse");
    
    // Read the file
    std::ifstream file;
    file.open(filen.c_str(), std::ios::binary | std::ios::in);
    if(!file.is_open())
      {
      std::cout << "Cannot open file: " << filen.c_str() << std::endl;
      return 0;
      }
    file.seekg(0,std::ios::end);
    unsigned long fileSize = file.tellg();
    file.seekg(0,std::ios::beg);

    char* buf = new char[fileSize+1];
    file.read(buf,fileSize);
    buf[fileSize] = 0; 
    std::string buffer(buf);
    buffer.resize(fileSize);
    delete [] buf;

    std::string appname = command.GetValueAsString("appname");
    ParseXMLOutput(filename.c_str(),buffer.c_str(),appname.c_str());
    }
  else
    {
    AddField(filename,name,value);
    }

  return 0;
}
