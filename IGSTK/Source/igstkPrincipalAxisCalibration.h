/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    $RCSfile: igstkPrincipalAxisCalibration.h,v $
  Language:  C++
  Date:      $Date: 2007/02/09 23:29:44 $
  Version:   $Revision: 1.7 $

  Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __igstkPrincipalAxisCalibration_h
#define __igstkPrincipalAxisCalibration_h

#ifdef _MSC_VER
#pragma warning ( disable : 4018 )
//Warning about: identifier was truncated to '255' characters in the debug
//information (MVC6.0 Debug)
#pragma warning( disable : 4284 )
#endif

#include "itkCovariantVector.h"
#include "itkVectorContainer.h"

#include "igstkToolCalibration.h"
#include "igstkStateMachine.h"
#include "igstkEvents.h"
#include "igstkMacros.h"

namespace igstk
{

/** \class PrincipalAxisCalibration
 * 
 * \brief Create a calibration transform for tracker tools.
 * 
 * This class calibrates the tracker tools and get the transform
 * from the initial tracked tool's orientation, which is defined by
 * the principal axis and plane normal to the user specified orientation.
 *
 * \ingroup Calibration
 */

class PrincipalAxisCalibration : public ToolCalibration
{
public:

  /** Macro with standard traits declarations. */
  igstkStandardClassTraitsMacro( PrincipalAxisCalibration, ToolCalibration );

public:

  /** Typedefs for the internal computation */
  typedef Transform                       TransformType;

  typedef TransformType::VersorType       VersorType;

  typedef TransformType::VectorType       VectorType;

  typedef VersorType::MatrixType          MatrixType;

  typedef itk::CovariantVector< double >  CovariantVectorType;
public:

  /** Method to get the final calibration transform */
  igstkGetMacro( CalibrationTransform, TransformType );

  /** Method to get the initial principal axis */
  igstkGetMacro( InitialPrincipalAxis, VectorType );

  /** Method to get the initial plane normal */
  igstkGetMacro( InitialPlaneNormal, CovariantVectorType );

  /** Method to get the desired principal axis */
  igstkGetMacro( DesiredPrincipalAxis, VectorType );

  /** Method to get the desired plane normal */
  igstkGetMacro( DesiredPlaneNormal, CovariantVectorType );

  /** Method to indicate a valid rotation is calculated */
  igstkGetMacro( ValidRotation, bool );

  /** Method invoked by the user to reset the calibration process */
  void RequestReset();

  /** Method invoked by the user to set the initial orientation of the tool */
  void RequestSetInitialOrientation( const VectorType & axis, 
                                     const CovariantVectorType & normal );

  /** Method invoked by the user to set the desired orientation of the tool */
  void RequestSetDesiredOrientation( const VectorType & axis, 
                                     const CovariantVectorType & normal );

  /** Method invoked by the user to calculate the rotation */
  void RequestCalculateRotation();

protected:

  /** Constructor */
  PrincipalAxisCalibration();

  /** Destructor */
  virtual ~PrincipalAxisCalibration();

  /** Print the object information in a stream */
  virtual void PrintSelf( std::ostream& os, itk::Indent indent ) const;

  /** Null operation for a transition in the State Machine */
  void NoProcessing();

  /** Reset the calibration matrix */
  void ResetProcessing();

  /** Set the initial orientation of the tool */
  void SetInitialOrientationProcessing();

  /** Internal function to set initial orientation of the tool */
  void InternalSetInitialOrientationProcessing( const VectorType & axis, 
                                        const CovariantVectorType & normal );

  /** Set the desired orientation of the tool */
  void SetDesiredOrientationProcessing();

  /** Internal function to set the desired orientation of the tool */
  void InternalSetDesiredOrientationProcessing( const VectorType & axis, 
                                        const CovariantVectorType & normal );

  /** Calculate the rotation */
  void CalculateRotationProcessing();

  /** Internal function to adjust plane normal */
  void InternalAdjustPlaneNormalProcessing();

  /** Internal function to adjust plane normal */
  CovariantVectorType InternalAdjustPlaneNormalProcessing( 
                const VectorType & axis, const CovariantVectorType & normal );

  /** Internal function to construct the orthogonal matrix */
  MatrixType InternalBuildOrthogonalMatrixProcessing( 
                const VectorType & axis, const CovariantVectorType & normal );

private:

  /** List of States */
  igstkDeclareStateMacro( Idle );
  igstkDeclareStateMacro( InitialOrientationSet );
  igstkDeclareStateMacro( DesiredOrientationSet );
  igstkDeclareStateMacro( OrientationAllSet );
  igstkDeclareStateMacro( RotationCalculated );

  /** List of Inputs */
  igstkDeclareInputMacro( ResetCalibration );
  igstkDeclareInputMacro( InitialOrientation );
  igstkDeclareInputMacro( DesiredOrientation );
  igstkDeclareInputMacro( CalculateRotation );

  /** Temporary input variables for state machine */
  VectorType                        m_VectorToBeSent;

  CovariantVectorType               m_CovariantVectorToBeSent;

  VectorType                        m_VectorToBeReceived;

  CovariantVectorType               m_CovariantVectorToBeReceived;

  /** Variable to save the calibration transform */
  TransformType                     m_CalibrationTransform;

  /** Variable to save the initial principal axis */
  VectorType                        m_InitialPrincipalAxis;

  /** Variable to save the initial plane normal */
  CovariantVectorType               m_InitialPlaneNormal;

  /** Variable to save the initial adjusted plane normal */
  CovariantVectorType               m_InitialAdjustedPlaneNormal;

  /** Variable to save the desired principal axis */
  VectorType                        m_DesiredPrincipalAxis;

  /** Variable to save the desired plane normal */
  CovariantVectorType               m_DesiredPlaneNormal;

  /** Variable to save the desired adjusted plane normal */
  CovariantVectorType               m_DesiredAdjustedPlaneNormal;

  /** Variable to indicate whether a valid rotation is calculated */
  bool                              m_ValidRotation;

};

}

#endif // _igstkPrincipalAxisCalibration_h
