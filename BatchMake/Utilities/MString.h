/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: MString.h,v $
  Language:  C++
  Date:      $Date: 2007/05/11 21:07:12 $
  Version:   $Revision: 1.8 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.


     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifndef __MString_h_
#define __MString_h_

#include "MString.h"
#include <ostream>
#include <iostream>
#include <stdio.h>

class MString
{
public:
  MString();
  MString(MString* string);
  MString(int value);
  MString(float value);
  MString(double value);
  MString(std::string string);
  MString(const char* s) {m_value = s;}
  ~MString();

  std::string GetValue();
  const std::string GetConstValue() const;
  MString arg(int value);
  MString arg(float value);
  MString arg(unsigned int value);
  MString arg(unsigned long value);

  const char* latin1();
  const char* toChar();

  int toInt();
  float toFloat();
  double toDouble();
  bool toBool();
  int length();
  MString mid(int begin,int nb=-1);
  bool operator==(MString);
  bool operator==(const char*);
  bool operator!=(MString);
  bool operator!=(const char*);
  void operator=(const MString &);
  void operator=(const char*);
  void operator+=(MString);
  void operator+=(const char*);
  void operator+=(const char);
  bool operator<(MString);
  MString operator+(int);
  MString operator+(MString);
  MString operator+(const char*);
  int find(const char*,int offset=0);
  int rfind(const char*,int offset=-1);
  MString begin(const char* key,int offset=0);
  MString end(const char* key,int offset=0);
  MString rbegin(const char* key,int offset=-1);
  MString rend(const char* key,int offset=-1);
  char operator[](int);

  bool isFloat();
  bool isInt();

  bool startWith(const char key);
  bool endWith(const char key);
  MString removeChar(const char key,bool onlyfirst=false);
  MString duplicateChar(const char key);
  MString replaceChar(const char key,const char key2);
  MString replace(const char* key,const char* key2);
  MString toLower();
  MString toUpper();
  int count(const char key);

  bool isInBetweenChar(char val,long int pos);

  static std::string ConvertWildcardToRegEx(const char* wildcard);

private:
  std::string m_value;

};

#endif

