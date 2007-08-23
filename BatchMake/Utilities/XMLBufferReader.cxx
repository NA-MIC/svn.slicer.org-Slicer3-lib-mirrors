/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: XMLBufferReader.cxx,v $
  Language:  C++
  Date:      $Date: 2006/06/30 14:46:04 $
  Version:   $1.0$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __XMLBufferReader_txx
#define __XMLBufferReader_txx

#include "XMLBufferReader.h"
#include "itkExceptionObject.h"
#include <itksys/SystemTools.hxx>
#include <fstream>

namespace itk
{

//----------------------------------------------------------------------------
static void itkXMLParserStartElement(void* parser, const char *name,
                              const char **atts)
{
  // Begin element handler that is registered with the XML_Parser.
  // This just casts the user data to a itkXMLParser and calls
  // StartElement.
  static_cast<XMLBufferReader*>(parser)->StartElement(name, atts);
}

//----------------------------------------------------------------------------
static void itkXMLParserEndElement(void* parser, const char *name)
{
  // End element handler that is registered with the XML_Parser.  This
  // just casts the user data to a itkXMLParser and calls EndElement.
  static_cast<XMLBufferReader*>(parser)->EndElement(name);
}

//----------------------------------------------------------------------------
static void itkXMLParserCharacterDataHandler(void* parser, const char* data,
                                      int length)
{
  // Character data handler that is registered with the XML_Parser.
  // This just casts the user data to a itkXMLParser and calls
  // CharacterDataHandler.
  static_cast<XMLBufferReader*>(parser)->CharacterDataHandler(data, length);
}

void
XMLBufferReader::Parse(const char* buffer,unsigned long buffersize)
{
  XML_Parser Parser = XML_ParserCreate(0);

  XML_SetElementHandler(Parser,
                        &itkXMLParserStartElement,
                        &itkXMLParserEndElement);

  XML_SetCharacterDataHandler(Parser,
                              &itkXMLParserCharacterDataHandler);
  XML_SetUserData(Parser,this);

  /*std::ifstream inputstream;

  inputstream.open(m_Filename.c_str(), std::ios::binary | std::ios::in);
  if(inputstream.fail())
    {
    ExceptionObject exception(__FILE__, __LINE__);
    std::string message = "Can't open ";
    message += m_Filename;
    message += '\n';
    exception.SetDescription(message.c_str());
    throw exception;
    }

  // Default stream parser just reads a block at a time.
  std::streamsize filesize = 
    itksys::SystemTools::FileLength(m_Filename.c_str());

  char *buffer = new char [filesize];
  
  inputstream.read(buffer,filesize);

  if(static_cast<std::streamsize>(inputstream.gcount()) != filesize)
    {
    ExceptionObject exception(__FILE__, __LINE__);
    exception.SetDescription("File Read Error");
    throw exception;
    }*/
  bool result = XML_Parse(Parser,buffer,buffersize,false);
  if(!result)
    {
    ExceptionObject exception(__FILE__, __LINE__);
    std::string message(XML_ErrorString(XML_GetErrorCode(Parser)));
    message += '\n';
    exception.SetDescription(message.c_str());
    throw exception;
    }
  XML_ParserFree(Parser);
  return;
}

 /** Callback function -- called from XML parser with start-of-element
  * information. */
void XMLBufferReader::StartElement(const char * name,const char **atts)
 {
    tag currentTag;
    currentTag.name = name;

    PairType value;
    for(int i = 0; atts[i] != 0; i++)
      {
      if(i%2==0)
        {
        value.first = atts[i];
        }
      else
        {
        value.second = atts[i];
        currentTag.values.push_back(value);
        }
      }
    m_Tags.push_back(currentTag);
 }

/** Callback function -- called from XML parser when ending tag
 * encountered */
void XMLBufferReader::EndElement(const char *name)
    {
    }

/** Callback function -- called from XML parser with the character data
 * for an XML element */
void XMLBufferReader::CharacterDataHandler(const char *inData, int inLength)
{
  PairType value;
  for(int i = 0; i < inLength; i++)
    {
    if(inData[i]!= ' ' && inData[i]!= '\n' && inData[i]!= '\r')
      {
      value.first += inData[i];
      }
    else
      {
      value.second = "";
      if(value.first.size()>0)
        {
        m_Tags[m_Tags.size()-1].values.push_back(value);
        }
      value.first = "";
      }   
    }
  if(value.first.size()>0)
    {
    m_Tags[m_Tags.size()-1].values.push_back(value);
    }
  }

const XMLBufferReader::TagVectorType & 
XMLBufferReader::GetTags() const {return m_Tags;}

}

#endif
