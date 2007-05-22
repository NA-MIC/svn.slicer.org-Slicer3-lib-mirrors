# Package index for tile pixmap themes.

if {[file isdirectory [file join $dir Aquativo-1.5]]} {
    package ifneeded tile::theme::Aquativo 0.0.1 \
        [list source [file join $dir Aquativo.tcl]]
}

if {[file isdirectory [file join $dir WinXP-Blue]]} {
    package ifneeded tile::theme::WinXPBlue 0.0.1 \
        [list source [file join $dir WinXP-Blue.tcl]]
}

if {[file isdirectory [file join $dir blue]]} {
    package ifneeded tile::theme::blue 0.0.1 \
        [list source [file join $dir blue.tcl]]
}

if {[file isdirectory [file join $dir kroc]]} {
    package ifneeded tile::theme::kroc 0.0.1 \
        [list source [file join $dir kroc.tcl]]
}

