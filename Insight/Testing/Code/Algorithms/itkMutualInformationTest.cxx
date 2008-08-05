/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit (ITK)
  Module:    $RCSfile: itkMutualInformationTest.cxx,v $
  Language:  C++
  Date:      $Date: 2001-04-27 13:13:04 $
  Version:   $Revision: 1.12 $

Copyright (c) 2001 Insight Consortium
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * The name of the Insight Consortium, nor the names of any consortium members,
   nor of any contributors, may be used to endorse or promote products derived
   from this software without specific prior written permission.

  * Modified source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "itkImage.h"
#include "itkScalar.h"
#include "itkImageRegionIterator.h"
#include "itkParzenWindowMutualInformationAffineRegistrator.h"
#include "itkKernelFunction.h"

#include "vnl/vnl_sample.h"

int main()
{

  /* ---------------------------------------------------
   * create a simple image with simple derivatives
   */
  typedef itk::Image<float,2> ImageType;

  ImageType::SizeType size = {{64,64}};
  ImageType::IndexType index = {{0,0}};
  ImageType::RegionType region;
  region.SetSize( size );
  region.SetIndex( index );

  ImageType::Pointer imgReference = ImageType::New();
  imgReference->SetLargestPossibleRegion( region );
  imgReference->SetBufferedRegion( region );
  imgReference->Allocate();

  ImageType::Pointer imgTest = ImageType::New();
  imgTest->SetLargestPossibleRegion( region );
  imgTest->SetBufferedRegion( region );
  imgTest->Allocate();

  ImageType::Pointer imgDeriv[ImageType::ImageDimension];
  for( int j = 0; j < ImageType::ImageDimension; j++ )
    {
    imgDeriv[j] = ImageType::New();
    imgDeriv[j]->SetLargestPossibleRegion( region );
    imgDeriv[j]->SetBufferedRegion( region );
    imgDeriv[j]->Allocate();
    }

  typedef
    itk::ImageRegionIterator<ImageType> Iterator;
  Iterator testIter( imgTest, region );
  Iterator refIter( imgReference, region );
  Iterator colIter( imgDeriv[0], region );
  Iterator rowIter( imgDeriv[1], region );

  while ( !testIter.IsAtEnd() )
    {

    index = testIter.GetIndex();

    double factor = 0.5;
    testIter.Set( vnl_math_sqrt( vnl_math_sqr( float(index[0]) - 32.5 ) +
                             factor * vnl_math_sqr( float(index[1]) - 32.5 )));
    colIter.Set( ( float(index[0]) - 32.5 ) / testIter.Get() );
    rowIter.Set( factor * ( float(index[1]) - 32.5 ) / testIter.Get() );

    double value = 33.0;
    testIter.Set( testIter.Get() / value );
    refIter.Set( testIter.Get() + vnl_sample_uniform( -0.005, 0.005) );
    colIter.Set( colIter.Get() / value );
    rowIter.Set( rowIter.Get() / value );

    ++testIter;
    ++refIter;
    ++colIter;
    ++rowIter;
    }

  /*------------------------------------------------------------
   * Create mutual information registration object
   */
  typedef
    itk::ParzenWindowMutualInformationAffineRegistrator<ImageType,ImageType,ImageType>
      RegistratorType;

  RegistratorType::Pointer registrator = RegistratorType::New();

  registrator->SetReferenceImage( imgReference );
  registrator->SetTestImage( imgTest );
  registrator->SetTestImageDerivative( imgDeriv[0], 0 );
  registrator->SetTestImageDerivative( imgDeriv[1], 1 );

  RegistratorType::MatrixType initMatrix;
  RegistratorType::VectorType initVector;

  initMatrix.set_identity();
  initMatrix.put( 0, 1, -0.16 );
  initMatrix.put( 1, 0, 0.16 );
  initVector.fill( 32.5 );
  initVector -= initMatrix * initVector;

  registrator->SetInitialAffineMatrix( initMatrix );
  registrator->SetInitialAffineVector( initVector );

  // setup the internal calculator
  typedef RegistratorType::DefaultCalculatorType CalculatorType;
  CalculatorType::Pointer calculator =
    dynamic_cast<CalculatorType*> (
      registrator->GetMutualInformationImageMetricCalculator().GetPointer() );

  calculator->SetReferenceStdDev( 0.1 );
  calculator->SetTestStdDev( 0.1 );
  calculator->SetNumberOfSamples( 100 );

  typedef itk::KernelFunction KernelFunction;
  itk::KernelFunction::Pointer kernel = 
    dynamic_cast<KernelFunction*>(
      itk::GaussianKernelFunction::New().GetPointer() );

  calculator->SetKernelFunction( kernel );

  // printout the internal calculator parameters
  std::cout << "Calculator parameters" << std::endl;
  std::cout << "No.of Samples: " << calculator->GetNumberOfSamples();
  std::cout << std::endl;
  std::cout << "Reference Std. Dev: " << calculator->GetReferenceStdDev();
  std::cout << std::endl;
  std::cout << "Test Std. Dev: " << calculator->GetTestStdDev();
  std::cout << std::endl;

  registrator->SetLearningRate( 5.0 );
  registrator->SetNumberOfIterations( 200 );

  registrator->SetDebugOn( false );

  registrator->Maximize();

  std::cout << "last parameters" << std::endl;
  std::cout << registrator->GetLastAffineMatrix() << std::endl;
  std::cout << registrator->GetLastAffineVector() << std::endl;

  return EXIT_SUCCESS;

}
