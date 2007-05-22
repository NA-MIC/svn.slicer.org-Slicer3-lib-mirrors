# tcldomConfig.sh --
# 
# This shell script (for sh) is generated automatically by tcldomxml's
# configure script.  It will create shell variables for most of
# the configuration options discovered by the configure script.
# This script is intended to be included by the configure scripts
# for tcldomxml extensions so that they don't have to figure this all
# out for themselves.  This file does not duplicate information
# already provided by tclConfig.sh, so you may need to use that
# file in addition to this one.
#
# The information in this file is specific to a single platform.

# tcldomxml's version number.
tcldomxml_VERSION='2.6'
tcldomxml_MAJOR_VERSION='2'
tcldomxml_MINOR_VERSION='6'
tcldomxml_RELEASE_LEVEL=''

# The name of the tcldomxml library (may be either a .a file or a shared library):
tcldomxml_LIB_FILE=tcldomxml26.dll

# String to pass to linker to pick up the tcldomxml library from its
# build directory.
tcldomxml_BUILD_LIB_SPEC='-L/home/Administrator/dbn/lba/night/builds/win32-ix86/tcldom/src-libxml2/win32-ix86 -ltcldomxml26'

# String to pass to linker to pick up the tcldomxml library from its
# installed directory.
tcldomxml_LIB_SPEC='-LC:/cygwin/home/nicole/slicer26/Lib/win32/win32/tcl-build/lib/tcldomxml2.6 -ltcldomxml26'

# The name of the tcldomxml stub library (a .a file):
tcldomxml_STUB_LIB_FILE=tcldomxmlstub26.lib

# String to pass to linker to pick up the tcldomxml stub library from its
# build directory.
tcldomxml_BUILD_STUB_LIB_SPEC='-L/home/Administrator/dbn/lba/night/builds/win32-ix86/tcldom/src-libxml2/win32-ix86 -ltcldomxmlstub26'

# String to pass to linker to pick up the tcldomxml stub library from its
# installed directory.
tcldomxml_STUB_LIB_SPEC='-LC:/cygwin/home/nicole/slicer26/Lib/win32/win32/tcl-build/lib/tcldomxml2.6 -ltcldomxmlstub26'

# String to pass to linker to pick up the tcldomxml stub library from its
# build directory.
tcldomxml_BUILD_STUB_LIB_PATH='/home/Administrator/dbn/lba/night/builds/win32-ix86/tcldom/src-libxml2/win32-ix86/tcldomxmlstub26.lib'

# String to pass to linker to pick up the tcldomxml stub library from its
# installed directory.
tcldomxml_STUB_LIB_PATH='C:/cygwin/home/nicole/slicer26/Lib/win32/win32/tcl-build/lib/tcldomxml2.6/tcldomxmlstub26.lib'

# Location of the top-level source directories from which [incr Tcl]
# was built.  This is the directory that contains generic, unix, etc.
# If [incr Tcl] was compiled in a different place than the directory
# containing the source files, this points to the location of the sources,
# not the location where [incr Tcl] was compiled.
tcldomxml_SRC_DIR='..'
