/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkImageRegistrationMethodImageSource.h,v $
  Language:  C++
  Date:      $Date: 2003/09/10 14:30:03 $
  Version:   $Revision: 1.7 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "itkImage.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkCommandIterationUpdate.h"

/** 
 *  This class generates two 2D-Gaussians (standard deviation RegionSize/2)
 *  One is shifted by 7,3 pixels from the other.
 * 
 *  These two images are provided to the registration methods in this
 *  directory in order to obtain a uniform test input data.
 *
 *  Therefore the solution of the registration is |-7 -3|
 */ 
namespace itk 
{

namespace testhelper 
{

template< typename TFixedPixelType, 
          typename TMovingPixelType, 
          unsigned int NDimension     >
class ImageRegistrationMethodImageSource : public itk::Object
{
public:

  typedef ImageRegistrationMethodImageSource    Self;
  typedef Object                                Superclass;
  typedef SmartPointer<Self>                    Pointer;
  typedef SmartPointer<const Self>              ConstPointer;
  typedef Array<double>                         ParametersType;
  
  /** Method for creation through the object factory. */
  itkNewMacro(Self);  

  /** Run-time type information (and related methods). */
  itkTypeMacro(Image, Object);

  
  typedef itk::Image<TMovingPixelType,NDimension> MovingImageType;
  typedef itk::Image<TFixedPixelType,NDimension > FixedImageType;



const MovingImageType * GetMovingImage(void) const
  {
  return m_MovingImage.GetPointer();
  }

const FixedImageType * GetFixedImage(void) const
  {
  return m_FixedImage.GetPointer();
  }

const ParametersType & GetActualParameters(void) const
{
  return m_Parameters;
}


void GenerateImages( const typename MovingImageType::SizeType & size )
{
  typename MovingImageType::IndexType index;
  index.Fill(0);
  typename MovingImageType::RegionType region;
  region.SetSize( size );
  region.SetIndex( index );

  m_MovingImage->SetLargestPossibleRegion( region );
  m_MovingImage->SetBufferedRegion( region );
  m_MovingImage->SetRequestedRegion( region );
  m_MovingImage->Allocate();

  m_FixedImage->SetLargestPossibleRegion( region );
  m_FixedImage->SetBufferedRegion( region );
  m_FixedImage->SetRequestedRegion( region );
  m_FixedImage->Allocate();

  /* Fill images with a 2D gaussian*/
  typedef  itk::ImageRegionIteratorWithIndex<MovingImageType> MovingImageIteratorType;

  typedef  itk::ImageRegionIteratorWithIndex<FixedImageType> FixedImageIteratorType;


  itk::Point<double,2> center;
  center[0] = (double)region.GetSize()[0]/2.0;
  center[1] = (double)region.GetSize()[1]/2.0;

  const double s = (double)region.GetSize()[0]/2.0;

  itk::Point<double,2>  p;
  itk::Vector<double,2> d;

  /* Set the displacement */
  itk::Vector<double,2> displacement;
  displacement[0] = m_Parameters[0];
  displacement[1] = m_Parameters[1];


  MovingImageIteratorType ri(m_MovingImage,region);
  FixedImageIteratorType ti(m_FixedImage,region);
  while(!ri.IsAtEnd())
  {
    p[0] = ri.GetIndex()[0];
    p[1] = ri.GetIndex()[1];
    d = p-center;
    d += displacement;
    const double x = d[0];
    const double y = d[1];
    const double value = 200.0 * exp( - ( x*x + y*y )/(s*s) );
    ri.Set( static_cast<typename MovingImageType::PixelType>(value) );
    ++ri;
  }


  while(!ti.IsAtEnd())
  {
    p[0] = ti.GetIndex()[0];
    p[1] = ti.GetIndex()[1];
    d = p-center;
    const double x = d[0];
    const double y = d[1];
    const double value = 200.0 * exp( - ( x*x + y*y )/(s*s) );
    ti.Set( static_cast<typename FixedImageType::PixelType>(value) );
    ++ti;
  }

  
}

protected:

ImageRegistrationMethodImageSource()
{
  m_MovingImage = MovingImageType::New();
  m_FixedImage  = FixedImageType::New();
  m_Parameters  = ParametersType(2);
  m_Parameters[0] = 7.0;
  m_Parameters[1] = 3.0;
}


private:
  
  typename FixedImageType::Pointer     m_FixedImage;
  typename MovingImageType::Pointer    m_MovingImage;

  ParametersType                       m_Parameters;

};



}  // end namespace testhelper

}  // end namespace itk

