if {[package vsatisfies 8.0 [package provide Tcl]]} { 
    set add 80
} else {
    set add {}
}
if {[info exists ::tcl_platform(debug)] && $::tcl_platform(debug) && \
        [file exists [file join $dir itk34${add}g.dll]]} {
    package ifneeded Itk 3.4 [list load [file join $dir itk34${add}g.dll] Itk]
} else {
    package ifneeded Itk 3.4 [list load [file join $dir itk34${add}.dll] Itk]
}
unset add
