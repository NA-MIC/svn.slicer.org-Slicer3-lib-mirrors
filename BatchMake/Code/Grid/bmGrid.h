/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmGrid.h,v $
  Language:  C++
  Date:      $Date: 2007/09/06 14:09:48 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifndef __Grid_h_
#define __Grid_h_

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <iostream>
#include <vector>
#include <ApplicationWrapper.h>

namespace bm {

class Grid
{
public:
  Grid();
  ~Grid();

  void SetFileName(const char* filename) {m_FileName = filename;}
    
  /** Add an application to the list of applications to generate */
  void AddApplication(ApplicationWrapper* app,
                      const char* datadir=NULL,const char* outputdir=NULL);
  
  unsigned long GetNumberOfApplications()
    {
    return m_ApplicationsList.size();
    }

  void WriteCondor();
  void WriteGAD();
  void WriteShell();
  const char* GetCurrentScopeFile();

  void SetDataHost(const char* hostname) {m_DataHost = hostname;}
  void SetOutputHost(const char* hostname) {m_OutputHost = hostname;}
  void SetDataDirectory(const char* directory) {m_DataDirectory = directory;}
  void SetOutputDirectory(const char* directory) {m_OutputDirectory = directory;}
  void SetGridTempDirectory(const char* directory) {m_GridTempDirectory = directory;}
  void SetGridExecutableDirectory(const char* directory) {m_GridExecutableDirectory = directory;}
 
  const char* GetDataDirectory() {return m_DataDirectory.c_str();}
  const char* GetOutputDirectory() {return m_OutputDirectory.c_str();}
  const char* GetGridTempDirectory() {return m_GridTempDirectory.c_str();}
  const char* GetGridExecutableDirectory() {return m_GridExecutableDirectory.c_str();}
 
  void SetDistributed(bool val);

  void SetGridBarrier();
  void RemoveGridBarrier();

  void SetSingleNode(bool single) 
    {
    if(single != m_SingleNode)
      {
      m_SingleNodeTransition = true;
      }
    m_SingleNode = single;
    }

  /** Set the maximum number of nodes to the grid */
  void SetMaxNodes(int nodes) {m_MaxNodes = nodes;}
  int GetMaxNodes() {return m_MaxNodes;}
  
  /** Set if grouping should be used for GAD generation */
  void SetGrouping(bool group) {m_Grouping = group;}
  bool GetGrouping() {return m_Grouping;}


protected:

  std::string AddQuotes(std::string value);

  /** Return a filename for a full path */
  std::string GetFilename(const char* fullpath); 

  std::string m_FileName;
  std::vector<ApplicationWrapper> m_ApplicationsList; // we create a copy of the app
  std::string m_CurrentScopeFile;
  std::string m_DataHost;
  std::string m_OutputHost;
  std::string m_DataDirectory;
  std::string m_OutputDirectory;
  std::string m_GridTempDirectory;
  std::string m_GridExecutableDirectory;

  bool             m_Distributed;
  bool             m_DistributedTransition;
  std::vector<int> m_DistributedSyncBarrier;
  bool             m_SingleNode;
  bool             m_SingleNodeTransition;
  int              m_MaxNodes;
  bool             m_Grouping;
  bool             m_NextAppIsAfterEndBarrier;
  unsigned long    m_CurrentScope;

  struct DAGnode
    {
    int id;
    std::vector<int> parents;
    bool isAfterbarrier;
    bool isBarrier;
    unsigned long scope;
    bool isVirtual;
    };

  void AddDAGParent(FILE* fic,std::vector<DAGnode>* dag,int parentid);
  void ConsolidateDAG(std::vector<DAGnode>* dag);
  void AddLeavesAsParent(std::vector<DAGnode>* dag,int node,DAGnode* nodeToAdd);

  typedef std::pair<int,int> ParentChildType;
  std::vector<ParentChildType> m_ParentChildList;

};

} // end namespace bm

#endif
