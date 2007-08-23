/*=========================================================================

Program:   Insight Segmentation & Registration Toolkit
Module:    $RCSfile: bmSystemInfoTest.cxx,v $
Language:  C++
Date:      $Date: 2007/05/15 14:43:50 $
Version:   $Revision: 1.5 $

Copyright (c) Insight Software Consortium. All rights reserved.
See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include "SystemInfo.h"
#include <iostream>

int SystemInfoTest(int argc, char* argv[])
{
  SystemInfo info;
  info.RunCPUCheck();
  info.RunOSCheck();
  info.RunMemoryCheck();

  std::cout << "Vendor = " << info.GetVendorID() << std::endl;
  std::cout << "Model = " <<info.GetModelID() << std::endl;
  std::cout << "Extended ProcessorName = " << info.GetExtendedProcessorName() << std::endl;
  std::cout << "Processor Speed = " << info.GetProcessorClockFrequency() << "Mhz" << std::endl;
  std::cout << "Number of physical CPUs = " << info.GetNumberOfPhysicalCPU() << std::endl;
  std::cout << "Number of logical CPUs = " << info.GetNumberOfLogicalCPU() << std::endl;
  std::cout << "Cache Size = " << info.GetProcessorCacheSize() << std::endl;

  std::cout << "Total Virtual Memory = "  << info.GetTotalVirtualMemory() << "Mb" << std::endl;
  std::cout << "Available Virtual Memory = "  << info.GetAvailableVirtualMemory() << "Mb" << std::endl;
  std::cout << "Total Physical Memory = "  << info.GetTotalPhysicalMemory() << "Mb" << std::endl;
  std::cout << "AvailablePhysicalMemory = "  << info.GetAvailablePhysicalMemory() << "Mb" << std::endl;
    
  std::cout << "Hostname = " << info.GetHostname() << std::endl;
  std::cout << "Operating System Platform = " << info.GetOSPlatform() << std::endl;
  std::cout << "Operating System Name = " << info.GetOSName() << std::endl;
  std::cout << "Operating System Release = " << info.GetOSRelease() << std::endl;
  std::cout << "Operating System Version = " << info.GetOSVersion() << std::endl;

  if(info.Is64Bits())
    {
    std::cout << "64-bits machine" << std::endl;
    }
  else
    {
    std::cout << "32-bits machine" << std::endl;
    }

  return EXIT_SUCCESS;
}
