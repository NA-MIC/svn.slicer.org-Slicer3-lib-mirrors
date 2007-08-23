/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptActionManagerDashboard.cxx,v $
  Language:  C++
  Date:      $Date: 2007/07/26 21:49:15 $
  Version:   $Revision: 1.2 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptActionManager.h"

namespace bm{

/** Add an experiement to the dashboard */
bool ScriptActionManager::AddDashboardExperiment(const char* var, const char* projectName, const char* experimentName)
{
  // Check that the experiment does not exist
  std::vector<DashboardExperiment>::const_iterator it = m_Dashboard.experiments.begin();
  while(it != m_Dashboard.experiments.end())
    {
    if((!strcmp((*it).project.c_str(),projectName)) 
      && (!strcmp((*it).name.c_str(),experimentName)))
      {
      return false;
      }
    it++;
    }

  DashboardExperiment exp;
  exp.variable = var;
  exp.project = projectName;
  exp.name = experimentName;
  m_Dashboard.experiments.push_back(exp);
  return true;
}

/** Add a method to an experiment */
bool ScriptActionManager::AddDashboardMethod(const char* var, const char* expvar, const char* methodName)
{
  // Check that the experiment exist
  std::vector<DashboardExperiment>::iterator it = m_Dashboard.experiments.begin();
  while(it != m_Dashboard.experiments.end())
    {
    if((!strcmp((*it).variable.c_str(),expvar)))
      {
      // Check that the method doesn't exit
      std::vector<DashboardMethod>::const_iterator itMeth = (*it).methods.begin();
      while(itMeth != (*it).methods.end())
        {
        if((!strcmp((*itMeth).expVariable.c_str(),expvar))
          && (!strcmp((*itMeth).name.c_str(),methodName))
          )
          {
          return false;
          }
        itMeth++;
        }
      DashboardMethod meth;
      meth.variable = var;
      meth.expVariable = expvar;
      meth.name = methodName;  
      (*it).methods.push_back(meth);
      return true;
      }
    it++;
    }
  return false;
}

/** Add a parameter to a specific method */
bool ScriptActionManager::AddDashboardMethodParameter(const char* var, 
                                                      const char* methVar, 
                                                      const char* name,
                                                      bool output,
                                                      bool ideal,
                                                      const char* type)
{
  // Check that the experiment exist
  std::vector<DashboardExperiment>::iterator it = m_Dashboard.experiments.begin();
  while(it != m_Dashboard.experiments.end())
    {
    std::vector<DashboardMethod>::iterator itMeth = (*it).methods.begin();
    while(itMeth != (*it).methods.end())
      {
      if((!strcmp((*itMeth).variable.c_str(),methVar)))
        {
        std::vector<DashboardMethodParameter>::const_iterator itParam 
                                                    = (*itMeth).parameters.begin();
        bool found = false;
        while(itParam != (*itMeth).parameters.end())
          {
          if(!strcmp((*itParam).variable.c_str(),var) && (*itParam).ideal == ideal)
            {
            found = true;
            break;
            }
          itParam++;
          }
        if(!found)
          {
          DashboardMethodParameter param;
          param.variable = var;
          param.method = methVar;
          param.name = name;
          param.output = output;
          param.ideal = ideal;
          if(type)
            {
            param.type = type;
            }
          (*itMeth).parameters.push_back(param);
          }
        }
      itMeth++;
      }
    it++;
    }
  return false;
}

/** Add a BatchBoard */
ScriptActionManager::BatchBoard* 
ScriptActionManager::AddBatchBoard(const char* var,
                                   const char* experimentVar,
                                   const char* title,
                                   const char* isPublic)
{
  bool found = false;

  // Get the project name
  DashboardExperiment* exp = NULL;
  std::vector<DashboardExperiment>::iterator it = m_Dashboard.experiments.begin();
  while(it != m_Dashboard.experiments.end())
    {
    if(!strcmp((*it).variable.c_str(),experimentVar))
      {
      exp = &(*it);
      break;
      }
    it++;
    }

  if(!exp)
    {
    std::cout << "Cannot find experiment!" << std::endl;
    return NULL;
    }
  else
    {
    BatchBoard dash;
    dash.variable = var;
    dash.title = title;
    if(!strcmp(isPublic,"1"))
      {
      dash.isPublic = true;
      }
    else
      {
      dash.isPublic = false;
      }
    (*exp).batchboards.push_back(dash);
    return &((*exp).batchboards[(*exp).batchboards.size()-1]);
    }

  return NULL;
}


} // end namespace bm
