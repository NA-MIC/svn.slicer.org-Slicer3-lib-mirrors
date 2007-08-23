/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptActionManager.h,v $
  Language:  C++
  Date:      $Date: 2007/07/26 21:48:50 $
  Version:   $Revision: 1.7 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifndef __ScriptActionManager_h_
#define __ScriptActionManager_h_

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include "bmScriptAction.h"
#include "bmScriptError.h"
#include "bmProgressManager.h"
#include "ApplicationWrapper.h"
#include <vector>
#include <iostream>
#include "MString.h"
#include "TCPSocket.h"

#ifdef BM_GRID
  #include "bmGrid.h"
#endif // BM_GRID

namespace bm {

class ScriptAction;

class ScriptActionManager
{
public:

  typedef std::vector<ApplicationWrapper*> ApplicationWrapperListType;
  typedef std::pair<std::string,ApplicationWrapper*> ApplicationNameType;
  typedef std::vector<ApplicationNameType> ApplicationsListType;

  ScriptActionManager();
  ~ScriptActionManager();
  
  struct variablestruct
    {
    MString name;
    MString value;
    };
    
  struct variablestructsocket
    {
    MString   name;
    TCPSocket socket;
    };

  struct DashboardMethodParameter
    {
    std::string variable;
    std::string method;
    std::string name;
    bool output;
    bool ideal;
    std::string type;
    };

  struct DashboardMethod
    {
    std::string variable;
    std::string expVariable;
    std::string name;
    std::vector<DashboardMethodParameter> parameters;
    };

  struct BatchBoard
    {
    std::string variable;
    std::string title;
    bool isPublic;
    std::string SQLid;
    };

  struct DashboardExperiment
    {
    std::string variable;
    std::string project;
    std::string name;
    std::vector<DashboardMethod> methods;
    std::vector<BatchBoard> batchboards;
    };

  struct Dashboard
    {
    std::string url;
    std::string user;
    std::string key;
    std::string password;
    std::vector<DashboardExperiment> experiments;
    };
   
  ScriptAction* CreateAction(MString option);

  void        SetApplicationPath(MString applicationpath);
  void        SetScriptPath(MString scriptpath);
  void        SetScriptFullPath(const char* scriptpath);
  const char* GetScriptFullPath() 
         { return m_ScriptFullPath.c_str(); }

  void AddAction(ScriptAction* action);
  void AddAction(MString option, std::vector<MString> param);

  void Execute(); 

  MString Convert(MString param);
  MString ConvertExtra(MString param);

  void                 SetVariable(MString name, MString value);
  void                 SetSocketVariable(MString name);
  std::vector<MString> GetVariable(MString name);
  
  void DisplayVariableList();
  void SetLineNumber(int linenumber);
  int  GetLineNumber() 
         { return m_LineNumber; }

  void         SetError(ScriptError* error);
  ScriptError* GetError();

  void             SetProgressManager(ProgressManager* progressmanager);
  ProgressManager* GetProgressManager();

  bool TestParam();
  void Reset();
  bool TestConvert(MString param, int linenumber);

  void SetTestVariable(MString name);
  bool IsTestVariable(MString name);

  std::vector<MString> GetParamsFromVariable(MString name);
  MString              GetVariableFromParams(const std::vector<MString> &
                                             params);

  void SetParser(void* parser) 
         { m_Parser = parser; }
  void* GetParser() 
         { return m_Parser; }

  /** Set/Get the list of wrapped applications */
  void   SetApplicationWrapperList(ApplicationWrapperListType* applicationlist);
  ApplicationWrapperListType* GetApplicationWrapperList() 
         { return m_ApplicationWrapperList; }

  /** Set/Get the list of application from the editor associating an app 
   *  and a keyword */
  void SetApplicationsList(ApplicationsListType* list) 
         { m_ApplicationsList = list; }
  ApplicationsListType* GetApplicationsList() 
         { return m_ApplicationsList; }

  std::vector<MString> GetKeywordList();

#ifdef BM_GRID
  void SetGridModule(Grid* grid) 
         { m_GridModule = grid; }
#endif // BM_GRID

#ifdef BM_DASHBOARD 
  /** Set/Get variables relative to the dashboard */
  void SetDashboardURL(const char* url) 
         { m_Dashboard.url = url; }
  const char* GetDashboardURL() 
         { return m_Dashboard.url.c_str(); }
  
  void SetDashboardUser(const char* user) 
         { m_Dashboard.user = user; }
  void SetDashboardKey(const char* key) 
         { m_Dashboard.key = key; }

  const char* GetDashboardUser() 
         { return m_Dashboard.user.c_str(); }
  const char* GetDashboardKey() 
         { return m_Dashboard.key.c_str(); }

  bool AddDashboardExperiment(const char* var,
                              const char* projectName,
                              const char* experimentName);
  bool AddDashboardMethod(const char* var,
                          const char* expvar,
                          const char* methodName);

  bool AddDashboardMethodParameter(const char* var,
                                   const char* methVar, 
                                   const char* name,
                                   bool output=false,
                                   bool ideal=false,
                                   const char* type=NULL);

  BatchBoard*  AddBatchBoard(const char* var,
                             const char* experimentVar,
                             const char* title,
                             const char* isPublic);

  /** Get the dashboard as a const method, onlt to retrieve data from it */
  const Dashboard * GetDashboard() const 
          { return &m_Dashboard; }
#endif

  TCPSocket* GetVariableSocket(MString name);
  bool RemoveSocket(MString name);

protected:
  std::vector<ScriptAction*>         m_ActionList;
  ScriptAction                     * m_ParentAction;
  std::vector<variablestruct*>       m_VariableList;
  std::vector<MString>               m_VariableTestList;
  std::vector<variablestructsocket*> m_VariableSocketList;
  int                                m_LineNumber;
  ScriptError                      * m_Error;
  ProgressManager                  * m_ProgressManager;
  MString                            m_ApplicationPath;
  MString                            m_ScriptPath;
  std::string                        m_ScriptFullPath;
  ApplicationWrapperListType       * m_ApplicationWrapperList;
  ApplicationsListType             * m_ApplicationsList;
  void                             * m_Parser;

  // Tells the output of the Run command to generate grid scripts
#ifdef BM_GRID 
  Grid* m_GridModule;
#endif

#ifdef BM_DASHBOARD 
  // The Script Action manager keeps global variables regarding the dashboard
  Dashboard m_Dashboard;
#endif

private:
  
  std::vector<ScriptAction*> m_InternalActionList;

};

} // end namespace bm

#endif
