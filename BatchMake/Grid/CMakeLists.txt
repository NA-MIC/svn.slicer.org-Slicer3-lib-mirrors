#------------------------------------------------------------------
# Source

SET(BatchMakeGrid_HDRS
  bmGrid.h
  bmScriptDataDirectoryAction.h
  bmScriptGridDataHostAction.h
  bmScriptGridOutputHostAction.h
  bmScriptGridSingleNodeAction.h
  bmScriptOutputDirectoryAction.h
  bmScriptGridSetGroupingAction.h
  bmScriptGridMaxNodesAction.h
  bmScriptGridTempDirectoryAction.h
  bmScriptGridExecutableDirectoryAction.h
  )

SET(BatchMakeGrid_SRCS
  bmGrid.cxx
  bmScriptGridSingleNodeAction.cxx
  bmScriptGridDataHostAction.cxx
  bmScriptGridOutputHostAction.cxx
  bmScriptOutputDirectoryAction.cxx
  bmScriptDataDirectoryAction.cxx
  bmScriptExtractSliceActionGrid.cxx
  bmScriptRunActionGrid.cxx
  bmScriptSetActionGrid.cxx
  bmScriptGridSetGroupingAction.cxx
  bmScriptGridMaxNodesAction.cxx
  bmScriptGridTempDirectoryAction.cxx
  bmScriptGridExecutableDirectoryAction.cxx
  )

IF(DASHBOARD_SUPPORT)
  SET(BatchMakeGrid_SRCS
    ${BatchMakeGrid_SRCS}
    bmScriptSetIdealOutputActionGrid.cxx
    bmScriptAddMethodIdealOutputActionGrid.cxx
    bmScriptCreateExperimentActionGrid.cxx
    bmScriptAddMethodOutputActionGrid.cxx
    bmScriptAddMethodInputActionGrid.cxx
    bmScriptDashboardSendActionGrid.cxx
    bmScriptCreateMethodActionGrid.cxx
    )
ENDIF(DASHBOARD_SUPPORT)


#------------------------------------------------------------------
# Build BatchMakeGrid library

ADD_LIBRARY(BatchMakeGrid ${BatchMakeGrid_SRCS})


#------------------------------------------------------------------
# Installation file

INSTALL(TARGETS BatchMakeGrid 
        LIBRARY DESTINATION lib/BatchMake
        ARCHIVE DESTINATION lib/BatchMake)
INSTALL(FILES ${BatchMakeGrid_HDRS} DESTINATION include/BatchMake/Grid)
INSTALL(FILES CMakeLists.txt DESTINATION source/Grid)
INSTALL(FILES ${BatchMakeGrid_SRCS} DESTINATION source/Grid)
