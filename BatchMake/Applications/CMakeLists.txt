
#------------------------------------------------------------------------
# Create BatchMake application

SET(BatchMake_SRCS BatchMake.cxx)
IF(WIN32)
  IF(NOT CYGWIN)
    IF(NOT MINGW)
      SET(BatchMake_SRCS ${BatchMake_SRCS} BatchMake.rc)
    ENDIF(NOT MINGW)
  ENDIF(NOT CYGWIN)

 OPTION(HIDE_CONSOLE "Hide the console" OFF)
 IF(HIDE_CONSOLE)
   SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /subsystem:windows")
 ENDIF(HIDE_CONSOLE)
ENDIF(WIN32)


ADD_EXECUTABLE(BatchMake BatchMake.cxx)

TARGET_LINK_LIBRARIES(BatchMake ${BatchMake_LIBRARIES})

ADD_DEPENDENCIES(BatchMake ${BatchMake_LIBRARIES})

#------------------------------------------------------------------------
# Create the dashboard send application

ADD_EXECUTABLE(bmGridSend bmGridSend.cxx)
TARGET_LINK_LIBRARIES(bmGridSend ${BatchMake_LIBRARIES})
UTILITY_SOURCE(BatchMake_GRID_SEND_EXE bmGridSend "" bmGridSend.cxx)
MARK_AS_ADVANCED(BatchMake_GRID_SEND_EXE)

#------------------------------------------------------------------------
# Create the dashboard send application

ADD_EXECUTABLE(bmGridStore bmGridStore.cxx)
TARGET_LINK_LIBRARIES(bmGridStore ${BatchMake_LIBRARIES})
UTILITY_SOURCE(BatchMake_GRID_STORE_EXE bmGridStore "" bmGridStore.cxx)
MARK_AS_ADVANCED(BatchMake_GRID_STORE_EXE)

#------------------------------------------------------------------------
# Create the dashboard send application

ADD_EXECUTABLE(bmSliceExtractor bmSliceExtractor.cxx)
TARGET_LINK_LIBRARIES(bmSliceExtractor ${BatchMake_LIBRARIES})
UTILITY_SOURCE(BatchMake_SLICE_EXTRACTOR_EXE bmSliceExtractor
               "" bmSliceExtractor.cxx)
MARK_AS_ADVANCED(BatchMake_SLICE_EXTRACTOR_EXE)

#------------------------------------------------------------------------
#  Check Memory

ADD_EXECUTABLE(bmCheckMemory CheckMemory.cxx)
TARGET_LINK_LIBRARIES(bmCheckMemory ${BatchMake_LIBRARIES})

#------------------------------------------------------------------------
#  Get CPU Info

ADD_EXECUTABLE(bmGetCPUInfo GetCPUInfo.cxx)
TARGET_LINK_LIBRARIES(bmGetCPUInfo ${BatchMake_LIBRARIES})
UTILITY_SOURCE(BatchMake_GET_CPU_INFO_EXE bmGetCPUInfo
               "" GetCPUInfo.cxx)
               MARK_AS_ADVANCED(BatchMake_GET_CPU_INFO_EXE)

#------------------------------------------------------------------------
#  Get Memory Info

ADD_EXECUTABLE(bmGetMemoryInfo GetMemoryInfo.cxx)
TARGET_LINK_LIBRARIES(bmGetMemoryInfo ${BatchMake_LIBRARIES})
UTILITY_SOURCE(BatchMake_GET_MEMORY_INFO_EXE bmGetMemoryInfo
               "" GetMemoryInfo.cxx)
               MARK_AS_ADVANCED(BatchMake_GET_MEMORY_INFO_EXE)

#------------------------------------------------------------------------
#  Get CPU Speed

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

#------------------------------------------------------------------------
# Move apps to where CMake can find them

MACRO(MOVE_EXECUTABLE_TOP_BINARY NAME CACHE_VAR)
  GET_TARGET_PROPERTY(EXECUTABLE_PATH ${NAME} LOCATION)
  GET_FILENAME_COMPONENT(EXECUTABLE_NAME ${EXECUTABLE_PATH} NAME)

  ADD_CUSTOM_COMMAND(
    TARGET ${NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
       -E copy ${EXECUTABLE_PATH} ${BatchMake_BINARY_DIR}/bin/${EXECUTABLE_NAME}    )
  SET(${CACHE_VAR} ${BatchMake_BINARY_DIR}/bin/${EXECUTABLE_NAME}
    CACHE FILEPATH "" FORCE
    )
ENDMACRO(MOVE_EXECUTABLE_TOP_BINARY NAME)

MOVE_EXECUTABLE_TOP_BINARY(bmGridSend BatchMake_GRID_SEND_EXE )
MOVE_EXECUTABLE_TOP_BINARY(bmGridStore BatchMake_GRID_STORE_EXE )
MOVE_EXECUTABLE_TOP_BINARY(bmSliceExtractor BatchMake_SLICE_EXTRACTOR_EXE )
MOVE_EXECUTABLE_TOP_BINARY(bmGetMemoryInfo BatchMake_GET_MEMORY_INFO_EXE )
MOVE_EXECUTABLE_TOP_BINARY(bmGetCPUSpeed BatchMake_GET_CPU_SPEED_EXE )
MOVE_EXECUTABLE_TOP_BINARY(bmGetCPUInfo BatchMake_GET_CPU_INFO_EXE )

#------------------------------------------------------------------------
# Installation file

INSTALL(TARGETS BatchMake bmGetCPUInfo bmGetMemoryInfo bmGetCPUSpeed
        bmSliceExtractor bmGridStore bmGridSend bmCheckMemory
        LIBRARY DESTINATION lib/BatchMake
        ARCHIVE DESTINATION lib/BatchMake
        RUNTIME DESTINATION bin
        )
INSTALL(FILES BatchMake.cxx BatchMake.ico BatchMake.rc Whetstone.h
        GetCPUInfo.cxx GetMemoryInfo.cxx GetCPUSpeed.cxx Whetstone.cxx
        bmGridSend.cxx bmGridStore.cxx bmSliceExtractor.cxx CheckMemory.cxx
        DESTINATION source/BatchMake/Applications
        )
INSTALL(FILES CMakeLists.txt 
        DESTINATION source/BatchMake/Applications
        )

