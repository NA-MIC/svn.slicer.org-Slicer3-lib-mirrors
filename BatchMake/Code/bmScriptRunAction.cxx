/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptRunAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/11/13 20:17:16 $
  Version:   $Revision: 1.11 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptRunAction.h"
#include "bmLaunch.h"
#include "Timer.h"
#include "XMLBufferReader.h"

namespace bm {

ScriptRunAction::ScriptRunAction()
: ScriptAction()
{
}

ScriptRunAction::~ScriptRunAction()
{
}

bool ScriptRunAction::TestParam(ScriptError* error,int linenumber)
{
   if (m_Parameters.size() <2)
   {
     error->SetError(MString("No enough parameter for Run"),linenumber);
     return false;
   }

   if (m_Parameters.size() >2)
   {
     error->SetWarning(MString("Too much parameter for Run"),linenumber);
     return false;
   }
  
  m_Manager->SetTestVariable(m_Parameters[0]);


  return (m_Manager->TestConvert(m_Parameters[1],linenumber));
}

MString ScriptRunAction::Help()
{
  return "Run(<outputvariable> 'program.exe param1 param2 ...')";
}

/** Parse the XML output and set some variables */
void ScriptRunAction::ParseXMLOutput(const char* output)
{
  typedef itk::XMLBufferReader XMLParserType;
  XMLParserType::Pointer xmlReader = XMLParserType::New();
  std::string Buffer = output;

  // Trim the buffer
  long int begin = Buffer.find("<?xml");
  std::string endstring = "</MetaOutputFile>";
  long int end = Buffer.find(endstring);

  if(begin == -1 || end == -1)
    {
    //std::cout << "ScriptRunAction::ParseXMLOutput cannot find XML description" 
    //          << std::endl;
    return;
    }

  Buffer = Buffer.substr(begin,end-begin+endstring.size());

  try
    {
    xmlReader->Parse(Buffer.c_str(),Buffer.size());
    }
  catch(...)
    {
    std::cout << "Cannot parse output" << std::endl;
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
        else if(!strncmp((*itValues).first.c_str(),"value",5))
          {
          if(value.size()>0)
            {
            value += " ";
            }
          value += "'"+(*itValues).second+"'";
          }
        itValues++;
        }
      // We only add the output values
      if((*it).name == "Output")
        {
        MString app = m_Parameters[1];
        app = app.removeChar('$');
        app = app.removeChar('{');
        app = app.removeChar('}');
        std::string fullname = app.toChar();
        fullname += "."+name;
        m_Manager->SetVariable(fullname,value);
        }
      }
    it++;
    }
}

/** Execute the action */
void ScriptRunAction::Execute()
{
#ifdef BM_GRID
  if(m_GridModule)
    {
    this->GenerateGrid(m_Parameters[1].toChar());
    return;
    }
#endif

  Launch m_launch;
  Timer m_timer;
  m_timer.start();
  m_launch.SetProgressManager(m_ProgressManager);

  m_ProgressManager->SetStatus(MString("Start ") + m_Manager->Convert(m_Parameters[1]).removeChar('\''));
  MString m_actioname = m_Manager->ConvertExtra(m_Parameters[1]);

  m_actioname = m_actioname.removeChar(' ',true);
  m_actioname = m_actioname.removeChar('\'',true);
  m_actioname = m_actioname.begin("'").rend("\\");
 
  m_ProgressManager->AddAction(m_actioname);

  m_launch.Execute(m_Manager->Convert(m_Parameters[1]).removeChar('\''));
  MString m_Output = m_launch.GetOutput();
  MString m_Error = m_launch.GetError();

  // Parse the output of the application and set the variables
  this->ParseXMLOutput(m_Output.toChar());

  m_Manager->SetVariable(m_Parameters[0],MString("'") + m_Output + "'");
  m_ProgressManager->SetStatus(MString("Finish: Execution time %1ms").arg(m_timer.getMilliseconds()));
  m_ProgressManager->FinishAction(MString("Execution time: %1ms").arg(m_timer.getMilliseconds()));
  
  // Display only the first 1000 errors in the manager
  int n=0;

  int m_Offset = 0;
  int m_Offset2 = 0;
  while (m_Offset != -1 && n<100)
    {
    m_Offset = m_Output.find("\n");
    m_ProgressManager->AddOutput(m_Output.begin("\n"));
    if (m_Offset != -1)
      {
      m_Output = m_Output.end("\n")+1;
      }
    n++;
    }

  m_Offset = 0;
  n=0;
  while (m_Offset != -1 && n<100)
    {
    m_Offset = m_Error.find("\n");
    m_ProgressManager->AddError(m_Error.begin("\n"));
    if (m_Offset != -1)
      {
      m_Error = m_Error.end("\n")+1;
      }
    n++;
    }

  m_timer.stop();
}

} // end namespace bm
