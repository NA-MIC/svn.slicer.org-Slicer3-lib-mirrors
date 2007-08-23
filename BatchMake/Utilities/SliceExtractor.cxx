#include "SliceExtractor.h"

SliceExtractor::SliceExtractor()
{
  m_inputimage = 0;
  m_outputimage = 0;
  m_slice = -1;
  m_orientation = -1;
  m_FromMiddleSlice = false;
}

SliceExtractor::~SliceExtractor()
{
}

void SliceExtractor::Extract(std::string input,std::string output)
{
  typedef itk::ImageFileReader<Image3DType>      ReaderType;
  typedef itk::Image<unsigned char,2>            Image2DWriterType;
  typedef itk::ImageFileWriter<Image2DWriterType> WriterType;

  ReaderType::Pointer m_reader = ReaderType::New();
  m_reader->SetFileName(input.c_str());
  try
    {
    m_reader->Update();
    }
  catch (itk::ExceptionObject & e)
    {
    std::cerr << "Exception in file writer " << std::endl;
    std::cerr << e.GetDescription() << std::endl;
    std::cerr << e.GetLocation() << std::endl;
    }
  
  SetInput(m_reader->GetOutput());
  Update();

  typedef itk::RescaleIntensityImageFilter<Image2DType,Image2DWriterType> CasterType;
  CasterType::Pointer m_caster = CasterType::New();
  m_caster->SetOutputMinimum(  0 );
  m_caster->SetOutputMaximum( 255 );
  m_caster->SetInput(GetOutput());
  m_caster->Update();

  WriterType::Pointer m_writer = WriterType::New();
  m_writer->SetInput(m_caster->GetOutput());
  m_writer->SetFileName(output.c_str());
  try
    {
    m_writer->Update();
    }
  catch (itk::ExceptionObject & e)
    {
    std::cerr << "Exception in file writer " << std::endl;
    std::cerr << e.GetDescription() << std::endl;
    std::cerr << e.GetLocation() << std::endl;
    }
}


void SliceExtractor::SetInput(Image3DTypePointer inputimage)
{
  m_inputimage = inputimage;
}

void SliceExtractor::SetFromMiddleSlice(bool fromMiddle)
{
  m_FromMiddleSlice = fromMiddle;
}

SliceExtractor::Image2DTypePointer SliceExtractor::GetOutput()
{
  return m_outputimage;
}

void SliceExtractor::SetSlice(unsigned int slice)
{
  m_slice = slice;
}

void SliceExtractor::SetOrientation(unsigned int orientation)
{
  m_orientation = orientation;
}


void SliceExtractor::Update()
{
  Image3DType::SizeType m_size = m_inputimage->GetLargestPossibleRegion().GetSize();
  Iterator3DType itS(m_inputimage,m_inputimage->GetLargestPossibleRegion());
 
  if(m_orientation == -1)
    {
    m_orientation = 2;
    }

  int slice = 0;
  if(m_slice == -1 && !m_FromMiddleSlice)
    {
    slice = m_size[m_orientation]/2;;
    }

  if(m_FromMiddleSlice)
    {
    slice =  m_size[m_orientation]/2+m_slice;
    }

  //Create 2D image
  int imagesize[2];

  if (m_orientation == 2)
  {
    imagesize[0] = m_size[0];
    imagesize[1] = m_size[1];
  }

  if (m_orientation == 1)
  {
    imagesize[0] = m_size[0];
    imagesize[1] = m_size[2];
  }

  if (m_orientation == 0)
  {
    imagesize[0] = m_size[1];
    imagesize[1] = m_size[2];
  }


  m_outputimage = Image2DType::New();
  float values[2];
  values[0]= 1; 
  values[1]= 1;

  float origin_x= ((imagesize[0]/2)*values[0]*(-1));
  float origin_y=((imagesize[1]/2)*values[1]*(-1));
  float origin[2] = {origin_x, origin_y};

  Image2DType::RegionType region;
  Image2DType::SizeType size;
  size[0]= imagesize[0];
  size[1]= imagesize[1];
  region.SetSize( size );
  m_outputimage->SetRegions( region );
  m_outputimage->Allocate();
  m_outputimage->SetOrigin(origin);
  m_outputimage->SetSpacing(values);

  Iterator2DType itS2(m_outputimage,m_outputimage->GetLargestPossibleRegion());

  if (slice > m_size[m_orientation])
    slice = m_size[m_orientation]-1;

  if (m_orientation == 2)
  {
    for (int z=0;z<m_size[2];z++)
      for (int y=0;y<m_size[1];y++)
         for (int x=0;x<m_size[0];x++)
         {
            if(z == slice)
            {
              itS2.Set((ImagePixelType)itS.Get());
              ++itS2;
            }

            ++itS;
          }
  }


  if (m_orientation == 1)
  {
    for (int z=0;z<m_size[2];z++)
      for (int y=0;y<m_size[1];y++)
         for (int x=0;x<m_size[0];x++)
         {
            if(y == slice)
            {
              itS2.Set((ImagePixelType)itS.Get());
              ++itS2;
            }

            ++itS;
          }
  }

  if (m_orientation == 0)
  {
    for (int z=0;z<m_size[2];z++)
      for (int y=0;y<m_size[1];y++)
         for (int x=0;x<m_size[0];x++)
         {
            if(x == slice)
            {
              itS2.Set((ImagePixelType)itS.Get());
              ++itS2;
            }

            ++itS;
          }
  }
}
