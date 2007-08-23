/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: main.cxx,v $
  Language:  C++
  Date:      $Date: 2006/09/07 02:20:14 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "metaCommand.h"
#include "metaOutput.h"
#include <itkImageFileReader.h>
#include <itkImageRegionConstIterator.h>

int main(int argc, char* argv[])
{
  MetaCommand command;
  
  command.SetName("PixelCounter");
  command.SetVersion("1.0");
  command.SetAuthor("Kitware Inc");
  command.SetDescription("Count the number of pixel above a certain threshold");

  command.AddField("filename","Image filename",MetaCommand::STRING,MetaCommand::DATA_IN);
  command.AddField("threshold","threshold",MetaCommand::FLOAT);

  // This should be put before the parsing
  MetaOutput output;
  output.SetMetaCommand(&command);

  // Parsing
  if(!command.Parse(argc,argv))
    {
    return 1;
    }

  std::string filename = command.GetValueAsString("filename");
  float threshold = command.GetValueAsFloat("threshold");

  // Read the image
  typedef short PixelType;
  typedef itk::Image<PixelType,3> ImageType;
  typedef itk::ImageFileReader<ImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(filename.c_str());
  reader->Update();
  ImageType::Pointer image = reader->GetOutput();
  
  typedef itk::ImageRegionConstIterator<ImageType> IteratorType;
  IteratorType it(image,image->GetLargestPossibleRegion());

  int result = 0;
  it.GoToBegin();
  while(!it.IsAtEnd())
    {
    if(it.Get()>threshold)
      {
      result++;
      }
    ++it;
    }

  output.AddIntField("NPixels","Number of pixels selected",result);
  output.Write();

  return 0;
}
