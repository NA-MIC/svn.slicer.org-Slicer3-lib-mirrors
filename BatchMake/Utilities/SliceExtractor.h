#ifndef SliceExtractor_H
#define SliceExtractor_H

#include <itkImage.h>
#include <itkImageRegionIterator.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkRescaleIntensityImageFilter.h>

class SliceExtractor
{
public:
  SliceExtractor();
  ~SliceExtractor();
  
  typedef unsigned short                        ImagePixelType;
  typedef itk::Image<ImagePixelType,3>          Image3DType;
  typedef Image3DType::Pointer                  Image3DTypePointer;
  typedef itk::ImageRegionIterator<Image3DType> Iterator3DType;
  typedef unsigned short                         Image2DPixelType;
  typedef itk::Image<Image2DPixelType,2>         Image2DType;
  typedef Image2DType::Pointer                  Image2DTypePointer;
  typedef itk::ImageRegionIterator<Image2DType>  Iterator2DType;

  void SetInput(Image3DTypePointer);
  void Update();
  void SetFromMiddleSlice(bool fromMiddle);
  void SetSlice(unsigned int slice);
  void SetOrientation(unsigned int orientation);
  Image2DTypePointer GetOutput();
  void Extract(std::string input,std::string output);

private:
  Image3DTypePointer m_inputimage;
  Image2DTypePointer m_outputimage;
  int m_slice;
  int m_orientation;
  bool m_FromMiddleSlice;
};

#endif
