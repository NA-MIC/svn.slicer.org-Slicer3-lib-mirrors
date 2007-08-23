/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmEditor.h,v $
  Language:  C++
  Date:      $Date: 2007/02/06 15:28:25 $
  Version:   $Revision: 1.3 $

  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __bmEditor_h
#define __bmEditor_h

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include "FL/Fl_Text_Editor.H"
#include "FL/fl_ask.H"
#include "FL/Fl_Browser.H"
#include "Fl/Fl_Output.h"
#include "FLU/Flu_Tree_Browser.h"
#include <FL/Fl_Window.H>
#include "MString.h"
#include <list>

#include <errno.h>
#include "bmScriptParser.h"
#include <vector>

namespace bm {

class Editor : public Fl_Text_Editor
{
public:

  Editor(int X, int Y, int W, int H, const char* l=0);
  ~Editor();

  typedef ScriptActionManager::ApplicationWrapperListType ApplicationWrapperListType;
  typedef std::pair<std::string,ApplicationWrapper*>      ApplicationNameType;

  static void style_parse(const char *text, char *style, int length);
  static void style_update(int pos,          // I - Position of update
             int        nInserted,    // I - Number of inserted chars
             int        nDeleted,     // I - Number of deleted chars
             int        nRestyled,    // I - Number of restyled chars
             const char *deletedText, // I - Text that was deleted
             void       *cbArg);
  static int compare_keywords(const void *a,const void *b);
  static void style_unfinished_cb(int, void*);

  /** Callback when a tab completion is done on an action */
  static void SelectOption(Flu_Tree_Browser*, void*);

  /** Callback when a tab completion is done on an application */
  static void SelectApplication(Flu_Tree_Browser*, void*);

  /** Callback when a tab completion is done on an application */
  static void SelectApplicationOption(Flu_Tree_Browser*, void*);

  /** Add the applications to browse (by SetApp()) */
  void AddApplicationsToBrowse();

  void Load(const char* filename);
  void Save(const char* filename);
  void ShowOptionFinder(int x,int y);
  void draw();
  static void TabPressed(void*);
  int handle( int event );
  static bool Find(std::list<MString> array,MString key);
  void UpdateKeyword();
  void UpdateApplicationsList();
  void UpdateVariable();
  void SetParentWindow(Fl_Window* parentwindow);
  static void SetModified(bool flag);
  
  /** Return true if the current buffer is empty. 
   *  i.e the script is empty*/
  bool IsBufferEmpty();

  /** Set the action manager and initialize everything */
  void SetParser(ScriptParser* parser);

  /** Show the options for an applications */
  bool ShowApplicationOptions(const char* appVarName);

  /** Set the script editor GUI */
  void SetScriptEditorGUI(void* editorGUI) {m_ScriptEditorGUI=editorGUI;}

protected:

  Fl_Text_Buffer*   m_Stylebuf;
  Fl_Text_Buffer*   m_Buffer;
  Flu_Tree_Browser* m_Browser;
  Flu_Tree_Browser* m_ApplicationBrowser;
  Flu_Tree_Browser* m_ApplicationOptionBrowser;
  Fl_Output*        m_Helper;
  bool              m_DrawBrowser;
  bool              m_DrawApplicationBrowser;
  bool              m_DrawApplicationOptionBrowser;
  bool              m_DrawHelper;
  MString           m_CurrentWord;

  void*                m_ScriptEditorGUI;
  ScriptParser*        m_Parser;
  ScriptActionManager* m_Manager;

  // List of variable use for applications
  std::vector<ApplicationNameType> m_ApplicationsList;

};

} // end namespace bm

#endif
