/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: XMLWriter.h,v $
  Language:  C++
  Date:      $Date: 2004/11/30 20:04:23 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifndef _XMLWRITER_H
#define _XMLWRITER_H

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <MString.h>

//using namespace std;

class XMLWriter
{
public:
  XMLWriter();
  ~XMLWriter();
  void Start(char *);
  void End(char *);
  void Write(MString balise,MString value);
  void Write(MString balise,int value);
  void Write(MString balise,float value);
  int Open(const char* filename);
  void Close();
  void startTab();
  void endTab();

protected:

private:
  char filename[300];
  int moduleid;
  std::ofstream networkfile;
  int treeoffset;
};

#endif
