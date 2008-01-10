/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: bmSliceExtractor.cxx,v $
  Language:  C++
  Date:      $Date: 2007/12/18 19:30:02 $
  Version:   $Revision: 1.3 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/
#include "metaCommand.h"
#include "MString.h"
#include "SliceExtractor.h"

int main(int argc, char* argv[])
{
  MetaCommand command;

  command.SetName("bmSliceExtractor");
  command.SetVersion("1.0");
  command.SetAuthor("Kitware Inc");
  command.SetDescription("Extract a slice from a volume");

  // Required field
  command.AddField("volume","3D Image Filename",MetaCommand::STRING,MetaCommand::DATA_IN);
  command.AddField("slice","2D Image Filename",MetaCommand::STRING,MetaCommand::DATA_OUT);
  command.AddField("orientation","Orientation to extract",MetaCommand::INT);
  command.AddField("sliceNumber","Slice number",MetaCommand::INT);
  command.SetOption("middle","middle",false,"Extract from middle");

  // Parsing
  if(!command.Parse(argc,argv))
    {
    return 1;
    }

  std::string volume = command.GetValueAsString("volume");
  std::string slice = command.GetValueAsString("slice");
  int orientation = command.GetValueAsInt("orientation");
  int sliceNumber = command.GetValueAsInt("sliceNumber");

  SliceExtractor sliceExtractor;
  sliceExtractor.SetOrientation(orientation);
  sliceExtractor.SetSlice(sliceNumber);

  if(command.GetOptionWasSet("middle"))
    {
    sliceExtractor.SetFromMiddleSlice(true);
    }


  sliceExtractor.Extract(volume.c_str(),slice.c_str());

  return 0;
}
