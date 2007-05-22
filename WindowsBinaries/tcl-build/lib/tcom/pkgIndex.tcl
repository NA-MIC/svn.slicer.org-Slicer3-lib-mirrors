# $Id: pkgIndex.tcl,v 1.15 2002/02/26 23:10:47 cthuang Exp $
package ifneeded tcom 3.8 \
[list load [file join $dir tcom.dll]]\n[list source [file join $dir tcom.tcl]]
