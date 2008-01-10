PROJECT(ImageRegTool)


FIND_PACKAGE(ITK)
IF(ITK_FOUND)
  INCLUDE(${ITK_USE_FILE})
ELSE(ITK_FOUND)
  MESSAGE(FATAL_ERROR "Please set ITK_DIR.")
ENDIF(ITK_FOUND)

INCLUDE_DIRECTORIES(
  ${ITK_SOURCE_DIR}/Code/Common
  ${ITK_SOURCE_DIR}/Code/BasicFilters
  ${ITK_SOURCE_DIR}/Code/IO
  ${ITK_SOURCE_DIR}/Code/Numerics
  ${ITK_SOURCE_DIR}/Code/Numerics/vxl
  ${ITK_SOURCE_DIR}/Code/Numerics/Statistics
  ${ITK_SOURCE_DIR}/Utilities/MetaIO
  ${ITK_SOURCE_DIR}/Utilities/MetaIO/SaptialObject
  ${Applications_SOURCE_DIR}/ImageRegTool
  ${Applications_BINARY_DIR}/ImageRegTool
)

ADD_EXECUTABLE(ImageRegTool
    LandmarkRegistrator.cxx
    main.cxx
    )
INSTALL_TARGETS(/bin ImageRegTool)

TARGET_LINK_LIBRARIES (ImageRegTool
ITKIO
ITKMetaIO
ITKBasicFilters
ITKStatistics
ITKNumerics
)

