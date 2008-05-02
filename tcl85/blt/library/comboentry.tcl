
namespace eval blt {
    set Priv(afterId) -1
    set Priv(b1) ""
}

proc blt::ComboEntryAutoScan {w} {
    variable Priv

    set x $Priv(x)
    if { ![winfo exists $w] } {
	return
    }
    if { $x >= [winfo width $w] } {
	$w xview scroll 2 units
    } elseif { $x < 0 } {
	$w xview scroll -2 units
    }
    set Priv(afterId) [after 50 [list blt::ComboEntryAutoScan $w]]
}

bind ComboEntry <Enter> {
}

bind ComboEntry <Leave> {
    %W activate off
}

# Standard Motif bindings:

bind ComboEntry <Motion> {
    if { [%W identify %x %y] == "button" } {
	%W activate on
    } else {
	%W activate off
    }
}

bind ComboEntry <ButtonPress-1> {
    puts stderr "ComboEntry %W ButtonPress-1 state=[%W cget -state]"
    set blt::Priv(b1) [%W identify %x %y]
    if { [%W cget -state] == "posted" } {
	%W unpost
	blt::MbUnpost %W
    } elseif { $blt::Priv(b1) == "button" } {
	set blt::Priv(relief) [%W cget -buttonrelief]
	puts stderr "relief=$blt::Priv(relief)"
	if { [%W cget -state] != "disabled" } {
	    %W configure -buttonrelief sunken
	}
	grab -global %W
	blt::MbPost %W
    } else {
	focus %W
	%W icursor [%W closest %x]
	%W selection clear
	%W selection from insert
	#%W configure -buttonrelief $blt::Priv(relief)
    }
}

bind ComboEntry <ButtonRelease-1> {
    set x [expr [winfo pointerx %W] - [winfo rootx %W]]
    set y [expr [winfo pointery %W] - [winfo rooty %W]]
    puts stderr "ComboEntry %W at %X,%Y <ButtonRelease-1> state=[%W cget -state], grab=[grab current]"
    after cancel $blt::Priv(afterId)
    if { [%W identify $x $y] == "button" } {
	puts stderr "invoke"
	%W invoke
    } else { 
	puts stderr "unpost"
	blt::MbUnpost %W
    }	
    if { [info exists blt::Priv(relief)] } {
	%W configure -buttonrelief $blt::Priv(relief)
    }
}

bind ComboEntry <B1-Motion> {
    if { $blt::Priv(b1) != "button" } {
	%W selection to [%W closest %x]
    }
}

bind ComboEntry <B1-Enter> {
    if { $blt::Priv(afterId) != "" } {
	after cancel $blt::Priv(afterId)
    }
}

bind ComboEntry <B1-Leave> {
    puts stderr "ComboEntry B1-Leave"
    if { $blt::Priv(b1) == "text" } {
	set blt::Priv(x) %x
	blt::ComboEntryAutoScan %W
    }
}


bind ComboEntry <KeyPress-Down> {
    if { [%W cget -state] != "disabled" } {
	blt::MbPost %W
    }
}


bind ComboEntry <Double-1> {
    puts stderr "Double-1"
    if { [%W identify %x %y] != "button" } {
	%W icursor [%W closest %x]
	%W selection range \
	    [string wordstart [%W get] [%W index previous]] \
	    [string wordend   [%W get] [%W index insert]]
	%W icursor sel.last
	%W see insert
    }
}

bind ComboEntry <Triple-1> {
    puts stderr "Triple-1"
    if { [%W identify %x %y] != "button" } {
	%W selection range 0 end
	%W icursor sel.last
    }
}

bind ComboEntry <Shift-1> {
    %W selection adjust @%x
}


bind ComboEntry <ButtonPress-2> {
    %W scan mark %x
}

bind ComboEntry <ButtonRelease-2> {
    if { abs([%W scan mark] - %x) <= 3 } {
	catch { 
	    %W insert insert [selection get]
	    %W see insert
	}
    }
}

bind ComboEntry <B2-Motion> {
    %W scan dragto %x
}

bind ComboEntry <Control-1> {
    %W icursor @%x
}

bind ComboEntry <KeyPress-Left> {
    if { [%W selection present] } {
	%W icursor sel.last
	%W selection clear
    } 
    %W icursor previous
    %W see insert
}

bind ComboEntry <KeyPress-Right> {
    if { [%W selection present] } {
	%W icursor sel.last
	%W selection clear
    } 
    %W icursor next
    %W see insert
}

bind ComboEntry <Shift-Left> {
    if {![%W selection present]} {
	%W selection range previous insert 
    } else {
	%W selection adjust previous
    }
    %W icursor previous
    %W see insert
}

bind ComboEntry <Shift-Right> {
    if {![%W selection present]} {
	%W selection range insert next
    } else {
	%W selection adjust next
    }
    %W icursor next
    %W see insert
}

bind ComboEntry <Shift-Control-Left> {
    set previous [string wordstart [%W get] [%W index previous]]
    if {![%W selection present]} {
	%W selection range $previous insert 
    } else {
	%W selection adjust $previous
    }
    %W icursor $previous
    %W see insert
}

bind ComboEntry <Shift-Control-Right> {
    set next [string wordend [%W get] [%W index insert]]
    if {![%W selection present]} {
	%W selection range insert $next
    } else {
	%W selection adjust $next
    }
    %W icursor $next
    %W see insert
}

bind ComboEntry <Home> {
    %W icursor 0
    %W see insert
}

bind ComboEntry <Shift-Home> {
    if {![%W selection present]} {
	%W selection range 0 insert
    } else {
	%W selection adjust 0
    }
    %W icursor 0
    %W see insert
}

bind ComboEntry <End> {
    %W icursor end
    %W see insert
}

bind ComboEntry <Shift-End> {
    if {![%W selection present]} {
	%W selection range insert end
    } else {
	%W selection adjust end
    }
    %W icursor end
    %W see insert
}

bind ComboEntry <Delete> {
    if {[%W selection present]} {
	%W delete sel.first sel.last
    } else {
	%W delete insert next
    }
}

bind ComboEntry <BackSpace> {
    if {[%W selection present]} {
	%W delete sel.first sel.last
    } else {
	%W delete previous insert 
	%W see insert
    }
}

bind ComboEntry <Control-space> {
    %W selection from insert
}

bind ComboEntry <Select> {
    %W selection from insert
}

bind ComboEntry <Control-Shift-space> {
    %W selection adjust insert
}

bind ComboEntry <Shift-Select> {
    %W selection adjust insert
}

bind ComboEntry <Control-slash> {
    %W selection range 0 end
}

bind ComboEntry <Control-backslash> {
    %W selection clear
}

bind ComboEntry <Control-z> {
    %W undo
    %W see insert
}

bind ComboEntry <Control-Z> {
    %W redo
    %W see insert
}

bind ComboEntry <Control-y> {
    %W redo
    %W see insert
}

bind ComboEntry <<Cut>> {
    if { [%W selection present] } {
	clipboard clear -displayof %W
	clipboard append -displayof %W [selection get]
	%W delete sel.first sel.last
    }
}

bind ComboEntry <<Copy>> {
    if { [%W selection present] } {
	clipboard clear -displayof %W
	clipboard append -displayof %W [selection get]
    }
}

bind ComboEntry <<Paste>> {
    %W insert insert [selection get]
    %W see insert
}

bind ComboEntry <<Clear>> {
    %W delete sel.first sel.last
}


bind Entry <<PasteSelection>> {
    if {$tk_strictMotif || ![info exists tk::Priv(mouseMoved)]
	|| !$tk::Priv(mouseMoved)} {
	tk::EntryPaste %W %x
    }
}

# Paste
bind ComboEntry <Control-v> {
    %W insert insert [selection get]
}

# Cut
bind ComboEntry <Control-x> {
    if { [%W selection present] } {
	clipboard clear -displayof %W
	clipboard append -displayof %W [selection get]
	%W delete sel.first sel.last
    }
}
# Copy
bind ComboEntry <Control-c> {
    if { [%W selection present] } {
	clipboard clear -displayof %W
	clipboard append -displayof %W [selection get]
    }
}

bind ComboEntry <Return> {
    %W invoke 
}

bind ComboEntry <KeyPress> {
    if { [string compare %A {}] == 0 } {
	continue
    }
    if { [%W selection present] } {
	%W delete sel.first sel.last
    }
    %W insert insert %A
    %W see insert
}

# Additional emacs-like bindings:

bind ComboEntry <Control-a> {
    %W icursor 0
    %W see insert
}

bind ComboEntry <Control-b> {
    %W icursor previous 
    %W see insert
}

bind ComboEntry <Control-d> {
    if {[%W selection present]} {
	%W delete sel.first sel.last
    } else {
	%W delete insert next
    }
}

bind ComboEntry <Control-e> {
    %W icursor end
    %W see insert
}

bind ComboEntry <Control-f> {
    %W icursor next
    %W see insert
}

bind ComboEntry <Control-h> {
    if {[%W selection present]} {
	%W delete sel.first sel.last
    } else {
	%W delete previous insert 
	%W see insert
    }
}

bind ComboEntry <Control-k> {
    %W delete insert end
}

bind ComboEntry <Control-t> {
    set index [%W index insert]
    if { $index != 0 && $index != [%W index end] } {
	set a [string index [%W get] [%W index previous]]
	set b [string index [%W get] [%W index insert]]
	%W delete previous next
	%W insert insert "$b$a"
    }
}

bind ComboEntry <Alt-b> {
    %W icursor [string wordstart [%W get] [%W index previous]]
    %W see insert
}

bind ComboEntry <Alt-d> {
    %W delete insert [string wordend [%W get] [%W index insert]]
    %W see insert
}

bind ComboEntry <Alt-f> {
    %W icursor [string wordend [%W get] [%W index insert]]
    %W see insert
}

bind ComboEntry <Alt-BackSpace> {
    %W delete [string wordstart [%W get] [%W index previous]] insert
    %W see insert
}

bind ComboEntry <Alt-Delete> {
    %W delete insert [string wordend [%W get] [%W index insert]]
    %W see insert
}

####

bind ComboEntry <Meta-b> {
    %W icursor [string wordstart [%W get] [%W index previous]]
    %W see insert
}

bind ComboEntry <Meta-d> {
    %W delete insert [string wordend [%W get] [%W index insert]]
    %W see insert
}

bind ComboEntry <Meta-f> {
    %W icursor [string wordend [%W get] [%W index insert]]
    %W see insert
}

bind ComboEntry <Meta-BackSpace> {
    %W delete [string wordstart [%W get] [%W index previous]] insert
    %W see insert
}

bind ComboEntry <Meta-Delete> {
    %W delete insert [string wordend [%W get] [%W index insert]]
    %W see insert
}


# Ignore all Alt, Meta, and Control keypresses unless explicitly bound.
# Otherwise, if a widget binding for one of these is defined, the
# <KeyPress> class binding will also fire and insert the character,
# which is wrong.  Ditto for Escape, Return, and Tab.

bind ComboEntry <Alt-KeyPress> {# nothing}
bind ComboEntry <Meta-KeyPress> { puts %K }
bind ComboEntry <Control-KeyPress> {# nothing}
bind ComboEntry <Escape> {# nothing}
#bind ComboEntry <KP_Enter> {# nothing}
bind ComboEntry <Tab> {# nothing}
if {[string equal [tk windowingsystem] "classic"] || 
    [string equal [tk windowingsystem] "aqua"]} {
    bind ComboEntry <Command-KeyPress> {# nothing}
}


namespace eval blt {
    variable Priv
    array set Priv {
	postedMb {}
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
        postedMb        {}
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

# ::blt::MbPost --
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

proc ::blt::MbPost { w } {
    variable Priv

    puts stderr "proc MbPost $w, state=[$w cget -state]"
    if { [$w cget -state] == "disabled" } {
	return
    }
    if { [$w cget -state] == "posted" } {
	MbUnpost $w
	return
    } 
    set menu [$w cget -menu]
    if { $menu == "" } {
	return
    }
    set cur $Priv(postedMb)
    if { $cur != "" } {
	#
	MbUnpost $cur
    }
    set Priv(cursor) [$w cget -cursor]
    $w configure -cursor arrow
    
    set Priv(postedMb) $w
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
	MbUnpost $w 
	error $msg $savedInfo
    }

    focus $menu
    if { [winfo viewable $menu] } {
	SaveGrab $menu
	puts stderr "setting global grab on $menu"
	#grab -global $menu
    }
}

# ::blt::MbUnpost --
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

proc ::blt::MbUnpost { w } {
    variable Priv

    puts stderr "proc MbUnpost $w"
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

