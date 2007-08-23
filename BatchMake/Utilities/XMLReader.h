/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: XMLReader.h,v $
  Language:  C++
  Date:      $Date: 2004/11/30 20:04:23 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifndef _XMLREADER_H
#define _XMLREADER_H

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <MString.h>

using namespace std;


class XMLReader
{
public:
  XMLReader();
  ~XMLReader();
  int Open(const char* filename);
  MString GetBalise();
  MString GetCurrentBalise();
  MString GetValue();
  void Close();

private:
  std::ifstream networkfile;
  MString m_value;
  MString m_balise;


};

#endif
