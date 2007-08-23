/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: BatchMake.cxx,v $
  Language:  C++
  Date:      $Date: 2007/08/03 14:04:26 $
  Version:   $Revision: 1.16 $

  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifdef BM_USE_FLTK
  #include "bmScriptEditorGUIControls.h"
  #include "FL/Fl.H"
  #include "ApplicationWrapper.h"
#endif

#include "MString.h"

#include <iostream>
#include "metaCommand.h"
#ifdef BM_GRID
  #include "bmGrid.h"
#endif
 
#include "bmScriptParser.h"
#include "bmConfigure.h"

int main(int argc, char **argv)
{
#ifdef WIN32
  MString m_ApplicationPath = MString(argv[0]).rbegin("\\");
  m_ApplicationPath += "\\Applications";
#else
  MString m_ApplicationPath = MString(argv[0]).rbegin("/");
  m_ApplicationPath += "/Applications";
#endif

#ifdef BM_USE_FLTK
  if (argc < 2)
    {
    // Create a UI object
    bm::ScriptEditorGUIControls* ui = new bm::ScriptEditorGUIControls();
    MString m_windowtitle("BatchMake - ");
    m_windowtitle += BatchMake_EXTENDED_VERSION_STRING;
    ui->g_Scripteditorgui->label(m_windowtitle.toChar());
    ui->SetBatchMakeBinaryPath(m_ApplicationPath);
    // Initialize FLTK
    Fl::visual(FL_DOUBLE|FL_INDEX); 
    Fl::background(236,233,216);

    // Show BatchMake Editor Interface
    ui->Show();
  
    // Run the FL driver
    Fl::run();
  
    delete ui;
    }
  else
#endif
    { 
    MetaCommand command;
  
    command.SetName("BatchMake");
    command.SetVersion(BatchMake_EXTENDED_VERSION_STRING);
    command.SetAuthor("Kitware Inc");
    command.SetDescription("BatchMake");

    // Compile a script
    command.SetOption("compileScript","c",false,"Compile a script");
    command.AddOptionField("compileScript","filename",MetaCommand::STRING,true);
    
    // Execute a script
    command.SetOption("executeScript","e",false,"Execute a script");
    command.AddOptionField("executeScript","filename",MetaCommand::STRING,true);

    // Specify batchmake path
    command.SetOption("path","p",false,"Specify a the batchmake path");
    command.AddOptionField("path","path",MetaCommand::STRING,true);

    // Add application
    command.SetOption("addApplication","a",false,"Add an application");
    command.AddOptionField("addApplication","appname",MetaCommand::STRING,true);

    // Generate Shell
    command.SetOption("generateShell","sh",false,"Generate shell script");
    command.AddOptionField("generateShell","scriptname",MetaCommand::STRING,true);
    command.AddOptionField("generateShell","outputname",MetaCommand::STRING,true);

    // Generate Condor
    command.SetOption("generateCondor","condor",false,"Generate condor script");
    command.AddOptionField("generateCondor","scriptname",MetaCommand::STRING,true);
    command.AddOptionField("generateCondor","outputname",MetaCommand::STRING,true);

    // Generate GAD
    command.SetOption("generateGAD","kwgrid",false,"Generate gad script");
    command.AddOptionField("generateGAD","scriptname",MetaCommand::STRING,true);
    command.AddOptionField("generateGAD","outputname",MetaCommand::STRING,true);

    // Parsing
    if(!command.Parse(argc,argv))
      {
      return 1;
      }
      
    if(command.GetOptionWasSet("path"))
      {
      m_ApplicationPath = command.GetValueAsString("path","path");
      }

    if(command.GetOptionWasSet("compileScript"))
      {
      std::string filename = command.GetValueAsString("compileScript","filename");
      std::cout << "Compiling ..." << std::endl;
      bm::ScriptParser m_Parser;
      m_Parser.LoadWrappedApplication(m_ApplicationPath.toChar());
      m_Parser.SetBatchMakeBinaryPath(m_ApplicationPath.toChar());
      m_Parser.Compile(filename);
      return 0;
      }
    if(command.GetOptionWasSet("executeScript"))
      {
      std::string filename = command.GetValueAsString("executeScript","filename");
      std::cout << "Executing ..." << std::endl;
      bm::ScriptParser m_Parser;
      m_Parser.LoadWrappedApplication(m_ApplicationPath.toChar());
      m_Parser.SetBatchMakeBinaryPath(m_ApplicationPath.toChar());
      m_Parser.Execute(filename);
      return 0;
      }
    if(command.GetOptionWasSet("addApplication"))
      {
      std::string appname = command.GetValueAsString("addApplication","appname");
      ApplicationWrapper m_ApplicationWrapper;
      m_ApplicationWrapper.AutomaticCommandLineParsing(appname.c_str());
      std::string moduleName = m_ApplicationWrapper.GetName().toChar();
       
      std::string output = m_ApplicationPath.toChar();
      output += "/";
      output += moduleName; 
      output += ".bmm";
      std::cout << "Saving application description as: " 
                << output.c_str() << std::endl;
      m_ApplicationWrapper.Save(output.c_str());
 
      return 0;
      }
    if(command.GetOptionWasSet("generateShell"))
      {
#ifdef BM_GRID 
      std::string scriptname = command.GetValueAsString("generateShell","scriptname");
      std::string outputname = command.GetValueAsString("generateShell","outputname");
      bm::ScriptParser m_Parser;
      m_Parser.LoadWrappedApplication(m_ApplicationPath.toChar());
      m_Parser.SetBatchMakeBinaryPath(m_ApplicationPath);
     
      std::cout << "Generating shell script ...";
      bm::Grid grid;
      grid.SetFileName(outputname.c_str());
      m_Parser.SetGridModule(&grid);
      m_Parser.Execute(scriptname);
      grid.WriteShell();
#else
      std::cout << "Cannot write Shell scripts with this version of BatchMake."
                << " Please contact Kitware for more information." << std::endl;
#endif
      std::cout << "Done." << std::endl;
      }
    if(command.GetOptionWasSet("generateCondor"))
      {
#ifdef BM_GRID
      std::string scriptname = command.GetValueAsString("generateCondor","scriptname");
      std::string outputname = command.GetValueAsString("generateCondor","outputname");
      bm::ScriptParser m_Parser;
      m_Parser.LoadWrappedApplication(m_ApplicationPath.toChar());
      m_Parser.SetBatchMakeBinaryPath(m_ApplicationPath);
     
      std::cout << "Generating condor script ...";
      bm::Grid grid;
      grid.SetFileName(outputname.c_str());
      m_Parser.SetGridModule(&grid);
      m_Parser.Execute(scriptname);
      grid.WriteCondor();
      std::cout << "Done." << std::endl;
#else
      std::cout << "Cannot write Condor scripts with this version of BatchMake."
                << " Please contact Kitware for more information." << std::endl;
#endif
      }
    if(command.GetOptionWasSet("generateGAD"))
      {
#ifdef BM_GRID
      std::string scriptname = command.GetValueAsString("generateGAD","scriptname");
      std::string outputname = command.GetValueAsString("generateGAD","outputname");
      bm::ScriptParser m_Parser;
      m_Parser.LoadWrappedApplication(m_ApplicationPath.toChar());
      m_Parser.SetBatchMakeBinaryPath(m_ApplicationPath);
     
      std::cout << "Generating kwgrid script ...";
        
      bm::Grid grid;
      grid.SetFileName(outputname.c_str());
      m_Parser.SetGridModule(&grid);
      m_Parser.Execute(scriptname);
      grid.WriteGAD();
      std::cout << "Done." << std::endl;
#else
      std::cout << "Cannot write KWGrid scripts with this version of BatchMake."
                << " Please contact Kitware for more information." << std::endl;
#endif
      }
    }
  return 0;
}
