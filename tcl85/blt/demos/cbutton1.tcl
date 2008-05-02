
set imgData {
    R0lGODlhEAANAMIAAAAAAH9/f///////AL+/vwAA/wAAAAAAACH5BAEAAAUALAAAAAAQAA0A
    AAM8WBrM+rAEQWmIb5KxiWjNInCkV32AJHRlGQBgDA7vdN4vUa8tC78qlrCWmvRKsJTquHkp
    ZTKAsiCtWq0JADs=
}

#set icon [image create picture -data $imgData]
set icon [image create picture -file ~/src/blt/demos/images/mini-book1.gif]
#set icon [image create picture -file ~/src/blt/demos/images/blt98.gif]
#set image [image create picture -file ~/images.jpeg]
set bg [blt::bgstyle create gradient -high  grey70 -low grey95 \
	-jitter yes -log yes -relativeto self]

set image ""

set t "Hello, World"
blt::combobutton .b1 \
    -postedbackground grey \
    -postedforeground black \
    -activebackground grey72 \
    -activeforeground black \
    -textvariable t \
    -image $image \
    -iconvariable icon \
    -bg $bg \
    -postcommand { [.b1 cget -menu] configure -bg green } \
    -font { Arial 10 bold } -justify left \
    -underline 1 \
    -borderwidth 2 \
    -indicatoron yes \
    -menuanchor sw \
    -menu .b1.m

option add *ComboButton.Menu.background cyan2

blt::combobutton .b2 \
    -postedbackground grey \
    -postedforeground black \
    -activebackground grey72 \
    -activeforeground black \
    -textvariable t \
    -image $image \
    -iconvariable icon \
    -bg $bg \
    -font { Arial 10 bold } -justify left \
    -underline 1 \
    -borderwidth 2 \
    -indicatoron no \
    -menuanchor sw \
    -menu .b2.m

blt::table . \
    1,0 .b1 -fill both \
    1,1 .b2 -fill both \

menu .b1.m  -tearoff no
.b1.m add command -label "one" -accelerator "^A" -command "set t one"
.b1.m add command -label "two" -accelerator "^B" -command "set t two"
.b1.m add command -label "three" -accelerator "^C" -command "set t three"
.b1.m add command -label "four" -accelerator "^D" -command "set t four"
.b1.m add cascade -label "cascade" -accelerator "^E" -menu .b1.m.m

menu .b1.m.m -tearoff no
.b1.m.m add command -label "five" -accelerator "^A" -command "set t five"
.b1.m.m add command -label "six" -accelerator "^B" -command "set t six"
.b1.m.m add command -label "seven" -accelerator "^C" -command "set t seven"
.b1.m.m add command -label "eight" -accelerator "^D" -command "set t eight"
.b1.m.m add cascade -label "cascade" -accelerator "^E" 

menu .b2.m  -tearoff no
.b2.m add command -label "one" -accelerator "^A" -command "set t one"
.b2.m add command -label "two" -accelerator "^B" -command "set t two"
.b2.m add command -label "three" -accelerator "^C" -command "set t three"
.b2.m add command -label "four" -accelerator "^D" -command "set t four"
.b2.m add cascade -label "cascade" -accelerator "^E" -menu .b2.m.m

menu .b2.m.m -tearoff no
.b2.m.m add command -label "five" -accelerator "^A" -command "set t five"
.b2.m.m add command -label "six" -accelerator "^B" -command "set t six"
.b2.m.m add command -label "seven" -accelerator "^C" -command "set t seven"
.b2.m.m add command -label "eight" -accelerator "^D" -command "set t eight"
.b2.m.m add cascade -label "cascade" -accelerator "^E" 

focus .b1

namespace eval zzz {
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

#-------------------------------------------------------------------------
# The code below creates the default class bindings for menus
# and menubuttons.
#-------------------------------------------------------------------------

bind ComboButton <FocusIn> {}
bind ComboButton <Enter> {
    zzz::MbEnter %W
}
bind ComboButton <Leave> {
    zzz::MbLeave %W
}

bind ComboButton <1> {
    if { $zzz::Priv(inComboButton) !=  "" } {
	zzz::MbPost $zzz::Priv(inComboButton) %X %Y
    }
}
bind ComboButton <Motion> {
    zzz::MbMotion %W up %X %Y
}
bind ComboButton <B1-Motion> {
    zzz::MbMotion %W down %X %Y
}
bind ComboButton <ButtonRelease-1> {
    zzz::MbButtonUp %W
}

bind ComboButton <space> {
    zzz::MbPost %W
    zzz::MenuFirstEntry [%W cget -menu]
}

bind ComboButton <3> {
    grab release [grab current]
}

# -----------------------------------------------------------------------------

# Must set focus when mouse enters a menu, in order to allow
# mixed-mode processing using both the mouse and the keyboard.
# Don't set the focus if the event comes from a grab release,
# though:  such an event can happen after as part of unposting
# a cascaded chain of menus, after the focus has already been
# restored to wherever it was before menu selection started.

bind Menu <FocusIn> {}

bind Menu <Enter> {
    set zzz::Priv(window) %W
    if { [%W cget -type] == "tearoff" } {
	if {"%m" ne "NotifyUngrab"} {
	    if {[tk windowingsystem] eq "x11"} {
		tk_menuSetFocus %W
	    }
	}
    }
    zzz::MenuMotion %W %x %y %s
}

bind Menu <Leave> {
    zzz::MenuLeave %W %X %Y %s
}
bind Menu <Motion> {
    zzz::MenuMotion %W %x %y %s
}
bind Menu <ButtonPress> {
    zzz::MenuButtonDown %W
}
bind Menu <ButtonRelease> {
   zzz::MenuInvoke %W 1
}
bind Menu <space> {
    zzz::MenuInvoke %W 0
}
bind Menu <Return> {
    zzz::MenuInvoke %W 0
}
bind Menu <Escape> {
    zzz::MenuEscape %W
}
bind Menu <Left> {
    zzz::MenuLeftArrow %W
}
bind Menu <Right> {
    zzz::MenuRightArrow %W
}
bind Menu <Up> {
    zzz::MenuUpArrow %W
}
bind Menu <Down> {
    zzz::MenuDownArrow %W
}
bind Menu <KeyPress> {
    zzz::TraverseWithinMenu %W %A
}

# The following bindings apply to all windows, and are used to
# implement keyboard menu traversal.

if { [tk windowingsystem] == "x11" } {
    bind all <Alt-KeyPress> {
	zzz::TraverseToMenu %W %A
    }
    bind all <F10> {
	zzz::FirstMenu %W
    }
} else {
    bind Menubutton <Alt-KeyPress> {
	zzz::TraverseToMenu %W %A
    }

    bind Menubutton <F10> {
	zzz::FirstMenu %W
    }
}

# ::zzz::MbEnter --
# This procedure is invoked when the mouse enters a menubutton
# widget.  It activates the widget unless it is disabled.  Note:
# this procedure is only invoked when mouse button 1 is *not* down.
# The procedure ::zzz::MbB1Enter is invoked if the button is down.
#
# Arguments:
# w -			The  name of the widget.

proc ::zzz::MbEnter { w }  {
    variable ::zzz::Priv

    if { $Priv(inComboButton) != "" } {
	# has to be a different menu button.
	MbLeave $Priv(inComboButton)
    }
    set Priv(inComboButton) $w
    if { [$w cget -state] == "normal" } {
	$w configure -state active
    }
}

# ::zzz::MbLeave --
# This procedure is invoked when the mouse leaves a menubutton widget.
# It de-activates the widget, if the widget still exists.
#
# Arguments:
# w -			The  name of the widget.

proc ::zzz::MbLeave w {
    variable ::zzz::Priv

    set Priv(inComboButton) {}
    if {![winfo exists $w]} {
	return
    }
    set state [$w cget -state]
    if { $state != "disabled" && $state != "posted" } {
	$w configure -state normal
    }
}

# ::zzz::MbPost --
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

proc ::zzz::MbPost { w {x {}} {y {}} } {
    global errorInfo
    variable ::zzz::Priv
    global tcl_platform

    if { [$w cget -state] == "disabled" || $w == $Priv(postedMb) } {
	return
    }
    set menu [$w cget -menu]
    if { $menu == "" } {
	return
    }
    set tearoff [expr {[tk windowingsystem] eq "x11" \
	    || [$menu cget -type] eq "tearoff"}]
    set cur $Priv(postedMb)
    if { $cur != "" } {
	#
	MbUnpost $cur
    }
    set Priv(cursor) [$w cget -cursor]
    if { [$w cget -indicatoron] } {
	set Priv(relief) [$w cget -indicatorrelief]
	$w configure -indicatorrelief sunken
    } else {
	set Priv(relief) [$w cget -relief]
	$w configure -relief raised
    }
    $w configure -cursor arrow
    
    set Priv(postedMb) $w
    set Priv(focus) [focus]
    $menu activate none
    GenerateMenuSelect $menu

    if { [$w cget -state] != "disabled" } {
	$w configure -state posted
    }

    # If this looks like an option menubutton then post the menu so
    # that the current entry is on top of the mouse.  Otherwise post
    # the menu just below the menubutton, as for a pull-down.

    update idletasks
    if { [catch { $w post $x $y } msg] != 0 } {
	# Error posting menu (e.g. bogus -postcommand). Unpost it and
	# reflect the error.
	set savedInfo $errorInfo
	#
	MbUnpost $w 
	error $msg $savedInfo
    }

    set Priv(tearoff) $tearoff
    if {$tearoff != 0} {
    	focus $menu
	if {[winfo viewable $w]} {
	    SaveGrabInfo $w
	    grab -global $w
	}
    }
}

# ::zzz::MbUnpost --
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

proc ::zzz::MbUnpost { mb } {
    global tcl_platform
    variable ::zzz::Priv

    # Restore focus right away (otherwise X will take focus away when
    # the menu is unmapped and under some window managers (e.g. olvwm)
    # we'll lose the focus completely).

    catch { focus $Priv(focus) }
    set Priv(focus) ""

    # Unpost menu(s) and restore some stuff that's dependent on
    # what was posted.

    $mb unpost
    set Priv(postedMb) {}
    $mb configure -cursor $Priv(cursor)
    if { [$mb cget -indicatoron] } {
	$mb configure -indicatorrelief $Priv(relief)
    } else {
	$mb configure -relief $Priv(relief)
    }
    if { [$mb cget -state] != "disabled" } {
	$mb configure -state normal
    }

    set menu [$mb cget -menu]
    puts TEAROFF=$Priv(tearoff)
    puts MENUBAR=$Priv(menuBar)
    puts MENU=$menu
    puts GRAB=[grab current $menu]
    if { $Priv(tearoff) != 0 } {
    	# Release grab, if any, and restore the previous grab, if there
    	# was one.
	if { $menu != "" } {
	    set grab [grab current $menu]
	    if { $grab != "" } {
		grab release $grab
	    }
	}
	RestoreOldGrab
	if {[tk windowingsystem] != "x11"} {
	    set Priv(tearoff) 0
	}
    }
}

# ::zzz::MenuUnpost --
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

proc ::zzz::MenuUnpost menu {
    global tcl_platform
    variable ::zzz::Priv
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
	    if { [$mb cget -indicatoron] } {
		$mb configure -indicatorrelief $Priv(relief)
	    } else {
		$mb configure -relief $Priv(relief)
	    }
	} elseif {[string compare $Priv(popup) ""]} {
	    $Priv(popup) unpost
	    set Priv(popup) {}
	} elseif {[string compare [$menu cget -type] "menubar"] \
		&& [string compare [$menu cget -type] "tearoff"]} {
	    # We're in a cascaded sub-menu from a torn-off menu or popup.
	    # Unpost all the menus up to the toplevel one (but not
	    # including the top-level torn-off one) and deactivate the
	    # top-level torn off menu if there is one.

	    while {1} {
		set parent [winfo parent $menu]
		if {[string compare [winfo class $parent] "Menu"] \
			|| ![winfo ismapped $parent]} {
		    break
		}
		$parent activate none
		$parent postcascade none
		GenerateMenuSelect $parent
		set type [$parent cget -type]
		if {[string equal $type "menubar"] || \
			[string equal $type "tearoff"]} {
		    break
		}
		set menu $parent
	    }
	    if {[string compare [$menu cget -type] "menubar"]} {
		$menu unpost
	    }
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
		grab release $grab
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

# ::zzz::MbMotion --
# This procedure handles mouse motion events inside menubuttons, and
# also outside menubuttons when a menubutton has a grab (e.g. when a
# menu selection operation is in progress).
#
# Arguments:
# w -			The name of the menubutton widget.
# upDown - 		"down" means button 1 is pressed, "up" means
#			it isn't.
# rootx, rooty -	Coordinates of mouse, in (virtual?) root window.

proc ::zzz::MbMotion {w upDown rootx rooty} {
    variable ::zzz::Priv

    if { $Priv(inComboButton) == "$w" } {
	return
    }
    set new [winfo containing $rootx $rooty]
    if { $new != $Priv(inComboButton) && 
	 ($new == "" || [winfo toplevel $new] == [winfo toplevel $w])
     } {
	if { $Priv(inComboButton) != "" } {
	    MbLeave $Priv(inComboButton)
	}
	if { $new != "" && [winfo class $new] == "ComboButton" && 
	     ![$new cget -indicatoron] && ![$w cget -indicatoron] 
	 } {
	    if { $upDown == "down" } {
		MbPost $new $rootx $rooty
	    } else {
		MbEnter $new
	    }
	}
    }
}

# ::zzz::MbButtonUp --
# This procedure is invoked to handle button 1 releases for menubuttons.
# If the release happens inside the menubutton then leave its menu
# posted with element 0 activated.  Otherwise, unpost the menu.
#
# Arguments:
# w -			The name of the menubutton widget.

proc ::zzz::MbButtonUp w {
    variable ::zzz::Priv
    global tcl_platform

    set menu [$w cget -menu]
    set tearoff [expr {[tk windowingsystem] eq "x11" || \
	    ($menu ne "" && [$menu cget -type] eq "tearoff")}]
    puts tearoff=$tearoff
    puts postedMb=$Priv(postedMb)
    puts inComboButton=$Priv(inComboButton)
    puts menu=$menu
    puts class=[winfo class $Priv(postedMb)]
    puts grab=[grab current]
    if { $tearoff != 0 && $Priv(postedMb) == $w && $Priv(inComboButton) == $w} {
	MenuFirstEntry [$Priv(postedMb) cget -menu]
    } else {
	MbUnpost $w
    }
}

# ::zzz::MenuMotion --
# This procedure is called to handle mouse motion events for menus.
# It does two things.  First, it resets the active element in the
# menu, if the mouse is over the menu.  Second, if a mouse button
# is down, it posts and unposts cascade entries to match the mouse
# position.
#
# Arguments:
# menu -		The menu window.
# x -			The x position of the mouse.
# y -			The y position of the mouse.
# state -		Modifier state (tells whether buttons are down).

proc ::zzz::MenuMotion {menu x y state} {
    variable ::zzz::Priv
    if {[string equal $menu $Priv(window)]} {
	if {[string equal [$menu cget -type] "menubar"]} {
	    if {[info exists Priv(focus)] && \
		    [string compare $menu $Priv(focus)]} {
		$menu activate @$x,$y
		GenerateMenuSelect $menu
	    }
	} else {
	    $menu activate @$x,$y
	    GenerateMenuSelect $menu
	}
    }
    if {($state & 0x1f00) != 0} {
	$menu postcascade active
    }
}

# ::zzz::MenuButtonDown --
# Handles button presses in menus.  There are a couple of tricky things
# here:
# 1. Change the posted cascade entry (if any) to match the mouse position.
# 2. If there is a posted menubutton, must grab to the menubutton;  this
#    overrrides the implicit grab on button press, so that the menu
#    button can track mouse motions over other menubuttons and change
#    the posted menu.
# 3. If there's no posted menubutton (e.g. because we're a torn-off menu
#    or one of its descendants) must grab to the top-level menu so that
#    we can track mouse motions across the entire menu hierarchy.
#
# Arguments:
# menu -		The menu window.

proc ::zzz::MenuButtonDown menu {
    variable ::zzz::Priv
    global tcl_platform

    if {![winfo viewable $menu]} {
        return
    }
    $menu postcascade active
    if {[string compare $Priv(postedMb) ""] && \
	    [winfo viewable $Priv(postedMb)]} {
	grab -global $Priv(postedMb)
    } else {
	while {[string equal [$menu cget -type] "normal"] \
		&& [string equal [winfo class [winfo parent $menu]] "Menu"] \
		&& [winfo ismapped [winfo parent $menu]]} {
	    set menu [winfo parent $menu]
	}

	if {[string equal $Priv(menuBar) {}]} {
	    set Priv(menuBar) $menu
	    set Priv(cursor) [$menu cget -cursor]
	    $menu configure -cursor arrow
        }

	# Don't update grab information if the grab window isn't changing.
	# Otherwise, we'll get an error when we unpost the menus and
	# restore the grab, since the old grab window will not be viewable
	# anymore.

	if {[string compare $menu [grab current $menu]]} {
	    SaveGrabInfo $menu
	}

	# Must re-grab even if the grab window hasn't changed, in order
	# to release the implicit grab from the button press.

	if {[string equal [tk windowingsystem] "x11"]} {
	    grab -global $menu
	}
    }
}

# ::zzz::MenuLeave --
# This procedure is invoked to handle Leave events for a menu.  It
# deactivates everything unless the active element is a cascade element
# and the mouse is now over the submenu.
#
# Arguments:
# menu -		The menu window.
# rootx, rooty -	Root coordinates of mouse.
# state -		Modifier state.

proc ::zzz::MenuLeave {menu rootx rooty state} {
    variable ::zzz::Priv
    set Priv(window) {}
    if {[string equal [$menu index active] "none"]} {
	return
    }
    if {[string equal [$menu type active] "cascade"]
          && [string equal [winfo containing $rootx $rooty] \
                  [$menu entrycget active -menu]]} {
	return
    }
    $menu activate none
    GenerateMenuSelect $menu
}

# ::zzz::MenuInvoke --
# This procedure is invoked when button 1 is released over a menu.
# It invokes the appropriate menu action and unposts the menu if
# it came from a menubutton.
#
# Arguments:
# w -			Name of the menu widget.
# buttonRelease -	1 means this procedure is called because of
#			a button release;  0 means because of keystroke.

proc ::zzz::MenuInvoke {w buttonRelease} {
    variable ::zzz::Priv

    if {$buttonRelease && [string equal $Priv(window) {}]} {
	# Mouse was pressed over a menu without a menu button, then
	# dragged off the menu (possibly with a cascade posted) and
	# released.  Unpost everything and quit.

	$w postcascade none
	$w activate none
	event generate $w <<MenuSelect>>
	MenuUnpost $w
	return
    }
    if {[string equal [$w type active] "cascade"]} {
	$w postcascade active
	set menu [$w entrycget active -menu]
	MenuFirstEntry $menu
    } elseif {[string equal [$w type active] "tearoff"]} {
	::zzz::TearOffMenu $w
	MenuUnpost $w
    } elseif {[string equal [$w cget -type] "menubar"]} {
	$w postcascade none
	set active [$w index active]
	set isCascade [string equal [$w type $active] "cascade"]

	# Only de-activate the active item if it's a cascade; this prevents
	# the annoying "activation flicker" you otherwise get with 
	# checkbuttons/commands/etc. on menubars

	if { $isCascade } {
	    $w activate none
	    event generate $w <<MenuSelect>>
	}

	MenuUnpost $w

	# If the active item is not a cascade, invoke it.  This enables
	# the use of checkbuttons/commands/etc. on menubars (which is legal,
	# but not recommended)

	if { !$isCascade } {
	    uplevel #0 [list $w invoke $active]
	}
    } else {
	set active [$w index active]
	if {$Priv(popup) eq "" || $active ne "none"} {
	    MenuUnpost $w
	}
	uplevel #0 [list $w invoke active]
    }
}

# ::zzz::MenuEscape --
# This procedure is invoked for the Cancel (or Escape) key.  It unposts
# the given menu and, if it is the top-level menu for a menu button,
# unposts the menu button as well.
#
# Arguments:
# menu -		Name of the menu window.

proc ::zzz::MenuEscape menu {
    set parent [winfo parent $menu]
    if {[string compare [winfo class $parent] "Menu"]} {
	MenuUnpost $menu
    } elseif {[string equal [$parent cget -type] "menubar"]} {
	MenuUnpost $menu
	RestoreOldGrab
    } else {
	MenuNextMenu $menu left
    }
}

# The following routines handle arrow keys. Arrow keys behave
# differently depending on whether the menu is a menu bar or not.

proc ::zzz::MenuUpArrow {menu} {
    if {[string equal [$menu cget -type] "menubar"]} {
	MenuNextMenu $menu left
    } else {
	MenuNextEntry $menu -1
    }
}

proc ::zzz::MenuDownArrow {menu} {
    if {[string equal [$menu cget -type] "menubar"]} {
	MenuNextMenu $menu right
    } else {
	MenuNextEntry $menu 1
    }
}

proc ::zzz::MenuLeftArrow {menu} {
    if {[string equal [$menu cget -type] "menubar"]} {
	MenuNextEntry $menu -1
    } else {
	MenuNextMenu $menu left
    }
}

proc ::zzz::MenuRightArrow {menu} {
    if {[string equal [$menu cget -type] "menubar"]} {
	MenuNextEntry $menu 1
    } else {
	MenuNextMenu $menu right
    }
}

# ::zzz::MenuNextMenu --
# This procedure is invoked to handle "left" and "right" traversal
# motions in menus.  It traverses to the next menu in a menu bar,
# or into or out of a cascaded menu.
#
# Arguments:
# menu -		The menu that received the keyboard
#			event.
# direction -		Direction in which to move: "left" or "right"

proc ::zzz::MenuNextMenu {menu direction} {
    variable ::zzz::Priv

    # First handle traversals into and out of cascaded menus.

    if {[string equal $direction "right"]} {
	set count 1
	set parent [winfo parent $menu]
	set class [winfo class $parent]
	if {[string equal [$menu type active] "cascade"]} {
	    $menu postcascade active
	    set m2 [$menu entrycget active -menu]
	    if {[string compare $m2 ""]} {
		MenuFirstEntry $m2
	    }
	    return
	} else {
	    set parent [winfo parent $menu]
	    while {[string compare $parent "."]} {
		if {[string equal [winfo class $parent] "Menu"] \
			&& [string equal [$parent cget -type] "menubar"]} {
		    zzz_menuSetFocus $parent
		    MenuNextEntry $parent 1
		    return
		}
		set parent [winfo parent $parent]
	    }
	}
    } else {
	set count -1
	set m2 [winfo parent $menu]
	if {[string equal [winfo class $m2] "Menu"]} {
	    $menu activate none
	    GenerateMenuSelect $menu
	    zzz_menuSetFocus $m2

	    $m2 postcascade none

	    if {[string compare [$m2 cget -type] "menubar"]} {
		return
	    }
	}
    }

    # Can't traverse into or out of a cascaded menu.  Go to the next
    # or previous menubutton, if that makes sense.

    set m2 [winfo parent $menu]
    if {[string equal [winfo class $m2] "Menu"]} {
	if {[string equal [$m2 cget -type] "menubar"]} {
	    zzz_menuSetFocus $m2
	    MenuNextEntry $m2 -1
	    return
	}
    }

    set w $Priv(postedMb)
    if {[string equal $w ""]} {
	return
    }
    set buttons [winfo children [winfo parent $w]]
    set length [llength $buttons]
    set i [expr {[lsearch -exact $buttons $w] + $count}]
    while {1} {
	while {$i < 0} {
	    incr i $length
	}
	while {$i >= $length} {
	    incr i -$length
	}
	set mb [lindex $buttons $i]
	if {[string equal [winfo class $mb] "ComboButton"] \
		&& [string compare [$mb cget -state] "disabled"] \
		&& [string compare [$mb cget -menu] ""] \
		&& [string compare [[$mb cget -menu] index last] "none"]} {
	    break
	}
	if {[string equal $mb $w]} {
	    return
	}
	incr i $count
    }
    MbPost $mb
    MenuFirstEntry [$mb cget -menu]
}

# ::zzz::MenuNextEntry --
# Activate the next higher or lower entry in the posted menu,
# wrapping around at the ends.  Disabled entries are skipped.
#
# Arguments:
# menu -			Menu window that received the keystroke.
# count -			1 means go to the next lower entry,
#				-1 means go to the next higher entry.

proc ::zzz::MenuNextEntry {menu count} {

    if {[string equal [$menu index last] "none"]} {
	return
    }
    set length [expr {[$menu index last]+1}]
    set quitAfter $length
    set active [$menu index active]
    if {[string equal $active "none"]} {
	set i 0
    } else {
	set i [expr {$active + $count}]
    }
    while {1} {
	if {$quitAfter <= 0} {
	    # We've tried every entry in the menu.  Either there are
	    # none, or they're all disabled.  Just give up.

	    return
	}
	while {$i < 0} {
	    incr i $length
	}
	while {$i >= $length} {
	    incr i -$length
	}
	if {[catch {$menu entrycget $i -state} state] == 0} {
	    if {$state ne "disabled" && \
		    ($i!=0 || [$menu cget -type] ne "tearoff" \
		    || [$menu type 0] ne "tearoff")} {
		break
	    }
	}
	if {$i == $active} {
	    return
	}
	incr i $count
	incr quitAfter -1
    }
    $menu activate $i
    GenerateMenuSelect $menu

    if {[string equal [$menu type $i] "cascade"] \
	    && [string equal [$menu cget -type] "menubar"]} {
	set cascade [$menu entrycget $i -menu]
	if {[string compare $cascade ""]} {
	    # Here we auto-post a cascade.  This is necessary when
	    # we traverse left/right in the menubar, but undesirable when
	    # we traverse up/down in a menu.
	    $menu postcascade $i
	    MenuFirstEntry $cascade
	}
    }
}

# ::zzz::MenuFind --
# This procedure searches the entire window hierarchy under w for
# a menubutton that isn't disabled and whose underlined character
# is "char" or an entry in a menubar that isn't disabled and whose
# underlined character is "char".
# It returns the name of that window, if found, or an
# empty string if no matching window was found.  If "char" is an
# empty string then the procedure returns the name of the first
# menubutton found that isn't disabled.
#
# Arguments:
# w -				Name of window where key was typed.
# char -			Underlined character to search for;
#				may be either upper or lower case, and
#				will match either upper or lower case.

proc ::zzz::MenuFind {w char} {
    set char [string tolower $char]
    set windowlist [winfo child $w]

    foreach child $windowlist {
	# Don't descend into other toplevels.
        if {[string compare [winfo toplevel $w] [winfo toplevel $child]]} {
	    continue
	}
	if {[string equal [winfo class $child] "Menu"] && \
		[string equal [$child cget -type] "menubar"]} {
	    if {[string equal $char ""]} {
		return $child
	    }
	    set last [$child index last]
	    for {set i [$child cget -tearoff]} {$i <= $last} {incr i} {
		if {[string equal [$child type $i] "separator"]} {
		    continue
		}
		set char2 [string index [$child entrycget $i -label] \
			[$child entrycget $i -underline]]
		if {[string equal $char [string tolower $char2]] \
			|| [string equal $char ""]} {
		    if {[string compare [$child entrycget $i -state] "disabled"]} {
			return $child
		    }
		}
	    }
	}
    }

    foreach child $windowlist {
	# Don't descend into other toplevels.
        if {[string compare [winfo toplevel $w] [winfo toplevel $child]]} {
	    continue
	}
	switch [winfo class $child] {
	    ComboButton {
		set char2 [string index [$child cget -text] \
			[$child cget -underline]]
		if {[string equal $char [string tolower $char2]] \
			|| [string equal $char ""]} {
		    if {[string compare [$child cget -state] "disabled"]} {
			return $child
		    }
		}
	    }

	    default {
		set match [MenuFind $child $char]
		if {[string compare $match ""]} {
		    return $match
		}
	    }
	}
    }
    return {}
}

# ::zzz::TraverseToMenu --
# This procedure implements keyboard traversal of menus.  Given an
# ASCII character "char", it looks for a menubutton with that character
# underlined.  If one is found, it posts the menubutton's menu
#
# Arguments:
# w -				Window in which the key was typed (selects
#				a toplevel window).
# char -			Character that selects a menu.  The case
#				is ignored.  If an empty string, nothing
#				happens.

proc ::zzz::TraverseToMenu {w char} {
    variable ::zzz::Priv
    if {[string equal $char ""]} {
	return
    }
    while { [winfo class $w] == "Menu" } {
	if { [$w cget -type] == "menubar" } {
	    break
	}
	if { $Priv(postedMb) == "" } {
	    return
	}
	set w [winfo parent $w]
    }
    set w [MenuFind [winfo toplevel $w] $char]
    if { $w != "" } {
	if { [winfo class $w] == "Menu" } {
	    zzz_menuSetFocus $w
	    set Priv(window) $w
	    SaveGrabInfo $w
	    grab -global $w
	    TraverseWithinMenu $w $char
	} else {
	    MbPost $w
	    MenuFirstEntry [$w cget -menu]
	}
    }
}

# ::zzz::FirstMenu --
# This procedure traverses to the first menubutton in the toplevel
# for a given window, and posts that menubutton's menu.
#
# Arguments:
# w -				Name of a window.  Selects which toplevel
#				to search for menubuttons.

proc ::zzz::FirstMenu w {
    variable ::zzz::Priv
    set w [MenuFind [winfo toplevel $w] ""]
    if {[string compare $w ""]} {
	if {[string equal [winfo class $w] "Menu"]} {
	    zzz_menuSetFocus $w
	    set Priv(window) $w
	    SaveGrabInfo $w
	    grab -global $w
	    MenuFirstEntry $w
	} else {
	    MbPost $w
	    MenuFirstEntry [$w cget -menu]
	}
    }
}

# ::zzz::TraverseWithinMenu
# This procedure implements keyboard traversal within a menu.  It
# searches for an entry in the menu that has "char" underlined.  If
# such an entry is found, it is invoked and the menu is unposted.
#
# Arguments:
# w -				The name of the menu widget.
# char -			The character to look for;  case is
#				ignored.  If the string is empty then
#				nothing happens.

proc ::zzz::TraverseWithinMenu {w char} {
    if {[string equal $char ""]} {
	return
    }
    set char [string tolower $char]
    set last [$w index last]
    if {[string equal $last "none"]} {
	return
    }
    for {set i 0} {$i <= $last} {incr i} {
	if {[catch {set char2 [string index \
		[$w entrycget $i -label] [$w entrycget $i -underline]]}]} {
	    continue
	}
	if {[string equal $char [string tolower $char2]]} {
	    if {[string equal [$w type $i] "cascade"]} {
		$w activate $i
		$w postcascade active
		event generate $w <<MenuSelect>>
		set m2 [$w entrycget $i -menu]
		if {[string compare $m2 ""]} {
		    MenuFirstEntry $m2
		}
	    } else {
		MenuUnpost $w
		uplevel #0 [list $w invoke $i]
	    }
	    return
	}
    }
}

# ::zzz::MenuFirstEntry --
# Given a menu, this procedure finds the first entry that isn't
# disabled or a tear-off or separator, and activates that entry.
# However, if there is already an active entry in the menu (e.g.,
# because of a previous call to zzz::PostOverPoint) then the active
# entry isn't changed.  This procedure also sets the input focus
# to the menu.
#
# Arguments:
# menu -		Name of the menu window (possibly empty).

proc ::zzz::MenuFirstEntry menu {
    if {[string equal $menu ""]} {
	return
    }
    zzz_menuSetFocus $menu
    if {[string compare [$menu index active] "none"]} {
	return
    }
    set last [$menu index last]
    if {[string equal $last "none"]} {
	return
    }
    for {set i 0} {$i <= $last} {incr i} {
	if {([catch {set state [$menu entrycget $i -state]}] == 0) \
		&& [string compare $state "disabled"] \
		&& [string compare [$menu type $i] "tearoff"]} {
	    $menu activate $i
	    GenerateMenuSelect $menu
	    # Only post the cascade if the current menu is a menubar;
	    # otherwise, if the first entry of the cascade is a cascade,
	    # we can get an annoying cascading effect resulting in a bunch of
	    # menus getting posted (bug 676)
	    if {[string equal [$menu type $i] "cascade"] && \
		[string equal [$menu cget -type] "menubar"]} {
		set cascade [$menu entrycget $i -menu]
		if {[string compare $cascade ""]} {
		    $menu postcascade $i
		    MenuFirstEntry $cascade
		}
	    }
	    return
	}
    }
}

# ::zzz::MenuFindName --
# Given a menu and a text string, return the index of the menu entry
# that displays the string as its label.  If there is no such entry,
# return an empty string.  This procedure is tricky because some names
# like "active" have a special meaning in menu commands, so we can't
# always use the "index" widget command.
#
# Arguments:
# menu -		Name of the menu widget.
# s -			String to look for.

proc ::zzz::MenuFindName {menu s} {
    set i ""
    if {![regexp {^active$|^last$|^none$|^[0-9]|^@} $s]} {
	catch {set i [$menu index $s]}
	return $i
    }
    set last [$menu index last]
    if {[string equal $last "none"]} {
	return
    }
    for {set i 0} {$i <= $last} {incr i} {
	if {![catch {$menu entrycget $i -label} label]} {
	    if {[string equal $label $s]} {
		return $i
	    }
	}
    }
    return ""
}

# ::zzz::PostOverPoint --
# This procedure posts a given menu such that a given entry in the
# menu is centered over a given point in the root window.  It also
# activates the given entry.
#
# Arguments:
# menu -		Menu to post.
# x, y -		Root coordinates of point.
# entry -		Index of entry within menu to center over (x,y).
#			If omitted or specified as {}, then the menu's
#			upper-left corner goes at (x,y).

proc ::zzz::PostOverPoint {menu x y {entry {}}}  {
    global tcl_platform
    
    if {[string compare $entry {}]} {
	if {$entry == [$menu index last]} {
	    incr y [expr {-([$menu yposition $entry] \
		    + [winfo reqheight $menu])/2}]
	} else {
	    incr y [expr {-([$menu yposition $entry] \
		    + [$menu yposition [expr {$entry+1}]])/2}]
	}
	incr x [expr {-[winfo reqwidth $menu]/2}]
    }
    if {$tcl_platform(platform) == "windows"} {
	# We need to fix some problems with menu posting on Windows.
	set yoffset [expr {[winfo screenheight $menu] \
		- $y - [winfo reqheight $menu]}]
	if {$yoffset < 0} {
	    # The bottom of the menu is offscreen, so adjust upwards
	    incr y $yoffset
	    if {$y < 0} { set y 0 }
	}
	# If we're off the top of the screen (either because we were
	# originally or because we just adjusted too far upwards),
	# then make the menu popup on the top edge.
	if {$y < 0} {
	    set y 0
	}
    }
    $menu post $x $y
    if {$entry ne "" && [$menu entrycget $entry -state] ne "disabled"} {
	$menu activate $entry
	GenerateMenuSelect $menu
    }
}

# ::zzz::SaveGrabInfo --
# Sets the variable zzz::Priv(oldGrab) to record
# the state of any existing grab on the w's display.
#
# Arguments:
# w -			Name of a window;  used to select the display
#			whose grab information is to be recorded.

proc zzz::SaveGrabInfo w {
    variable ::zzz::Priv
    set grab [grab current $w]
    set Priv(oldGrab) ""
    if { $grab != "" } {
	set type [grab status $grab]
	if { $type == "global" } {
	    set Priv(oldGrab) [list grab set -global $grab]
	} else {
	    set Priv(oldGrab) [list grab set $grab]
	}	    
    } 
}

# ::zzz::RestoreOldGrab --
# Restores the grab to what it was before TkSaveGrabInfo was called.
#

proc ::zzz::RestoreOldGrab {} {
    variable ::zzz::Priv

    if { $Priv(oldGrab) != "" } {
    	# Be careful restoring the old grab, since it's window may not
	# be visible anymore.
	catch $Priv(oldGrab)
	set Priv(oldGrab) ""
    }
}

proc ::zzz_menuSetFocus {menu} {
    variable ::zzz::Priv
    if {![info exists Priv(focus)] || [string equal $Priv(focus) {}]} {
	set Priv(focus) [focus]
    }
    focus $menu
}
    
proc ::zzz::GenerateMenuSelect {menu} {
    variable ::zzz::Priv

    if {[string equal $Priv(activeMenu) $menu] \
          && [string equal $Priv(activeItem) [$menu index active]]} {
	return
    }
    set Priv(activeMenu) $menu
    set Priv(activeItem) [$menu index active]
    event generate $menu <<MenuSelect>>
}

# ::zzz_popup --
# This procedure pops up a menu and sets things up for traversing
# the menu and its submenus.
#
# Arguments:
# menu -		Name of the menu to be popped up.
# x, y -		Root coordinates at which to pop up the
#			menu.
# entry -		Index of a menu entry to center over (x,y).
#			If omitted or specified as {}, then menu's
#			upper-left corner goes at (x,y).

proc ::zzz_popup {menu x y {entry {}}} {
    variable ::zzz::Priv
    global tcl_platform
    if {$Priv(popup) ne "" || $Priv(postedMb) ne ""} {
	zzz::MenuUnpost {}
    }
    zzz::PostOverPoint $menu $x $y $entry
    if {[tk windowingsystem] eq "x11" && [winfo viewable $menu]} {
        zzz::SaveGrabInfo $menu
	grab -global $menu
	set Priv(popup) $menu
	zzz_menuSetFocus $menu
    }
}
