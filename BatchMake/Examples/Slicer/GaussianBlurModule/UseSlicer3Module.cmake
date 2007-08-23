#
# This module find Slicer3 and build the slicer logic libraries 
#
# Variables: SLICER3_FOUND

# include a test for endianness (used to be done by default)
INCLUDE (${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
TEST_BIG_ENDIAN(CMAKE_WORDS_BIGENDIAN)

FIND_PATH(SLICER3_SOURCE_DIR "slicer_variables.tcl")

IF(SLICER3_SOURCE_DIR)
	
SET(SLICER3_FOUND 1)
## FreeSurfer Library ##
SET(FreeSurfer_SOURCE_DIR ${SLICER3_SOURCE_DIR}/Libs/FreeSurfer)
SET(FreeSurfer_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/FreeSurfer)

CONFIGURE_FILE(
  ${FreeSurfer_SOURCE_DIR}/FreeSurferConfigure.h.in
  ${FreeSurfer_BINARY_DIR}/FreeSurferConfigure.h
)

# --------------------------------------------------------------------------
# Sources
SET(FreeSurfer_SRCS
 ${FreeSurfer_SOURCE_DIR}/vtkFSIO.cxx
 ${FreeSurfer_SOURCE_DIR}/vtkFSSurfaceReader.cxx
 ${FreeSurfer_SOURCE_DIR}/vtkFSSurfaceAnnotationReader.cxx
 ${FreeSurfer_SOURCE_DIR}/vtkFSSurfaceScalarReader.cxx
 ${FreeSurfer_SOURCE_DIR}/vtkFSSurfaceWFileReader.cxx
 ${FreeSurfer_SOURCE_DIR}/vtkFSLookupTable.cxx
)

# Build the library
ADD_LIBRARY(FreeSurfer ${FreeSurfer_SRCS})

TARGET_LINK_LIBRARIES(FreeSurfer vtkIO)

## vtkITK ##
SET(vtkITK_SOURCE_DIR ${SLICER3_SOURCE_DIR}/Libs/vtkITK)
SET(vtkITK_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/vtkITK)

CONFIGURE_FILE(
  ${vtkITK_SOURCE_DIR}/vtkITKConfigure.h.in 
  ${vtkITK_BINARY_DIR}/vtkITKConfigure.h
)

SET(vtkITK_SRCS
  ${vtkITK_SOURCE_DIR}/vtkITKArchetypeImageSeriesReader.cxx
  ${vtkITK_SOURCE_DIR}/vtkITKArchetypeImageSeriesScalarReader.cxx
  ${vtkITK_SOURCE_DIR}/vtkITKArchetypeImageSeriesVectorReader.cxx
  ${vtkITK_SOURCE_DIR}/vtkITKImageWriter.cxx
  ${vtkITK_SOURCE_DIR}/vtkITKImageToImageFilter.h
  ${vtkITK_SOURCE_DIR}/vtkITKImageToImageFilter2DFF.h
  ${vtkITK_SOURCE_DIR}/vtkITKImageToImageFilterFF.h
  ${vtkITK_SOURCE_DIR}/vtkITKImageToImageFilterFUL.h
  ${vtkITK_SOURCE_DIR}/vtkITKImageToImageFilterULUL.h
  ${vtkITK_SOURCE_DIR}/vtkITKImageToImageFilterUSUL.h
  ${vtkITK_SOURCE_DIR}/vtkITKImageToImageFilterUSUS.h
  ${vtkITK_SOURCE_DIR}/vtkITKImageToImageFilterSS.h
  ${vtkITK_SOURCE_DIR}/vtkITKGradientAnisotropicDiffusionImageFilter.cxx
)

INCLUDE_DIRECTORIES(
  ${vtkITK_SOURCE_DIR}
  ${vtkITK_BINARY_DIR}
  ${ITK_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  )

ADD_LIBRARY(vtkITK ${vtkITK_SRCS})

TARGET_LINK_LIBRARIES(vtkITK 
  ITKAlgorithms
  ITKNumerics 
  ITKCommon
  ITKBasicFilters
  ITKNumerics
  ITKStatistics
  ITKBasicFilters
  ITKIO
  ITKDICOMParser
  )

## MRML Library ##
SET(MRML_SOURCE_DIR ${SLICER3_SOURCE_DIR}/Libs/MRML)
SET(MRML_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/MRML)

# --------------------------------------------------------------------------
# Special build configs (to handle linking on windows)
CONFIGURE_FILE(
  ${MRML_SOURCE_DIR}/vtkMRMLConfigure.h.in 
  ${MRML_BINARY_DIR}/vtkMRMLConfigure.h
)

# --------------------------------------------------------------------------
# Sources
SET(MRML_SRCS
   ${MRML_SOURCE_DIR}/vtkMRMLScene.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLParser.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLVolumeNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLScalarVolumeNode.cxx        
   ${MRML_SOURCE_DIR}/vtkMRMLVectorVolumeNode.cxx        
   ${MRML_SOURCE_DIR}/vtkMRMLModelNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLModelStorageNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLFreeSurferModelStorageNode.cxx    
   ${MRML_SOURCE_DIR}/vtkMRMLModelDisplayNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLClipModelsNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLStorageNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLVolumeArchetypeStorageNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLVolumeHeaderlessStorageNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLVolumeDisplayNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLTransformableNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLTransformNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLLinearTransformNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLFiducial.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLFiducialListNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLSliceNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLSliceCompositeNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLSelectionNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLInteractionNode.cxx
   ${MRML_SOURCE_DIR}/vtkObserverManager.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLColorNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLColorTableNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLProceduralColorNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLFreeSurferProceduralColorNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLCameraNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLViewNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLTensorVolumeNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLDiffusionTensorDisplayPropertiesNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLDiffusionWeightedVolumeDisplayNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLDiffusionWeightedVolumeNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLDiffusionTensorVolumeNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLDiffusionTensorVolumeDisplayNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLVectorVolumeDisplayNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLFiberBundleNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLFiberBundleDisplayNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLFiberBundleStorageNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLHierarchyNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLModelHierarchyNode.cxx
   ${MRML_SOURCE_DIR}/vtkMRMLSceneSnapshotNode.cxx
)

# Abstract/pure virtual classes
SET_SOURCE_FILES_PROPERTIES(
  ${MRML_SOURCE_DIR}/vtkMRMLNode.cxx 
  ${MRML_SOURCE_DIR}/vtkMRMLVolumeNode.cxx
  ${MRML_SOURCE_DIR}/vtkMRMLTransformNode.cxx
  ABSTRACT
  )

# Include dirs
INCLUDE_DIRECTORIES(
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  ${vtkITK_SOURCE_DIR}
  ${vtkITK_BINARY_DIR}
  ${vtkTeem_SOURCE_DIR}
  ${vtkTeem_BINARY_DIR}
  ${ITK_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  ${TEEM_INCLUDE_DIRS}
  ${FreeSurfer_SOURCE_DIR}
  ${FreeSurfer_BINARY_DIR}
  )


# Build the library
ADD_LIBRARY(MRML ${MRML_SRCS})

TARGET_LINK_LIBRARIES(MRML
  vtkITK
  vtkCommon
  vtkFiltering
  vtkIO
  vtkRendering
  vtkImaging
  vtkGraphics
  ITKAlgorithms
  ITKNumerics 
  ITKCommon
  ITKBasicFilters
  ITKNumerics
  ITKStatistics
  ITKBasicFilters
  ITKIO
  ITKDICOMParser
  FreeSurfer
  )

#
# Include the MRMLIDImageIO
#
SUBDIRS(${SLICER3_SOURCE_DIR}/Libs/MRMLIDImageIO)
INCLUDE_DIRECTORIES(
  ${SLICER3_SOURCE_DIR}/Libs/MRMLIDImageIO
  ${MRMLIDImageIO_BINARY_DIR}
  )

ENDIF(SLICER3_SOURCE_DIR)

