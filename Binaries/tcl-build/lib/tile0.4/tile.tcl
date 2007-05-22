#
# $Id: tile.tcl,v 1.60 2004/04/23 21:10:14 jenglish Exp $
#
# Tile widget set initialization script. 
#

if {![info exists tile_library]} {
    set tile_library [file dirname [info script]]
}

lappend auto_path $tile_library

source [file join $tile_library keynav.tcl]
source [file join $tile_library button.tcl]
source [file join $tile_library menubutton.tcl]
source [file join $tile_library scrollbar.tcl]
source [file join $tile_library scale.tcl]
source [file join $tile_library notebook.tcl]

#
# Helper routines:
#
namespace eval tile {
    proc takefocus {w} {
	return [expr {[winfo viewable $w] && [$w instate !disabled]}]
    }

    #
    # Routines for auto-repeat:
    #
    # NOTE: repeating widgets do not have -repeatdelay
    # or -repeatinterval resources as in standard Tk;
    # instead a single set of settings is applied application-wide.
    # (TODO: make this user-configurable)
    #
    # (@@@ Windows seems to use something like 500/50 milliseconds
    #  @@@ for -repeatdelay/-repeatinterval)
    #

    variable Repeat
    array set Repeat {
	    delay	300
	    interval	100
	    timer 	{}
	    script	{}
    }

    proc Repeatedly {args} {
	variable Repeat
	after cancel $Repeat(timer)
	set script [uplevel 1 [list namespace code $args]]
	set Repeat(script) $script
	uplevel #0 $script
	set Repeat(timer) [after $Repeat(delay) tile::Repeat]
    }

    proc Repeat {} {
	variable Repeat
	uplevel #0 $Repeat(script)
	set Repeat(timer) [after $Repeat(interval) tile::Repeat]
    }

    proc CancelRepeat {} {
	variable Repeat
	after cancel $Repeat(timer)
    }

    #
    # ThemeChanged --
    #	Called from [style theme use].
    #	Sends a <<ThemeChanged>> virtual event to all widgets.
    #	
    proc ThemeChanged {} {
	set Q .
	while {[llength $Q]} {
	    set QN [list]
	    foreach w $Q {
		event generate $w <<ThemeChanged>>
		foreach child [winfo children $w] {
		    lappend QN $child
		}
	    }
	    set Q $QN
	}
    }

    #
    # LoadImages $imgdir ?$patternList? -- utility routine for pixmap themes
    #
    #	Loads all image files in $imgdir matching $patternList.
    #	Returns: a paired list of filename/imagename pairs.
    proc LoadImages {imgdir {patterns {*.gif}}} {
	foreach pattern $patterns {
	    foreach file [glob -directory $imgdir $pattern] {
		set img [file tail [file rootname $file]]
		if {![info exists images($img)]} {
		    set images($img) [image create photo -file $file]
		}
	    }
	}
	return [array get images]
    }

    #
    # Widgets:
    #
    variable widgets {
	button checkbutton radiobutton menubutton label 
	frame labelframe scrollbar
	notebook progress 
    } ;# scale -- not quite ready  

    foreach widget $widgets {
	interp alias {} ::tile::$widget {} ::t$widget
	namespace export $widget
    }

}

#
# Appearance:
#
catch {font create ButtonFont}

namespace eval tile {

    switch $tcl_platform(platform) {
    windows {
    	font configure ButtonFont -family {MS Sans Serif} -size 8 -weight normal
	set defaultBG "SystemButtonFace"
	set activeBG  "SystemButtonFace"
	set troughColor "SystemScrollbar"
    }
    unix -
    default {
	font configure ButtonFont -family Helvetica -size -12 -weight bold
	set defaultBG #d9d9d9
	set activeBG  #ececec
	set troughColor #c3c3c3
    } }
}

#
# Load themes:
#
source [file join $tile_library defaults.tcl]
source [file join $tile_library altTheme.tcl]
source [file join $tile_library stepTheme.tcl]
source [file join $tile_library clamTheme.tcl]

if {[package provide tile::theme::winnative] != {}} {
    source [file join $tile_library winTheme.tcl]
}
if {[package provide tile::theme::xpnative] != {}} {
    source [file join $tile_library xpTheme.tcl]
}
if {[package provide tile::theme::aqua] != {}} {
    source [file join $tile_library aquaTheme.tcl]
}

#*EOF*
