if {[catch {package require Tcl 8.4}]} return
package ifneeded tile 0.4  [list load [file join $dir tile04.dll] tile]
