SET(BatchMakeDashboard_HDRS
  HttpRequest.h
  bmScriptDbSendValueAction.h
  bmScriptDbSendFileAction.h
  bmScriptDbClearAction.h
  bmScriptOpenTCPSocketAction.h
  bmScriptSendTCPAction.h
  bmScriptCloseTCPSocketAction.h
  bmScriptAddMethodInputAction.h
  bmScriptAddMethodIdealOutputAction.h
  bmScriptSetIdealOutputAction.h
  bmScriptDashboardHostAction.h
  bmScriptDashboardUserAction.h
  bmScriptDashboardKeyAction.h
  bmScriptDashboardSendAction.h
  bmScriptDashboardNotifyAction.h
  bmScriptCreateExperimentAction.h
  bmScriptCreateMethodAction.h  
  bmScriptCreateDashboardAction.h  
  bmScriptCreateGraphAction.h  
  bmScriptAddMethodOutputAction.h  
  bmScriptAddDashboardLabelAction.h
  bmDashboardSend.h
)

SET(BatchMakeDashboard_SRCS
  HttpRequest.cxx
  bmScriptDbSendValueAction.cxx
  bmScriptDbSendFileAction.cxx
  bmScriptDbClearAction.cxx
  bmScriptOpenTCPSocketAction.cxx
  bmScriptSendTCPAction.cxx
  bmScriptCloseTCPSocketAction.cxx
  bmScriptAddMethodInputAction.cxx
  bmScriptAddMethodIdealOutputAction.cxx
  bmScriptSetIdealOutputAction.cxx
  bmScriptDashboardHostAction.cxx
  bmScriptDashboardUserAction.cxx
  bmScriptDashboardKeyAction.cxx
  bmScriptDashboardSendAction.cxx
  bmScriptDashboardNotifyAction.cxx
  bmScriptCreateExperimentAction.cxx
  bmScriptCreateMethodAction.cxx
  bmScriptCreateDashboardAction.cxx 
  bmScriptCreateGraphAction.cxx
  bmScriptAddMethodOutputAction.cxx  
  bmScriptActionManagerDashboard.cxx
  bmScriptAddDashboardLabelAction.cxx
  bmDashboardSend.cxx
)

IF(WIN32)
SET(BatchMakeDashboard_LIBS
 ws2_32
)

ELSE(WIN32)
  INCLUDE(${CMAKE_ROOT}/Modules/CheckLibraryExists.cmake)
  CHECK_LIBRARY_EXISTS("c" gethostbyname "" NOT_NEED_LIBNSL)
  IF(NOT NOT_NEED_LIBNSL)
    CHECK_LIBRARY_EXISTS("nsl" gethostbyname "" HAVE_LIBNSL)
  ENDIF(NOT NOT_NEED_LIBNSL)
  IF(HAVE_LIBNSL)
    SET(BatchMakeDashboard_LIBS
      #socket
     nsl
    )
  ENDIF(HAVE_LIBNSL)
ENDIF(WIN32)

ADD_LIBRARY(BatchMakeDashboard ${BatchMakeDashboard_SRCS})
TARGET_LINK_LIBRARIES(BatchMakeDashboard ${BatchMakeDashboard_LIBS} BatchMakeLib)

IF(GRID_SUPPORT)
  TARGET_LINK_LIBRARIES(BatchMakeDashboard BatchMakeGrid)
ENDIF(GRID_SUPPORT)

# Create the dashboard send application
ADD_EXECUTABLE(bmGridSend bmGridSend.cxx)
TARGET_LINK_LIBRARIES(bmGridSend BatchMakeDashboard BatchMakeUtilities)
UTILITY_SOURCE(BatchMake_GRID_SEND_EXE bmGridSend "" bmGridSend.cxx) 
MARK_AS_ADVANCED(BatchMake_GRID_SEND_EXE)

ADD_EXECUTABLE(bmGridStore bmGridStore.cxx)
TARGET_LINK_LIBRARIES(bmGridStore BatchMakeDashboard BatchMakeUtilities)
UTILITY_SOURCE(BatchMake_GRID_STORE_EXE bmGridStore "" bmGridStore.cxx) 
MARK_AS_ADVANCED(BatchMake_GRID_STORE_EXE)

ADD_EXECUTABLE(bmSliceExtractor bmSliceExtractor.cxx)
TARGET_LINK_LIBRARIES(bmSliceExtractor BatchMakeDashboard BatchMakeUtilities)
UTILITY_SOURCE(BatchMake_SLICE_EXTRACTOR_EXE bmSliceExtractor 
               "" bmSliceExtractor.cxx) 
MARK_AS_ADVANCED(BatchMake_SLICE_EXTRACTOR_EXE)

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

MOVE_EXECUTABLE_TOP_BINARY(bmGridSend BatchMake_GRID_SEND_EXE )
MOVE_EXECUTABLE_TOP_BINARY(bmGridStore BatchMake_GRID_STORE_EXE )
MOVE_EXECUTABLE_TOP_BINARY(bmSliceExtractor BatchMake_SLICE_EXTRACTOR_EXE )

# Installation file
INSTALL(TARGETS BatchMakeDashboard bmSliceExtractor bmGridStore bmGridSend
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib/BatchMake
        ARCHIVE DESTINATION lib/BatchMake
        )
INSTALL(FILES ${BatchMakeDashboard_HDRS} 
        DESTINATION include/BatchMake/Dashboard)
INSTALL(FILES ${BatchMakeDashboard_SRCS} 
        bmGridSend.cxx bmGridStore.cxx bmSliceExtractor.cxx 
        DESTINATION source/Dashboard)
INSTALL(FILES CMakeLists.txt DESTINATION source/Dashboard)
