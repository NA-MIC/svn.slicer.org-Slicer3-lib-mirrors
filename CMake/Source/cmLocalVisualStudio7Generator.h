/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmLocalVisualStudio7Generator.h,v $
  Language:  C++
  Date:      $Date: 2006/05/14 19:22:42 $
  Version:   $Revision: 1.22.2.1 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmLocalVisualStudio7Generator_h
#define cmLocalVisualStudio7Generator_h

#include "cmLocalGenerator.h"

class cmMakeDepend;
class cmTarget;
class cmSourceFile;
class cmCustomCommand;
class cmSourceGroup;
struct cmVS7FlagTable;

/** \class cmLocalVisualStudio7Generator
 * \brief Write a LocalUnix makefiles.
 *
 * cmLocalVisualStudio7Generator produces a LocalUnix makefile from its
 * member Makefile.
 */
class cmLocalVisualStudio7Generator : public cmLocalGenerator
{
public:
  ///! Set cache only and recurse to false by default.
  cmLocalVisualStudio7Generator();

  virtual ~cmLocalVisualStudio7Generator();
  
  /**
   * Generate the makefile for this directory. 
   */
  virtual void Generate();

  enum BuildType {STATIC_LIBRARY, DLL, EXECUTABLE, WIN32_EXECUTABLE, UTILITY};

  /**
   * Specify the type of the build: static, dll, or executable.
   */
  void SetBuildType(BuildType,const char *name);

  /**
   * Return array of created DSP names in a STL vector.
   * Each executable must have its own dsp.
   */
  std::vector<std::string> GetCreatedProjectNames() 
    {
    return this->CreatedProjectNames;
    }
  void SetVersion71() {this->Version = 71;}
  void SetVersion8() {this->Version = 8;}
  void SetPlatformName(const char* n) { this->PlatformName = n;}
  virtual void ConfigureFinalPass();
private:
  void FillFlagMapFromCommandFlags(std::map<cmStdString, 
                                   cmStdString>& flagMap,
                                   cmVS7FlagTable* flagTable,
                                   std::string& flags);
  std::string GetBuildTypeLinkerFlags(std::string rootLinkerFlags,
                                      const char* configName);
  void OutputVCProjFile();
  void WriteVCProjHeader(std::ostream& fout, const char *libName,
                         cmTarget &tgt, std::vector<cmSourceGroup> &sgs);
  void WriteVCProjFooter(std::ostream& fout);
  void CreateSingleVCProj(const char *lname, cmTarget &tgt);
  void WriteVCProjFile(std::ostream& fout, const char *libName, 
                       cmTarget &tgt);
  void AddVCProjBuildRule(cmTarget& tgt);
  void WriteConfigurations(std::ostream& fout,
                           const char *libName, cmTarget &tgt);
  void WriteConfiguration(std::ostream& fout,
                          const char* configName,
                          const char* libName, cmTarget &tgt); 
  std::string EscapeForXML(const char* s);
  std::string ConvertToXMLOutputPath(const char* path);
  std::string ConvertToXMLOutputPathSingle(const char* path);
  void OutputDefineFlags(const char* flags,
                         std::ostream& fout);
  void OutputTargetRules(std::ostream& fout, cmTarget &target, 
                         const char *libName);
  void OutputBuildTool(std::ostream& fout, const char* configName,
                       const char* libname, cmTarget& t);
  void OutputLibraries(std::ostream& fout,
                       std::vector<cmStdString> const& libs);
  void OutputLibraryDirectories(std::ostream& fout,
                                std::vector<cmStdString> const& dirs);
  void OutputModuleDefinitionFile(std::ostream& fout, cmTarget &target);
  void WriteProjectStart(std::ostream& fout, const char *libName,
                         cmTarget &tgt, std::vector<cmSourceGroup> &sgs);
  void WriteVCProjBeginGroup(std::ostream& fout, 
                          const char* group,
                          const char* filter);
  void WriteVCProjEndGroup(std::ostream& fout);
  void WriteCustomRule(std::ostream& fout,
                       const char* source,
                       const char* command,
                       const char* comment,
                       const std::vector<std::string>& depends,
                       const std::vector<std::string>& outputs,
                       const char* extraFlags);

  void WriteGroup(const cmSourceGroup *sg, 
                  cmTarget target, std::ostream &fout, 
                  const char *libName, std::vector<std::string> *configs);
  virtual std::string GetTargetDirectory(cmTarget&);

  std::vector<std::string> CreatedProjectNames;
  std::string LibraryOutputPath;
  std::string ExecutableOutputPath;
  std::string ModuleDefinitionFile;
  int Version;
  std::string PlatformName; // Win32 or x64 
};

#endif

