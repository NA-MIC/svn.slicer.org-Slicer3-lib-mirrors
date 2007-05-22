SET(CMAKE_SHARED_LIBRARY_PREFIX "lib")
SET(CMAKE_SHARED_LIBRARY_SUFFIX ".dylib")
SET(CMAKE_SHARED_MODULE_PREFIX "lib")
SET(CMAKE_SHARED_MODULE_SUFFIX ".so")
SET(CMAKE_MODULE_EXISTS 1)
SET(CMAKE_DL_LIBS "")
SET(CMAKE_C_LINK_FLAGS "-headerpad_max_install_names")
SET(CMAKE_CXX_LINK_FLAGS "-headerpad_max_install_names")
SET(CMAKE_PLATFORM_HAS_INSTALLNAME 1)
SET(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-dynamiclib -headerpad_max_install_names")
SET(CMAKE_SHARED_MODULE_CREATE_C_FLAGS "-bundle -headerpad_max_install_names")
SET(CMAKE_FIND_LIBRARY_SUFFIXES ".dylib" ".so" ".a")

# setup for universal binaries if sysroot exists
IF(EXISTS /Developer/SDKs/MacOSX10.4u.sdk)
  # set the sysroot to be used if CMAKE_OSX_ARCHITECTURES
  # has more than one value
  SET(CMAKE_OSX_SYSROOT /Developer/SDKs/MacOSX10.4u.sdk CACHE STRING
    "isysroot used for universal binary support")
  # set _CMAKE_OSX_MACHINE to umame -m
  EXEC_PROGRAM(uname ARGS -m OUTPUT_VARIABLE _CMAKE_OSX_MACHINE)
  # check for Power PC and change to ppc
  IF("${_CMAKE_OSX_MACHINE}" MATCHES "Power")
    SET(_CMAKE_OSX_MACHINE ppc)
  ENDIF("${_CMAKE_OSX_MACHINE}" MATCHES "Power")
  # check for environment variable CMAKE_OSX_ARCHITECTURES
  # if it is set.
  IF(NOT "$ENV{CMAKE_OSX_ARCHITECTURES}" STREQUAL "")
    SET(_CMAKE_OSX_MACHINE "$ENV{CMAKE_OSX_ARCHITECTURES}")
  ENDIF(NOT "$ENV{CMAKE_OSX_ARCHITECTURES}" STREQUAL "")
  # now put _CMAKE_OSX_MACHINE into the cache
  SET(CMAKE_OSX_ARCHITECTURES ${_CMAKE_OSX_MACHINE}
    CACHE STRING "Build architectures for OSX")
ENDIF(EXISTS /Developer/SDKs/MacOSX10.4u.sdk)


IF("${CMAKE_BACKWARDS_COMPATIBILITY}" MATCHES "^1\\.[0-6]$")
  SET(CMAKE_SHARED_MODULE_CREATE_C_FLAGS
    "${CMAKE_SHARED_MODULE_CREATE_C_FLAGS} -flat_namespace -undefined suppress")
ENDIF("${CMAKE_BACKWARDS_COMPATIBILITY}" MATCHES "^1\\.[0-6]$")

IF(NOT XCODE)
  # Enable shared library versioning.  This flag is not actually referenced
  # but the fact that the setting exists will cause the generators to support
  # soname computation.
  SET(CMAKE_SHARED_LIBRARY_SONAME_C_FLAG "-install_name")
  SET(CMAKE_SHARED_LIBRARY_SONAME_CXX_FLAG "-install_name")
ENDIF(NOT XCODE)

SET(CMAKE_MacOSX_Content_COMPILE_OBJECT "\"${CMAKE_COMMAND}\" -E copy_if_different <SOURCE> <OBJECT>")

SET(CMAKE_C_CREATE_SHARED_LIBRARY_FORBIDDEN_FLAGS -w)
SET(CMAKE_CXX_CREATE_SHARED_LIBRARY_FORBIDDEN_FLAGS -w)
SET(CMAKE_C_CREATE_SHARED_LIBRARY
  "<CMAKE_C_COMPILER> <LANGUAGE_COMPILE_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS> <LINK_FLAGS> -o <TARGET> -install_name <TARGET_INSTALLNAME_DIR><TARGET_SONAME> <OBJECTS> <LINK_LIBRARIES>")
SET(CMAKE_CXX_CREATE_SHARED_LIBRARY
  "<CMAKE_CXX_COMPILER> <LANGUAGE_COMPILE_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS> <LINK_FLAGS> -o <TARGET> -install_name <TARGET_INSTALLNAME_DIR><TARGET_SONAME> <OBJECTS> <LINK_LIBRARIES>")

SET(CMAKE_CXX_CREATE_SHARED_MODULE
      "<CMAKE_CXX_COMPILER> <LANGUAGE_COMPILE_FLAGS> <CMAKE_SHARED_MODULE_CREATE_CXX_FLAGS> <LINK_FLAGS> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>")

SET(CMAKE_C_CREATE_SHARED_MODULE
      "<CMAKE_C_COMPILER>  <LANGUAGE_COMPILE_FLAGS> <CMAKE_SHARED_MODULE_CREATE_C_FLAGS> <LINK_FLAGS> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>")


SET(CMAKE_PLATFORM_IMPLICIT_INCLUDE_DIRECTORIES /usr/local/include)
# default to searching for frameworks first
SET(CMAKE_FIND_FRAMEWORK FIRST)
# set up the default search directories for frameworks
SET(CMAKE_SYSTEM_FRAMEWORK_PATH
  ~/Library/Frameworks
  /Library/Frameworks
  /Network/Library/Frameworks
  /System/Library/Frameworks)

# default to searching for application bundles first
SET(CMAKE_FIND_APPBUNDLE FIRST)
# set up the default search directories for application bundles
SET(CMAKE_SYSTEM_APPBUNDLE_PATH
  ~/Applications
  /Applications
  /Developer/Applications)

INCLUDE(Platform/UnixPaths)
SET(CMAKE_SYSTEM_INCLUDE_PATH ${CMAKE_SYSTEM_INCLUDE_PATH} /sw/include)
SET(CMAKE_SYSTEM_LIBRARY_PATH ${CMAKE_SYSTEM_LIBRARY_PATH} /sw/lib)
