/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: MString.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/11 21:07:12 $
  Version:   $Revision: 1.10 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "MString.h"

MString::MString()
{
}

MString::MString(MString* m_string)
{
  m_value = m_string->GetValue();
}

MString::MString(std::string m_string)
{
  m_value = m_string;
}
  

MString::MString(int value)
{
  char text[200];
  sprintf(text,"%i",value);
  m_value = text;
}

MString::MString(float value)
{
  char text[200];
  sprintf(text,"%f",value);
  m_value = text;
}

MString::MString(double value)
{
  char text[200];
  sprintf(text,"%f",value);
  m_value = text;
}

MString::~MString()
{
}
  
MString MString::arg(int value)
{
  char text[200];
  sprintf(text,"%i",value);
  m_value.replace(m_value.find("%"),2,text);
  return m_value;
}

MString MString::arg(unsigned int value)
{
  char text[200];
  sprintf(text,"%i",value);
  m_value.replace(m_value.find("%"),2,text);
  return m_value;
}

MString MString::arg(unsigned long value)
{
  char text[200];
  sprintf(text,"%i",value);
  m_value.replace(m_value.find("%"),2,text);
  return m_value;
}


MString MString::arg(float value)
{
  char text[200];
  sprintf(text,"%f",value);
  m_value.replace(m_value.find("%"),2,text);
  return m_value;
}

std::string MString::GetValue()
{
  return m_value;
}

const std::string MString::GetConstValue() const
{
  return m_value;
}

const char* MString::latin1()
{
  return m_value.c_str();
}


bool MString::operator==(MString value)
{
  if (m_value.compare(value.GetValue()) == 0)
    return true;
  else
    return false;
}

bool MString::operator==(const char* value)
{
  if (m_value.compare(value) == 0)
    return true;
  else
    return false;
}

bool MString::operator!=(MString value)
{
  if (m_value.compare(value.GetValue()) == 0)
    return false;
  else
    return true;
}

bool MString::operator!=(const char* value)
{
  if (m_value.compare(value) == 0)
    return false;
  else
    return true;
}

bool MString::operator<(MString value)
{
  if (m_value.compare(value.GetValue()) < 0)
    return true;
  else
    return false;
}



void MString::operator=(const MString & value)
{
  m_value= ((MString)value).GetValue();
}


void MString::operator=(const char* value)
{
  if(strlen(value)>0)
    {  
    m_value = value;
    }
  else
   {
   m_value = "";
   }
}

void MString::operator+=(MString value)
{
  m_value+=value.GetValue();
}

void MString::operator+=(const char* value)
{
   m_value+=value;
}

void MString::operator+=(const char value)
{
   m_value+=value;
}

MString MString::operator+(MString value)
{
  std::string m_text = m_value;
  m_text+=value.GetValue();
  return m_text;
}

MString MString::operator+(const char* value)
{
  std::string m_text = m_value;
  m_text+=value;
  return m_text;
}

MString MString::operator+(int value)
{
  return m_value.substr(value);
}


char MString::operator[](int offset)
{
  return m_value[offset];
}

const char* MString::toChar()
{
  if (m_value.length() == 0)
    return "";

  return m_value.c_str();
}

int MString::toInt()
{
  return atoi(m_value.c_str());
}

float MString::toFloat()
{
  return (float)atof(m_value.c_str());
}

double MString::toDouble()
{
  return atof(m_value.c_str());
}

bool MString::toBool()
{
  if (atoi(m_value.c_str()) == 0)
    return false;
  else
   return true;
}

int MString::length()
{ 
  return strlen(m_value.c_str());
}

int MString::find(const char* key,int offset)
{
  return m_value.find(key,offset);
}

int MString::rfind(const char* key,int offset)
{
  if (offset == -1)
     return m_value.rfind(key);

  return m_value.rfind(key,offset);
}

MString MString::mid(int begin,int nb)
{
  if (nb == -1)
    return m_value.substr(begin,m_value.length());

  return  m_value.substr(begin,nb);
}

MString MString::begin(const char* key,int offset)
{
  int m_offset =  m_value.find(key,offset);
  if (m_offset == -1)
    return m_value;

  return m_value.substr(0,m_offset);
}

MString MString::rbegin(const char* key,int offset)
{
  int m_offset = m_value.rfind(key,offset);
  if (m_offset == -1)
    return m_value;

  return m_value.substr(0,m_offset);
}

MString MString::end(const char* key,int offset)
{
  int m_offset = m_value.find(key,offset);
  if (m_offset == -1)
    return m_value;

  return m_value.substr(m_offset);
}


MString MString::rend(const char* key,int offset)
{
  int m_offset = m_value.rfind(key,offset);
  if (m_offset == -1)
    return m_value;

  return m_value.substr(m_offset);
}


bool MString::startWith(const char key)
{
  for (unsigned int i=0;i<m_value.length();i++)
  {
      if ((m_value[i] != ' ') && (m_value[i] != '\t'))
        if (m_value[i] == key)
          return true;
        else
          return false;
  }
  return false;
}

bool MString::endWith(const char key)
{
  if(m_value[m_value.length()-1] == key)
    {
    return true;
    }
  return false;
}

MString MString::removeChar(const char key,bool onlyfirst)
{
  std::string m_newvalue = "";
  bool keep = false;
  for (unsigned int i=0;i<m_value.length();i++)
  {
    if ((m_value[i] != key) && onlyfirst)
      keep = true;


    if ((m_value[i] != key) || keep)
      m_newvalue += m_value[i];
  }
  return m_newvalue;
}

MString MString::duplicateChar(const char key)
{
  std::string m_newvalue = "";
  for (unsigned int i=0;i<m_value.length();i++)
  {
    m_newvalue += m_value[i];
    if (m_value[i] == key)
      m_newvalue += m_value[i];
  }
  return m_newvalue;
}


MString MString::replaceChar(const char key,const char key2)
{
  std::string m_newvalue = "";
  for (unsigned int i=0;i<m_value.length();i++)
  {
    if (m_value[i] == key)
      m_newvalue += key2;
    else
      m_newvalue += m_value[i];
  }
  return m_newvalue;
}

MString MString::replace(const char* key,const char* key2)
{
  std::string m_newvalue = m_value;
  long pos = -1;
  pos = m_newvalue.find(key);
  while(pos != -1)
    {
    m_newvalue= m_newvalue.replace(pos,strlen(key),key2);
    pos = m_newvalue.find(key);
    }

  return m_newvalue;
}


MString MString::toLower()
{
  std::string m_newvalue = "";
  for (unsigned int i=0;i<m_value.length();i++)
  {
      m_newvalue += tolower(m_value[i]);
  }
  return m_newvalue;
}

MString MString::toUpper()
{
  std::string m_newvalue = "";
  for (unsigned int i=0;i<m_value.length();i++)
  {
      m_newvalue += toupper(m_value[i]);
  }
  return m_newvalue;
}

int MString::count(const char key)
{
  int m_nbchar = 0;
  for (unsigned int i=0;i<m_value.length();i++)
    if (m_value[i] == key) 
      m_nbchar++;

  return m_nbchar;
}


bool MString::isFloat()
{
  for (unsigned int i=0;i<m_value.length();i++)
    {
    if ((!isdigit(m_value[i])) && (m_value[i] != '.') && (m_value[i] != '-'))
      {
      return false;
      }
    }

  return true;
}

bool MString::isInt()
{
  for (unsigned int i=0;i<m_value.length();i++)
    {
    if (!isdigit(m_value[i]) && (m_value[i] != '-'))
      {
      return false;
      }
    }

  return true;
}

/** Return if the current pos is between chars
 *  And we assume pairing */
bool MString::isInBetweenChar(char val,long int pos)
{
  long int pos1 = m_value.find(val);
  while(pos1 != -1)
    {
    long int pos2 = m_value.find(val,pos1+1);
    if(pos2 == -1)
      {
      return false;
      }

    if(pos>=pos1 && pos<=pos2)
      {
      return true;
      }
    pos1 = m_value.find(val,pos2+1);
    }
  return false;
}

/** Convert Wildcars to RegEx */
std::string MString::ConvertWildcardToRegEx(const char* wildcard)
{
  std::string s;
  s += '^';
  for (unsigned int i = 0; i< strlen(wildcard); i++) 
    {
    char c = wildcard[i];
    switch(c)
      {
      case '*':
         s+= ".*";
         break;
      case '?':
         s += ".";
         break;
       // escape special regexp-characters
      case '(': case ')': case '[': case ']': case '$':
      case '^': case '.': case '{': case '}': case '|':
      case '\\':
        s += "\\";
        s += c;
      break;
      default:
        s += c;
      break;
      }
    }
  s += '$';
  return s;
}
