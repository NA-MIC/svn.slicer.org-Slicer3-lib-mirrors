# Generate the BatchMakeConfig.cmake file in the build tree. Also configure
# one for installation. The file tells external projects how to use
# BatchMake.

# Help store a literal dollar in a string.  CMake 2.2 allows escaped
# dollars but we have to support CMake 2.0.
SET(DOLLAR "$")

#-----------------------------------------------------------------------------
# Settings shared between the build tree and install tree.

EXPORT_LIBRARY_DEPENDENCIES(
  ${BatchMake_BINARY_DIR}/BatchMakeLibraryDepends.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/CMakeExportBuildSettings.cmake)
CMAKE_EXPORT_BUILD_SETTINGS(
  ${BatchMake_BINARY_DIR}/BatchMakeBuildSettings.cmake)

#-----------------------------------------------------------------------------
# Settings specific to the build tree.

# The install-only section is empty for the build tree.
SET(BatchMake_CONFIG_INSTALL_ONLY)

# The "use" file.
SET(BatchMake_USE_FILE_CONFIG 
  ${CMAKE_CURRENT_SOURCE_DIR}/UseBatchMake.cmake)

# The build settings file.
SET(BatchMake_BUILD_SETTINGS_FILE_CONFIG 
  ${BatchMake_BINARY_DIR}/BatchMakeBuildSettings.cmake)

# The library directories.
SET(BatchMake_LIBRARY_DIRS_CONFIG ${BatchMake_LIBRARY_DIR})

# The runtime directories.
SET(BatchMake_RUNTIME_DIRS_CONFIG ${BatchMake_RUNTIME_DIR})

SET(BatchMake_APPLICATIONS_DIRS_CONFIG ${BatchMake_APPLICATIONS_DIR})

# The include directories.
SET(BatchMake_INCLUDE_DIRS_CONFIG ${BatchMake_INCLUDE_PATH})

# The library dependencies file.
SET(BatchMake_LIBRARY_DEPENDS_FILE 
  ${BatchMake_BINARY_DIR}/BatchMakeLibraryDepends.cmake)

# The CMake macros dir.
SET(BatchMake_CMAKE_DIR_CONFIG ${BatchMake_CMAKE_DIR})

# The ITK options.
SET(BatchMake_ITK_DIR_CONFIG ${ITK_DIR})

# The FLTK options.
SET(BatchMake_FLTK_DIR_CONFIG ${FLTK_DIR})

# Utilities
SET(BatchMake_GET_CPU_INFO_EXE_CONFIG ${BatchMake_GET_CPU_INFO_EXE})
SET(BatchMake_GET_MEMORY_INFO_EXE_CONFIG ${BatchMake_GET_MEMORY_INFO_EXE})
SET(BatchMake_GET_CPU_SPEED_EXE_CONFIG ${BatchMake_GET_CPU_SPEED_EXE})
SET(BatchMake_CONDOR_WATCHER_TOOL_EXE_CONFIG 
    ${BatchMake_CONDOR_WATCHER_TOOL_EXE})
SET(BatchMake_GRID_SEND_EXE_CONFIG ${BatchMake_GRID_SEND_EXE})
SET(BatchMake_GRID_STORE_EXE_CONFIG ${BatchMake_GRID_STORE_EXE})
SET(BatchMake_SLICE_EXTRACTOR_EXE_CONFIG ${BatchMake_SLICE_EXTRACTOR_EXE})

# The build configuration information.
SET(BatchMake_CONFIGURATION_TYPES_CONFIG ${CMAKE_CONFIGURATION_TYPES})
SET(BatchMake_BUILD_TYPE_CONFIG ${CMAKE_BUILD_TYPE})

# Configure BatchMakeConfig.cmake for the build tree.
CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/BatchMakeConfig.cmake.in
  ${BatchMake_BINARY_DIR}/BatchMakeConfig.cmake @ONLY IMMEDIATE)

#-----------------------------------------------------------------------------
# Settings specific to the install tree.
# Get back to Seb and Brad for this one, once you specify install targets
# in your CMakeLists.txt file
