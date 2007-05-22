/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    $RCSfile: igstkBoxObjectTest.cxx,v $
  Language:  C++
  Date:      $Date: 2007/02/09 23:29:44 $
  Version:   $Revision: 1.9 $

  Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#if defined(_MSC_VER)
// Warning about: identifier was truncated to '255' characters
// in the debug information (MVC6.0 Debug)
#pragma warning( disable : 4786 )
#endif

#include <iostream>

#include "igstkBoxObject.h"
#include "igstkBoxObjectRepresentation.h"
#include "igstkView2D.h"
#include "igstkRealTimeClock.h"
#include "igstkVTKLoggerOutput.h"
#include "itkLogger.h"
#include "itkStdStreamLogOutput.h"

namespace igstk
{
namespace BoxObjectTest
{

class TransformObserver : public ::itk::Command 
{
public:
  typedef  TransformObserver          Self;
  typedef  ::itk::Command             Superclass;
  typedef  ::itk::SmartPointer<Self>  Pointer;
  itkNewMacro( Self );

protected:
  TransformObserver() 
    {
    m_GotTransform = false;
    }
  ~TransformObserver() {}
public:
    
  typedef ::igstk::TransformModifiedEvent  EventType;
        
  void Execute(itk::Object *caller, const itk::EventObject & event)
    {
    const itk::Object * constCaller = caller;
    this->Execute( constCaller, event );
    }

  void Execute(const itk::Object *caller, const itk::EventObject & event)
    {
    m_GotTransform = false;
    if( EventType().CheckEvent( &event ) )
      {
      const EventType * transformEvent = 
                 dynamic_cast< const EventType *>( &event );
      if( transformEvent )
        {
        m_Transform = transformEvent->Get();
        m_GotTransform = true;
        }
      }
    }

  bool GotTransform() const
    {
    return m_GotTransform;
    }

  const ::igstk::Transform & GetTransform() const
    {
    return m_Transform;
    }
        
private:

  ::igstk::Transform  m_Transform;
  bool                m_GotTransform;
};

} // end namespace BoxObjectTest
} // end namespace igstk


int igstkBoxObjectTest( int, char * [] )
{

  igstk::RealTimeClock::Initialize();

  typedef itk::Logger              LoggerType;
  typedef itk::StdStreamLogOutput  LogOutputType;

  // logger object created for logging mouse activities
  LoggerType::Pointer   logger = LoggerType::New();
  LogOutputType::Pointer logOutput = LogOutputType::New();
  logOutput->SetStream( std::cout );
  logger->AddLogOutput( logOutput );
  logger->SetPriorityLevel( itk::Logger::DEBUG );

  // Create an igstk::VTKLoggerOutput and then test it.
  igstk::VTKLoggerOutput::Pointer vtkLoggerOutput 
                                                = igstk::VTKLoggerOutput::New();
  vtkLoggerOutput->OverrideVTKWindow();
  vtkLoggerOutput->SetLogger(logger);  // redirect messages from VTK 
                                       // OutputWindow -> logger

  typedef igstk::BoxObjectRepresentation  ObjectRepresentationType;
  ObjectRepresentationType::Pointer BoxRepresentation 
                                              = ObjectRepresentationType::New();
  BoxRepresentation->SetLogger( logger );

  typedef igstk::BoxObject  ObjectType;
  ObjectType::Pointer BoxObject = ObjectType::New();
  BoxObject->SetLogger( logger );
    
  // Test Set/GetRadius()
  std::cout << "Testing Set/GetSize() : ";
  igstk::BoxObject::ArrayType size;
  size[0] = 10;
  size[1] = 20;
  size[2] = 30;
  BoxObject->SetSize(size);

  igstk::BoxObject::ArrayType sizeRead = BoxObject->GetSize();
  for(unsigned int i=0;i<3;i++)
    {
    if(sizeRead[i] != size[i])
      {
      std::cerr << "Size error : " << sizeRead[i] << " v.s " << size 
                << std::endl; 
      return EXIT_FAILURE;
      }
    }

  size[0] = 11;
  size[1] = 22;
  size[2] = 33;
  BoxObject->SetSize(size[0],size[1],size[2]);
  sizeRead = BoxObject->GetSize();
  for(unsigned int i=0;i<3;i++)
    {
    if(sizeRead[i] != size[i])
      {
      std::cerr << "Size error : " << sizeRead[i] << " v.s " << size 
                << std::endl; 
      return EXIT_FAILURE;
      }
    }
  std::cout << "[PASSED]" << std::endl;

  // Test Property
  std::cout << "Testing Property : ";
  BoxRepresentation->SetColor(0.1,0.2,0.3);
  BoxRepresentation->SetOpacity(0.4);
  if(fabs(BoxRepresentation->GetRed()-0.1)>0.00001)
    {
    std::cerr << "GetRed() [FAILED]" << std::endl;
    return EXIT_FAILURE;
    }
  if(fabs(BoxRepresentation->GetGreen()-0.2)>0.00001)
    {
    std::cerr << "GetGreen()[FAILED]" << std::endl;
    return EXIT_FAILURE;
    }
  if(fabs(BoxRepresentation->GetBlue()-0.3)>0.00001)
    {
    std::cerr << "GetBlue() [FAILED]" << std::endl;
    return EXIT_FAILURE;
    }
  if(fabs(BoxRepresentation->GetOpacity()-0.4)>0.00001)
    {
    std::cerr << "GetOpacity() [FAILED]" << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "[PASSED]" << std::endl;

  // Testing PrintSelf()
  BoxRepresentation->RequestSetBoxObject(BoxObject);
  BoxRepresentation->Print(std::cout);
  BoxObject->Print(std::cout);
  BoxObject->GetNameOfClass();
  BoxRepresentation->GetNameOfClass();

  // testing actors
  std::cout << "Testing actors : ";

  typedef igstk::View2D  View2DType;
  View2DType * view2D = new View2DType(0,0,200,200,"View 2D");
  
  // this will indirectly call CreateActors() 
  view2D->RequestAddObject( BoxRepresentation );
  view2D->SetLogger( logger );
    
  std::cout << "[PASSED]" << std::endl;

  // Testing UpdateRepresentationFromGeometry. Changing the Spatial Object
  // geometrical parameters should trigger an update in the representation
  // class.
  std::cout << "Testing UpdateRepresentationFromGeometry() : ";
  BoxObject->SetSize( 20.0, 30.0, 40.0 );

  BoxRepresentation->SetColor(0.3,0.7,0.2);
  

  // Test GetTransform()
  std::cout << "Testing Set/GetTransform(): ";

  const double tolerance = 1e-8;
  double validityTimeInMilliseconds = 20000.0;  // 20 seconds
  igstk::Transform transform;
  igstk::Transform::VectorType translation;
  translation[0] = 0;
  translation[1] = 1;
  translation[2] = 2;
  igstk::Transform::VersorType rotation;
  rotation.Set( 0.707, 0.0, 0.707, 0.0 );
  igstk::Transform::ErrorType errorValue = 0.01; // 10 microns

  transform.SetTranslationAndRotation( 
      translation, rotation, errorValue, validityTimeInMilliseconds );

  BoxObject->RequestSetTransform( transform );

  typedef ::igstk::BoxObjectTest::TransformObserver  TransformObserverType;

  TransformObserverType::Pointer transformObserver 
                                                 = TransformObserverType::New();

  BoxObject->AddObserver( ::igstk::TransformModifiedEvent(),transformObserver );
  
  BoxObject->RequestSetTransform( transform );
  BoxObject->RequestGetTransform();
  
  if( !transformObserver->GotTransform() )
    {
    std::cerr << "The BoxObject did not returned a Transform event" 
              << std::endl;
    return EXIT_FAILURE;
    }
      
  igstk::Transform  transform2 = transformObserver->GetTransform();

  igstk::Transform::VectorType translation2 = transform2.GetTranslation();
  for( unsigned int i=0; i<3; i++ )
    {
    if( fabs( translation2[i]  - translation[i] ) > tolerance )
      {
      std::cerr << "Translation component is out of range [FAILED]" 
                << std::endl;
      std::cerr << "input  translation = " << translation << std::endl;
      std::cerr << "output translation = " << translation2 << std::endl;
      return EXIT_FAILURE;
      }
    }

  igstk::Transform::VersorType rotation2 = transform2.GetRotation();
  if( fabs( rotation2.GetX() - rotation.GetX() ) > tolerance ||
      fabs( rotation2.GetY() - rotation.GetY() ) > tolerance ||
      fabs( rotation2.GetZ() - rotation.GetZ() ) > tolerance ||
      fabs( rotation2.GetW() - rotation.GetW() ) > tolerance     )
    {
    std::cerr << "Rotation component is out of range [FAILED]" << std::endl;
    std::cerr << "input  rotation = " << rotation << std::endl;
    std::cerr << "output rotation = " << rotation2 << std::endl;
    return EXIT_FAILURE;
    }

  std::cout << "[PASSED]" << std::endl;

  // Testing the Copy() function
  std::cout << "Testing Copy(): ";
  ObjectRepresentationType::Pointer copy = BoxRepresentation->Copy();
  if(copy->GetOpacity() != BoxRepresentation->GetOpacity())
    {
    std::cerr << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "[PASSED]" << std::endl;

  // Exercise RequestSetBoxObject() with a null pointer as argument
  std::cout << "Testing RequestSetBoxObject() with NULL argument: ";
  ObjectRepresentationType::Pointer BoxRepresentation3 
                                              = ObjectRepresentationType::New();
  BoxRepresentation3->RequestSetBoxObject( 0 );

  // Exercise RequestSetBoxObject() called twice. 
  // The second call should be ignored.
  std::cout << "Testing RequestSetBoxObject() called twice: ";
  ObjectRepresentationType::Pointer BoxRepresentation4 
                                              = ObjectRepresentationType::New();
  ObjectType::Pointer BoxObjectA = ObjectType::New();
  ObjectType::Pointer BoxObjectB = ObjectType::New();
  BoxRepresentation4->RequestSetBoxObject( BoxObjectA );
  BoxRepresentation4->RequestSetBoxObject( BoxObjectB );

  // Set properties again in order to exercise the loop that goes through
  // Actors
  std::cout << "Testing set properties : ";
  BoxRepresentation->SetColor(0.9,0.7,0.1);
  BoxRepresentation->SetOpacity(0.8);

  std::cout << BoxRepresentation << std::endl;
  std::cout << BoxObjectA << std::endl;

  std::cout << "Test [DONE]" << std::endl;

  delete view2D;

  if( vtkLoggerOutput->GetNumberOfErrorMessages()  > 0 )
    {
    return EXIT_FAILURE;
    }
  
  return EXIT_SUCCESS;
}
