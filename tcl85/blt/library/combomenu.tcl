
namespace eval blt {
    variable Priv
    array set Priv {
	postedMb {}
	activeComboMenu {}
	inComboButton {}
    }
    array set Priv {
        activeComboMenu      {}
        activeItem      {}
        afterId         {}
        buttons         0
        buttonWindow    {}
        dragging        0
        focus           {}
        grab            {}
        initPos         {}
        inComboMenubutton    {}
        listboxPrev     {}
        menuBar         {}
        mouseMoved      0
        oldGrab         {}
        popup           {}
        popup           {}
        postedMb        {}
        pressX          0
        pressY          0
        prevPos         0
        selectMode      char
    }
}

rename focus focus.orig
proc focus { args } {
    puts stderr "focus $args"
    eval focus.orig $args
}

# -----------------------------------------------------------------------------

# Must set focus when mouse enters a menu, in order to allow
# mixed-mode processing using both the mouse and the keyboard.
# Don't set the focus if the event comes from a grab release,
# though:  such an event can happen after as part of unposting
# a cascaded chain of menus, after the focus has already been
# restored to wherever it was before menu selection started.

bind ComboMenu <FocusIn> {}

bind ComboMenu <Enter> { 
    puts stderr "ComboMenu %W <Enter>"
    focus %W
}

bind ComboMenu <Leave> { 
    puts stderr "ComboMenu %W <Leave> %s"
    if { %s == 0 } {
	#%W activate none 
    }
}

bind ComboMenu <Motion> { 
    #puts stderr "ComboMenu Motion %x,%y"
    %W activate @%x,%y 
    %W postcascade active
}

bind ComboMenu <ButtonRelease-1> { 
    puts stderr "ComboMenu %W at %x,%y ButtonRelease-1"
    set index [%W index @%x,%y]
    if { $index == -1 } {
	puts stderr "failed to find item in %W"
	event generate [grab current] <ButtonRelease-1> 
    } else {
	%W activate $index
	if { [%W type active] == "cascade" } {
	    %W postcascade active
	    blt::ComboMenuPost %W active
	} else {
	    %W invoke active
	    event generate [grab current] <ButtonRelease-1> 
	}
    }
}

bind ComboMenu <ButtonPress-1> { 
    puts stderr "ComboMenu ButtonPress-1"
}


bind ComboMenu <ButtonPress-2> { 
    set w [grab current]
    $w configure -cursor diamond_cross
    update
    %W scan mark %x %y
}
bind ComboMenu <B2-Motion> { 
    %W scan dragto %x %y
}

bind ComboMenu <ButtonRelease-2> { 
    set w [grab current]
    $w configure -cursor arrow
}

bind ComboMenu <KeyPress-space> {
    if { [%W type active] == "cascade" } {
	%W postcascade active
	blt::ComboMenuPost %W active
    } else {
	%W invoke active
	event generate [grab current] <ButtonRelease-1> 
    }
}

bind ComboMenu <KeyPress> {
    if { [string compare %A {}] == 0 } {
	continue
    }
    set index [%W find underline %A]
    if { $index > 0 } {
	%W activate $index
    }
}

bind ComboMenu <KeyPress-Return> {
    if { [%W type active] == "cascade" } {
	%W postcascade active
	blt::ComboMenuPost %W active
    } else {
	%W invoke active
	event generate [grab current] <ButtonRelease-1> 
    }
}

bind ComboMenu <Escape> {
    event generate [grab current] <ButtonRelease-1> 
}

bind ComboMenu <Left> {
    set menu [winfo parent %W]
    if { [winfo class $menu] == "ComboMenu" } {
	$menu postcascade none
	focus $menu
    }
}

bind ComboMenu <Right> {
    if { [%W type active] == "cascade" } {
	%W postcascade active
	set menu [%W item cget active -menu]
	focus $menu
    }
}

bind ComboMenu <KeyPress-Up> {
    %W activate previous
    %W see active
}

bind ComboMenu <KeyPress-Down> {
    %W activate next
    %W see active
}

bind ComboMenu <KeyPress-Home> {
    %W activate first
    %W see active
}

bind ComboMenu <KeyPress-End> {
    %W activate end
    %W see active
}

bind ComboMenu <KeyPress-Prior> {
    %W yview scroll -1 page
    %W activate view.top
    %W see active
}

bind ComboMenu <KeyPress-Next> {
    %W yview scroll 1 page
    %W activate view.bottom
    %W see active
}

# The following bindings apply to all windows, and are used to
# implement keyboard menu traversal.

if { [tk windowingsystem] == "x11" } {
    bind all <Alt-KeyPress> {
	blt::TraverseToComboMenu %W %A
    }
    bind all <F10> {
	blt::FirstComboMenu %W
    }
} else {
    bind ComboMenubutton <Alt-KeyPress> {
	blt::TraverseToComboMenu %W %A
    }

    bind ComboMenubutton <F10> {
	blt::FirstComboMenu %W
    }
}

if { [tk windowingsystem] == "x11" } {
    bind ComboMenu <4> {
	%W yview scroll -5 units
    }
    bind ComboMenu <5> {
	%W yview scroll 5 units
    }
} else {
    bind ComboMenu <MouseWheel> {
	%W yview scroll [expr {- (%D / 120) * 4}] units
    }
}

# ::blt::ComboMenuUnpost --
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

proc ::blt::ComboMenuUnpost menu {
    variable Priv

    set mb $Priv(postedMb)

    # Restore focus right away (otherwise X will take focus away when
    # the menu is unmapped and under some window managers (e.g. olvwm)
    # we'll lose the focus completely).

    catch {focus $Priv(focus)}
    set Priv(focus) ""

    # Unpost menu(s) and restore some stuff that's dependent on
    # what was posted.

    catch {
	if {[string compare $mb ""]} {
	    set menu [$mb cget -menu]
	    $menu unpost
	    set Priv(postedMb) {}
	    $mb configure -cursor $Priv(cursor)
	    if  { [$mb cget -buttonrelief] == "sunken" } {
		$mb configure -buttonrelief $Priv(relief)
	    }
	} elseif {[string compare $Priv(popup) ""]} {
	    $Priv(popup) unpost
	    set Priv(popup) {}
	} else {
	    # We're in a cascaded sub-menu from a torn-off menu or popup.
	    # Unpost all the menus up to the toplevel one (but not
	    # including the top-level torn-off one) and deactivate the
	    # top-level torn off menu if there is one.

	    while {1} {
		set parent [winfo parent $menu]
		if { [winfo class $parent] != "ComboMenu"] || 
			![winfo ismapped $parent]} {
		    break
		}
		$parent activate none
		$parent postcascade none
		ComboMenuGenerateSelect $parent
		set menu $parent
	    }
	    $menu unpost
	}
    }
    if { $mb != "" && [$mb cget -state] != "disabled" } {
	$mb configure -state normal
    }

    puts tearoff=$Priv(tearoff)
    puts menuBar=$Priv(menuBar)
    puts menu=$menu
    puts grab=[grab current $menu]
    if {($Priv(tearoff) != 0) || $Priv(menuBar) ne ""} {
    	# Release grab, if any, and restore the previous grab, if there
    	# was one.
	if {[string compare $menu ""]} {
	    set grab [grab current $menu]
	    if {[string compare $grab ""]} {
		#grab release $grab
	    }
	}
	RestoreOldGrab
	if {$Priv(menuBar) ne ""} {
	    $Priv(menuBar) configure -cursor $Priv(cursor)
	    set Priv(menuBar) {}
	}
	if {[tk windowingsystem] ne "x11"} {
	    set Priv(tearoff) 0
	}
    }
}

proc ::blt::ComboMenuPost { w item } {
    variable Priv

    puts stderr "proc ComboMenuPost $w, item=$item"
    if { [$w item cget $item -state] == "disabled" } {
	return
    }

    set menu [$w item cget $item -menu]
    if { $menu == "" } {
	return
    }
    #$menu activate none
    set Priv(focus) [focus]
    GenerateMenuSelect $menu


    # If this looks like an option menubutton then post the menu so
    # that the current entry is on top of the mouse.  Otherwise post
    # the menu just below the menubutton, as for a pull-down.

    update idletasks
    if { [catch { $w postcascade $item } msg] != 0 } {
	# Error posting menu (e.g. bogus -postcommand). Unpost it and
	# reflect the error.
	global errorInfo
	set savedInfo $errorInfo
	#
	#MbUnpost $w 
	error $msg $savedInfo
    }
    focus $menu
    $menu activate first
    if { [winfo viewable $menu] } {
	SaveGrab $menu
	puts stderr "setting global grab on $menu"
	#grab -global $menu
    }
}

# ::blt::xxxComboMenuUnpost --
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

proc ::blt::xxxComboMenuUnpost { w } {
    variable Priv

    puts stderr "proc ComboMenuUnpost $w"
    catch { 
	# Restore focus right away (otherwise X will take focus away when the
	# menu is unmapped and under some window managers (e.g. olvwm) we'll
	# lose the focus completely).
	focus $Priv(focus) 
    }
    set Priv(focus) ""

    # Unpost menu(s) and restore some stuff that's dependent on what was
    # posted.

    $w unpost
    set Priv(postedMb) {}
    $w configure -cursor $Priv(cursor)

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
	    #grab release $grab
	}
    }
    RestoreOldGrab
}

# ::blt::SaveGrab --
# Sets the variable blt::Priv(oldGrab) to record
# the state of any existing grab on the w's display.
#
# Arguments:
# w -			Name of a window;  used to select the display
#			whose grab information is to be recorded.

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

    puts stderr "RestoreOldGrab: current grab=[grab current]"
    if { [info exists $Priv(oldGrab)] } {
	puts stderr "restore old=$Priv(oldGrab)"
    }
    if { $Priv(oldGrab) != "" } {
    	# Be careful restoring the old grab, since it's window may not
	# be visible anymore.
	catch $Priv(oldGrab)
	set Priv(oldGrab) ""
    } else {
	grab release [grab current]
    }	
}

proc ::blt::xxxComboMenuSetFocus {menu} {
    variable Priv
    if { ![info exists Priv(focus)] || $Priv(focus) == "" } {
	set Priv(focus) [focus]
    }
    focus $menu
}
    
proc ::blt::ComboMenuGenerateSelect {menu} {
    variable Priv

    if {[string equal $Priv(activeComboMenu) $menu] \
          && [string equal $Priv(activeItem) [$menu index active]]} {
	return
    }
    set Priv(activeComboMenu) $menu
    set Priv(activeItem) [$menu index active]
    event generate $menu <<MenuSelect>>
}
