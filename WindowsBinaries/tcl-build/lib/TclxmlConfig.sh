# tclxmlConfig.sh --
# 
# This shell script (for sh) is generated automatically by Tclxml's
# configure script.  It will create shell variables for most of
# the configuration options discovered by the configure script.
# This script is intended to be included by the configure scripts
# for Tclxml extensions so that they don't have to figure this all
# out for themselves.  This file does not duplicate information
# already provided by tclConfig.sh, so you may need to use that
# file in addition to this one.
#
# The information in this file is specific to a single platform.

# Tclxml's version number.
Tclxml_VERSION='2.6'
Tclxml_MAJOR_VERSION='2'
Tclxml_MINOR_VERSION='6'
Tclxml_RELEASE_LEVEL=''

# The name of the Tclxml library (may be either a .a file or a shared library):
Tclxml_LIB_FILE=Tclxml26.dll

# String to pass to linker to pick up the Tclxml library from its
# build directory.
Tclxml_BUILD_LIB_SPEC='-L/home/Administrator/dbn/lba/night/builds/win32-ix86/tclxml/win32-ix86 -lTclxml26'

# String to pass to linker to pick up the Tclxml library from its
# installed directory.
Tclxml_LIB_SPEC='-LC:/cygwin/home/nicole/slicer26/Lib/win32/win32/tcl-build/lib/Tclxml2.6 -lTclxml26'

# The name of the Tclxml stub library (a .a file):
Tclxml_STUB_LIB_FILE=Tclxmlstub26.lib

# String to pass to linker to pick up the Tclxml stub library from its
# build directory.
Tclxml_BUILD_STUB_LIB_SPEC='-L/home/Administrator/dbn/lba/night/builds/win32-ix86/tclxml/win32-ix86 -lTclxmlstub26'

# String to pass to linker to pick up the Tclxml stub library from its
# installed directory.
Tclxml_STUB_LIB_SPEC='-LC:/cygwin/home/nicole/slicer26/Lib/win32/win32/tcl-build/lib/Tclxml2.6 -lTclxmlstub26'

# String to pass to linker to pick up the Tclxml stub library from its
# build directory.
Tclxml_BUILD_STUB_LIB_PATH='/home/Administrator/dbn/lba/night/builds/win32-ix86/tclxml/win32-ix86/Tclxmlstub26.lib'

# String to pass to linker to pick up the Tclxml stub library from its
# installed directory.
Tclxml_STUB_LIB_PATH='C:/cygwin/home/nicole/slicer26/Lib/win32/win32/tcl-build/lib/Tclxml2.6/Tclxmlstub26.lib'

# Location of the top-level source directories from which [incr Tcl]
# was built.  This is the directory that contains generic, unix, etc.
# If [incr Tcl] was compiled in a different place than the directory
# containing the source files, this points to the location of the sources,
# not the location where [incr Tcl] was compiled.
Tclxml_SRC_DIR='..'
