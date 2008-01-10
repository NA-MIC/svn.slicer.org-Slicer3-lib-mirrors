/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptEditorGUIControls.cxx,v $
  Language:  C++
  Date:      $Date: 2007/12/15 17:40:19 $
  Version:   $Revision: 1.13 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptEditorGUIControls.h"
#include "bmScriptParser.h"
#include "bmScriptErrorGUI.h"
#include "bmProgressManagerGUI.h"
#include "bmProgressGUIControls.h"
#include "bmApplicationListGUIControls.h"

#include <iostream>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Check_Button.H>
#include <bmSplashScreenControls.h>
#include <FL/Fl_Timer.H>

#include "bmLaunch.h"
#include "bmConfigure.h"

#ifndef WIN32
 #include <dirent.h>
 #include <stdio.h>
#endif

namespace bm {

ScriptEditorGUIControls::ScriptEditorGUIControls():ScriptEditorGUI()
{
  m_Filename = "";
  m_Parser = new ScriptParser();

  MakeWindow();

  // Center the window on the screen
  g_Scripteditorgui->position(Fl::w()/2-g_Scripteditorgui->w()/2,
                              Fl::h()/2-g_Scripteditorgui->h()/2);

  m_Errorgui = new ScriptErrorGUI();
  m_Errorbuffer = new Fl_Text_Buffer();
  g_output->buffer(m_Errorbuffer);

  // Give a pointer of the parser to the editor (to get the list of apps, etc...)
  g_editor->SetParser(m_Parser);
  g_editor->SetScriptEditorGUI(this);

#ifdef BM_GRID
  m_CondorWatcher = new CondorWatcher;
#endif

  m_WrappedApplicationsPath = "/Applications";
  m_InitFile = new XMLIniIO();
}

ScriptEditorGUIControls::~ScriptEditorGUIControls()
{
  delete m_Parser;
  delete m_InitFile;

#ifdef BM_GRID
  delete m_CondorWatcher;
#endif
}

void ScriptEditorGUIControls::SetBatchMakeBinaryPath(MString applicationpath)
{
  m_ApplicationPath = applicationpath;
  m_Parser->SetBatchMakeBinaryPath(m_ApplicationPath);
}


void ScriptEditorGUIControls::Timer(void* ui)
{
  int newvalue = (int)((SplashScreenControls*)ui)->g_progress->value()+1;
  ((SplashScreenControls*)ui)->g_progress->value((float)newvalue);
  if (((SplashScreenControls*)ui)->g_progress->value() == ((SplashScreenControls*)ui)->g_progress->maximum()+1)
  {
    ((SplashScreenControls*)ui)->g_Splashscreen->hide();
  }
  else
  {
     Fl::add_timeout(0.1,Timer,ui);
  }
}

void ScriptEditorGUIControls::Show()
{
  // disable the splash screen for the moment
#ifdef BM_SPLASHSCREEN
  if(!g_Scripteditorgui->shown())
  {
    SplashScreenControls* ui = new SplashScreenControls();
    g_Scripteditorgui->show();
    int x = g_Scripteditorgui->x() + (g_Scripteditorgui->h() - ui->g_Splashscreen->h())/2;
    int y = g_Scripteditorgui->y() + (g_Scripteditorgui->w() - ui->g_Splashscreen->w())/2;
    ui->g_Splashscreen->position(x,y);

    m_SplashBuffer = "BatchMake 1.0.4\n";
    m_SplashBuffer += "Copyright (c) 2007 Kitware Inc.\n\n";
#ifdef BM_DASHBOARD     
    m_SplashBuffer += "Dashboard Module: [enabled]\n";
#else
    m_SplashBuffer += "Dashboard Module: [disabled]\n";
#endif

#ifdef BM_GRID    
    m_SplashBuffer += "Grid Module: [enabled]\n";
#else
    m_SplashBuffer += "Grid Module: [disabled]\n";
#endif
     
    m_SplashBuffer += "\nWebsite: http://www.batchmake.org\n";
    ui->copyright->label(m_SplashBuffer.c_str());
    g_Scripteditorgui->show();
    ui->Show();
    ui->g_progress->maximum(100);
    Fl::add_timeout(0.1,Timer,ui);
  }
#endif

  std::cout << "BatchMake " << BatchMake_EXTENDED_VERSION_STRING 
            << " by Kitware Inc." << std::endl;

  //Load .ini
  m_InitFile->SetFileName(m_ApplicationPath + "/BatchMake.ini");
  if (m_InitFile->Read() != -1)
    {
    MString wrappedApplicationPath = m_InitFile->Find("WrappedApplicationsPath");
    if(wrappedApplicationPath.GetValue().size()>0)
      {      
      m_WrappedApplicationsPath = wrappedApplicationPath.toChar();  
      }  
    }
  else // if the init file doesn't exist we write the default value
    {
    std::string apppath = m_ApplicationPath.toChar();
    apppath += "/Applications";
    m_InitFile->Update("WrappedApplicationsPath",apppath.c_str());
    m_InitFile->Write();
    }

  // Load the applications wrapper list
  m_Parser->LoadWrappedApplication(m_WrappedApplicationsPath);

  g_Scripteditorgui->show();
  m_Title = MString(g_Scripteditorgui->label()) + " [script]";
  g_Scripteditorgui->label(m_Title.toChar());
  g_editor->SetParentWindow(g_Scripteditorgui);


}


void ScriptEditorGUIControls::OnOpenScript()
{
  const char* filename = 0;
  filename = fl_file_chooser("Load script", "BatchMake Script(*.bms)", NULL);
 
  if(filename)
    {
    m_Filename = filename;
    g_editor->Load(m_Filename.toChar());
    m_Parser->SetScriptPath(m_Filename);
    m_Title = MString(g_Scripteditorgui->label()).begin("[") + " [" + (m_Filename.rend("/")+1) + "]" ;
    g_Scripteditorgui->label(m_Title.toChar());
    }
}

void ScriptEditorGUIControls::hide()
{
  if (MString(g_Scripteditorgui->label()).find("*") != -1)
  OnSaveScript();     
  g_Scripteditorgui->hide();
}

void ScriptEditorGUIControls::OnSaveScript()
{
  int resultempty = 1;
  if(g_editor->IsBufferEmpty())
    {
    resultempty = fl_choice("Your script is empty! Are you sure you want to write it?!","Cancel","Ok",0L);
    }
  if (resultempty == 0)
    {
    return;
    }


  const char* filename = 0;
  int result = 2;
  if (m_Filename == "")
    {
    result = 2;
    }
  else
    {
    result = fl_choice("Script already exists !","Cancel","Overwrite","Save As ...");
    }

  if (result == 0)
    {
    return;
    }

  if (result == 2)
    {
    filename = fl_file_chooser("Save script", "BatchMake Script(*.bms)", NULL);
    }

  if(filename || (m_Filename != ""))
    {
    if (filename != 0)
      {
      m_Filename = filename;
      if (MString(m_Filename).rend(".") != ".bms")
        {
        m_Filename = MString(m_Filename) +  ".bms";
        }
      }

    g_editor->Save(m_Filename.toChar());
    m_Parser->SetScriptPath(m_Filename);

    m_Title = MString(g_Scripteditorgui->label()).begin("[") + " [" + (m_Filename.rend("/")+1) + "]" ;
    g_Scripteditorgui->label(m_Title.toChar());
    }
}


void ScriptEditorGUIControls::OnCompile()
{
  m_Errorbuffer->text("");
  m_Parser->Reset();
  m_Errorgui->Reset();
  m_Errorgui->SetTextDisplay(g_output);
  m_Parser->SetCurrentFilename(m_Filename.toChar());
  m_Parser->SetError(m_Errorgui);
  int m_Offset = 0;
  for (int i=0;i<g_editor->buffer()->count_lines(0,g_editor->buffer()->length())+1;i++)
    {
    const char* text = g_editor->buffer()->line_text(m_Offset);
    m_Parser->AddCodeLine(MString(text));
    m_Offset = g_editor->buffer()->line_end(m_Offset)+1;
    }
  m_Errorgui->SetStatus(MString("Compiling ..."));
  m_Parser->Parse();
  m_Errorgui->DisplaySummary();
}

void ScriptEditorGUIControls::OnExecute()
{
  m_Parser->Reset();
  m_Errorgui->Reset();
  m_Errorbuffer->text("");
  m_Errorgui->SetTextDisplay(g_output);
  m_Parser->SetError(m_Errorgui);
  int m_Offset = 0;
  for (int i=0;i<g_editor->buffer()->count_lines(0,g_editor->buffer()->length())+1;i++)
    {
    const char* text = g_editor->buffer()->line_text(m_Offset);
    m_Parser->AddCodeLine(MString(text));
    m_Offset = g_editor->buffer()->line_end(m_Offset)+1;
    }

  m_Errorgui->SetStatus(MString("Compiling ..."));
    
  ProgressManagerGUI* m_ProgressManagergui = new ProgressManagerGUI();
  ProgressGUIControls* m_Progressgui = new ProgressGUIControls();
  m_Progressgui->g_progress->box( FL_DOWN_BOX );
  // m_Progressgui->g_progress->auto_branches( true );  
  m_Progressgui->g_progress->show_leaves( true );
  m_Progressgui->g_progress->show_branches( true );
  m_Progressgui->g_progress->get_root()->always_open( true );
  m_Progressgui->g_progress->insertion_mode(FLU_INSERT_BACK);

  m_Progressgui->g_progress->label( "BatchMake processing ..." );
  m_ProgressManagergui->SetProgressGUI(m_Progressgui);
  m_Parser->SetProgressManager(m_ProgressManagergui);

  if (m_Parser->Parse())
    {
    m_Errorgui->DisplaySummary();
    m_Errorgui->SetStatus(MString("Running ..."));
    m_Progressgui->Show();
    m_Parser->Execute();
    }
  else
    {
    m_Errorgui->DisplaySummary();
    }
}

/** Launch the application wrapper */
void ScriptEditorGUIControls::OnApplicationWrapper()
{
  ApplicationListGUIControls* ui = new ApplicationListGUIControls();
  m_Parser->LoadWrappedApplication(m_WrappedApplicationsPath);
  ui->SetEditorGUI(this);
  ui->SetWrappedApplicationPath(m_WrappedApplicationsPath);
  ui->SetApplicationList(m_Parser->GetApplicationList());
  ui->Show();
}

/** Generate a KWGrid script */
void ScriptEditorGUIControls::OnGenerateGAD()
{
#ifdef BM_GRID
  const char* filename = 0;
  filename = fl_file_chooser("Save KWGrid script", "Grid Script(*.gad)", NULL);

  if(!filename)
    {
    return;
    }

  // Check first if the script is valid
  m_Parser->Reset();
  m_Errorgui->Reset();
  m_Errorbuffer->text("");
  m_Errorgui->SetTextDisplay(g_output);
  m_Parser->SetError(m_Errorgui);
  int m_Offset = 0;
  for (int i=0;i<g_editor->buffer()->count_lines(0,g_editor->buffer()->length())+1;i++)
    {
    const char* text = g_editor->buffer()->line_text(m_Offset);
    m_Parser->AddCodeLine(MString(text));
    m_Offset = g_editor->buffer()->line_end(m_Offset)+1;
    }

  m_Errorgui->SetStatus(MString("Generating kwgrid script ..."));
    
  Grid grid;
  grid.SetFileName(filename);
  m_Parser->SetGridModule(&grid);
  if (m_Parser->Parse())
    {
    m_Errorgui->DisplaySummary();
    m_Parser->Execute();
    }
  else
    {
    m_Errorgui->DisplaySummary();
    }

  grid.WriteGAD();

  m_Parser->SetGridModule(NULL);
  m_Errorgui->SetStatus(MString("Generation done.")); 
#else
  fl_alert("To generate grid script turn GRID_SUPPORT=ON when configuring with CMake.");
#endif
}

/** Generate a Condor script */
void ScriptEditorGUIControls::OnGenerateCondor()
{
#ifdef BM_GRID
  const char* filename = 0;
  filename = fl_file_chooser("Save Condor script", "Condor Script(*.bmc)", NULL);

  if(!filename)
    {
    return;
    }

  // Check first if the script is valid
  m_Parser->Reset();
  m_Errorgui->Reset();
  m_Errorbuffer->text("");
  m_Errorgui->SetTextDisplay(g_output);
  m_Parser->SetError(m_Errorgui);
  m_Parser->SetCurrentFilename(m_Filename.toChar());

  int m_Offset = 0;
  for (int i=0;i<g_editor->buffer()->count_lines(0,g_editor->buffer()->length())+1;i++)
    {
    const char* text = g_editor->buffer()->line_text(m_Offset);
    m_Parser->AddCodeLine(MString(text));
    m_Offset = g_editor->buffer()->line_end(m_Offset)+1;
    }

  m_Errorgui->SetStatus(MString("Generating condor script ..."));
    
  Grid grid;
  grid.SetFileName(filename);
  m_Parser->SetGridModule(&grid);
  if (m_Parser->Parse())
    {
    m_Errorgui->DisplaySummary();
    m_Parser->Execute();
    }
  else
    {
    m_Errorgui->DisplaySummary();
    }

  grid.WriteCondor();
  
  if(fl_ask("Run condor watcher?"))
    {
    m_CondorWatcher->Window->show();
    m_CondorWatcher->Watch();
    }

  if(fl_ask("Run current script?"))
    {
    Launch starter;
    std::string run = "condor_submit_dag ";
    run += filename;
    std::cout << "Running Condor script " << run.c_str() << std::endl;
    starter.Execute(run.c_str());
    MString output = starter.GetOutput();
    output = output.replaceChar(13,' ');
    output = output.replaceChar(10,' ');
    m_Errorgui->SetStatus(output);
    MString error = starter.GetError();
    error = error.replaceChar(13,' ');
    error = error.replaceChar(10,' ');
    m_Errorgui->SetStatus(error);
    }

  m_Parser->SetGridModule(NULL);
  m_Errorgui->SetStatus(MString("Generation done.")); 
#else
  fl_alert("To generate grid script turn GRID_SUPPORT=ON when configuring with CMake.");
#endif
}

/** Generate a Shell script */
void ScriptEditorGUIControls::OnGenerateShell()
{
#ifdef BM_GRID
  const char* filename = 0;
  filename = fl_file_chooser("Save Shell script", "Shell script (*.sh)", NULL);

  if(!filename)
    {
    return;
    }

  // Check first if the script is valid
  m_Parser->Reset();
  m_Errorgui->Reset();
  m_Errorbuffer->text("");
  m_Errorgui->SetTextDisplay(g_output);
  m_Parser->SetError(m_Errorgui);
  int m_Offset = 0;
  for (int i=0;i<g_editor->buffer()->count_lines(0,g_editor->buffer()->length())+1;i++)
    {
    const char* text = g_editor->buffer()->line_text(m_Offset);
    m_Parser->AddCodeLine(MString(text));
    m_Offset = g_editor->buffer()->line_end(m_Offset)+1;
    }

  m_Errorgui->SetStatus(MString("Generating shell script ..."));
    
  Grid grid;
  grid.SetFileName(filename);
  m_Parser->SetGridModule(&grid);
  if (m_Parser->Parse())
    {
    m_Errorgui->DisplaySummary();
    m_Parser->Execute();
    }
  else
    {
    m_Errorgui->DisplaySummary();
    }

  grid.WriteShell();

  m_Parser->SetGridModule(NULL);
  m_Errorgui->SetStatus(MString("Generation done.")); 
#else
  fl_alert("To generate grid script turn GRID_SUPPORT=ON when configuring with CMake.");
#endif
}

/** Generate a batch scripts */
void ScriptEditorGUIControls::OnGenerateScripts()
{
  // Check first if the script is valid
  this->OnCompile();
}

} // end namespace bm
