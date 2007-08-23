/*=========================================================================

Program:   Insight Segmentation & Registration Toolkit
Module:    $RCSfile: bmDashboardSendTest.cxx,v $
Language:  C++
Date:      $Date: 2007/04/02 22:03:49 $
Version:   $Revision: 1.1 $

Copyright (c) Insight Software Consortium. All rights reserved.
See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include "itkImage.h"
#include "bmDashboardSend.h"

int DashboardSendTest(int argc, char* argv[])
{
  bm::DashboardSend sender;

  sender.SetUrl("http://www.insight-journal.com/batchmake");
  sender.SetUserName("My username");
  sender.SetProjectName("My Project");
  sender.SetExperimentName("Remote experiment");
  sender.SetMethodName("Method1");

  // Create an experiment
  sender.CreateExperiment("Remote testing");
 
  // Create a method
  sender.CreateMethod("Remote Method");

  sender.CreateMethodParameter("input1",bm::DashboardSend::INPUT,"float");
  sender.CreateMethodParameter("output1",bm::DashboardSend::OUTPUT,"float");
  sender.CreateMethodParameter("output2",bm::DashboardSend::OUTPUT,"float");

  sender.ClearValueList();
  sender.ClearDataList();

  // Send to the dashboard
  sender.AddValue("input1","1.0");
  sender.AddValue("output1","2.0");
  sender.AddValue("output2","3.0");

  sender.Send();

  return EXIT_SUCCESS;
}
