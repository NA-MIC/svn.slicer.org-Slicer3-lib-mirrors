PROJECT (EMSegmentBatchMakeModule)

# Disable MSVC 8 warnings
IF(WIN32)
  OPTION(DISABLE_MSVC8_DEPRECATED_WARNINGS "Disable Visual Studio 8 deprecated warnings" ON)
  MARK_AS_ADVANCED(FORCE DISABLE_MSVC8_DEPRECATED_WARNINGS)  
  IF(DISABLE_MSVC8_DEPRECATED_WARNINGS)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
  ENDIF(DISABLE_MSVC8_DEPRECATED_WARNINGS)
ENDIF(WIN32)

# Find BatchMake
FIND_PACKAGE( BatchMake REQUIRED )
INCLUDE( ${BatchMake_USE_FILE} )

# Find program to configure the module
IF(BatchMake_DIR)
  FIND_PROGRAM(CondorWatcherTool
            NAMES CondorWatcherTool.exe CondorWatcherTool
            PATHS ${BatchMake_DIR}/bin/relwithdebinfo
                  ${BatchMake_DIR}/bin/release
                  ${BatchMake_DIR}/bin/debug
                  ${BatchMake_DIR}/bin)

  FIND_PROGRAM(bmGridSend
            NAMES bmGridSend.exe bmGridSend
            PATHS ${BatchMake_DIR}/bin/relwithdebinfo
                  ${BatchMake_DIR}/bin/release
                  ${BatchMake_DIR}/bin/debug
                  ${BatchMake_DIR}/bin)

  FIND_PROGRAM(bmGridStore
            NAMES bmGridStore.exe bmGridStore
            PATHS ${BatchMake_DIR}/bin/relwithdebinfo
                  ${BatchMake_DIR}/bin/release
                  ${BatchMake_DIR}/bin/debug
                  ${BatchMake_DIR}/bin)

  FIND_PROGRAM(bmSliceExtractor
            NAMES bmSliceExtractor.exe bmSliceExtractor
            PATHS ${BatchMake_DIR}/bin/relwithdebinfo
                  ${BatchMake_DIR}/bin/release
                  ${BatchMake_DIR}/bin/debug
                  ${BatchMake_DIR}/bin)

ENDIF(BatchMake_DIR)

FIND_PATH(Slicer_BINARY_DIR NAMES Base/Logic/SlicerBaseLogicInit.cxx)
IF(Slicer_BINARY_DIR)
  
  FIND_PROGRAM(EMSegmentCommandLine_GUIVersion
            NAMES EMSegmentCommandLine_GUIVersion.exe EMSegmentCommandLine_GUIVersion
            PATHS ${Slicer_BINARY_DIR}/bin/relwithdebinfo
                  ${Slicer_BINARY_DIR}/bin/release
                  ${Slicer_BINARY_DIR}/bin/debug
                  ${Slicer_BINARY_DIR}/bin)

  SET(BATCHMAKE_WRAPPED_APPLICATION_DIR ${EMSegmentBatchMakeModule_BINARY_DIR}/BatchMakeApplications)

  # Copy the wrapped application for BatchMake.
  CONFIGURE_FILE("${EMSegmentBatchMakeModule_SOURCE_DIR}/EMSegmentCommandLine_GUIVersion.bmm.in"
                 "${BATCHMAKE_WRAPPED_APPLICATION_DIR}/EMSegmentCommandLine_GUIVersion.bmm")
  CONFIGURE_FILE("${EMSegmentBatchMakeModule_SOURCE_DIR}/bmSliceExtractor.bmm.in"
                 "${BATCHMAKE_WRAPPED_APPLICATION_DIR}/bmSliceExtractor.bmm")
  CONFIGURE_FILE("${EMSegmentBatchMakeModule_SOURCE_DIR}/bmGridSend.bmm.in"
                 "${BATCHMAKE_WRAPPED_APPLICATION_DIR}/bmGridSend.bmm")
  CONFIGURE_FILE("${EMSegmentBatchMakeModule_SOURCE_DIR}/bmGridStore.bmm.in"
                 "${BATCHMAKE_WRAPPED_APPLICATION_DIR}/bmGridStore.bmm")

  # Configure a header needed by igstkSystemInformation.
  CONFIGURE_FILE("${EMSegmentBatchMakeModule_SOURCE_DIR}/EMSegmentBatchMakeModuleConfig.h.in"
               "${EMSegmentBatchMakeModule_BINARY_DIR}/EMSegmentBatchMakeModuleConfig.h")
 
ENDIF(Slicer_BINARY_DIR)

# Find GenerateCLP
FIND_PACKAGE(GenerateCLP REQUIRED)
INCLUDE(${GenerateCLP_SOURCE_DIR}/UseGenerateCLP.cmake)

SET ( EMSegmentBatchMakeModule_SOURCE 
  EMSegmentBatchMakeModule.cxx
  )

GENERATECLP(EMSegmentBatchMakeModule_SOURCE EMSegmentBatchMakeModule.xml ${EMSegmentBatchMakeModule_SOURCE_DIR}/logo.h)

ADD_EXECUTABLE (EMSegmentBatchMakeModule ${EMSegmentBatchMakeModule_SOURCE})
TARGET_LINK_LIBRARIES(EMSegmentBatchMakeModule BatchMakeLib FLU)