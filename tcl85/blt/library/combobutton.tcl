
namespace eval blt {
    set Priv(afterId) -1
    set Priv(b1) ""
}

proc blt::ComboButtonAutoScan {w} {
    variable Priv

    set x $Priv(x)
    if { ![winfo exists $w] } {
	return
    }
    if { $x >= [winfo width $w] } {
(	$w xview scroll 2 units
    } elseif { $x < 0 } {
	$w xview scroll -2 units
    }
    set Priv(afterId) [after 50 [list blt::ComboButtonAutoScan $w]]
}

bind ComboButton <Enter> {
    %W activate on
}

bind ComboButton <Leave> {
    %W activate off
}

# Standard Motif bindings:

bind ComboButton <ButtonPress-1> {
    puts stderr "ComboButton <ButtonPress-1> state=[%W cget -state]"
    if { [%W cget -state] == "posted" } {
	puts stderr "unpost"
	%W unpost
	blt::ComboButtonUnpost %W
    } else {
	set blt::Priv(relief) [%W cget -buttonrelief]
	puts stderr "relief=$blt::Priv(relief)"
	if { [%W cget -state] != "disabled" } {
	    %W configure -buttonrelief sunken
	}
	grab -global %W
	blt::ComboButtonPost %W
    }
}

bind ComboButton <ButtonRelease-1> {
    puts stderr "ComboButton <ButtonRelease-1> state=[%W cget -state]"
    if { [winfo containing -display  %W %X %Y] == "%W" } {
	puts stderr "invoke"
	%W invoke
    } else { 
	puts stderr "unpost"
	blt::ComboButtonUnpost %W
    }	
    if { [info exists blt::Priv(relief)] } {
	%W configure -buttonrelief $blt::Priv(relief)
    }
}

bind ComboButton <KeyPress-Down> {
    if { [%W cget -state] != "disabled" } {
	blt::ComboButtonPost %W
    }
}

# Ignore all Alt, Meta, and Control keypresses unless explicitly bound.
# Otherwise, if a widget binding for one of these is defined, the
# <KeyPress> class binding will also fire and insert the character,
# which is wrong.  Ditto for Escape, Return, and Tab.

bind ComboButton <Alt-KeyPress> {# nothing}
bind ComboButton <Meta-KeyPress> { puts %K }
bind ComboButton <Control-KeyPress> {# nothing}
bind ComboButton <Escape> {# nothing}
#bind ComboButton <KP_Enter> {# nothing}
bind ComboButton <Tab> {# nothing}
if {[string equal [tk windowingsystem] "classic"] || 
    [string equal [tk windowingsystem] "aqua"]} {
    bind ComboButton <Command-KeyPress> {# nothing}
}

namespace eval blt {
    variable Priv
    array set Priv {
	postedComboButton {}
	activeMenu {}
	inComboButton {}
    }
    array set Priv {
        activeMenu      {}
        activeItem      {}
        afterId         {}
        buttons         0
        buttonWindow    {}
        dragging        0
        focus           {}
        grab            {}
        initPos         {}
        inMenubutton    {}
        listboxPrev     {}
        menuBar         {}
        mouseMoved      0
        oldGrab         {}
        popup           {}
        popup           {}
        postedComboButton        {}
        pressX          0
        pressY          0
        prevPos         0
        selectMode      char
    }
}

proc blt::SaveGrab { w } {
    variable Priv

    set grab [grab current $w]
    set Priv(oldGrab) ""
    if { $grab != "" } {
	set type [grab status $grab]
	if { $type == "global" } {
	    #set Priv(oldGrab) [list grab set -global $grab]
	} else {
	    #set Priv(oldGrab) [list grab set $grab]
	}	    
    } 
}

# ::blt::RestoreOldGrab --
# Restores the grab to what it was before TkSaveGrabInfo was called.
#

proc ::blt::RestoreOldGrab {} {
    variable Priv

    if { $Priv(oldGrab) != "" } {
    	# Be careful restoring the old grab, since it's window may not
	# be visible anymore.
	catch $Priv(oldGrab)
	set Priv(oldGrab) ""
    }
}

# ::blt::ComboButtonPost --
# Given a menubutton, this procedure does all the work of posting
# its associated menu and unposting any other menu that is currently
# posted.
#
# Arguments:
# w -			The name of the menubutton widget whose menu
#			is to be posted.
# x, y -		Root coordinates of cursor, used for positioning
#			option menus.  If not specified, then the center
#			of the menubutton is used for an option menu.

proc ::blt::ComboButtonPost { w } {
    variable Priv

    puts stderr "proc ComboButtonPost $w, state=[$w cget -state]"
    if { [$w cget -state] == "disabled" } {
	return
    }
    if { [$w cget -state] == "posted" } {
	ComboButtonUnpost $w
	return
    } 
    set menu [$w cget -menu]
    if { $menu == "" } {
	return
    }
    set cur $Priv(postedComboButton)
    if { $cur != "" } {
	#
	ComboButtonUnpost $cur
    }
    set Priv(cursor) [$w cget -cursor]
    $w configure -cursor arrow
    
    set Priv(postedComboButton) $w
    set Priv(focus) [focus]
    $menu activate none
    GenerateMenuSelect $menu


    # If this looks like an option menubutton then post the menu so
    # that the current entry is on top of the mouse.  Otherwise post
    # the menu just below the menubutton, as for a pull-down.

    update idletasks
    if { [catch { $w post } msg] != 0 } {
	# Error posting menu (e.g. bogus -postcommand). Unpost it and
	# reflect the error.
	global errorInfo
	set savedInfo $errorInfo
	#
	ComboButtonUnpost $w 
	error $msg $savedInfo
    }

    focus $menu
    if { [winfo viewable $menu] } {
	SaveGrab $menu
	puts stderr "setting global grab on $menu"
	#grab -global $menu
    }
}

# ::blt::ComboButtonUnpost --
# This procedure unposts a given menu, plus all of its ancestors up
# to (and including) a menubutton, if any.  It also restores various
# values to what they were before the menu was posted, and releases
# a grab if there's a menubutton involved.  Special notes:
# 1. It's important to unpost all menus before releasing the grab, so
#    that any Enter-Leave events (e.g. from menu back to main
#    application) have mode NotifyGrab.
# 2. Be sure to enclose various groups of commands in "catch" so that
#    the procedure will complete even if the menubutton or the menu
#    or the grab window has been deleted.
#
# Arguments:
# menu -		Name of a menu to unpost.  Ignored if there
#			is a posted menubutton.

proc ::blt::ComboButtonUnpost { w } {
    variable Priv

    puts stderr "proc ComboButtonUnpost $w"
    catch { 
	# Restore focus right away (otherwise X will take focus away when the
	# menu is unmapped and under some window managers (e.g. olvwm) we'll
	# lose the focus completely).
	focus $Priv(focus) 
    }
    set Priv(focus) ""

    # Unpost menu(s) and restore some stuff that's dependent on what was
    # posted.
    puts stderr "unposting $w"
    $w unpost
    set Priv(postedComboButton) {}
    if { [info exists Priv(cursor)] } {
	$w configure -cursor $Priv(cursor)
    }
    if { [$w cget -state] != "disabled" } {
	#$w configure -state normal
    }
    set menu [$w cget -menu]
    puts MENU=$menu
    puts GRAB=[grab current $menu]
    # Release grab, if any, and restore the previous grab, if there
    # was one.
    if { $menu != "" } {
	set grab [grab current $menu]
	if { $grab != "" } {
	    grab release $grab
	}
    }
    RestoreOldGrab
}

proc ::blt::GenerateMenuSelect {menu} {
    if 0 {
    variable Priv
    if { $Priv(activeComboMenu) != $menu ||
	 $Priv(activeItem) != [$menu index active] } {
	set Priv(activeComboMenu) $menu
	set Priv(activeItem) [$menu index active]
	event generate $menu <<MenuSelect>>
    }
    }
}

