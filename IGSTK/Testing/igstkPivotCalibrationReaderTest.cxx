/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    $RCSfile: igstkPivotCalibrationReaderTest.cxx,v $
  Language:  C++
  Date:      $Date: 2007/02/09 23:29:44 $
  Version:   $Revision: 1.3 $

  Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#if defined(_MSC_VER)
//  Warning about: identifier was truncated to '255' characters 
//  in the debug information (MVC6.0 Debug)
#pragma warning( disable : 4786 )
#endif

#include <iostream>
#include <fstream>
#include "igstkPivotCalibrationReader.h"
#include <itksys/SystemTools.hxx>

#include "itkLogger.h"
#include "itkStdStreamLogOutput.h"

namespace ToolCalibrationTest
{
igstkObserverMacro(Calibration,::igstk::CalibrationModifiedEvent,
                                           ::igstk::PivotCalibration::Pointer)
igstkObserverMacro(String,::igstk::StringEvent,std::string)
}


int igstkPivotCalibrationReaderTest( int argc, char * argv[] )
{
  if(argc<3)
    {
    std::cout << "Usage = " << argv[0] << " calibrationFile" 
              << " calibrationFileCorrupted" << std::endl;
    return EXIT_FAILURE;
    }

  typedef itk::Logger                       LoggerType; 
  typedef itk::StdStreamLogOutput           LogOutputType;

  // Logger object created for logging calibration computation
  LoggerType::Pointer                       logger = LoggerType::New();
  LogOutputType::Pointer                    logOutput = LogOutputType::New();

  logOutput->SetStream( std::cout );
  logger->AddLogOutput( logOutput );
  logger->SetPriorityLevel( itk::Logger::DEBUG );

  // Create the pivot calibration reader and attach the logger
  igstk::PivotCalibrationReader::Pointer reader = 
                              igstk::PivotCalibrationReader::New();

  reader->SetLogger( logger );

  // Set an empty filename
  reader->RequestSetFileName("");
  reader->RequestSetFileName("This.file.does.not.exists");

  std::string dir = itksys::SystemTools::GetFilenamePath(argv[1]);
  reader->RequestSetFileName(dir.c_str());

  reader->RequestSetFileName(argv[2]);
  reader->RequestReadObject();

  reader->RequestSetFileName(argv[1]);
  reader->RequestReadObject();

  reader->Print(std::cout);

  typedef ToolCalibrationTest::CalibrationObserver CalibrationObserverType;
  CalibrationObserverType::Pointer calibrationObserver 
                                              = CalibrationObserverType::New();

  reader->AddObserver(::igstk::CalibrationModifiedEvent(),calibrationObserver);
  reader->RequestGetCalibration();

  igstk::PivotCalibration::Pointer calibration = NULL;

  std::cout << "Testing Calibration: ";
  if( calibrationObserver->GotCalibration() )
    {
    calibration = calibrationObserver->GetCalibration();
    }
  else
    {
    std::cout << "No calibration!" << std::endl;
    return EXIT_FAILURE;
    }

  std::cout << "[PASSED]" << std::endl;

  typedef ToolCalibrationTest::StringObserver StringObserverType;
  StringObserverType::Pointer stringObserver = StringObserverType::New();
  calibration->AddObserver( ::igstk::StringEvent(), stringObserver );
    
  std::cout << "Testing Date: ";
  calibration->RequestGetDate();
  if(stringObserver->GotString())
    {
    if(strcmp(stringObserver->GetString().c_str(),"20050824"))
      {
      std::cout << "Date = " <<  stringObserver->GetString().c_str()
                << " expected 20050824" << std::endl; 
      return EXIT_FAILURE;
      }
    }
  else
    {
    std::cout << "No date!" << std::endl;
    return EXIT_FAILURE;
    }

  std::cout << "[PASSED]" << std::endl;
 
  std::cout << "Testing Time: ";
  calibration->RequestGetTime();
  if(stringObserver->GotString())
    {
    if(strcmp(stringObserver->GetString().c_str(),"070907.0705"))
      {
      std::cout << "Time = " <<  stringObserver->GetString().c_str()
                << " expected 070907.0705" << std::endl; 
      return EXIT_FAILURE;
      }
    }
  else
    {
    std::cout << "No time!" << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "[PASSED]" << std::endl;

  std::cout << "Testing Tooltype: ";
  calibration->RequestGetToolType();
  if(stringObserver->GotString())
    {
    if(strcmp(stringObserver->GetString().c_str(),"Pointer"))
      {
      std::cout << "ToolType = " <<  stringObserver->GetString().c_str()
                << " expected Pointer" << std::endl; 
      return EXIT_FAILURE;
      }
    }
  else
    {
    std::cout << "No tool type!" << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "[PASSED]" << std::endl;

  std::cout << "Testing Manufacturer: ";
  calibration->RequestGetToolManufacturer();
  if(stringObserver->GotString())
    {
    if(strcmp(stringObserver->GetString().c_str(),"Traxtal"))
      {
      std::cout << "Manufacturer = " <<  stringObserver->GetString().c_str()
                << " expected Traxtal" << std::endl; 
      return EXIT_FAILURE;
      }
    }
  else
    {
    std::cout << "No tool manufacturer!" << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "[PASSED]" << std::endl;

  std::cout << "Testing Part Number: ";
  calibration->RequestGetToolPartNumber();
  if(stringObserver->GotString())
    {
    if(strcmp(stringObserver->GetString().c_str(),"023-X"))
      {
      std::cout << "Part Number = " <<  stringObserver->GetString().c_str()
                << " expected 023-X" << std::endl; 
      return EXIT_FAILURE;
      }
    }
  else
    {
    std::cout << "No tool part number!" << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "[PASSED]" << std::endl;

  std::cout << "Testing Serial Number: ";
  calibration->RequestGetToolSerialNumber();
  if(stringObserver->GotString())
    {
    if(strcmp(stringObserver->GetString().c_str(),"200501268"))
      {
      std::cout << "Serial Number = " <<  stringObserver->GetString().c_str()
                << " expected 200501268" << std::endl; 
      return EXIT_FAILURE;
      }
    }
  else
    {
    std::cout << "No tool serial number!" << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "[PASSED]" << std::endl;

  // Test the calibration transform
  std::cout << "Testing Translation: ";
  typedef igstk::Transform TransformType;
  TransformType::VectorType translation = 
                      calibration->GetCalibrationTransform().GetTranslation();
  
  if(translation[0] != 5
     || translation[1] != 2 
     || translation[2] != 3)
    {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "[PASSED]" << std::endl;

  std::cout << "Testing Rotation: ";
  TransformType::VersorType rotation = 
                      calibration->GetCalibrationTransform().GetRotation();
  
  if( fabs(rotation.GetX()-0.994833) > 1e-6
     || fabs(rotation.GetY()-0.0942473) > 1e-6
     || fabs(rotation.GetZ()+0.0209438) > 1e-6
     || fabs(rotation.GetW()-0.0314158) > 1e-6 )
    {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "[PASSED]" << std::endl;

  std::cout << "[Test DONE]" << std::endl;
  return EXIT_SUCCESS;
}
