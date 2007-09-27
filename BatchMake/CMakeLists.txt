PROJECT(BatchMake)

IF(NOT LIBRARY_OUTPUT_PATH)
  SET (LIBRARY_OUTPUT_PATH ${BatchMake_BINARY_DIR}/bin CACHE PATH 
    "Single output directory for building all libraries.")
ENDIF(NOT LIBRARY_OUTPUT_PATH)
MARK_AS_ADVANCED(LIBRARY_OUTPUT_PATH)
SET(BatchMake_LIBRARY_DIR "${LIBRARY_OUTPUT_PATH}")
SET(BatchMake_RUNTIME_DIR "${LIBRARY_OUTPUT_PATH}")

IF(NOT EXECUTABLE_OUTPUT_PATH)
  SET (EXECUTABLE_OUTPUT_PATH ${BatchMake_BINARY_DIR}/bin CACHE PATH 
    "Single output directory for building all executables.")
ENDIF(NOT EXECUTABLE_OUTPUT_PATH)
MARK_AS_ADVANCED(EXECUTABLE_OUTPUT_PATH)
SET(BatchMake_APPLICATIONS_DIR "${EXECUTABLE_OUTPUT_PATH}/Applications")

SET(CXX_TEST_PATH ${EXECUTABLE_OUTPUT_PATH})

SET(BatchMake_CMAKE_DIR "${BatchMake_SOURCE_DIR}/CMake" CACHE INTERNAL "")
SUBDIRS(CMake)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${BatchMake_CMAKE_DIR}")

SET(BatchMake_CONFIGURATION_TYPES ${CMAKE_CONFIGURATION_TYPES})

MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)

#-----------------------------------------------------------------------------
# BatchMake version number.  An even minor number corresponds to releases.

SET(BatchMake_VERSION_MAJOR "1")
SET(BatchMake_VERSION_MINOR "0")
SET(BatchMake_VERSION_PATCH "4")

# Version string should not include patch level.  The major.minor is
# enough to distinguish available features of the toolkit.
SET(BatchMake_VERSION_STRING 
  "${BatchMake_VERSION_MAJOR}.${BatchMake_VERSION_MINOR}")
SET(BatchMake_EXTENDED_VERSION_STRING 
  "${BatchMake_VERSION_STRING}.${BatchMake_VERSION_PATCH}")

#-----------------------------------------------------------------------------
# Disable MSVC 8 warnings

IF(WIN32)
  OPTION(DISABLE_MSVC8_DEPRECATED_WARNINGS 
    "Disable Visual Studio 8 deprecated warnings" ON)
  MARK_AS_ADVANCED(FORCE DISABLE_MSVC8_DEPRECATED_WARNINGS)  
  IF(DISABLE_MSVC8_DEPRECATED_WARNINGS)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
  ENDIF(DISABLE_MSVC8_DEPRECATED_WARNINGS)
ENDIF(WIN32)

#-----------------------------------------------------------------------------
# We need ITK

FIND_PACKAGE(ITK REQUIRED)
IF(ITK_FOUND)
  INCLUDE(${USE_ITK_FILE}) 
ENDIF(ITK_FOUND)
LINK_LIBRARIES(${ITK_LIBRARIES})

SET(BUILD_SHARED_LIBS ${ITK_BUILD_SHARED})

SET(BatchMake_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})

#-----------------------------------------------------------------------------
# Check for support FLTK

OPTION(USE_FLTK "Add FLTK GUI support to BatchMake" OFF) 
IF(USE_FLTK)
  FIND_PACKAGE(FLTK)
  IF(FLTK_FOUND)
    INCLUDE_DIRECTORIES(${FLTK_INCLUDE_DIR})
  ENDIF(FLTK_FOUND)
  ADD_DEFINITIONS(-DBM_USE_FLTK)
ENDIF(USE_FLTK)

#-----------------------------------------------------------------------------
# Display the splashscreen or not

OPTION(USE_SPLASHSCREEN "Display the splashscreen" ON)
IF(USE_SPLASHSCREEN)
 ADD_DEFINITIONS(-DBM_SPLASHSCREEN)
ENDIF(USE_SPLASHSCREEN)

#-----------------------------------------------------------------------------
# Setup Paths

IF(BatchMake_INCLUDE_DIR)
  SET(BatchMake_INCLUDE_PATH
    ${BatchMake_INCLUDE_DIR}/Code
    ${BatchMake_INCLUDE_DIR}/Utilities
    ${BatchMake_INCLUDE_DIR}/Utilities/Zip
    )
ENDIF(BatchMake_INCLUDE_DIR)

SET(BatchMake_INCLUDE_PATH
  ${BatchMake_INCLUDE_PATH}
  ${BatchMake_SOURCE_DIR}/Code
  ${BatchMake_SOURCE_DIR}/Utilities
  ${BatchMake_BINARY_DIR}
  ${BatchMake_BINARY_DIR}/Code
  )

IF(USE_FLTK)
  IF(BatchMake_INCLUDE_DIR)
    SET(BatchMake_INCLUDE_PATH
      ${BatchMake_INCLUDE_PATH}
      ${BatchMake_INCLUDE_DIR}/Utilities/Flu
      )
  ENDIF(BatchMake_INCLUDE_DIR)
  SET(BatchMake_INCLUDE_PATH
    ${BatchMake_INCLUDE_PATH}
    ${BatchMake_SOURCE_DIR}/Utilities/Flu
    )
ENDIF(USE_FLTK)

INCLUDE_DIRECTORIES(${BatchMake_INCLUDE_PATH})

#-----------------------------------------------------------------------------
# If this is the install version of BathMake we look for the include files

FIND_PATH(BatchMake_INCLUDE_DIR 
  Code/ApplicationWrapper.h 
  ${BatchMake_SOURCE_DIR}/../include/BatchMake)
MARK_AS_ADVANCED(FORCE BatchMake_INCLUDE_DIR)


#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
#Compile the Utilities

SUBDIRS( Utilities )

#-----------------------------------------------------------------------------
#Compile the main libraries

SUBDIRS( Code )

SET(BatchMake_LIBRARIES
  BatchMakeLib
  BatchMakeUtilities
  )

#-----------------------------------------------------------------------------
# Build BatchBoard Support

OPTION(DASHBOARD_SUPPORT "Validation Dashboard Support" OFF)
IF(DASHBOARD_SUPPORT)

  ADD_DEFINITIONS(-DBM_DASHBOARD)

  IF(BatchMake_INCLUDE_DIR)
    SET(BatchMake_INCLUDE_PATH
      ${BatchMake_INCLUDE_PATH}
      ${BatchMake_INCLUDE_DIR}/Dashboard
      )
    INCLUDE_DIRECTORIES(${BatchMake_INCLUDE_DIR}/Grid)
  ENDIF(BatchMake_INCLUDE_DIR)
  SET(BatchMake_INCLUDE_PATH
    ${BatchMake_INCLUDE_PATH}
    ${BatchMake_SOURCE_DIR}/Code/Dashboard
    )
  INCLUDE_DIRECTORIES(${BatchMake_SOURCE_DIR}/Code/Dashboard)

ENDIF(DASHBOARD_SUPPORT)

#-----------------------------------------------------------------------------
# Grid Support

OPTION(GRID_SUPPORT "Grid Support" OFF)
IF(GRID_SUPPORT)

  ADD_DEFINITIONS(-DBM_GRID)

  IF(BatchMake_INCLUDE_DIR)
    SET(BatchMake_INCLUDE_PATH
      ${BatchMake_INCLUDE_PATH}
      ${BatchMake_INCLUDE_DIR}/Grid
      )
    INCLUDE_DIRECTORIES(${BatchMake_INCLUDE_DIR}/Grid)
  ENDIF(BatchMake_INCLUDE_DIR)
  SET(BatchMake_INCLUDE_PATH
    ${BatchMake_INCLUDE_PATH}
    ${BatchMake_SOURCE_DIR}/Code/Grid
    )
  INCLUDE_DIRECTORIES(${BatchMake_SOURCE_DIR}/Code/Grid)

  IF(USE_FLTK)
    IF(BatchMake_INCLUDE_DIR)
      SET(BatchMake_INCLUDE_PATH
        ${BatchMake_INCLUDE_PATH}
        ${BatchMake_INCLUDE_DIR}/Utilities/CondorWatcher
        )
      INCLUDE_DIRECTORIES(${BatchMake_INCLUDE_DIR}/Utilities/CondorWatcher)
    ENDIF(BatchMake_INCLUDE_DIR)
    SET(BatchMake_INCLUDE_PATH
      ${BatchMake_INCLUDE_PATH}
      ${BatchMake_SOURCE_DIR}/Utilities/CondorWatcher
      ${BatchMake_BINARY_DIR}/Utilities/CondorWatcher
      )
    INCLUDE_DIRECTORIES(${BatchMake_SOURCE_DIR}/Utilities/CondorWatcher)
    INCLUDE_DIRECTORIES(${BatchMake_BINARY_DIR}/Utilities/CondorWatcher)
  ENDIF(USE_FLTK)

ENDIF(GRID_SUPPORT)

#-----------------------------------------------------------------------------
# Build Testing

SUBDIRS( Testing/Code )

MARK_AS_ADVANCED(DART_ROOT)
INCLUDE(Dart)
IF(NOT DART_ROOT) 
  SET(DROP_LOCATION /DartClient/BatchMake)
ENDIF(NOT DART_ROOT)

#-----------------------------------------------------------------------------
# Build the Applications

SUBDIRS(Applications)

#-----------------------------------------------------------------------------
# Build the Examples

OPTION(BUILD_EXAMPLES "BatchMake Examples" OFF)
IF(BUILD_EXAMPLES)
  SUBDIRS(Examples)
ENDIF(BUILD_EXAMPLES)

#-----------------------------------------------------------------------------
# Doc
SUBDIRS(Documentation/Doxygen)


#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
# Use KWStyle for checking the coding style

OPTION(BatchMake_USE_KWSTYLE 
  "Run KWStyle in order to check for violations of the coding standard." OFF)
MARK_AS_ADVANCED(BatchMake_USE_KWSTYLE)
IF(BatchMake_USE_KWSTYLE)
  SUBDIRS(Utilities/KWStyle)
ENDIF(BatchMake_USE_KWSTYLE)

#-----------------------------------------------------------------------------
# Configure files with settings for use by the build.
CONFIGURE_FILE(${BatchMake_SOURCE_DIR}/bmConfigure.h.in 
               ${BatchMake_BINARY_DIR}/bmConfigure.h)

INSTALL(FILES ${BatchMake_BINARY_DIR}/bmConfigure.h 
  DESTINATION include/BatchMake)

INSTALL(FILES CMakeLists.txt Copyright.txt ReadMe.txt DartConfig.cmake
  DESTINATION source/BatchMake)

FILE(GLOB IMAGES Images/?*.*)
INSTALL(FILES ${IMAGES} DESTINATION source/BatchMake/Images)

#-----------------------------------------------------------------------------
# CPack options
INCLUDE(InstallRequiredSystemLibraries)

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "BatchMake")
SET(CPACK_PACKAGE_VENDOR "Kitware Inc.")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ReadMe.txt")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/Copyright.txt")
SET(CPACK_PACKAGE_VERSION_MAJOR ${BatchMake_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${BatchMake_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${BatchMake_VERSION_PATCH})
SET(CPACK_PACKAGE_INSTALL_DIRECTORY 
  "BatchMake ${BatchMake_EXTENDED_VERSION_STRING}")
IF(WIN32 AND NOT UNIX)
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
  SET(CPACK_PACKAGE_ICON 
    "${BatchMake_SOURCE_DIR}/Documentation/images\\\\InstallImage.bmp")
  SET(CPACK_NSIS_INSTALLED_ICON_NAME "relwithdebinfo\\\\BatchMake.exe")
  SET(CPACK_NSIS_DISPLAY_NAME 
    "${CPACK_PACKAGE_INSTALL_DIRECTORY} The Batch Processing Scripting Tool")
  SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\public.kitware.com/BatchMake")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\public.kitware.com/BatchMake")
  SET(CPACK_NSIS_MODIFY_PATH ON)
ELSE(WIN32 AND NOT UNIX)
  SET(CPACK_STRIP_FILES "bin/BatchMake")
  SET(CPACK_SOURCE_STRIP_FILES "")
ENDIF(WIN32 AND NOT UNIX)
SET(CPACK_PACKAGE_EXECUTABLES "BatchMake" "BatchMake")
INCLUDE(CPack)

#-----------------------------------------------------------------------------
# Export the configuration files
SUBDIRS(CMake/ExportConfiguration)

