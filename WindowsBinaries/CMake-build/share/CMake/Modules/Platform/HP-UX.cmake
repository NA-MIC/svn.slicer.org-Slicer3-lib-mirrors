SET(CMAKE_SHARED_LIBRARY_SUFFIX ".sl")          # .so
SET(CMAKE_DL_LIBS "dld")
SET(CMAKE_FIND_LIBRARY_SUFFIXES ".sl" ".so" ".a")

SET(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG_SEP ":")   # : or empty

# fortran
IF(CMAKE_COMPILER_IS_GNUG77)
  SET(CMAKE_SHARED_LIBRARY_Fortran_FLAGS "-fPIC")            # -pic 
  SET(CMAKE_SHARED_LIBRARY_CREATE_Fortran_FLAGS "-shared -Wl,-E -Wl,-b")       # -shared
  SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "-Wl,+s -Wl,-E")  # +s, flag for exe link to use shared lib
  SET(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG "-Wl,+b")       # -rpath
  SET(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG_SEP ":")   # : or empty
  SET(CMAKE_SHARED_LIBRARY_SONAME_Fortran_FLAG "-Wl,+h")
  SET(CMAKE_SHARED_LIBRARY_Fortran_FLAGS "-fPIC")     # -pic 
ELSE(CMAKE_COMPILER_IS_GNUG77)
  # use ld directly to create shared libraries for hp cc
  SET(CMAKE_Fortran_CREATE_SHARED_LIBRARY
      "ld <CMAKE_SHARED_LIBRARY_CREATE_Fortran_FLAGS> <CMAKE_SHARED_LIBRARY_SONAME_Fortran_FLAG><TARGET_SONAME> <LINK_FLAGS> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>")
  SET(CMAKE_SHARED_LIBRARY_Fortran_FLAGS "+Z")            # -pic 
  SET(CMAKE_SHARED_LIBRARY_CREATE_Fortran_FLAGS "-E -b -L/usr/lib")       # -shared
  SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "-Wl,+s -Wl,-E")  # +s, flag for exe link to use shared lib
  SET(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG "+b")       # -rpath
  SET(CMAKE_SHARED_LIBRARY_SONAME_Fortran_FLAG "+h")
ENDIF(CMAKE_COMPILER_IS_GNUG77)
# C compiler
IF(CMAKE_COMPILER_IS_GNUCC)
  # gnu gcc
  SET(CMAKE_SHARED_LIBRARY_C_FLAGS "-fPIC")            # -pic 
  SET(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-shared -Wl,-E -Wl,-b")       # -shared
  SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "-Wl,+s -Wl,-E")  # +s, flag for exe link to use shared lib
  SET(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG "-Wl,+b")       # -rpath
  SET(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG_SEP ":")   # : or empty
  SET(CMAKE_SHARED_LIBRARY_SONAME_C_FLAG "-Wl,+h")
  SET(CMAKE_SHARED_LIBRARY_C_FLAGS "-fPIC")     # -pic 
ELSE(CMAKE_COMPILER_IS_GNUCC)
  # hp cc
  SET(CMAKE_ANSI_CFLAGS "-Aa -Ae")
  # use ld directly to create shared libraries for hp cc
  SET(CMAKE_C_CREATE_SHARED_LIBRARY
      "ld <CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS> <CMAKE_SHARED_LIBRARY_SONAME_C_FLAG><TARGET_SONAME> <LINK_FLAGS> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>")
  SET(CMAKE_SHARED_LIBRARY_C_FLAGS "+Z")            # -pic 
  SET(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-E -b -L/usr/lib")       # -shared
  SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "-Wl,+s -Wl,-E")  # +s, flag for exe link to use shared lib
  SET(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG "+b")       # -rpath
  SET(CMAKE_SHARED_LIBRARY_SONAME_C_FLAG "+h")
ENDIF(CMAKE_COMPILER_IS_GNUCC)

# CXX compiler
IF(CMAKE_COMPILER_IS_GNUCXX) 
  # for gnu C++
  SET(CMAKE_SHARED_LIBRARY_CXX_FLAGS "-fPIC")            # -pic 
  SET(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "-shared -Wl,-E -Wl,-b")       # -shared
  SET(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "-Wl,+s -Wl,-E")  # +s, flag for exe link to use shared lib
  SET(CMAKE_SHARED_LIBRARY_RUNTIME_CXX_FLAG "-Wl,+b")       # -rpath
  SET(CMAKE_SHARED_LIBRARY_CXX_FLAGS "-fPIC")     # -pic 
  SET(CMAKE_SHARED_LIBRARY_SONAME_CXX_FLAG "-Wl,+h")
ELSE(CMAKE_COMPILER_IS_GNUCXX)
  # for hp aCC
  SET(CMAKE_SHARED_LIBRARY_CXX_FLAGS "+Z")            # -pic 
  SET(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "+Z -Wl,-E -b -L/usr/lib")       # -shared
  SET(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "-Wl,+s -Wl,-E")  # +s, flag for exe link to use shared lib
  SET(CMAKE_SHARED_LIBRARY_RUNTIME_CXX_FLAG "-Wl,+b")       # -rpath
  SET(CMAKE_SHARED_LIBRARY_SONAME_CXX_FLAG "-Wl,+h")
  SET (CMAKE_CXX_FLAGS_INIT "")
  SET (CMAKE_CXX_FLAGS_DEBUG_INIT "-g")
  SET (CMAKE_CXX_FLAGS_MINSIZEREL_INIT "+O3 -DNDEBUG")
  SET (CMAKE_CXX_FLAGS_RELEASE_INIT "+O2 -DNDEBUG")
  SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "-g")
  SET (CMAKE_C_FLAGS_INIT "")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)
# set flags for gcc support
INCLUDE(Platform/UnixPaths)
