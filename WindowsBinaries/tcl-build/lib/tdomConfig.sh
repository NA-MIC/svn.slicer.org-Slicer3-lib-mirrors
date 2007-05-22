#
# tdomConfig.sh --
# 
# This shell script (for Bourne sh and alike) is generated 
# automatically by tDOM configure script. It will create 
# shell variables for some of the configuration options 
# discovered by the configure. This script is intended to be
# sourced by the configure scripts for tDOM extensions so 
# that they don't have to figure this all out for themselves.
# This file does not duplicate information already provided
# by tclConfig.sh, so you may need to use that file in addition
# to this one. To be able to locate this file easily, extensions
# might want to include the tdom.m4 file in their configure
# scripts and use the TDOM_PATH_CONFIG and TDOM_LOAD_CONFIG.
#
# The information in this file is specific to a single platform.
#

#
# tDOM version number
#
TDOM_MAJOR_VERSION='0'
TDOM_MINOR_VERSION='8'
TDOM_PATCHLEVEL='0'
TDOM_VERSION='080'

#
# The name of the tDOM stub library file
#
TDOM_STUB_FILE=tdomstub080.lib

#
# String to pass to linker to pick up the tDOM library from
# its build directory.
#
TDOM_BUILD_STUB_LIB_SPEC='-L/home/Administrator/dbn/lba/night/builds/win32-ix86/tdom/win -ltdomstub080'

#
# String to pass to linker to pick up the tDOM library from
# its installed directory.
#
TDOM_STUB_LIB_SPEC='-LC:/cygwin/home/nicole/slicer26/Lib/win32/win32/tcl-build/lib/tdom080 -ltdomstub080'

#
# Location of the top-level source directories from which tDOM
# was built.  This is the directory that contains generic, unix,
# win etc. If tDOM was compiled in a different place than the 
# directory containing the source files, this points to the 
# location of the sources, not the location where tDOM was compiled.
#
TDOM_SRC_DIR='/home/Administrator/dbn/lba/night/builds/win32-ix86/tdom'

# EOF

