/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmApplicationListGUIControls.cxx,v $
  Language:  C++
  Date:      $Date: 2007/03/30 21:03:36 $
  Version:   $Revision: 1.2 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/
#include "bmScriptEditorGUIControls.h" // should be first
#include "bmApplicationListGUIControls.h"
#include "bmApplicationWrapperGUIControls.h"

#include <FL/fl_ask.H>
#include <itksys/SystemTools.hxx>
#include <FL/Fl_File_Chooser.H>


namespace bm {

ApplicationListGUIControls::ApplicationListGUIControls()
:ApplicationListGUI()
{
  make_window();
  g_list->box( FL_DOWN_BOX );
  g_list->show_leaves( true );
  g_list->show_branches( true );
  g_list->get_root()->always_open( true );
  g_list->show_root( false );
  g_list->insertion_mode(FLU_INSERT_BACK);
  g_list->selection_color(97);
  m_WrappedApplicationsPath = "/Applications";
  m_EditorGUI = NULL;  
}

ApplicationListGUIControls::~ApplicationListGUIControls()
{
}


void ApplicationListGUIControls::SetWrappedApplicationPath(MString applicationpath)
{
  m_WrappedApplicationsPath = applicationpath;
}

void ApplicationListGUIControls::SetApplicationList(std::vector<ApplicationWrapper*>* applicationlist)
{
  m_applicationlist = applicationlist;
  //Update list
  for (unsigned int i=0;i<m_applicationlist->size();i++)
    {
    g_list->add((*m_applicationlist)[i]->GetName().toChar());
    if (i==0)
      {
      g_list->select_all();
      }
    }
}

void ApplicationListGUIControls::Show()
{
  g_path->value(m_WrappedApplicationsPath.toChar());
  if(!g_Applicationlistgui->shown())
    {
    g_Applicationlistgui->show();
    }
}

void ApplicationListGUIControls::OnAccept()
{
  g_Applicationlistgui->hide();
}


void ApplicationListGUIControls::OnSelectDir()
{
  char* fDir = fl_dir_chooser("Select a directory where the description of the application should go","");

  if(fDir)
    {
    g_path->value(fDir);
    this->OnChangePath();
    }
}

void ApplicationListGUIControls::OnChangePath()
{
  const char* fDir = g_path->value();
  m_WrappedApplicationsPath = fDir;
  if(m_EditorGUI)
    {
    g_list->clear();
    m_EditorGUI->GetInitFile()->Update("WrappedApplicationsPath",fDir);
    m_EditorGUI->GetInitFile()->Write();
    m_EditorGUI->SetWrappedApplicationsPath(fDir);
    m_EditorGUI->GetParser()->LoadWrappedApplication(m_WrappedApplicationsPath);
    this->SetApplicationList(m_EditorGUI->GetParser()->GetApplicationList());
    }
}

void ApplicationListGUIControls::OnNew()
{
  ApplicationWrapperGUIControls* ui = new ApplicationWrapperGUIControls();
  ui->SetApplicationListGUIControls(this);
  ui->SetWrappedApplicationsPath(m_WrappedApplicationsPath);
  ui->Show();
}

void ApplicationListGUIControls::OnEdit()
{
  if (g_list->num_selected() == 0)
  {
    fl_alert("Select a module to edit !");
    return;
  }

  int m_index = g_list->get_selected(1)->index();
  if (m_index < 0) m_index = 0;
  ApplicationWrapperGUIControls* ui = new ApplicationWrapperGUIControls();

  ui->SetApplicationListGUIControls(this);
  ui->SetWrappedApplicationsPath(m_WrappedApplicationsPath);
  ui->SetApplicationWrapper((*m_applicationlist)[m_index]);
  ui->Refresh();
  g_Applicationlistgui->hide();
  ui->Show();
}

void ApplicationListGUIControls::OnRemove()
{
  if (g_list->num_selected() == 0)
    {
    fl_alert("Select an application to remove!");
    return;
    }
  int index = g_list->get_selected(1)->index();
  if(index < 0)
    {
    return;
    }
  g_list->remove(index+1);
  g_list->sort();

  // Remove the actual file application file
  std::vector<ApplicationWrapper*>::iterator it = m_applicationlist->begin();
  for(int i=0;i<index;i++)
    {
    it++;
    }
  
  ApplicationWrapper* wrapper = *it;
  std::string app = m_WrappedApplicationsPath.toChar();
  app += "/";
  app += wrapper->GetName().toChar();
  app += ".bmm";
  itksys::SystemTools::RemoveFile(app.c_str());

  // Remove from the applications list
  m_applicationlist->erase(it);
}


void ApplicationListGUIControls::OnSelect()
{
}

void ApplicationListGUIControls::Update(MString name)
{
  g_list->get_selected(1)->label(name.toChar());
}

void ApplicationListGUIControls::UpdateNew(ApplicationWrapper* wrapper)
{
  g_list->add(wrapper->GetName().toChar());
  m_applicationlist->push_back(wrapper);
}

} // end namespace bm
