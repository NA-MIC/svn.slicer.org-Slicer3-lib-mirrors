/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmScriptConvertImageAction.cxx,v $
  Language:  C++
  Date:      $Date: 2007/08/28 14:16:51 $
  Version:   $Revision: 1.2 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "bmScriptConvertImageAction.h"
#include "SliceExtractor.h"

namespace bm {

ScriptConvertImageAction::ScriptConvertImageAction()
: ScriptAction()
{
}

ScriptConvertImageAction::~ScriptConvertImageAction()
{
}

bool ScriptConvertImageAction::TestParam(ScriptError* error,int linenumber)
{
  for (unsigned int i=0;i<m_Parameters.size();i++)
      m_Manager->TestConvert(m_Parameters[i],linenumber);
    
  if (m_Parameters.size() < 2)
  {
    error->SetError(MString("No enough parameter for ConvertImage"),linenumber);
    return false;
  }   

  return true;
}

MString ScriptConvertImageAction::Help()
{
  return "ConvertImage(<input image filename> <output image filename>)";
}

/** Convert the image */
template<class T> int ScriptConvertImageActionConvertImage(const char* inputImage, 
                                                         const char* outputImage,
                                                         T)
{
  typedef  T     InputPixelType;
  typedef  T     OutputPixelType;

  typedef itk::Image< InputPixelType,  3 >   InputImageType;
  typedef itk::Image< OutputPixelType, 3 >   OutputImageType;

  typedef itk::ImageFileReader< InputImageType >  ReaderType;
  typedef itk::ImageFileWriter< OutputImageType > WriterType;
  typename ReaderType::Pointer reader = ReaderType::New();

  reader->SetFileName( inputImage );

  typename WriterType::Pointer writer = WriterType::New();
  writer->SetFileName( outputImage );
  writer->SetInput( reader->GetOutput() );
  writer->UseCompressionOn();
  try
    {
    writer->Update();
    }
  catch( itk::ExceptionObject &excep)
    {
    std::cerr << excep << std::endl;
    return 0;
    }

  return 1;
}

void ScriptConvertImageAction::Execute()
{
/*#ifdef BM_GRID
  if(m_GridModule)
    {
    this->GenerateGrid();
    return;
    }
#endif
*/
  MString inputImage = m_Manager->Convert(m_Parameters[0]).removeChar('\'').latin1();
  MString outputImage = m_Manager->Convert(m_Parameters[1]).removeChar('\'').latin1();
   
  itk::ImageIOBase::IOPixelType pixelType;
  itk::ImageIOBase::IOComponentType componentType;

  try
    {
    // Get the image type
    typedef itk::Image<short, 3> ImageType;
    itk::ImageFileReader<ImageType>::Pointer imageReader =
                                  itk::ImageFileReader<ImageType>::New();
    imageReader->SetFileName(inputImage.toChar());
    imageReader->UpdateOutputInformation();

    pixelType = imageReader->GetImageIO()->GetPixelType();
    componentType = imageReader->GetImageIO()->GetComponentType();

    // This filter handles all types on input, but only produces
    // signed types
    switch (componentType)
      {
      case itk::ImageIOBase::UCHAR:
        ScriptConvertImageActionConvertImage( inputImage.toChar(), outputImage.toChar(), static_cast<unsigned char>(0));
        break;
      case itk::ImageIOBase::CHAR:
        ScriptConvertImageActionConvertImage( inputImage.toChar(), outputImage.toChar(), static_cast<char>(0));
        break;
      case itk::ImageIOBase::USHORT:
        ScriptConvertImageActionConvertImage( inputImage.toChar(), outputImage.toChar(), static_cast<unsigned short>(0));
        break;     
      case itk::ImageIOBase::SHORT:
        ScriptConvertImageActionConvertImage( inputImage.toChar(), outputImage.toChar(), static_cast<short>(0));
        break;
      case itk::ImageIOBase::UINT:
        ScriptConvertImageActionConvertImage( inputImage.toChar(), outputImage.toChar(), static_cast<unsigned int>(0));
      case itk::ImageIOBase::INT:
        ScriptConvertImageActionConvertImage( inputImage.toChar(), outputImage.toChar(), static_cast<int>(0));
        break;
      case itk::ImageIOBase::ULONG:
        ScriptConvertImageActionConvertImage( inputImage.toChar(), outputImage.toChar(), static_cast<unsigned long>(0));
      case itk::ImageIOBase::LONG:
        ScriptConvertImageActionConvertImage( inputImage.toChar(), outputImage.toChar(), static_cast<long>(0));
        break;
      case itk::ImageIOBase::FLOAT:
        ScriptConvertImageActionConvertImage( inputImage.toChar(), outputImage.toChar(), static_cast<float>(0));
        break;
      case itk::ImageIOBase::DOUBLE:
        ScriptConvertImageActionConvertImage( inputImage.toChar(), outputImage.toChar(), static_cast<double>(0));
        break;
      case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:
      default:
        std::cout << "unknown component type" << std::endl;
        break;
      }
    }
  catch( itk::ExceptionObject &excep)
    {
    std::cerr << excep << std::endl;
    return;
    }
}

} // end namespace bm
