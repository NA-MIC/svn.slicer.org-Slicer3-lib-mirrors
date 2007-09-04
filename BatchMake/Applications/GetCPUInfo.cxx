#include <iostream>
// This file is located in BatchMake_SOURCE_DIR/Utilities
#include "SystemInfo.h"
int main(int argc, char * argv[]) 
{ 
  SystemInfo info;
  info.RunCPUCheck();
  std::cout << "CPU_SPEED_MHZ=" << (int) info.GetProcessorClockFrequency() 
            << std::endl;
  std::cout << "CPU_PHYSICAL_CPUS=" << (int) info.GetNumberOfPhysicalCPU() 
            << std::endl;
  std::cout << "CPU_LOGICAL_CPUS=" << (int) info.GetNumberOfLogicalCPU() 
            << std::endl;
  return 1; 
}
