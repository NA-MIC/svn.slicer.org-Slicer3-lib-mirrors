IF(USE_FLTK)
  SUBDIRS(Flu)
  IF(GRID_SUPPORT)
    SUBDIRS(CondorWatcher)
  ENDIF(GRID_SUPPORT)
ENDIF(USE_FLTK)

SUBDIRS(Zip)

SET(BatchMakeUtilities_HDRS
  MString.h
  Timer.h
  XMLReader.h
  XMLWriter.h
  XMLIniIO.h
  TCPSocket.h
  XMLBufferReader.h
  SliceExtractor.h
  SystemInfo.h
  Whetstone.h
  )


SET(BatchMakeUtilities_SRCS
  MString.cxx
  Timer.cxx
  XMLReader.cxx
  XMLWriter.cxx
  XMLIniIO.cxx
  TCPSocket.cxx
  XMLBufferReader.cxx
  SliceExtractor.cxx
  SystemInfo.cxx
  )

ADD_LIBRARY(BatchMakeUtilities ${BatchMakeUtilities_SRCS})
TARGET_LINK_LIBRARIES(BatchMakeUtilities ITKCommon)

ADD_EXECUTABLE(bmCheckMemory CheckMemory.cxx)
TARGET_LINK_LIBRARIES(bmCheckMemory BatchMakeUtilities)


ADD_EXECUTABLE(bmGetCPUInfo GetCPUInfo.cxx)
TARGET_LINK_LIBRARIES(bmGetCPUInfo BatchMakeUtilities)
UTILITY_SOURCE(BatchMake_GET_CPU_INFO_EXE bmGetCPUInfo 
               "" GetCPUInfo.cxx)
MARK_AS_ADVANCED(BatchMake_GET_CPU_INFO_EXE)

ADD_EXECUTABLE(bmGetMemoryInfo GetMemoryInfo.cxx)
TARGET_LINK_LIBRARIES(bmGetMemoryInfo BatchMakeUtilities)
UTILITY_SOURCE(BatchMake_GET_MEMORY_INFO_EXE bmGetMemoryInfo 
               "" GetMemoryInfo.cxx)
MARK_AS_ADVANCED(BatchMake_GET_MEMORY_INFO_EXE)

ADD_EXECUTABLE(bmGetCPUSpeed GetCPUSpeed.cxx Whetstone.cxx)
IF(CMAKE_COMPILER_IS_GNUCXX)
  SET_SOURCE_FILES_PROPERTIES(Whetstone.cxx PROPERTIES COMPILE_FLAGS -O0)
ELSE(CMAKE_COMPILER_IS_GNUCXX)
  IF(WIN32)
    SET_SOURCE_FILES_PROPERTIES(Whetstone.cxx PROPERTIES COMPILE_FLAGS -Od)
  ENDIF(WIN32)
ENDIF(CMAKE_COMPILER_IS_GNUCXX)
UTILITY_SOURCE(BatchMake_GET_CPU_SPEED_EXE bmGetCPUSpeed 
               "" GetCPUSpeed.cxx)
MARK_AS_ADVANCED(BatchMake_GET_CPU_SPEED_EXE)

# Move the executable to the top binary tree
MACRO(MOVE_EXECUTABLE_TOP_BINARY NAME CACHE_VAR)
  GET_TARGET_PROPERTY(EXECUTABLE_PATH ${NAME} LOCATION)
  GET_FILENAME_COMPONENT(EXECUTABLE_NAME ${EXECUTABLE_PATH} NAME)

  ADD_CUSTOM_COMMAND(
    TARGET ${NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} 
       -E copy ${EXECUTABLE_PATH} ${BatchMake_BINARY_DIR}/bin/${EXECUTABLE_NAME}
    )
  SET(${CACHE_VAR} ${BatchMake_BINARY_DIR}/bin/${EXECUTABLE_NAME} 
      CACHE FILEPATH "" FORCE)
ENDMACRO(MOVE_EXECUTABLE_TOP_BINARY NAME)

MOVE_EXECUTABLE_TOP_BINARY(bmGetMemoryInfo BatchMake_GET_MEMORY_INFO_EXE )
MOVE_EXECUTABLE_TOP_BINARY(bmGetCPUSpeed BatchMake_GET_CPU_SPEED_EXE)
MOVE_EXECUTABLE_TOP_BINARY(bmGetCPUInfo BatchMake_GET_CPU_INFO_EXE)

# Installation file
INSTALL(TARGETS BatchMakeUtilities bmGetCPUInfo bmGetMemoryInfo bmGetCPUSpeed
        ARCHIVE DESTINATION lib/BatchMake
        LIBRARY DESTINATION lib/BatchMake
        RUNTIME DESTINATION bin)
INSTALL(FILES ${BatchMakeUtilities_HDRS} 
        DESTINATION include/BatchMake/Utilities)
INSTALL(FILES ${BatchMakeUtilities_SRCS} 
        DESTINATION source/Utilities)
INSTALL(FILES CMakeLists.txt 
        DESTINATION source/Utilities)
