FIND_PROGRAM(KWSTYLE_EXECUTABLE
  NAMES KWStyle 
  PATHS
  /usr/local/bin
  )
MARK_AS_ADVANCED(KWSTYLE_EXECUTABLE)
  
OPTION(KWSTYLE_USE_VIM_FORMAT 
  "Set KWStyle to generate errors in a VIM-compatible format." OFF)
IF(KWSTYLE_USE_VIM_FORMAT)
  SET(KWSTYLE_ARGUMENTS -vim ${KWSTYLE_ARGUMENTS})
ENDIF(KWSTYLE_USE_VIM_FORMAT)

OPTION(KWSTYLE_USE_MSVC_FORMAT 
  "Set KWStyle to generate errors in a VisualStudio-compatible format." OFF)
IF(KWSTYLE_USE_MSVC_FORMAT)
  SET(KWSTYLE_ARGUMENTS -msvc ${KWSTYLE_ARGUMENTS})
ENDIF(KWSTYLE_USE_MSVC_FORMAT)

ADD_CUSTOM_COMMAND(
  OUTPUT ${BatchMake_BINARY_DIR}/KWStyleReport.txt
  COMMAND ${KWSTYLE_EXECUTABLE}
  ARGS    -xml ${BatchMake_BINARY_DIR}/BatchMake.kws.xml -o ${BatchMake_SOURCE_DIR}/Utilities/KWStyle/BatchMakeOverwrite.txt ${KWSTYLE_ARGUMENTS} -D ${BatchMake_BINARY_DIR}/BatchMakeFiles.txt  
  COMMENT "Coding Style Checker"
  )
ADD_CUSTOM_TARGET(
  StyleCheck DEPENDS ${BatchMake_BINARY_DIR}/KWStyleReport.txt)

CONFIGURE_FILE(
  ${BatchMake_SOURCE_DIR}/Utilities/KWStyle/BatchMake.kws.xml.in
  ${BatchMake_BINARY_DIR}/BatchMake.kws.xml)

CONFIGURE_FILE(
  ${BatchMake_SOURCE_DIR}/Utilities/KWStyle/BatchMakeFiles.txt.in
  ${BatchMake_BINARY_DIR}/BatchMakeFiles.txt)
