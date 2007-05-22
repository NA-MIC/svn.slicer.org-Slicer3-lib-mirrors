#!/bin/sh
# ----------------------------------------------------------------------
#  PROGRAM: demo program for [incr Widgets]
# ----------------------------------------------------------------------
#  Michael J. McLennan
#  Bell Labs Innovations for Lucent Technologies
#  mmclennan@lucent.com
#  http://www.tcltk.com/itcl/
# ======================================================================
#  Copyright (c) 1993-1998  Lucent Technologies, Inc.
# ======================================================================
#\
exec wish "$0"

package require Iwidgets 4.0

# ----------------------------------------------------------------------
option add *Scrolledtext.textBackground white startupFile
option add *Scrolledlistbox.textBackground white startupFile
option add *Scrolledhtml.textBackground white startupFile
option add *Scrolledhtml.padX 10 startupFile
option add *boxColor blue startupFile
option add *boxTextColor white startupFile

# ----------------------------------------------------------------------
# USAGE:  iw_demo_file <name>
#
# Returns the proper demo file name for a demo called <name>.
# ----------------------------------------------------------------------
proc iw_demo_file {name} {
    global iwidgets::library
    return [file join $library demos $name]
}

# ----------------------------------------------------------------------
# USAGE:  iw_demo_manpage <name>
#
# Returns the proper man page file for a demo called <name>.
# ----------------------------------------------------------------------
proc iw_demo_manpage {name} {
    global iwidgets::library
    return [file join $library demos html $name.n.html]
}

# ----------------------------------------------------------------------
# USAGE:  iw_load_demo <name>
#
# Loads a demo program with the given <name>.  Demos can be written
# as if they will pop up in the main application window, but they will
# pop up inside the tab notebook instead.
# ----------------------------------------------------------------------
proc iw_load_demo {name} {
    global widgets

    catch {eval destroy [winfo children $widgets(info-example)]}
    iw_lock on
    iw_status "Loading..."

    set win [frame $widgets(info-example).inner]
    pack $win -expand yes

    set loadcmd {
        set fid [open [iw_demo_file $name] r]
        set code [read $fid]
        close $fid
    }
    if {[catch $loadcmd result] == 0} {
        regsub -all "(\"|\{|\\\[| |\n|^|\t)((\\.\[A-Za-z0-9\]+)+)" \
            $code "\\1$win\\2" code
        regsub -all "(\"|\{|\\\[| |\n|^)(\\. )" \
            $code "\\1$win " code
        if {[catch {uplevel #0 $code} result] == 0} {
            $widgets(info-code) clear
            $widgets(info-code) import [iw_demo_file $name]
            iw_draw_hier $name
            iw_load_manpage $name
            iw_lock off
            iw_status ""
            return
        }
    }
    catch {eval destroy [winfo children $win]}
    label $win.err -background white -wraplength 4i \
        -text "Can't load demo:\n$result"
    pack $win.err -expand yes
    iw_lock off
    iw_status ""
}

# overload a few critical functions that might be used by demo programs...
rename exit tcl_exit
proc exit {{status 0}} {
    # do nothing
}

rename puts tcl_puts
proc puts {args} {
    global widgets
    if {[llength $args] == 1} {
        iw_status [lindex $args 0]
    } else {
        eval tcl_puts $args
    }
}

# ----------------------------------------------------------------------
# USAGE:  iw_load_manpage
#
# Loads the man page for the current demo.  Man pages are not
# automatically loaded unless the man page viewer is visible.
# This procedure checks to see if the viewer is visible, and loads
# the man page if needed.
# ----------------------------------------------------------------------
set iwManPage ""
proc iw_load_manpage {{name ""}} {
    global widgets iwManPage 

    if {[winfo ismapped $widgets(info-manpage)]} {
        if {$name == ""} {
            set name [$widgets(list) getcurselection]
        }
        if {$name != $iwManPage} {
            iw_lock on
            iw_status "Loading man page..."
            $widgets(info-manpage) import [iw_demo_manpage $name]
            iw_lock off
            iw_status ""
	    place forget $widgets(info-manpage-feedback)
        }
        set iwManPage $name
    }
}

# ----------------------------------------------------------------------
# USAGE:  iw_manpage_progress
#
# Handles the progress meter whenever an HTML man page is rendered.
# If the progress meter is not showing, it is put up, and the current
# state is updated.  If the meter is at 100%, it is taken down.
# ----------------------------------------------------------------------
proc iw_manpage_progress {n} {
    global widgets remaining

    if {![winfo ismapped $widgets(info-manpage-feedback)]} {
      $widgets(info-manpage-feedback) configure -steps $n
      $widgets(info-manpage-feedback) reset
      place $widgets(info-manpage-feedback) -relx 0.5 -rely 0.5 \
          -anchor c -width 200
      update
      set remaining $n
    }

    $widgets(info-manpage-feedback) step [expr $remaining - $n]
    set remaining $n
}

# ----------------------------------------------------------------------
# USAGE:  iw_status <message>
#
# Displays a status <message> near the top of the window.
# ----------------------------------------------------------------------
proc iw_status {message} {
    global widgets
    $widgets(status) configure -text $message
    update
}

# ----------------------------------------------------------------------
# USAGE:  iw_lock <state>
#
# Locks or unlocks the main window.  Sets a grab on the main menu,
# so that all events are sent to it.
# ----------------------------------------------------------------------
proc iw_lock {state} {
    global widgets
    if {$state} {
        grab set $widgets(mainMenu)
        . configure -cursor watch
    } else {
        grab release $widgets(mainMenu)
        . configure -cursor ""
    }
}

# ----------------------------------------------------------------------
# USAGE:  iw_draw_hier <name>
#
# Queries the hierarchy for a particular class in demo <name> and
# draws a class diagram into a display window.  Usually invoked when
# a demo is loaded to display the class hierarchy for the associated
# widget.
# ----------------------------------------------------------------------
proc iw_draw_hier {name} {
    global widgets
    set canv $widgets(info-hier)
    $canv delete all

    set class [string toupper [string index $name 0]][string tolower [string range $name 1 end]]

    if {[catch [list namespace eval iwidgets [list itcl::find classes $class]]] == 0} {
      set objs [namespace eval iwidgets [list itcl::find objects -class $class]]
      if {[llength $objs]} {
        update idletasks
	set bases [[lindex $objs 0] info heritage]
        iw_draw_level [lrange $bases 1 end] $canv [lindex $bases 0]
        set bbox [$canv bbox all]

        $canv move all [expr [lindex $bbox 0] * -1] [expr [lindex $bbox 1] * -1]

	$canv xview moveto 0
	$canv yview moveto 0

        set bbox [$canv bbox all]
	set x [expr ([winfo width $widgets(info)] / 2) - ([lindex $bbox 2] / 2)]
	set y 10
        $canv move all $x $y
      }
    }
}

# ----------------------------------------------------------------------
# USAGE:  iw_draw_level <bases> <canv> <class>
#
# Draws one level of the hierarchy for <class>.
# ----------------------------------------------------------------------
proc iw_draw_level {bases canv class} {
    set org [iw_draw_box $canv $class]
    set top $org

    set offset 0

    if {[llength $bases]} {
      $canv lower [$canv create line $offset $org \
       $offset [expr $top-10] \
       -40 [expr $top-10] \
       -24 [expr $top-10] \
       -20 [expr $top-16] \
       -16 [expr $top-10] \
       -20 [expr $top-16] \
       -20 [expr $top-26]]
      $canv move all 20 [expr -($top-26+$org)]

      set del [iw_draw_level [lrange $bases 1 end] $canv [lindex $bases 0]]
      $canv move all -20 [expr $top-26+$org]
      set top [expr $top+$del-30+$org]
      incr offset 4
    }

    return $top
}

# ----------------------------------------------------------------------
# USAGE:  iw_draw_box <canv> <class>
#
# Draws one box for a class hierarchy onto a canvas window.
# ----------------------------------------------------------------------
proc iw_draw_box {canv class} {
    set bg [option get $canv boxColor BoxColor]
    set textbg [option get $canv boxTextColor BoxTextColor]

    set cname [string trimleft $class :]
    $canv create text 0 0 -anchor center -text $cname \
        -fill $textbg -tags $class

    set bbox [$canv bbox $class]
    set x0 [expr [lindex $bbox 0]-4]
    set y0 [expr [lindex $bbox 1]-4]
    set x1 [expr [lindex $bbox 2]+4]
    set y1 [expr [lindex $bbox 3]+4]

    $canv create rectangle $x0 $y0 $x1 $y1 \
        -outline black -fill $bg

    $canv raise $class

    return $y0
}

# ----------------------------------------------------------------------
wm title . {[incr Widgets] Demo}
wm geometry . 750x440

frame .mbar -borderwidth 2 -relief raised
pack .mbar -fill x
set widgets(mainMenu) [menubutton .mbar.main -text "Main" -menu .mbar.main.m]
pack .mbar.main -side left

menu .mbar.main.m
.mbar.main.m add command -label "About..." -command {.about activate}
.mbar.main.m add separator
.mbar.main.m add command -label "Quit" -command tcl_exit

iwidgets::panedwindow .pw -orient vertical
pack .pw -expand yes -fill both

.pw add "widgets"
set pane [.pw childsite "widgets"]
set widgets(list) $pane.wlist

iwidgets::scrolledlistbox $widgets(list) -labeltext "Select a widget:" \
    -selectioncommand {iw_load_demo [$widgets(list) getcurselection]} \
    -labelpos nw -vscrollmode dynamic -hscrollmode none \
    -exportselection no
pack $widgets(list) -expand yes -fill both -padx 8

.pw add "info"
set pane [.pw childsite "info"]
set widgets(info) $pane.info

.pw fraction 25 75

set widgets(status) [label $pane.status]
pack $pane.status -anchor w

iwidgets::tabnotebook $widgets(info) -tabpos s
pack $widgets(info) -expand yes -fill both

set widgets(info-example) [$widgets(info) add -label "Example"]
$widgets(info-example) configure -background white

set win [$widgets(info) add -label "Example Code"]
set widgets(info-code) [iwidgets::scrolledtext $win.code \
    -wrap none -vscrollmode dynamic -hscrollmode none]
pack $widgets(info-code) -expand yes -fill both -padx 4 -pady 4

set win [$widgets(info) add -label "Inheritance"]
set widgets(info-hier) [iwidgets::scrolledcanvas $win.canv -textbackground white \
    -vscrollmode dynamic -hscrollmode dynamic]
pack $widgets(info-hier) -expand yes -fill both -padx 4 -pady 4

set win [$widgets(info) add -label "Man Page"]
set widgets(info-manpage) [iwidgets::scrolledhtml $win.html \
    -wrap word -vscrollmode dynamic -hscrollmode none \
    -feedback "iw_manpage_progress" \
    -linkcommand "$win.html import -link"]
pack $widgets(info-manpage) -expand yes -fill both -padx 4 -pady 4

set widgets(info-manpage-feedback) [iwidgets::feedback $win.html.fb \
    -borderwidth 2]

bind $widgets(info-manpage) <Map> {iw_load_manpage}

$widgets(info) select "Example"

# ----------------------------------------------------------------------
# "About" window
# ----------------------------------------------------------------------
iwidgets::dialog .about -title {About: [incr Widgets] Demo} -modality none
.about hide "Apply"
.about hide "Help"
.about hide "Cancel"
.about buttonconfigure "OK" -command ".about deactivate"
.about default "OK"

set win [.about childsite]
label $win.title -text {[incr Widgets]}
pack $win.title
catch {$win.title configure -font -*-helvetica-bold-o-normal-*-*-180-*}

set file [file join ${iwidgets::library} demos iwidgets.gif]
label $win.icon -image [image create photo -file $file]
pack $win.icon -side left

label $win.by -text "Contributed By"
pack $win.by
catch {$win.by configure -font -*-helvetica-medium-r-normal-*-*-100-*}

label $win.authors -text "Mark L. Ulferts
Sue Yockey
John Sigler
Bill Scott
Alfredo Jahn
Tako Schotanus
Kris Raney
John Tucker
Mitch Gorman
John Reekie
Ken Copeland
Tony Parent 
Chad Smith
and
Michael McLennan
"
pack $win.authors
catch {$win.authors configure -font -*-helvetica-medium-o-normal-*-*-120-*}

# ----------------------------------------------------------------------
# Load up a list of demos...
# ----------------------------------------------------------------------
foreach file [lsort [glob [file join ${iwidgets::library} demos *]]] {
    set name [file tail $file]
    if {![file isdirectory $file] && ![string match *.* $name] 
        && ![string match catalog $name]
        && ![string match scopedobject $name]} {
	if {$name == "mainwindow"} {
	    # This demo is doesn't work well with the catalog
	    # so we skip it.

	    continue
	}
        $widgets(list) insert end $name
    }
}
$widgets(list) selection set 0
uplevel #0 [$widgets(list) cget -selectioncommand]


