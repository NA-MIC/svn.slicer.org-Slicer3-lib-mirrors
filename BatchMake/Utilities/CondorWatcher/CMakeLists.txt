PROJECT(CondorWatcher)

INCLUDE_DIRECTORIES(
  ${CondorWatcher_SOURCE_DIR}
  ${CondorWatcher_BINARY_DIR}
  )

SET(CondorWatcher_HDRS
  CondorWatcher.h
  )

SET(CondorWatcher_SRCS
  CondorWatcher.cxx
  )

SET(CondorWatcherGUI_SRCS
  CondorWatcherGUI.fl
  )

LINK_LIBRARIES(
  ${FLTK_LIBRARIES}
  FLU
  )

SET(FLTK_WRAP_UI 1)
FLTK_WRAP_UI( CondorWatcher  ${CondorWatcherGUI_SRCS} )

ADD_LIBRARY( CondorWatcher ${CondorWatcher_SRCS} ${CondorWatcher_FLTK_UI_SRCS})

ADD_EXECUTABLE( CondorWatcherTool main.cxx)
TARGET_LINK_LIBRARIES(CondorWatcherTool CondorWatcher)
UTILITY_SOURCE(BatchMake_CONDOR_WATCHER_TOOL_EXE CondorWatcherTool
               "" main.cxx)
MARK_AS_ADVANCED(BatchMake_CONDOR_WATCHER_TOOL_EXE)

#MACRO
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

MOVE_EXECUTABLE_TOP_BINARY(CondorWatcherTool BatchMake_CONDOR_WATCHER_TOOL_EXE)

# Installation file
INSTALL(TARGETS CondorWatcherTool CondorWatcher  
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib/BatchMake
        ARCHIVE DESTINATION lib/BatchMake)
INSTALL(FILES ${CondorWatcher_HDRS} 
        DESTINATION include/BatchMake/Utilities/CondorWatcher)
INSTALL(FILES ${CondorWatcher_SRCS} ${CondorWatcherGUI_SRCS} main.cxx 
        DESTINATION source/Utilities/CondorWatcher)
INSTALL(FILES CMakeLists.txt DESTINATION source/Utilities/CondorWatcher)
