MACRO(TRY_COMPILE_FROM_SOURCE SOURCE VAR)
  IF("${VAR}" MATCHES "^${VAR}$" OR "${VAR}" MATCHES "UNKNOWN")
    SET(MACRO_CHECK_FUNCTION_DEFINITIONS
      "-D${VAR} ${CMAKE_REQUIRED_FLAGS}")
    IF(CMAKE_REQUIRED_LIBRARIES)
      SET(TRY_COMPILE_FROM_SOURCE_ADD_LIBRARIES
        "-DLINK_LIBRARIES:STRING=${CMAKE_REQUIRED_LIBRARIES}")
    ENDIF(CMAKE_REQUIRED_LIBRARIES)
    SET(src "")
    FOREACH(def ${EXTRA_DEFINES})
      SET(src "${src}#define ${def} 1\n")
    ENDFOREACH(def)
    FOREACH(inc ${HEADER_INCLUDES})
      SET(src "${src}#include <${inc}>\n")
    ENDFOREACH(inc)

    SET(src "${src}\nint main() { ${SOURCE} ; return 0; }")
    FILE(WRITE "${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp/src2.c"
      "${src}")
    EXEC_PROGRAM("${CMAKE_COMMAND}" "${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp"
      ARGS -E copy src2.c src.c)
    MESSAGE(STATUS "Performing Test ${VAR}")
    TRY_COMPILE(${VAR}
      ${CMAKE_BINARY_DIR}
      ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp/src.c
      CMAKE_FLAGS
      "${TRY_COMPILE_FROM_SOURCE_ADD_LIBRARIES}"
      OUTPUT_VARIABLE OUTPUT)
    IF(${VAR})
      SET(${VAR} 1 CACHE INTERNAL "Test ${FUNCTION}")
      MESSAGE(STATUS "Performing Test ${VAR} - Success")
      FILE(WRITE ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeOutput.log
        "Performing C SOURCE FILE Test ${VAR} succeded with the following output:\n"
        "${OUTPUT}\n"
        "Source file was:\n${src}\n" APPEND)
    ELSE(${VAR})
      MESSAGE(STATUS "Performing Test ${VAR} - Failed")
      SET(${VAR} "" CACHE INTERNAL "Test ${FUNCTION}")
      FILE(WRITE ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeError.log
        "Performing C SOURCE FILE Test ${VAR} failed with the following output:\n"
        "${OUTPUT}\n"
        "Source file was:\n${src}\n" APPEND)
    ENDIF(${VAR})
  ENDIF("${VAR}" MATCHES "^${VAR}$" OR "${VAR}" MATCHES "UNKNOWN")
ENDMACRO(TRY_COMPILE_FROM_SOURCE)
