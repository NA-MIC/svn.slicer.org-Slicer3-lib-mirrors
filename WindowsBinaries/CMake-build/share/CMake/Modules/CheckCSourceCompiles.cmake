# - Check if the source code provided in the SOURCE argument compiles.
# CHECK_C_SOURCE_COMPILES(SOURCE VAR)
# - macro which checks if the source code compiles
#  SOURCE   - source code to try to compile
#  VAR - variable to store size if the type exists.
#
# The following variables may be set before calling this macro to
# modify the way the check is run:
#
#  CMAKE_REQUIRED_FLAGS = string of compile command line flags
#  CMAKE_REQUIRED_DEFINITIONS = list of macros to define (-DFOO=bar)
#  CMAKE_REQUIRED_INCLUDES = list of include directories
#  CMAKE_REQUIRED_LIBRARIES = list of libraries to link

MACRO(CHECK_C_SOURCE_COMPILES SOURCE VAR)
  IF("${VAR}" MATCHES "^${VAR}$")
    SET(MACRO_CHECK_FUNCTION_DEFINITIONS 
      "-D${VAR} ${CMAKE_REQUIRED_FLAGS}")
    IF(CMAKE_REQUIRED_LIBRARIES)
      SET(CHECK_C_SOURCE_COMPILES_ADD_LIBRARIES
        "-DLINK_LIBRARIES:STRING=${CMAKE_REQUIRED_LIBRARIES}")
    ELSE(CMAKE_REQUIRED_LIBRARIES)
      SET(CHECK_C_SOURCE_COMPILES_ADD_LIBRARIES)
    ENDIF(CMAKE_REQUIRED_LIBRARIES)
    IF(CMAKE_REQUIRED_INCLUDES)
      SET(CHECK_C_SOURCE_COMPILES_ADD_INCLUDES
        "-DINCLUDE_DIRECTORIES:STRING=${CMAKE_REQUIRED_INCLUDES}")
    ELSE(CMAKE_REQUIRED_INCLUDES)
      SET(CHECK_C_SOURCE_COMPILES_ADD_INCLUDES)
    ENDIF(CMAKE_REQUIRED_INCLUDES)
    FILE(WRITE "${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp/src.c"
      "${SOURCE}")

    MESSAGE(STATUS "Performing Test ${VAR}")
    TRY_COMPILE(${VAR}
      ${CMAKE_BINARY_DIR}
      ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp/src.c
      COMPILE_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS}
      CMAKE_FLAGS -DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_FUNCTION_DEFINITIONS}
      "${CHECK_C_SOURCE_COMPILES_ADD_LIBRARIES}"
      "${CHECK_C_SOURCE_COMPILES_ADD_INCLUDES}"
      OUTPUT_VARIABLE OUTPUT)
    IF(${VAR})
      SET(${VAR} 1 CACHE INTERNAL "Test ${FUNCTION}")
      MESSAGE(STATUS "Performing Test ${VAR} - Success")
      FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log 
        "Performing C SOURCE FILE Test ${VAR} succeded with the following output:\n"
        "${OUTPUT}\n"
        "Source file was:\n${SOURCE}\n")
    ELSE(${VAR})
      MESSAGE(STATUS "Performing Test ${VAR} - Failed")
      SET(${VAR} "" CACHE INTERNAL "Test ${FUNCTION}")
      FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeError.log 
        "Performing C SOURCE FILE Test ${VAR} failed with the following output:\n"
        "${OUTPUT}\n"
        "Source file was:\n${SOURCE}\n")
    ENDIF(${VAR})
  ENDIF("${VAR}" MATCHES "^${VAR}$")
ENDMACRO(CHECK_C_SOURCE_COMPILES)

