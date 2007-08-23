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
)

LINK_LIBRARIES(
 BatchMakeUtilities
)

IF(USE_FLTK)

SET(BatchMakeLib_HDRS
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

  LINK_LIBRARIES(${FLTK_LIBRARIES})

  SET(FLTK_WRAP_UI 1)
  FLTK_WRAP_UI( BatchMakeLib ${BatchMakeLib_GUI_SRCS} )
ENDIF(USE_FLTK)


ADD_LIBRARY(BatchMakeLib ${BatchMakeLib_SRCS} ${BatchMakeLib_FLTK_UI_SRCS})
IF(USE_FLTK)
  ADD_DEPENDENCIES(BatchMakeLib FLU)
ENDIF(USE_FLTK)

IF(GRID_SUPPORT)
  TARGET_LINK_LIBRARIES(BatchMakeLib BatchMakeGrid)
  IF(USE_FLTK)
    TARGET_LINK_LIBRARIES(BatchMakeLib CondorWatcher)
    ADD_DEPENDENCIES(BatchMakeLib CondorWatcher)
  ENDIF(USE_FLTK)
ENDIF(GRID_SUPPORT)

IF(DASHBOARD_SUPPORT)
  TARGET_LINK_LIBRARIES(BatchMakeLib BatchMakeDashboard)
ENDIF(DASHBOARD_SUPPORT)

SET(BatchMake_SRCS
BatchMake.cxx
)

IF(WIN32)
  IF(NOT CYGWIN)
    IF(NOT MINGW)
    SET(BatchMake_SRCS
      ${BatchMake_SRCS}
      BatchMake.rc)
    ENDIF(NOT MINGW)
  ENDIF(NOT CYGWIN)
ENDIF(WIN32)

SET(BATCHMAKE_LIBS
 ${BATCHMAKE_LIBS}
 BatchMakeLib
 BatchMakeUtilities
)

IF(USE_FLTK)
  SET(BATCHMAKE_LIBS
     ${BATCHMAKE_LIBS}
     FLU)
ENDIF(USE_FLTK)

# Add an option to hide the console
OPTION(HIDE_CONSOLE "Hide the console" OFF)
IF(HIDE_CONSOLE)
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /subsystem:windows") 
ENDIF(HIDE_CONSOLE)
ADD_EXECUTABLE(BatchMake ${BatchMake_SRCS})
TARGET_LINK_LIBRARIES(BatchMake ${BATCHMAKE_LIBS})

# Installation file
INSTALL(TARGETS BatchMake BatchMakeLib
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib/BatchMake
        ARCHIVE DESTINATION lib/BatchMake
        )
INSTALL(FILES ${BatchMake_HDRS}  ${BatchMakeLib_HDRS}  DESTINATION include/BatchMake/Code)
INSTALL(FILES CMakeLists.txt DESTINATION source/Code)
INSTALL(FILES BatchMake.ico DESTINATION source/Code)
INSTALL(FILES ${BatchMake_SRCS} ${BatchMakeLib_SRCS} ${BatchMakeLib_GUI_SRCS} DESTINATION source/Code)
