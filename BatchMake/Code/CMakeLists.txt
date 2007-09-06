SET(BatchMakeLib_SRCS
  ${BatchMakeLib_SRCS}
  bmLaunch.cxx
  bmProgressManager.cxx
  bmScriptAction.cxx
  bmScriptActionManager.cxx
  bmScriptError.cxx
  bmScriptParser.cxx
  )

SET(BatchMakeLib_HDRS
  bmLaunch.h
  bmProgressManager.h
  bmScriptAction.h
  bmScriptActionManager.h
  bmScriptError.h
  bmScriptParser.h
  bmSplashScreenControls.h
  bmIcons.h
  bmScriptEchoAction.h
  bmScriptForEachAction.h
  bmScriptForNFoldAction.h
  bmScriptGetParamAction.h
  bmScriptIfAction.h
  bmScriptRandomizeAction.h
  bmScriptRunAction.h
  bmScriptSequenceAction.h
  bmScriptSetAction.h
  bmScriptSetAppAction.h
  bmScriptSetAppOptionAction.h
  bmScriptListFileInDirAction.h
  bmScriptListDirInDirAction.h
  bmScriptIncAction.h
  bmScriptIntAction.h
  bmScriptWriteFileAction.h
  bmScriptAppendFileAction.h
  bmScriptExtractSliceAction.h
  bmScriptExtractStringAction.h
  bmScriptIncludeAction.h
  bmScriptSinAction.h
  bmScriptDeleteFileAction.h
  bmScriptRegExAction.h
  bmScriptMakeDirectoryAction.h
  bmEditor.h
  bmSplashScreenControls.h
  ApplicationWrapper.h
  ApplicationWrapperParam.h
  bmScriptGetTimeAction.h
  bmScriptGetCurrentDateTimeAction.h
  bmScriptGlobAction.h
  bmScriptGetFilenameAction.h
  bmScriptConvertImageAction.h
  bmScriptFileExistsAction.h
  )

#Add Commands
SET(BatchMakeLib_SRCS
  ${BatchMakeLib_SRCS}
  bmScriptEchoAction.cxx
  bmScriptForEachAction.cxx
  bmScriptForNFoldAction.cxx
  bmScriptGetParamAction.cxx
  bmScriptIfAction.cxx
  bmScriptRandomizeAction.cxx
  bmScriptRunAction.cxx
  bmScriptSequenceAction.cxx
  bmScriptSetAction.cxx
  bmScriptSetAppAction.cxx
  bmScriptSetAppOptionAction.cxx
  bmScriptListFileInDirAction.cxx
  bmScriptListDirInDirAction.cxx
  bmScriptIncAction.cxx
  bmScriptIntAction.cxx
  bmScriptWriteFileAction.cxx
  bmScriptAppendFileAction.cxx
  bmScriptExtractSliceAction.cxx
  bmScriptExtractStringAction.cxx
  bmScriptIncludeAction.cxx
  bmScriptSinAction.cxx
  bmScriptDeleteFileAction.cxx
  bmScriptRegExAction.cxx
  bmScriptMakeDirectoryAction.cxx
  ApplicationWrapper.cxx
  ApplicationWrapperParam.cxx
  bmScriptGetTimeAction.cxx
  bmScriptGetCurrentDateTimeAction.cxx
  bmScriptGlobAction.cxx
  bmScriptGetFilenameAction.cxx
  bmScriptConvertImageAction.cxx
  bmScriptFileExistsAction.cxx
  )

#
# Grid support
#
IF(GRID_SUPPORT)
  SET(BatchMakeLib_SRCS
    ${BatchMakeLib_SRCS}
    Grid/bmGrid.cxx
    Grid/bmScriptGridSingleNodeAction.cxx
    Grid/bmScriptGridDataHostAction.cxx
    Grid/bmScriptGridOutputHostAction.cxx
    Grid/bmScriptOutputDirectoryAction.cxx
    Grid/bmScriptDataDirectoryAction.cxx
    Grid/bmScriptExtractSliceActionGrid.cxx
    Grid/bmScriptRunActionGrid.cxx
    Grid/bmScriptSetActionGrid.cxx
    Grid/bmScriptGridSetGroupingAction.cxx
    Grid/bmScriptGridMaxNodesAction.cxx
    Grid/bmScriptGridTempDirectoryAction.cxx
    Grid/bmScriptGridExecutableDirectoryAction.cxx  
    )

  IF(DASHBOARD_SUPPORT)
    SET(BatchMakeLib_SRCS
      ${BatchMakeLib_SRCS}
      Grid/bmScriptSetIdealOutputActionGrid.cxx
      Grid/bmScriptAddMethodIdealOutputActionGrid.cxx
      Grid/bmScriptCreateExperimentActionGrid.cxx
      Grid/bmScriptAddMethodOutputActionGrid.cxx
      Grid/bmScriptAddMethodInputActionGrid.cxx
      Grid/bmScriptDashboardSendActionGrid.cxx
      Grid/bmScriptCreateMethodActionGrid.cxx
      )
  ENDIF(DASHBOARD_SUPPORT)

ENDIF(GRID_SUPPORT)

#
# Dashboard support
#
IF(DASHBOARD_SUPPORT)
  SET(BatchMakeLib_SRCS
    ${BatchMakeLib_SRCS}   
    Dashboard/bmScriptDbSendValueAction.cxx
    Dashboard/bmScriptDbSendFileAction.cxx
    Dashboard/bmScriptDbClearAction.cxx
    Dashboard/bmScriptOpenTCPSocketAction.cxx
    Dashboard/bmScriptSendTCPAction.cxx
    Dashboard/bmScriptCloseTCPSocketAction.cxx
    Dashboard/bmScriptAddMethodInputAction.cxx
    Dashboard/bmScriptAddMethodIdealOutputAction.cxx
    Dashboard/bmScriptSetIdealOutputAction.cxx
    Dashboard/bmScriptDashboardHostAction.cxx
    Dashboard/bmScriptDashboardUserAction.cxx
    Dashboard/bmScriptDashboardKeyAction.cxx
    Dashboard/bmScriptDashboardSendAction.cxx
    Dashboard/bmScriptDashboardNotifyAction.cxx
    Dashboard/bmScriptCreateExperimentAction.cxx
    Dashboard/bmScriptCreateMethodAction.cxx
    Dashboard/bmScriptCreateDashboardAction.cxx 
    Dashboard/bmScriptCreateGraphAction.cxx
    Dashboard/bmScriptAddMethodOutputAction.cxx  
    Dashboard/bmScriptActionManagerDashboard.cxx
    Dashboard/bmScriptAddDashboardLabelAction.cxx
    Dashboard/bmDashboardSend.cxx
    )
ENDIF(DASHBOARD_SUPPORT)

IF(USE_FLTK)

  SET(BatchMakeLib_HDRS
    ${BatchMakeLib_HDRS}
    bmEditor.h
    bmScriptEditorGUIBase.h
    bmScriptEditorGUIControls.h
    bmProgressGUIBase.h
    bmProgressGUIControls.h
    bmProgressManagerGUI.h
    bmScriptErrorGUI.h
    bmSplashScreenControls.h
    bmScriptEditorGUIControls.h
    bmSplashScreenControls.h
    bmScriptErrorGUI.h
    bmProgressManagerGUI.h
    bmProgressGUIControls.h
    bmApplicationListGUIBase.h
    bmApplicationListGUIControls.h
    bmApplicationWrapperGUIBase.h
    bmApplicationWrapperGUIControls.h
    )
    
  SET(BatchMakeLib_SRCS
    ${BatchMakeLib_SRCS}
    bmScriptEditorGUIControls.cxx
    bmSplashScreenControls.cxx
    bmScriptErrorGUI.cxx
    bmProgressManagerGUI.cxx
    bmProgressGUIControls.cxx
    bmApplicationListGUIControls.cxx
    bmApplicationWrapperGUIControls.cxx
    bmEditor.cxx
    )
  
  SET(BatchMakeLib_GUI_SRCS
    bmProgressGUI.fl
    bmScriptEditorGUI.fl
    bmSplashScreen.fl
    bmApplicationListGUI.fl
    bmApplicationWrapperGUI.fl
    )

  SET(FLTK_WRAP_UI 1)
  FLTK_WRAP_UI( BatchMakeLib ${BatchMakeLib_GUI_SRCS} )

  ADD_LIBRARY(BatchMakeLib ${BatchMakeLib_SRCS} ${BatchMakeLib_FLTK_UI_SRCS})

  # Following is needed to ensure the existence of CondorWatcherGUI
  #  generated by fluid when BatchMakeUtilities is built
  ADD_DEPENDENCIES(BatchMakeLib BatchMakeUtilities)

  IF(FLTK_FOUND)
    IF(CYGWIN)
      TARGET_LINK_LIBRARIES(BatchMakeLib 
        ole32 uuid comctl32 wsock32 supc++ -lm -lgdi32
	)
    ENDIF(CYGWIN)
  ENDIF(FLTK_FOUND)

ELSE(USE_FLTK)

  ADD_LIBRARY(BatchMakeLib ${BatchMakeLib_SRCS})

ENDIF(USE_FLTK)
 
TARGET_LINK_LIBRARIES(BatchMakeLib BatchMakeUtilities)

IF(DASHBOARD_SUPPORT)
 IF(WIN32)
  SET(BatchMakeDashboard_LIBS ws2_32)
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

 TARGET_LINK_LIBRARIES(BatchMakeLib ${BatchMakeDashboard_LIBS})
ENDIF(DASHBOARD_SUPPORT)

# Installation file
INSTALL(TARGETS BatchMakeLib
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib/BatchMake
        ARCHIVE DESTINATION lib/BatchMake
        )
INSTALL(FILES ${BatchMake_HDRS}  ${BatchMakeLib_HDRS}  
        DESTINATION include/BatchMake/Code)
INSTALL(FILES CMakeLists.txt DESTINATION source/Code)
INSTALL(FILES ${BatchMakeLib_SRCS} DESTINATION source/Code)
IF(USE_FLTK)
  INSTALL(FILES ${BatchMakeLib_GUI_SRCS} DESTINATION source/Code)
ENDIF(USE_FLTK)

