#include "metaCommand.h"

#include <iostream>
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkResampleImageFilter.h"
#include "itkLinearInterpolateImageFunction.h"


int main(int argc,char * argv[])
{
  MetaCommand command;

  command.SetName("Translation Transform");
  command.SetVersion("1.0");
  command.SetAuthor("Kitware Inc");

  command.AddField("InputImage","Input Image",MetaCommand::STRING,MetaCommand::DATA_IN);
  command.AddField("OutputImage","Output Image",MetaCommand::STRING,MetaCommand::DATA_OUT);

  command.SetOption("coordinate","c", false,"x y z coordinates");
  command.AddOptionField("coordinate","x",MetaCommand::FLOAT,true,"50");
  command.AddOptionField("coordinate","y",MetaCommand::FLOAT,true,"50");
  command.AddOptionField("coordinate","z",MetaCommand::FLOAT,true,"50");


  // Parsing
  if(!command.Parse(argc,argv))
    {
    if(command.GotXMLFlag())
      {
      return 0;
      }
    return 1;
    }

  const unsigned int Dimension = 3;
  typedef itk::Image< unsigned short, Dimension > ImageType;
  typedef itk::ImageFileReader< ImageType >  ReaderType;
  typedef itk::ImageFileWriter< ImageType >  WriterType;

  typedef itk::ResampleImageFilter <ImageType, ImageType>  FilterType;
  typedef itk::AffineTransform <double, Dimension>  TransformType;
  typedef itk::LinearInterpolateImageFunction <ImageType, double>  InterpolatorType;

  FilterType::Pointer filter = FilterType::New();
  TransformType::Pointer transform = TransformType::New();
  InterpolatorType::Pointer interpolator = InterpolatorType::New();
  ReaderType::Pointer reader = ReaderType::New();
  WriterType::Pointer writer = WriterType::New();

  std::string InputImage = command.GetValueAsString("InputImage"); 
  std::string OutputImage = command.GetValueAsString("OutputImage"); 
   
  reader->SetFileName(InputImage);
  reader->Update();
  filter->SetInput(reader->GetOutput());
  
  ImageType::SpacingType spacing;
  spacing[0]=reader->GetOutput()->GetSpacing()[0];
  spacing[1]=reader->GetOutput()->GetSpacing()[1];
  spacing[2]=reader->GetOutput()->GetSpacing()[2];

  ImageType::PointType inputOrigin;
  inputOrigin[0] = reader->GetOutput()->GetOrigin()[1];
  inputOrigin[1] = reader->GetOutput()->GetOrigin()[1];
  inputOrigin[2] = reader->GetOutput()->GetOrigin()[2];

  filter->SetSize(reader->GetOutput()->GetLargestPossibleRegion().GetSize());
  filter->SetOutputSpacing(spacing);
  filter->SetOutputOrigin(inputOrigin);

  TransformType::OutputVectorType translation;
  translation[0] = command.GetValueAsFloat("coordinate", "x"); 
  translation[1] = command.GetValueAsFloat("coordinate", "y"); 
  translation[2] = command.GetValueAsFloat("coordinate", "z"); 
  transform->Translate(translation);

  filter->SetTransform(transform);
  filter->SetInterpolator(interpolator);
  filter->SetDefaultPixelValue(0);

  filter->Update();
  writer->SetFileName(OutputImage);
  writer->SetInput( filter->GetOutput() );
  writer->Update();

  return 0;
}