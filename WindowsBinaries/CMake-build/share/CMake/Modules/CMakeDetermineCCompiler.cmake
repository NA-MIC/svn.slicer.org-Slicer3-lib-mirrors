
# determine the compiler to use for C programs
# NOTE, a generator may set CMAKE_C_COMPILER before
# loading this file to force a compiler.
# use environment variable CCC first if defined by user, next use 
# the cmake variable CMAKE_GENERATOR_CC which can be defined by a generator
# as a default compiler

IF(NOT CMAKE_C_COMPILER)
  SET(CMAKE_CXX_COMPILER_INIT NOTFOUND)

  # prefer the environment variable CC
  IF($ENV{CC} MATCHES ".+")
    GET_FILENAME_COMPONENT(CMAKE_C_COMPILER_INIT $ENV{CC} PROGRAM PROGRAM_ARGS CMAKE_C_FLAGS_ENV_INIT)
    IF(CMAKE_C_FLAGS_ENV_INIT)
      SET(CMAKE_C_COMPILER_ARG1 "${CMAKE_C_FLAGS_ENV_INIT}" CACHE STRING "First argument to C compiler")
    ENDIF(CMAKE_C_FLAGS_ENV_INIT)
    IF(EXISTS ${CMAKE_C_COMPILER_INIT})
    ELSE(EXISTS ${CMAKE_C_COMPILER_INIT})
      MESSAGE(FATAL_ERROR "Could not find compiler set in environment variable CC:\n$ENV{CC}.") 
    ENDIF(EXISTS ${CMAKE_C_COMPILER_INIT})
  ENDIF($ENV{CC} MATCHES ".+")

  # next try prefer the compiler specified by the generator
  IF(CMAKE_GENERATOR_CC) 
    IF(NOT CMAKE_C_COMPILER_INIT)
      SET(CMAKE_C_COMPILER_INIT ${CMAKE_GENERATOR_CC})
    ENDIF(NOT CMAKE_C_COMPILER_INIT)
  ENDIF(CMAKE_GENERATOR_CC)

  # if no compiler has been specified yet, then look for one
  IF(NOT CMAKE_C_COMPILER_INIT)
  # if not in the envionment then search for the compiler in the path
    SET(CMAKE_C_COMPILER_LIST gcc cc cl bcc xlc)  
    FIND_PROGRAM(CMAKE_C_COMPILER_FULLPATH NAMES ${CMAKE_C_COMPILER_LIST} )
    GET_FILENAME_COMPONENT(CMAKE_C_COMPILER_INIT
                           ${CMAKE_C_COMPILER_FULLPATH} NAME)
    SET(CMAKE_C_COMPILER_FULLPATH "${CMAKE_C_COMPILER_FULLPATH}" CACHE INTERNAL "full path to the compiler cmake found")
  ENDIF(NOT CMAKE_C_COMPILER_INIT)

  SET(CMAKE_C_COMPILER ${CMAKE_C_COMPILER_INIT} CACHE STRING "C compiler")
ENDIF(NOT CMAKE_C_COMPILER)
MARK_AS_ADVANCED(CMAKE_C_COMPILER)  
GET_FILENAME_COMPONENT(COMPILER_LOCATION "${CMAKE_C_COMPILER}"
  PATH)

FIND_PROGRAM(CMAKE_AR NAMES ar PATHS ${COMPILER_LOCATION} )

FIND_PROGRAM(CMAKE_RANLIB NAMES ranlib)
IF(NOT CMAKE_RANLIB)
   SET(CMAKE_RANLIB : CACHE INTERNAL "noop for ranlib")
ENDIF(NOT CMAKE_RANLIB)
MARK_AS_ADVANCED(CMAKE_RANLIB)

# do not test for GNU if the generator is visual studio
IF(${CMAKE_GENERATOR} MATCHES "Visual Studio")
  SET(CMAKE_COMPILER_IS_GNUCC_RUN 1)
ENDIF(${CMAKE_GENERATOR} MATCHES "Visual Studio") 


IF(NOT CMAKE_COMPILER_IS_GNUCC_RUN)
  # test to see if the c compiler is gnu
  
  IF(CMAKE_C_FLAGS)
    SET(CMAKE_BOOT_C_FLAGS ${CMAKE_C_FLAGS})
  ELSE(CMAKE_C_FLAGS)
    SET(CMAKE_BOOT_C_FLAGS $ENV{CFLAGS})
  ENDIF(CMAKE_C_FLAGS)
  EXEC_PROGRAM(${CMAKE_C_COMPILER} ARGS ${CMAKE_BOOT_C_FLAGS} -E "\"${CMAKE_ROOT}/Modules/CMakeTestGNU.c\"" OUTPUT_VARIABLE CMAKE_COMPILER_OUTPUT RETURN_VALUE CMAKE_COMPILER_RETURN)
  SET(CMAKE_COMPILER_IS_GNUCC_RUN 1)
  IF(NOT CMAKE_COMPILER_RETURN)
    IF("${CMAKE_COMPILER_OUTPUT}" MATCHES ".*THIS_IS_GNU.*" )
      SET(CMAKE_COMPILER_IS_GNUCC 1)
      FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeOutput.log
        "Determining if the C compiler is GNU succeeded with "
        "the following output:\n${CMAKE_COMPILER_OUTPUT}\n\n")
    ELSE("${CMAKE_COMPILER_OUTPUT}" MATCHES ".*THIS_IS_GNU.*" )
      FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeOutput.log
        "Determining if the C compiler is GNU failed with "
        "the following output:\n${CMAKE_COMPILER_OUTPUT}\n\n")
    ENDIF("${CMAKE_COMPILER_OUTPUT}" MATCHES ".*THIS_IS_GNU.*" )
    IF("${CMAKE_COMPILER_OUTPUT}" MATCHES ".*THIS_IS_MINGW.*" )
      SET(CMAKE_COMPILER_IS_MINGW 1)
    ENDIF("${CMAKE_COMPILER_OUTPUT}" MATCHES ".*THIS_IS_MINGW.*" )
    IF("${CMAKE_COMPILER_OUTPUT}" MATCHES ".*THIS_IS_CYGWIN.*" )
      SET(CMAKE_COMPILER_IS_CYGWIN 1)
    ENDIF("${CMAKE_COMPILER_OUTPUT}" MATCHES ".*THIS_IS_CYGWIN.*" )
  ENDIF(NOT CMAKE_COMPILER_RETURN)
ENDIF(NOT CMAKE_COMPILER_IS_GNUCC_RUN)


# configure variables set in this file for fast reload later on
CONFIGURE_FILE(${CMAKE_ROOT}/Modules/CMakeCCompiler.cmake.in 
               "${CMAKE_PLATFORM_ROOT_BIN}/CMakeCCompiler.cmake" IMMEDIATE)
MARK_AS_ADVANCED(CMAKE_AR)

SET(CMAKE_C_COMPILER_ENV_VAR "CC")
