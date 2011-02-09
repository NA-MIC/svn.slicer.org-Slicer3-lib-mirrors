if {[package vsatisfies 8.0 [package provide Tcl]]} { 
    set add 80
} else {
    set add {}
}
if {[info exists ::tcl_platform(debug)] && $::tcl_platform(debug) && \
        [file exists [file join $dir itcl34${add}g.dll]]} {
    package ifneeded Itcl 3.4 [list load [file join $dir itcl34${add}g.dll] Itcl]
} else {
    package ifneeded Itcl 3.4 [list load [file join $dir itcl34${add}.dll] Itcl]
}
unset add
