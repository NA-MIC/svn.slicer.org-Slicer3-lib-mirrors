# tcldomConfig.sh --
# 
# This shell script (for sh) is generated automatically by Tcldom's
# configure script.  It will create shell variables for most of
# the configuration options discovered by the configure script.
# This script is intended to be included by the configure scripts
# for Tcldom extensions so that they don't have to figure this all
# out for themselves.  This file does not duplicate information
# already provided by tclConfig.sh, so you may need to use that
# file in addition to this one.
#
# The information in this file is specific to a single platform.

# Tcldom's version number.
Tcldom_VERSION='2.6'
Tcldom_MAJOR_VERSION='2'
Tcldom_MINOR_VERSION='6'
Tcldom_RELEASE_LEVEL=''

# The name of the Tcldom library (may be either a .a file or a shared library):
Tcldom_LIB_FILE=Tcldom26.dll

# String to pass to linker to pick up the Tcldom library from its
# build directory.
Tcldom_BUILD_LIB_SPEC='-L/home/Administrator/dbn/lba/night/builds/win32-ix86/tcldom/win32-ix86 -lTcldom26'

# String to pass to linker to pick up the Tcldom library from its
# installed directory.
Tcldom_LIB_SPEC='-LC:/cygwin/home/nicole/slicer26/Lib/win32/win32/tcl-build/lib/Tcldom2.6 -lTcldom26'

# The name of the Tcldom stub library (a .a file):
Tcldom_STUB_LIB_FILE=Tcldomstub26.lib

# String to pass to linker to pick up the Tcldom stub library from its
# build directory.
Tcldom_BUILD_STUB_LIB_SPEC='-L/home/Administrator/dbn/lba/night/builds/win32-ix86/tcldom/win32-ix86 -lTcldomstub26'

# String to pass to linker to pick up the Tcldom stub library from its
# installed directory.
Tcldom_STUB_LIB_SPEC='-LC:/cygwin/home/nicole/slicer26/Lib/win32/win32/tcl-build/lib/Tcldom2.6 -lTcldomstub26'

# String to pass to linker to pick up the Tcldom stub library from its
# build directory.
Tcldom_BUILD_STUB_LIB_PATH='/home/Administrator/dbn/lba/night/builds/win32-ix86/tcldom/win32-ix86/Tcldomstub26.lib'

# String to pass to linker to pick up the Tcldom stub library from its
# installed directory.
Tcldom_STUB_LIB_PATH='C:/cygwin/home/nicole/slicer26/Lib/win32/win32/tcl-build/lib/Tcldom2.6/Tcldomstub26.lib'

# Location of the top-level source directories from which [incr Tcl]
# was built.  This is the directory that contains generic, unix, etc.
# If [incr Tcl] was compiled in a different place than the directory
# containing the source files, this points to the location of the sources,
# not the location where [incr Tcl] was compiled.
Tcldom_SRC_DIR='..'
