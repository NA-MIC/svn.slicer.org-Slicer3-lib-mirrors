# itclConfig.sh --
# 
# This shell script (for sh) is generated automatically by Itcl's
# configure script.  It will create shell variables for most of
# the configuration options discovered by the configure script.
# This script is intended to be included by the configure scripts
# for Itcl extensions so that they don't have to figure this all
# out for themselves.  This file does not duplicate information
# already provided by tclConfig.sh, so you may need to use that
# file in addition to this one.
#
# The information in this file is specific to a single platform.

# Itcl's version number.
itcl_VERSION='3.2'
itcl_MAJOR_VERSION='3'
itcl_MINOR_VERSION='2'
itcl_RELEASE_LEVEL='.1'

# The name of the Itcl library (may be either a .a file or a shared library):
itcl_LIB_FILE=itcl32.dll

# String to pass to linker to pick up the Itcl library from its
# build directory.
itcl_BUILD_LIB_SPEC='-L/home/Administrator/dbn/lba/night/builds/win32-ix86/itcl/win32-ix86/itcl -litcl32'

# String to pass to linker to pick up the Itcl library from its
# installed directory.
itcl_LIB_SPEC='-LC:/cygwin/home/nicole/slicer26/Lib/win32/win32/tcl-build/lib/itcl3.2 -litcl32'

# The name of the Itcl stub library (a .a file):
itcl_STUB_LIB_FILE=itclstub32.lib

# String to pass to linker to pick up the Itcl stub library from its
# build directory.
itcl_BUILD_STUB_LIB_SPEC='-L/home/Administrator/dbn/lba/night/builds/win32-ix86/itcl/win32-ix86/itcl -litclstub32'

# String to pass to linker to pick up the Itcl stub library from its
# installed directory.
itcl_STUB_LIB_SPEC='-LC:/cygwin/home/nicole/slicer26/Lib/win32/win32/tcl-build/lib/itcl3.2 -litclstub32'

# String to pass to linker to pick up the Itcl stub library from its
# build directory.
itcl_BUILD_STUB_LIB_PATH='/home/Administrator/dbn/lba/night/builds/win32-ix86/itcl/win32-ix86/itcl/itclstub32.lib'

# String to pass to linker to pick up the Itcl stub library from its
# installed directory.
itcl_STUB_LIB_PATH='C:/cygwin/home/nicole/slicer26/Lib/win32/win32/tcl-build/lib/itcl3.2/itclstub32.lib'

# Location of the top-level source directories from which [incr Tcl]
# was built.  This is the directory that contains generic, unix, etc.
# If [incr Tcl] was compiled in a different place than the directory
# containing the source files, this points to the location of the sources,
# not the location where [incr Tcl] was compiled.
itcl_SRC_DIR='../../itcl'
