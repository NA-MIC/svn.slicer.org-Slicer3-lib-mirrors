/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: XMLBufferReader.h,v $
  Language:  C++
  Date:      $Date: 2006/06/30 14:46:04 $
  Version:   $1.0$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __XMLBufferReader_h
#define __XMLBufferReader_h
#include "itkLightProcessObject.h"
#include "expat.h"
#include <fstream>

namespace itk
{

/** \class XMLReaderBase
 * XMLReaderBase encapsulates the expat library (Insight/Utilities/expat
 * and defines the methods needed in a derived class to receive the
 * contents of an XML file in a structured manner.  It's 'impure virtual'
 * in that some functions that are generic to opening and parsing a file
 * are implemented here.
 */
class
XMLBufferReader : public LightProcessObject
{
public:

  /** Standard typedefs */ 
  typedef XMLBufferReader                    Self;
  typedef LightProcessObject                 Superclass;
  typedef SmartPointer<Self>                 Pointer;
  typedef std::pair<std::string,std::string> PairType;

  struct tag
    {
    std::string name;
    std::vector<PairType> values;
    };
  typedef std::vector<tag>                   TagVectorType;

  /** Run-time type information (and related methods). */
  itkTypeMacro(XMLBufferReader, XMLReaderBase);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Callback function -- called from XML parser with start-of-element
   * information.
   */
  virtual void StartElement(const char * name,const char **atts);
  /** Callback function -- called from XML parser when ending tag
   * encountered
   */
  virtual void EndElement(const char *name);
  /** Callback function -- called from XML parser with the character data
   * for an XML element
   */
  virtual void CharacterDataHandler(const char *inData, int inLength);
  
  /** Instantiates and invokes the XML parser for the file named by
   * m_Filename.  The parser will throw an exception in the case of XML
   * syntax errors, missing filenames, unreadable input file, etc.
   */
  void Parse(const char* buffer,unsigned long buffersize);

  const TagVectorType & GetTags() const;

protected:

  XMLBufferReader() 
    {
    }
  ~XMLBufferReader() {};

  TagVectorType m_Tags;
};

} // end namespace itk
#endif
