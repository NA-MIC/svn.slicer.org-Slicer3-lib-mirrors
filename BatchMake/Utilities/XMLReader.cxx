/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: XMLReader.cxx,v $
  Language:  C++
  Date:      $Date: 2006/12/18 20:54:05 $
  Version:   $Revision: 1.2 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "XMLReader.h"

XMLReader::XMLReader()
{
}


XMLReader::~XMLReader()
{
}

int XMLReader::Open(const char* filename)
{
  /** Open file for reading */
  networkfile.open(filename,ifstream::binary);
  if (networkfile == NULL) return -1;

  
  return 0;
} 

MString XMLReader::GetValue()
{
  return m_value;
}

MString XMLReader::GetBalise()
{
  char* data = new char[1000];
  networkfile.getline(data,1000);
  MString line = data;
  int begin_balise_start = line.find("<");
  int begin_balise_end =  line.find(">",begin_balise_start+1);
  
  int end_balise_begin = line.find("<",begin_balise_end+1);

  m_balise = line.mid(begin_balise_start+1,begin_balise_end-begin_balise_start-1);

  if (end_balise_begin != -1)
    {
    m_value = line.mid(begin_balise_end+1,end_balise_begin-begin_balise_end-1);
    }
  else
    {
    m_value = "";
    }

  delete [] data;
 
  return m_balise;
}


MString XMLReader::GetCurrentBalise()
{
  return m_balise;
}


void XMLReader::Close()
{
  networkfile.close();
}
