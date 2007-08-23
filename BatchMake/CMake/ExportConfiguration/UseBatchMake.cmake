#
# This module is provided as BatchMake_USE_FILE by BatchMakeConfig.cmake.  
# It can be INCLUDEd in a project to load the needed compiler and linker
# settings to use BatchMake:
#   FIND_PACKAGE(BatchMake REQUIRED)
#   INCLUDE(${BatchMake_USE_FILE})
#

IF(NOT BatchMake_USE_FILE_INCLUDED)
  SET(BatchMake_USE_FILE_INCLUDED 1)

  # Load the compiler settings used for BatchMake.
  IF(BatchMake_BUILD_SETTINGS_FILE)
    INCLUDE(${CMAKE_ROOT}/Modules/CMakeImportBuildSettings.cmake)
    CMAKE_IMPORT_BUILD_SETTINGS(${BatchMake_BUILD_SETTINGS_FILE})
  ENDIF(BatchMake_BUILD_SETTINGS_FILE)

  # Add compiler flags needed to use BatchMake.
  SET(CMAKE_C_FLAGS 
    "${CMAKE_C_FLAGS} ${BatchMake_REQUIRED_C_FLAGS}")
  SET(CMAKE_CXX_FLAGS 
    "${CMAKE_CXX_FLAGS} ${BatchMake_REQUIRED_CXX_FLAGS}")
  SET(CMAKE_EXE_LINKER_FLAGS 
    "${CMAKE_EXE_LINKER_FLAGS} ${BatchMake_REQUIRED_EXE_LINKER_FLAGS}")
  SET(CMAKE_SHARED_LINKER_FLAGS 
    "${CMAKE_SHARED_LINKER_FLAGS} ${BatchMake_REQUIRED_SHARED_LINKER_FLAGS}")
  SET(CMAKE_MODULE_LINKER_FLAGS 
    "${CMAKE_MODULE_LINKER_FLAGS} ${BatchMake_REQUIRED_MODULE_LINKER_FLAGS}")

  # Add include directories needed to use BatchMake.
  INCLUDE_DIRECTORIES(${BatchMake_INCLUDE_DIRS})

  # Add link directories needed to use BatchMake.
  LINK_DIRECTORIES(${BatchMake_LIBRARY_DIRS})

  # Add cmake module path.
  SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${BatchMake_CMAKE_DIR}")

  # Use ITK.
  IF(NOT ITK_DIR)
    SET(ITK_DIR ${BatchMake_ITK_DIR})
    FIND_PACKAGE(ITK)
    IF(ITK_FOUND)
      INCLUDE(${ITK_USE_FILE})
    ELSE(ITK_FOUND)
      MESSAGE("ITK not found in BatchMake_ITK_DIR=\"${BatchMake_ITK_DIR}\".")
    ENDIF(ITK_FOUND)
  ENDIF(NOT ITK_DIR)

  # Use FLTK.
  IF(BatchMake_USE_FLTK AND NOT BatchMake_NO_USE_FLTK)
    IF(NOT FLTK_DIR)
      SET(FLTK_DIR ${BatchMake_FLTK_DIR})
      FIND_PACKAGE(FLTK)
      IF(FLTK_FOUND)
        IF(FLTK11_USE_FILE)
          SET(FLTK11_FOUND ${FLTK_FOUND})
          INCLUDE(${FLTK11_USE_FILE})
        ELSE(FLTK11_USE_FILE)
          MESSAGE("FLTK11_USE_FILE not found.")
        ENDIF(FLTK11_USE_FILE)
      ELSE(FLTK_FOUND)
        MESSAGE(
          "FLTK not found in BatchMake_FLTK_DIR=\"${BatchMake_FLTK_DIR}\".")
      ENDIF(FLTK_FOUND)
    ENDIF(NOT FLTK_DIR)
  ENDIF(BatchMake_USE_FLTK AND NOT BatchMake_NO_USE_FLTK)

  # Same thing should probably be done for Qt and Phantom, but
  # I do not know enough about those packages at the moment

ENDIF(NOT BatchMake_USE_FILE_INCLUDED)
