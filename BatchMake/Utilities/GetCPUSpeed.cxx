#include <iostream>
// This file is located in BatchMake_SOURCE_DIR/Utilities
#include "Whetstone.h"
int main(int argc, char * argv[]) 
{ 
  Whetstone info;
  info.SetDesiredDurations(5, 20);
  info.ComputeBenchmark();

  std::cout << "CPU_MIPS=" << (int) info.GetMIPS() 
            << std::endl;
  std::cout << "CPU_MFLOPS=" << (int) info.GetMFLOPS() 
            << std::endl;

  return 1; 
}
