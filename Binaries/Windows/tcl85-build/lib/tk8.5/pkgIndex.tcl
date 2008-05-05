if {[package vcompare [package provide Tcl] 8.5.0] != 0} { return }
package ifneeded Tk 8.5.0 [list load [file join $dir .. .. bin tk85tg.dll] Tk]
