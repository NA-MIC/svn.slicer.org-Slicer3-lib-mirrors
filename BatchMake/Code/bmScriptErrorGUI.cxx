/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptErrorGUI.cxx,v $
  Language:  C++
  Date:      $Date: 2007/01/28 18:30:48 $
  Version:   $Revision: 1.2 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptErrorGUI.h"

namespace bm {

ScriptErrorGUI::ScriptErrorGUI() 
: ScriptError()
{
  m_TextDisplay = 0;
}

ScriptErrorGUI::~ScriptErrorGUI()
{
}

void ScriptErrorGUI::SetTextDisplay(Fl_Text_Display* textdisplay)
{
  m_TextDisplay = textdisplay;
}

void ScriptErrorGUI::SetError(MString error,int linenumber)
{
   if (m_TextDisplay)
   {
    if (linenumber != -1)
     m_TextDisplay->buffer()->append((MString(" Error (%1): ").arg(linenumber) + error + "\n").toChar());
    else
     m_TextDisplay->buffer()->append((MString(" Error: ").arg(linenumber) + error + "\n").toChar());
   }

    nb_error++;
}

void ScriptErrorGUI::SetWarning(MString warning,int linenumber)
{
   if (m_TextDisplay)
      m_TextDisplay->buffer()->append((MString(" Warning (%1): ").arg(linenumber) + warning + "\n").toChar());
  nb_warning++;
}

void ScriptErrorGUI::SetStatus(MString status)
{
   if (m_TextDisplay)
      m_TextDisplay->buffer()->append( (status + "\n").toChar());
}

void ScriptErrorGUI::DisplaySummary()
{
  if (m_TextDisplay)
     m_TextDisplay->buffer()->append((MString("%1 error(s), ").arg(nb_error) +  MString("%1 warning(s)").arg(nb_warning) + "\n").toChar());

}

} // end namespace bm
