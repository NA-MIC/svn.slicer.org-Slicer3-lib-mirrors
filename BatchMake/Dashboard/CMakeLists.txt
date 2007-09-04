
#------------------------------------------------------------------------
# Sources

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

#------------------------------------------------------------------------
# Depends

IF(WIN32)

  SET(BatchMakeDashboard_LIBS ws2_32 )

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

#------------------------------------------------------------------------
# Build BatchBoard library

ADD_LIBRARY(BatchMakeDashboard ${BatchMakeDashboard_SRCS})

TARGET_LINK_LIBRARIES(BatchMakeDashboard ${BatchMakeDashboard_LIBS})

#------------------------------------------------------------------------
# Installation file

INSTALL(TARGETS BatchMakeDashboard 
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib/BatchMake
        ARCHIVE DESTINATION lib/BatchMake
        )
INSTALL(FILES ${BatchMakeDashboard_HDRS} 
        DESTINATION include/BatchMake/Dashboard)
INSTALL(FILES ${BatchMakeDashboard_SRCS} 
        DESTINATION source/Dashboard)
INSTALL(FILES CMakeLists.txt DESTINATION source/Dashboard)
