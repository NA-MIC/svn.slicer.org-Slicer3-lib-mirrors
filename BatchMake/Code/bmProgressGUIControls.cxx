/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmProgressGUIControls.cxx,v $
  Language:  C++
  Date:      $Date: 2007/01/28 18:30:48 $
  Version:   $Revision: 1.3 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmProgressGUIControls.h"
#include <iostream>
#include <FL/Fl_File_Chooser.H>
#include "ApplicationWrapperParam.h"
#include <FL/Fl_Check_Button.H>

#include "MString.h"

namespace bm {

ProgressGUIControls::ProgressGUIControls():ProgressGUI()
{
  make_window();
  m_Buffer = new Fl_Text_Buffer();
  g_output->buffer(m_Buffer);
  g_progress->set_hilighted( g_progress->get_root());
  g_progress->box( FL_DOWN_BOX );
  g_progress->show_root(false);
  g_progress->branch_text(g_progress->branch_color(),g_progress->branch_font(),10);
  g_progress->leaf_text(g_progress->leaf_color(),g_progress->leaf_font(),10);
  g_progress->selection_color(20);
  g_progress->show_leaves( true );
  g_progress->show_branches( true );
  g_progress->get_root()->always_open( true );
  g_progress->insertion_mode(FLU_INSERT_BACK);
  g_progress->selection_follows_hilight(true);
  g_progress->selection_mode(FLU_SINGLE_SELECT);
}

ProgressGUIControls::~ProgressGUIControls()
{
}

void ProgressGUIControls::Show()
{
  if(!g_Progressgui->shown())
    g_Progressgui->show();
}

void ProgressGUIControls::OnCancel()
{
  if (MString(g_cancel->label()) == "Ok")
    g_Progressgui->hide();
  
  if (m_ProgressManagergui)
    m_ProgressManagergui->Stop();

}

void ProgressGUIControls::SetProgressManagerGUI(ProgressManagerGUI* progressmanagergui)
{
  m_ProgressManagergui = progressmanagergui;
}

} // end namespace bm
