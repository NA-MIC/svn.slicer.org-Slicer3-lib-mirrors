#include <iostream>
// This file is located in BatchMake_SOURCE_DIR/Utilities
#include "SystemInfo.h"
int main(int argc, char * argv[]) 
{ 
  SystemInfo info;
  info.RunMemoryCheck();
  unsigned long memoryinMB = info.GetTotalPhysicalMemory();
  std::cout << "PHYSICAL_MEMORY_MB=" << memoryinMB << std::endl;
  return 1; 
}
