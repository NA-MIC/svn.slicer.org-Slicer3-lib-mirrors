/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: CondorWatcher.h,v $
  Language:  C++
  Date:      $Date: 2007/05/25 18:50:42 $
  Version:   $Revision: 1.2 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include "CondorWatcherGUI.h"
#include <iostream>
#include <stdio.h>
#include <cstring>
#include <vector>

class CondorWatcher : public CondorWatcherGUI
{
public:

  typedef std::pair<std::string,std::string> PairType;

  CondorWatcher();
  virtual ~CondorWatcher();

  /** Watch loop */
  void Watch();

  /** Update the current display */
  void Update();

  /** Add a client */
  void AddClient();

protected:

  /** Update the list of clients in the condor pool */
  void UpdateClients();

  /** Launch an application and get the result in a buffer */
  std::string Run(const char*);

  /** Parse the current status */
  void ParseStatus(std::string & buffer);
  void ParseQueue(std::string & buffer);

  /** Remove unused XML tag from a buffer */
  std::string RemoveExtraChar(std::string & buffer);

  struct machine
    {
    std::vector<PairType> StatusList;
    };

  std::vector<machine> m_MachineVector;

  struct job
    {
    std::vector<PairType> StatusList;
    };

  std::vector<job> m_JobVector;

};
