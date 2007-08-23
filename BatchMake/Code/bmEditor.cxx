/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmEditor.cxx,v $
  Language:  C++
  Date:      $Date: 2007/03/06 20:24:40 $
  Version:   $Revision: 1.6 $

  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "bmEditor.h"
#include "MString.h"
#include "bmScriptEditorGUIControls.h"

namespace bm {

Fl_Text_Display::Style_Table_Entry styletable[] = {  // Style table
         { FL_BLACK,      FL_COURIER,        12 }, // A - Plain
         { FL_DARK_GREEN, FL_COURIER_ITALIC, 12 }, // B - Line comments
         { FL_RED,        FL_COURIER,        12 }, // C - Variables
         { FL_BLUE,       FL_COURIER,        12 }, // D - Strings
         { FL_DARK_RED,   FL_COURIER,        12 }, // E - Directives
         { FL_DARK_RED,   FL_COURIER_BOLD,   12 }, // F - Types
         { FL_BLUE,       FL_COURIER_BOLD,   12 },  // G - Keywords
         { FL_RED,         FL_COURIER,   12 }       // H - Keywords
};


static std::list<MString> m_keywords;

const char *code_types[] = {  // List of known C/C++ types...
    "else",
    "endforeach",
    "endif",  
    "foreach",
    "if",
    "null"};

static Fl_Window* m_Parentwindow;
char m_Title[256];

Editor::Editor(int X, int Y, int W, int H, const char* l)
:Fl_Text_Editor(X, Y, W, H, l)
{
  m_Parser = NULL;
   
  // Specification for the word completion browser (for actions)  
  m_DrawBrowser = false;
  m_Browser = new Flu_Tree_Browser(0, 0,200,80);
  m_Browser->box( FL_DOWN_BOX );
  m_Browser->show_root(false);
  m_Browser->branch_text(m_Browser->branch_color(),m_Browser->branch_font(),10);
  m_Browser->leaf_text(m_Browser->leaf_color(),m_Browser->leaf_font(),10);
  m_Browser->vertical_gap(-2);
  m_Browser->selection_color(87);
  m_Browser->show_leaves( true );
  m_Browser->show_branches( true );
  m_Browser->get_root()->always_open( true );
  m_Browser->insertion_mode(FLU_INSERT_BACK);
  m_Browser->selection_follows_hilight(true);
  m_Browser->selection_mode(FLU_SINGLE_SELECT);
  m_Browser->callback((Fl_Callback*)SelectOption,this);
  m_Browser->resize(13,15,200,80);
  m_Browser->end();

  // Specification for the applications completion browser (for wrapped applications)
  m_DrawApplicationBrowser = false;
  m_ApplicationBrowser = new Flu_Tree_Browser(0, 0,200,80);
  m_ApplicationBrowser->box( FL_DOWN_BOX );
  m_ApplicationBrowser->show_root(false);
  m_ApplicationBrowser->branch_text(m_ApplicationBrowser->branch_color(),
                                    m_ApplicationBrowser->branch_font(),10);
  m_ApplicationBrowser->leaf_text(m_ApplicationBrowser->leaf_color(),
                                  m_ApplicationBrowser->leaf_font(),10);
  m_ApplicationBrowser->vertical_gap(-2);
  m_ApplicationBrowser->selection_color(87);
  m_ApplicationBrowser->show_leaves( true );
  m_ApplicationBrowser->show_branches( true );
  m_ApplicationBrowser->get_root()->always_open( true );
  m_ApplicationBrowser->insertion_mode(FLU_INSERT_BACK);
  m_ApplicationBrowser->selection_follows_hilight(true);
  m_ApplicationBrowser->selection_mode(FLU_SINGLE_SELECT);
  m_ApplicationBrowser->callback((Fl_Callback*)SelectApplication,this);
  m_ApplicationBrowser->resize(13,15,200,80);
  m_ApplicationBrowser->end();

  // Specification for the applications completion browser (for wrapped applications)
  m_DrawApplicationOptionBrowser = false;
  m_ApplicationOptionBrowser = new Flu_Tree_Browser(0, 0,200,80);
  m_ApplicationOptionBrowser->box( FL_DOWN_BOX );
  m_ApplicationOptionBrowser->show_root(false);
  m_ApplicationOptionBrowser->branch_text(m_ApplicationOptionBrowser->branch_color(),
                                          m_ApplicationOptionBrowser->branch_font(),10);
  m_ApplicationOptionBrowser->leaf_text(m_ApplicationOptionBrowser->leaf_color(),
                                        m_ApplicationOptionBrowser->leaf_font(),10);
  m_ApplicationOptionBrowser->vertical_gap(-2);
  m_ApplicationOptionBrowser->selection_color(87);
  m_ApplicationOptionBrowser->show_leaves( true );
  m_ApplicationOptionBrowser->show_branches( true );
  m_ApplicationOptionBrowser->get_root()->always_open( true );
  m_ApplicationOptionBrowser->insertion_mode(FLU_INSERT_BACK);
  m_ApplicationOptionBrowser->selection_follows_hilight(true);
  m_ApplicationOptionBrowser->selection_mode(FLU_SINGLE_SELECT);
  m_ApplicationOptionBrowser->callback((Fl_Callback*)SelectApplicationOption,this);
  m_ApplicationOptionBrowser->resize(13,15,200,80);
  m_ApplicationOptionBrowser->end();

  m_DrawHelper = false;
  m_Helper = new Fl_Output(0, 0,200,15);
  m_Helper->resize(0,0,200,15);
  m_Helper->box(FL_BORDER_BOX);
  m_Helper->color((Fl_Color)215);
  m_Helper->labelsize(10);
  m_Helper->textsize(10);

  m_Buffer = new Fl_Text_Buffer();
  char *style = new char[3];
  char *text = m_Buffer->text();
  style[0] = '\0';
  m_Stylebuf = new Fl_Text_Buffer(m_Buffer->length());
  m_Stylebuf->text(style);
  delete [] style;
  style = NULL;
  delete [] text;
  text = NULL;

  buffer(m_Buffer);
  highlight_data(m_Stylebuf, styletable,
                          sizeof(styletable) / sizeof(styletable[0]),
                          'A', style_unfinished_cb, 0);

  m_Buffer->add_modify_callback(style_update, this);
  m_Buffer->call_modify_callbacks();

  m_CurrentWord = "";
  m_Parentwindow = 0;
  m_ScriptEditorGUI = NULL;
}

/** Desctructor */
Editor::~Editor()
{
}


/** Return true if the current script is empty */
bool Editor::IsBufferEmpty()
{
  if(m_Buffer->length() == 0)
    {
    return true;
    }
  return false;
}


/** Set the parser and initialize */
void Editor::SetParser(ScriptParser* parser)
{
  m_Parser = parser;
  m_Manager = m_Parser->GetScriptActionManager();
  m_Manager->SetApplicationsList(&m_ApplicationsList);
  this->UpdateKeyword();
}

/** Update the list of wrapped applications */
void Editor::UpdateApplicationsList()
{ 
  if(!m_Manager->GetApplicationWrapperList())
    {
    return;
    }

  m_ApplicationBrowser->clear();
  
  ScriptActionManager::ApplicationWrapperListType::const_iterator it = 
                              m_Manager->GetApplicationWrapperList()->begin();
  if(m_Manager->GetApplicationWrapperList()->size() == 0)
    {
    return;
    }

  while (it != m_Manager->GetApplicationWrapperList()->end())
    { 
    m_ApplicationBrowser->add((*it)->GetName().toChar());
    it++;
    }

  Flu_Tree_Browser::Node* m_node = m_ApplicationBrowser->first();
  m_node = m_node->next();
  m_node->select_only();
  //m_ApplicationBrowser->set_hilighted(m_node);
}

/** Update the list of keywords */
void Editor::UpdateKeyword()
{ 
  m_keywords.clear();
  m_Browser->clear();

  std::vector<MString> m_list = m_Manager->GetKeywordList();
  for (unsigned int i=0;i<m_list.size();i++)
    { 
    m_keywords.push_back(m_list[i]);
    }
   
  m_keywords.sort();
  std::list<MString>::iterator it = m_keywords.begin();
  while (it != m_keywords.end())
    { 
    m_Browser->add((*it).toChar());
    it++;
    }
  m_Browser->resize(13,15,200,80);
  m_Browser->end();
}

void Editor::UpdateVariable()
{ 
}

void Editor::SetParentWindow(Fl_Window* parentwindow)
{
  m_Parentwindow = parentwindow;
}

void Editor::SetModified(bool flag)
{
  if (m_Parentwindow)
    {
    strcpy(m_Title,m_Parentwindow->label());
    if (MString(m_Title).rfind("*") == -1)
      {
      strcpy(m_Title,(MString(m_Title).rbegin("]") + "*]").toChar());
      m_Parentwindow->label(m_Title);
      }
    }
}


bool Editor::Find(std::list<MString> array,MString key)
{
  std::list<MString>::iterator it = m_keywords.begin();
  while (it != m_keywords.end())
    {
    if ((*it).toLower() == key)
      {
      return true;
      }
    it++;
    }
  return false;
}


int Editor::compare_keywords(const void *a,const void *b) 
{
  return (strcmp(*((const char **)a), *((const char **)b)));
}


void Editor::style_unfinished_cb(int, void*) {
}

void Editor::style_update(int        pos,          // I - Position of update
                          int        nInserted,    // I - Number of inserted chars
                          int        nDeleted,     // I - Number of deleted chars
                          int        nRestyled,    // I - Number of restyled chars
                          const char *deletedText, // I - Text that was deleted
                          void       *cbArg)       // I - Callback data
{  
  int  start,                         // Start of text
       end;                           // End of text
  char last;                          // Last style on line
  char *style = NULL;                 // Style data
  char *text  = NULL;                 // Text data

  if ((nInserted || nDeleted)) SetModified(true);
  Fl_Text_Buffer* stylebuf = ((Editor *)cbArg)->m_Stylebuf;
  Fl_Text_Buffer* textbuf = ((Editor *)cbArg)->m_Buffer;

  // If this is just a selection change, just unselect the style buffer...
  if (nInserted == 0 && nDeleted == 0) 
    {
    stylebuf->unselect();
    return;
    }
    
  // Track changes in the text buffer...
  if (nInserted > 0) 
    {
    // Insert characters into the style buffer...
    style = new char[nInserted + 1];
    memset(style, 'A', nInserted);
    style[nInserted] = '\0';

    stylebuf->replace(pos, pos + nDeleted, style);
    delete [] style;
    style = NULL;
    } 
  else 
    {
    // Just delete characters in the style buffer...
    stylebuf->remove(pos, pos + nDeleted);
    }

  // Select the area that was just updated to avoid unnecessary
  // callbacks...
  stylebuf->select(pos, pos + nInserted - nDeleted);

  // Re-parse the changed region; we do this by parsing from the
  // beginning of the line of the changed region to the end of
  // the line of the changed region...  Then we check the last
  // style character and keep updating if we have a multi-line
  // comment character...
  start = textbuf->line_start(pos);
  end   = textbuf->line_end(pos + nInserted);
  text  = textbuf->text_range(start, end);
  style = stylebuf->text_range(start, end);

  last  = style[end - start - 1];

  if ((pos == start))
    {
    style[0] = stylebuf->text_range(start-1,start)[0];
    style[1] = '\0';
    }

  if ((end - start) != 0)
    {
    style_parse(text, style, end - start);
    stylebuf->replace(start, end, style);
    ((Editor *)cbArg)->redisplay_range(start, end);
    }

  if (last != style[end - start - 1])
    {
    // The last character on the line changed styles, so reparse the
    // remainder of the buffer...
    delete [] text;
    text = NULL;
    delete [] style;
    style = NULL;

    end   = textbuf->length();
    text  = textbuf->text_range(start, end);
    style = stylebuf->text_range(start, end);

    style_parse(text, style, end - start);

    stylebuf->replace(start, end, style);
    ((Editor *)cbArg)->redisplay_range(start, end);
    }

  delete [] text;
  text = NULL;
   
  int m_x;
  int m_y;
  ((Editor *)cbArg)->position_to_xy(pos,&m_x,&m_y);
  ((Editor *)cbArg)->ShowOptionFinder(m_x,m_y);
}

void Editor:: style_parse(const char *text,char *style,int length) 
{
  char  current;
  int   col;
  int   last;
  char  buf[255],*bufptr;
  const char *temp;

  if ((style[0] != 'C') && (style[0] != 'D')) 
    {
    style[0] = 'A';
    }

  for (current = *style, col = 0, last = 0; length > 0; length --, text ++) 
    {
    if (current == 'B') 
      {
      current = 'A';
      }
    if (current == 'A') 
      {
      // Check for directives, comments, strings, and keywords...
      if (strncmp(text, "#", 1) == 0) 
        {
        current = 'B';
        for (; length > 0 && *text != '\n'; length --, text ++)
          {
          *style++ = 'B';
          }
        if (length == 0) break;
        }
      else if (strncmp(text, "$", 1) == 0) 
        {
        current = 'C';
        }
      else if (strncmp(text, "\\'", 2) == 0) 
        {
        // Quoted quote...
        *style++ = current;
        *style++ = current;
        text ++;
        length --;
        col += 2;
        continue;
        } 
      else if (*text == '\'') 
        {
        current = 'D';
        } 
      else 
        {
        temp = text;  
        bufptr = buf;

        unsigned int offset =0;
        while ((text[offset] != ' ') 
               && (offset<strlen(text)) && (text[offset] != '\n') 
               && (text[offset] != '('))
          {
           *bufptr++ = tolower(*temp++);
           offset++;
          }

    //{
        *bufptr = '\0';
        bufptr = buf;

        if (bsearch(&bufptr, code_types,
            sizeof(code_types) / sizeof(code_types[0]),
            sizeof(code_types[0]), compare_keywords)) 
          {
          while (text < temp) 
            {
            *style++ = 'F';
            text ++;
            length --;
            col ++;
            }

          text --;
          length ++;
          last = 1;
          continue;
          }
        else if (Find(m_keywords,bufptr))
          {
          while (text < temp) 
            {
            *style++ = 'G';
            text ++;
            length --;
            col ++;
            }

          text --;
          length ++;
          last = 1;
          continue;
          }
        }
      }
    //} 
    else if (current == 'C' && ((strncmp(text+1, ")", 1) == 0) 
               || (strncmp(text, " ", 1) == 0) || (strncmp(text, "}", 1) == 0)) )
      {
      // Close a C comment...
      *style++ = current;
      current = 'A';
      col += 1;
      continue;
      }
    else if (current == 'D') 
      {
      // Continuing in string...
      if (strncmp(text, "$", 1) == 0) 
        {
        current = 'H';
        }
      if (*text == '\'') 
        {
        // End quote...
        *style++ = current;
        col ++;
        current = 'A';
        continue;
        }
      }
    else if (current == 'H' && ((strncmp(text+1, ")", 1) == 0) 
            || (strncmp(text, " ", 1) == 0) || (strncmp(text, "}", 1) == 0)) )
      {
      // Close a C comment...
      *style++ = current;
      current = 'D';
      col += 1;
      continue;
      }

    // Copy style info...
    *style++ = current;
    col ++;
    last = isalnum(*text) || *text == '.';
    }
}

/** Add the application to browse */
void Editor::AddApplicationsToBrowse()
{
  unsigned long lpos=0;
  for(int l=0;l<buffer()->count_lines(0,buffer()->length());l++)
    {
    // get the current line
    std::string line = buffer()->line_text(buffer()->line_start(lpos));
    lpos =  buffer()->line_end(lpos)+1;
    long pos = 0;
    MString lowercaseLine = line;
    lowercaseLine = lowercaseLine.toLower();
    std::string lowercaseLine2 = lowercaseLine.toChar();
    if(pos = lowercaseLine2.find("setapp(") != -1)
      {
      long pos1 = line.find("(",pos);
      if(pos1 != -1)
        {
        long pos2 = line.find(")",pos1);
        if(pos2 != -1)
          {
          long posspace = line.find(" ",pos1);
          std::string name = line.substr(pos1+1,posspace-pos1-1);  
          std::string app = line.substr(posspace+2,pos2-posspace-2); // there should be a @
              
          long pos3 = app.find(" ");
          if(pos3 != -1)
            {
            app = app.substr(0,pos3); // there should be a @
            }

          // Search the correct app corresponding to the name
          if(m_Manager->GetApplicationWrapperList())
            {
            ScriptActionManager::ApplicationWrapperListType::const_iterator it = 
                                                  m_Manager->GetApplicationWrapperList()->begin();
            while (it != m_Manager->GetApplicationWrapperList()->end())
              { 
              if(!strcmp((*it)->GetName().toChar(),app.c_str()))
                {
                ApplicationNameType newapp;
                newapp.first = name;
                newapp.second = (*it);
                m_ApplicationsList.push_back(newapp);
                break;
                }
              it++;
              }
            }
          }
        }
      }
    }
}

void Editor::Load(const char* filename)
{
  int r = m_Buffer->loadfile(filename);
  if (r)
     fl_alert("Error reading from file \'%s\':\n%s.", filename, strerror(errno));

  int m_foundPos = 0;
  while(m_Buffer->findchars_forward(m_foundPos,"\r",&m_foundPos))
    m_Buffer->remove(m_foundPos,m_foundPos+1);
  
  m_Buffer->call_modify_callbacks();

  this->AddApplicationsToBrowse();

}

void Editor::Save(const char* filename)
{
  if (m_Buffer->savefile(filename))
    {
    fl_alert("Error writing to file \'%s\':\n%s.", filename, strerror(errno));
    }
  m_Buffer->call_modify_callbacks();
}


/** Place the browser at the correct location */
void Editor::ShowOptionFinder(int x,int y)
{
  m_Browser->resize(x+13,y+15,200,80);
  m_ApplicationBrowser->resize(x+13,y+15,200,80);
  m_ApplicationOptionBrowser->resize(x+13,y+15,200,80);
  m_Helper->resize(x+13,y+15,m_Helper->w(),m_Helper->h());
}

/** Draw */
void Editor::draw()
{
  Fl_Text_Display::draw();

  if (m_DrawHelper)
    {
    m_Helper->show();
    }
  else
    {
    m_Helper->hide();
    }

  if (m_DrawBrowser)
    {
    m_Browser->show();
    }
  else
    {
    m_Browser->hide();
    }

  if (m_DrawApplicationBrowser)
    {
    m_ApplicationBrowser->show_leaves( true );
    m_ApplicationBrowser->show_branches( false );
    m_ApplicationBrowser->show();
    }
  else
    {
    m_ApplicationBrowser->hide();
    }

  if (m_DrawApplicationOptionBrowser)
    {
    m_ApplicationOptionBrowser->show();
    }
  else
    {
    m_ApplicationOptionBrowser->hide();
    }
    
  m_ApplicationBrowser->redraw();
  m_ApplicationOptionBrowser->redraw();
  m_Browser->redraw();
  m_Helper->redraw();

  redraw();
}


/** Callback when a tab completion is done for an action */
void Editor::SelectOption(Flu_Tree_Browser* browser, void* widget)
{
  if (browser->get_selected(1) != 0)
    {
    if (browser->callback_reason() == FLU_DOUBLE_CLICK)
      {
      MString m_text(MString(browser->get_selected(1)->label()) + "()");
      m_text = m_text.mid(((Editor*)widget)->m_CurrentWord.length());
      ((Editor*)widget)->m_Buffer->insert(((Editor*)widget)->insert_position(),m_text.toChar());
      ((Editor*)widget)->insert_position(((Editor*)widget)->insert_position()+m_text.length()-1);
      ((Editor*) widget)->m_DrawBrowser = false;
      ((Editor*)widget)->m_CurrentWord = "";
      }

    if (browser->callback_reason() == FLU_TAB_KEY || Fl::event_key() == 65293 )
      {
      MString m_text(MString(browser->get_selected(1)->label()) + "()");
      m_text = m_text.mid(((Editor*)widget)->m_CurrentWord.length());
      ((Editor*)widget)->m_Buffer->insert(((Editor*)widget)->insert_position(),m_text.toChar());
      ((Editor*)widget)->insert_position(((Editor*)widget)->insert_position()+m_text.length()-1);
      ((Editor*) widget)->m_DrawBrowser = false;
      ((Editor*)widget)->m_CurrentWord = "";

      ScriptActionManager m_Manager;
      ScriptAction* m_help = 
                  m_Manager.CreateAction(MString(browser->get_selected(1)->label()).toLower());
      if (m_help)
        {
        ((Editor*)widget)->m_Helper->value(m_help->Help().toChar());
        if (MString(((Editor*)widget)->m_Helper->value()) != "")
          {
          ((Editor*)widget)->m_Helper->resize(((Editor*)widget)->m_Helper->x(),
                                             ((Editor*)widget)->m_Helper->y(),
                                             (int)(m_help->Help().length()*4.9),15);
          ((Editor*)widget)->m_DrawHelper = true;
          }
        else
          {
          ((Editor*)widget)->m_DrawHelper = false;
          }
        delete m_help;
        }
      else
        {
        ((Editor*)widget)->m_DrawHelper = false;
        }
      }
    }
} 

/** Callback when a tab completion is done for an action */
void Editor::SelectApplication(Flu_Tree_Browser* browser, void* widget)
{
  if (browser->get_selected(1) != 0)
    {
    if (browser->callback_reason() == FLU_DOUBLE_CLICK)
      {
      MString m_text(MString(browser->get_selected(1)->label()));
      ((Editor*)widget)->m_Buffer->insert(((Editor*)widget)->insert_position(),m_text.toChar());
      ((Editor*)widget)->insert_position(((Editor*)widget)->insert_position()+m_text.length());
      ((Editor*) widget)->m_DrawApplicationBrowser = false;
      ((Editor*) widget)->m_DrawBrowser = false;
      ((Editor*)widget)->m_DrawHelper = false;
      ((Editor*)widget)->m_CurrentWord = "";
      }

    if (browser->callback_reason() == FLU_TAB_KEY || Fl::event_key() == 65293 )
      {
      MString m_text(MString(browser->get_selected(1)->label()));
      ((Editor*)widget)->m_Buffer->insert(((Editor*)widget)->insert_position(),m_text.toChar());
      ((Editor*)widget)->insert_position(((Editor*)widget)->insert_position()+m_text.length());
      ((Editor*) widget)->m_DrawApplicationBrowser = false;
      ((Editor*) widget)->m_DrawBrowser = false;
      ((Editor*)widget)->m_DrawHelper = false;
      ((Editor*)widget)->m_CurrentWord = "";
      }
    }
} 

/** Callback when a tab completion is done for an action */
void Editor::SelectApplicationOption(Flu_Tree_Browser* browser, void* widget)
{
  if (browser->get_selected(1) != 0)
    {
    if (browser->callback_reason() == FLU_DOUBLE_CLICK)
      {
      MString m_text(MString(browser->get_selected(1)->label()));
      ((Editor*)widget)->m_Buffer->insert(((Editor*)widget)->insert_position(),m_text.toChar());
      ((Editor*)widget)->insert_position(((Editor*)widget)->insert_position()+m_text.length());
      ((Editor*)widget)->m_DrawApplicationOptionBrowser = false;
      ((Editor*)widget)->m_CurrentWord = "";
      }

    if (browser->callback_reason() == FLU_TAB_KEY || Fl::event_key() == 65293 )
      {
      MString m_text(MString(browser->get_selected(1)->label()));
      ((Editor*)widget)->m_Buffer->insert(((Editor*)widget)->insert_position(),m_text.toChar());
      ((Editor*)widget)->insert_position(((Editor*)widget)->insert_position()+m_text.length());
      ((Editor*)widget)->m_DrawApplicationOptionBrowser = false;
      ((Editor*)widget)->m_CurrentWord = "";
      }
    }
}

/** Show the option give the variable name of an app */
bool Editor::ShowApplicationOptions(const char* appVarName)
{
  if(!m_Manager->GetApplicationWrapperList())
    {
    return false;
    }

  m_ApplicationOptionBrowser->clear();
  
  std::vector<ApplicationNameType>::const_iterator it = m_ApplicationsList.begin();
  ApplicationWrapper* app = NULL;
  while (it != m_ApplicationsList.end())
    {
    if(!strcmp((*it).first.c_str(),appVarName))
      {
      app = (*it).second;
      break;
      }
    it++;
    }

  if(!app)
    {
    return false;
    }

  // Get the option from the application wrapper
  std::vector<ApplicationWrapperParam> & params = app->GetParams();
  // std::cout << "App Name = " << app->GetName().toChar() << std::endl;
  std::vector<ApplicationWrapperParam>::const_iterator itParams = params.begin();
  if(params.size() == 0)
    {
    return false;
    }
  //std::cout << "params.size() = " << params.size() << std::endl;
  //std::cout << "Has params" << std::endl;
  while(itParams!= params.end())
    {
    std::string text = "";
    int parent = (*itParams).GetParent();

    if(parent > 0)
      {
      unsigned int pos = 0;
      for(unsigned int j=0;j<params.size();j++)
        {
        if(params[j].GetParent() == 0)
          {
          pos++;
          }
        if(pos == parent)
          {
          text = params[j].GetName().toChar();
          text += ".";
          break;
          }
        }
      }
    text += (*itParams).GetName().toChar();
    m_ApplicationOptionBrowser->add(text.c_str());
    itParams++;
    }
  return true;
}

/** Handle function for the text editor */
int Editor::handle( int event )
{
  if (event == FL_PUSH)
    {
    m_CurrentWord = "";
    m_DrawBrowser = false;
    m_DrawApplicationBrowser = false;
    m_DrawApplicationOptionBrowser = false;
    draw();
    return Fl_Text_Editor::handle( event );
    }

  if ( event == FL_KEYDOWN )
    {
    // if @ is pressed then we show the list of wrapped applications
    if(Fl::event_state() == 1114112 && Fl::event_key() == 50)
      {
      if (!m_DrawApplicationBrowser)
        {
        this->UpdateApplicationsList();
        m_DrawApplicationBrowser = true;
        }
      else
        {
        m_ApplicationBrowser->handle(event);
        }
      take_focus();   
      }

    // if CTRL+S is pressed we save the script
    if(Fl::event_state() == 1310720 && Fl::event_key() == 115)
      {
      static_cast<ScriptEditorGUIControls*>(m_ScriptEditorGUI)->OnSaveScript();
      return 1;
      }
    
    if ( Fl::event_key() == FL_Escape)
      {
      m_DrawBrowser = false;
      m_DrawHelper = false;
      m_DrawApplicationBrowser = false;
      m_DrawApplicationOptionBrowser = false;
      Fl_Text_Editor::handle( event );
      draw();
      return 1;
      }

    if( Fl::event_key() == FL_Tab)
      {
      if (!m_DrawBrowser && !m_DrawApplicationBrowser && !m_DrawApplicationOptionBrowser)
        {
        m_DrawBrowser = true;
        }
      else if(m_DrawBrowser)
        {
        m_Browser->handle(event);
        }
      else if(m_DrawApplicationBrowser)
        {
        m_ApplicationBrowser->handle(event);
        } 
      else if(m_DrawApplicationOptionBrowser)
        {
        m_ApplicationOptionBrowser->handle(event);
        }
      take_focus();
      return 1;
      }
    else if(Fl::event_key() == FL_Enter)
      {
      if(m_DrawBrowser)
        {
        m_Browser->handle(event);
        take_focus();
        return 1;
        }
      else if(m_DrawApplicationBrowser)
        {
        m_ApplicationBrowser->handle(event);
        take_focus();
        return 1;
        } 
      else if(m_DrawApplicationOptionBrowser)
        {
        m_ApplicationOptionBrowser->handle(event);
        take_focus();
        return 1;
        }
      else // we check if the current line contains the word SetApp()
        {
        // get the current line   
        std::string line = buffer()->line_text(insert_position());
        long pos = 0;
        MString lowercaseLine = line;
        lowercaseLine = lowercaseLine.toLower();
        std::string lowercaseLine2 = lowercaseLine.toChar();

        if(pos = lowercaseLine2.find("setapp(") != -1)
          {
          long pos1 = line.find("(",pos);
          if(pos1 != -1)
            {
            long pos2 = line.find(")",pos1);
            if(pos2 != -1)
              {
              long posspace = line.find(" ",pos1);
              std::string name = line.substr(pos1+1,posspace-pos1-1);  
              std::string app = line.substr(posspace+2,pos2-posspace-2); // there should be a @
              long pos3 = app.find(" ");
              if(pos3 != -1)
                {
                app = app.substr(0,pos3); // there should be a @
                }

              // Search the correct app corresponding to the name
              ScriptActionManager::ApplicationWrapperListType::const_iterator it = 
                                                m_Manager->GetApplicationWrapperList()->begin();
              while (it != m_Manager->GetApplicationWrapperList()->end())
                { 
                if(!strcmp((*it)->GetName().toChar(),app.c_str()))
                  {
                  ApplicationNameType newapp;
                  newapp.first = name;
                  newapp.second = (*it);
                  std::cout << "Added " << name.c_str() << " = " 
                            << (*it)->GetName().toChar() << std::endl;
                  // Check if the variable is already assigned to a specific application
                  int apppos = -1;
                  bool appexists = false;
                  std::vector<ApplicationNameType>::const_iterator 
                                               it2 = m_ApplicationsList.begin();
                  while (it2 != m_ApplicationsList.end())
                    {  
                    apppos++;
                    if(!strcmp((*it2).first.c_str(),name.c_str()))
                      {
                      appexists = true;
                      break;
                      }
                    it2++;
                    }

                  if(appexists)
                    {
                    m_ApplicationsList[apppos] = newapp;
                    }
                  else
                    {
                    m_ApplicationsList.push_back(newapp);
                    }
                  break;
                  }
                it++;
                }
              }
            }
          }
        }
      }
    else if( Fl::event_key() == FL_Down)
      {
      if (m_DrawBrowser)
        {
        m_Browser->handle(event);
        take_focus();
        return 1;  
        }
      else if(m_DrawApplicationBrowser)
        {
        m_ApplicationBrowser->handle(event);
        take_focus();
        return 1;  
        }
      else if(m_DrawApplicationOptionBrowser)
        {
        m_ApplicationOptionBrowser->handle(event);
        take_focus();
        return 1;  
        }
      }
    else if( Fl::event_key() == FL_Up)
      {
      if (m_DrawBrowser)
        {
        m_Browser->handle(event);
        take_focus();
        return 1;  
        }
      else if(m_DrawApplicationBrowser)
        {
        m_ApplicationBrowser->handle(event);
        take_focus();
        return 1;  
        }
      else if(m_DrawApplicationOptionBrowser)
        {
        m_ApplicationOptionBrowser->handle(event);
        take_focus();
        return 1;  
        }
      }
    else
      { 
      if ((Fl::event_key() == ' ') || (Fl::event_key() == FL_Enter))
        {
        m_CurrentWord = "";
        }
      else if((Fl::event_key() == '.') || (Fl::event_key() == 65454)) 
      // if it's a dot and the previous word is in the list of known apps/name
        {
        if (!m_DrawApplicationOptionBrowser)
          {
          // look for the work before the .
          std::string reword;
          long int i = insert_position();
          i--;
          while(i>-1
            && buffer()->character(i) != '('
            && buffer()->character(i) != ')'
            && buffer()->character(i) != '"'
            && buffer()->character(i) != '.'
            && buffer()->character(i) != 65454
            && buffer()->character(i) != 32
            && buffer()->character(i) != 10
            && buffer()->character(i) != '$'
            )
            {
            reword += buffer()->character(i);
            i--;
            }
         
          // reverse the string
          std::string word = "";
          for(i=0;i<(int)reword.size();i++)
            {
            word += reword[reword.size()-i-1];
            }

          if(this->ShowApplicationOptions(word.c_str()))
            {
            m_DrawApplicationOptionBrowser = true;
            }
          else
            {
            m_DrawApplicationOptionBrowser = false;
            }
          }
        else
          {
          m_ApplicationOptionBrowser->handle(event);
          }
        }
      else
        {
        if ((Fl::event_key() == FL_BackSpace) || (Fl::event_key() == FL_BackSpace))
          {
          m_CurrentWord = m_CurrentWord.mid(0,m_CurrentWord.length()-1);
          }
        else
          {
          if ((Fl::event_key() != FL_Shift_L) &&  (Fl::event_key() != FL_Shift_R))
            {
            m_CurrentWord += Fl::event_key();
            }
          }
        }
      
      ScriptActionManager m_Manager;
      ScriptAction* m_help = m_Manager.CreateAction(m_CurrentWord.toLower());
      if (m_help)
        {
        m_Helper->value(m_help->Help().toChar());
        if (MString(m_Helper->value()) != "")
          {
          m_Helper->resize(m_Helper->x(),m_Helper->y(),(int)(m_help->Help().length()*4.9),15);
          m_DrawBrowser = false;
          m_DrawApplicationBrowser = false;
          m_DrawApplicationOptionBrowser = false;
          m_DrawHelper = true;
          }
        else
          {
          m_DrawHelper = false;
          }
        delete m_help;
        }
      else
        {
        m_DrawHelper = false;
        }

      int m_Offset = -1;
      int currentrating = 0;

      std::list<MString>::iterator it = m_keywords.begin();
      int i =0;
      while (it != m_keywords.end())
        { 
        int m_rating = 0;
        int m_correctword = true;
       
        for (int j=0;j<m_CurrentWord.length();j++)
          {
          if ((*it).length() > j)
            {
            if ((m_CurrentWord.toLower()[j] != (*it).toLower()[j]))
              {
              m_correctword = false;
              }
            else
              {
              m_rating++;
              }
            }
          else
            {
            m_correctword = false;
            }
          }
        
        if ((m_rating != 0) && (m_rating > currentrating) && (m_correctword))
          {
          m_Offset = i;
          currentrating = m_rating;
          }

        i++;
        it++;
        }

      if (m_Offset != -1)
        {
        if (!m_DrawHelper)
          {
          m_DrawBrowser = true;
          }
        
        Flu_Tree_Browser::Node* m_node = m_Browser->first();
        for (int k=0;k<=m_Offset;k++)
          {
          m_node = m_node->next();
          }

        if(m_node)
          {
          m_node->select(true);
          m_node->select_only();
          m_Browser->set_hilighted(m_node);
          }

        Fl_Text_Editor::handle( event );
        draw();
        
        return 1;
        }
      else
        {
        m_DrawBrowser = false;
        m_DrawHelper = false;
        Fl_Text_Editor::handle( event );
        draw();
        return 1;
        }
      }
    }
  return Fl_Text_Editor::handle( event );
}

} // end namespace bm
