/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkExhaustiveOptimizerTest.cxx,v $
  Language:  C++
  Date:      $Date: 2005-05-14 18:52:48 $
  Version:   $Revision: 1.2 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include "itkExhaustiveOptimizer.h"
#include <vnl/vnl_math.h>

/** 
 *  The objectif function is the quadratic form:
 *
 *  1/2 x^T A x - b^T x
 *
 *  Where A is a matrix and b is a vector
 *  The system in this example is:
 *
 *     | 3  2 ||x|   | 2|   |0|
 *     | 2  6 ||y| + |-8| = |0|
 *
 *
 *   the solution is the vector | 2 -2 |
 *
 */ 
class RSGCostFunction : public itk::SingleValuedCostFunction 
{
public:

  typedef RSGCostFunction                     Self;
  typedef itk::SingleValuedCostFunction      Superclass;
  typedef itk::SmartPointer<Self>            Pointer;
  typedef itk::SmartPointer<const Self>      ConstPointer;
  itkNewMacro( Self );

  enum { SpaceDimension=2 };
  
  typedef Superclass::ParametersType      ParametersType;
  typedef Superclass::DerivativeType      DerivativeType;
  typedef Superclass::MeasureType         MeasureType ;


  RSGCostFunction() 
  {
  }


  MeasureType  GetValue( const ParametersType & parameters ) const
  { 
    
    double x = parameters[0];
    double y = parameters[1];

    std::cout << "GetValue( " ;
    std::cout << x << " ";
    std::cout << y << ") = ";

    MeasureType measure = 0.5*(3*x*x+4*x*y+6*y*y) - 2*x + 8*y;

    std::cout << measure << std::endl; 
    return measure;

  }

  void GetDerivative( const ParametersType & parameters,
                            DerivativeType  & derivative ) const
  {

    double x = parameters[0];
    double y = parameters[1];

    std::cout << "GetDerivative( " ;
    std::cout << x << " ";
    std::cout << y << ") = ";

    derivative = DerivativeType( SpaceDimension ); 
    derivative[0] = 3 * x + 2 * y -2;
    derivative[1] = 2 * x + 6 * y +8;

  }

 
  unsigned int GetNumberOfParameters(void) const
    {
    return SpaceDimension;
    }



private:


};



int itkExhaustiveOptimizerTest(int, char* [] ) 
{
  std::cout << "ExhaustiveOptimizer Test ";
  std::cout << std::endl << std::endl;

  typedef  itk::ExhaustiveOptimizer  OptimizerType;

  typedef  OptimizerType::ScalesType            ScalesType;
  
  
  // Declaration of a itkOptimizer
  OptimizerType::Pointer  itkOptimizer = OptimizerType::New();


  // Declaration of the CostFunction 
  RSGCostFunction::Pointer costFunction = RSGCostFunction::New();


  itkOptimizer->SetCostFunction( costFunction.GetPointer() );

  
  typedef RSGCostFunction::ParametersType    ParametersType;


  const unsigned int spaceDimension = 
                      costFunction->GetNumberOfParameters();

  // We start not so far from  | 2 -2 |
  ParametersType  initialPosition( spaceDimension );
  initialPosition[0] =  0.0;
  initialPosition[1] = -4.0;
  
  itkOptimizer->SetInitialPosition( initialPosition );


  ScalesType    parametersScale( spaceDimension );
  parametersScale[0] = 1.0;
  parametersScale[1] = 1.0;

  itkOptimizer->SetScales( parametersScale );


  itkOptimizer->SetStepLength( 1.0 );


  typedef OptimizerType::StepsType  StepsType;
  StepsType steps( 2 );
  steps[0] = 10;
  steps[1] = 10;

  itkOptimizer->SetNumberOfSteps( steps );


  try 
    {
    itkOptimizer->StartOptimization();
    }
  catch( itk::ExceptionObject & e )
    {
    std::cout << "Exception thrown ! " << std::endl;
    std::cout << "An error ocurred during Optimization" << std::endl;
    std::cout << "Location    = " << e.GetLocation()    << std::endl;
    std::cout << "Description = " << e.GetDescription() << std::endl;
    return EXIT_FAILURE;
    }


  std::cout << "MinimumMetricValue = " << itkOptimizer->GetMinimumMetricValue() << std::endl;
  std::cout << "Minimum Position = " << itkOptimizer->GetMinimumMetricValuePosition() << std::endl;

  std::cout << "MaximumMetricValue = " << itkOptimizer->GetMaximumMetricValue() << std::endl;
  std::cout << "Maximum Position = " << itkOptimizer->GetMaximumMetricValuePosition() << std::endl;

  ParametersType finalPosition = itkOptimizer->GetMinimumMetricValuePosition();
  std::cout << "Solution        = (";
  std::cout << finalPosition[0] << "," ;
  std::cout << finalPosition[1] << ")" << std::endl;  

  //
  // check results to see if it is within range
  //
  bool pass = true;
  double trueParameters[2] = { 2, -2 };
  for( unsigned int j = 0; j < 2; j++ )
    {
    if( vnl_math_abs( finalPosition[j] - trueParameters[j] ) > 0.01 )
      {
      pass = false;
      }
    }

  if( !pass )
    {
    std::cout << "Test failed." << std::endl;
    return EXIT_FAILURE;
    }


  std::cout << "Testing PrintSelf " << std::endl;
  itkOptimizer->Print( std::cout );

  std::cout << "Test passed." << std::endl;
  return EXIT_SUCCESS;


}



