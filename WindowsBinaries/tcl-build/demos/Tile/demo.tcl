#
# $Id: demo.tcl,v 1.60 2004/04/27 01:01:02 patthoyts Exp $
#
# Demo for 'tile' package.
#

variable demodir [file dirname [info script]]
lappend auto_path . $demodir
package require tile

source [file join $demodir toolbutton.tcl]

# This forces an update of the available packages list. It's required
# for package names to find the themes in demos/themes/*.tcl
eval [package unknown] Tcl [package provide Tcl]

wm title . "Tile demo"
wm iconname . "Tile demo"

#X: interp alias {} tframe {} frame
#X: interp alias {} tlabelframe {} labelframe

# The descriptive names of the builtin themes.
set ::THEMELIST {
    default  	"Classic"
    alt      	"Revitalized"
    winnative	"Windows native"
    xpnative	"XP Native"
    aqua	"Aqua"
    step     	"OpenStep"
}
array set ::THEMES $THEMELIST;

# Add in any available loadable themes.
foreach pkg [package names] {
    if {![string match tile::theme::* $pkg]} { continue }
    set name [lindex [split $pkg :] end]
    if {![info exists ::THEMES($name)]} {
	lappend THEMELIST $name [set ::THEMES($name) [string totitle $name]]
    }
}

# This permits delayes loading of pixmap themes. So we dont load
# lots of images until we need them.
#
proc SetTheme {container theme} {
    if {[lsearch -exact [package names] tile::theme::$theme] >= 0} {
	package require tile::theme::$theme
    }
    if {[catch {style theme use $theme} msg]} {
	$container.s$theme state disabled
	return -code error $msg
    }
}

#
# Load icons...
#
proc loadIcons {file} {
    set ::BUTTONS [list open new save]
    set ::CHECKBOXES [list bold italic]

    source $file
    foreach icon [array names ImgData]  {
	set ::ICON($icon) [image create photo -data $ImgData($icon)]
    }
}
loadIcons [file join $demodir iconlib.tcl]

#
# Utilities:
#
proc foreachWidget {varname Q script} {
    upvar 1 $varname w
    while {[llength $Q]} {
    	set QN [list]
	foreach w $Q {
	    uplevel 1 $script
	    foreach child [winfo children $w] {
		lappend QN $child
	    }
	}
	set Q $QN
    }
}

# sbstub $sb --
#	Used as the -command option for a scrollbar,
#	updates the scrollbar's position.
#
proc sbstub {sb cmd number {units units}} { sbstub.$cmd $sb $number $units }
proc sbstub.moveto {sb number _} { $sb set $number [expr {$number + 0.5}] }
proc sbstub.scroll {sb number units} {
    if {$units eq "pages"} {
    	set delta 0.2
    } else {
	set delta 0.05
    }
    set current [$sb get]
    set new0 [expr {[lindex $current 0] + $delta*$number}]
    set new1 [expr {[lindex $current 1] + $delta*$number}]
    $sb set $new0 $new1
}

# ... for debugging:
bind all <ButtonPress-3> { set ::W %W }
bind all <Control-ButtonPress-3> { focus %W }

proc showHelp {} {
    tk_messageBox -message "No help yet..."
}

#
# See toolbutton.tcl.

option add *Toolbar.relief groove
option add *Toolbar.borderWidth 2

option add *Toolbar.Button.Pad 2

set ::ROOT "."
set ::BASE ""
eval destroy [winfo children .]

array set ::V {
    STYLE	default
    COMPOUND	top
    MENURADIO1	One
    MENUCHECK1	1
}

#
# Toolbar button standard vs. tile comparison:
#
proc makeToolbars {} {
    variable top

    #
    # Tile toolbar:
    #
    set tb [tframe $::BASE.tbar_styled -class Toolbar]
    set i 0
    foreach icon $::BUTTONS {
	set b [tbutton $tb.tb[incr i] \
	       -text $icon -image $::ICON($icon) -compound $::V(COMPOUND) \
	       -style Toolbutton]
	grid $b -row 0 -column $i -sticky news
    }
    foreach icon $::CHECKBOXES {
	set b [tcheckbutton $tb.cb[incr i] \
		-variable ::V($icon) \
	       -text $icon -image $::ICON($icon) -compound $::V(COMPOUND) \
	       -style Toolbutton]
	grid $b -row 0 -column $i -sticky news
    }

    tmenubutton $tb.compound \
    	-text "toolbar" -image $::ICON(file) -compound $::V(COMPOUND)
    $tb.compound configure -menu [makeCompoundMenu $tb.compound.menu]
    grid $tb.compound -row 0 -column [incr i] -sticky news

    grid columnconfigure $tb [incr i] -weight 1

    #
    # Standard toolbar:
    #
    set tb [frame $::BASE.tbar_orig -class Toolbar]
    set i 0
    foreach icon $::BUTTONS {
	set b [button $tb.tb[incr i] \
		  -text $icon -image $::ICON($icon) -compound $::V(COMPOUND) \
		  -relief flat -overrelief raised]
	grid $b -row 0 -column $i -sticky news
    }
    foreach icon $::CHECKBOXES {
	set b [checkbutton $tb.cb[incr i] -variable ::V($icon) \
		  -text $icon -image $::ICON($icon) -compound $::V(COMPOUND) \
		  -indicatoron false \
		  -selectcolor {} \
		  -relief flat \
		  -overrelief raised]
	grid $b -row 0 -column $i -sticky news
    }

    menubutton $tb.compound \
    	-text "toolbar" -image $::ICON(file) -compound $::V(COMPOUND) \
	-indicatoron true
    $tb.compound configure -menu [makeCompoundMenu $tb.compound.menu]
    grid $tb.compound -row 0 -column [incr i] -sticky news

    grid columnconfigure $tb [incr i] -weight 1
}

#
# Toolbar -compound control:
#
proc makeCompoundMenu {menu} {
    variable compoundStrings {text image none top bottom left right center}
    menu $menu
    foreach string $compoundStrings {
	$menu add radiobutton \
	    -label [string totitle $string] \
	    -variable ::V(COMPOUND) -value $string \
	    -command changeToolbars ;
    }
    return $menu
}

makeToolbars

## CONTROLS
tframe $::BASE.control

#
# Overall theme control:
#
proc makeThemeControl {f} {
    set c [tlabelframe $f.style -text "Theme"]
    foreach {theme name} $::THEMELIST {
	set b [tradiobutton $c.s$theme -text $name -anchor w \
		   -variable ::V(STYLE) -value $theme \
		   -command [list SetTheme $c $theme]]
	grid $b -sticky ew
    }
    grid columnconfigure $c 0 -weight 1
    foreach theme {winnative xpnative aqua} {
	if {[package provide tile::theme::$theme] == {}} {
	    $c.s$theme state disabled
	}
    }
    return $c
}

grid [makeThemeControl $::BASE.control] -sticky news -padx 6 -ipadx 6
grid rowconfigure $::BASE.control 99 -weight 1

proc changeToolbars {} {
    foreachWidget w [list $::BASE.tbar_styled $::BASE.tbar_orig] {
	catch { $w configure -compound $::V(COMPOUND) }
    }
}

proc ScrolledWidget {parent class themed args} {
    if {$themed} {
        set sbcmd tscrollbar
    } else {
        set sbcmd scrollbar
    }

    for {set n 0} {[winfo exists $parent.f$n]} {incr n} {}
    set f [tframe $parent.f$n]
    set t [eval [linsert $args 0 $class $f.$class]]
    set vs [$sbcmd $f.vs -orient vertical -command [list $t yview]]
    set hs [$sbcmd $f.hs -orient horizontal -command [list $t xview]]
    $t configure -yscrollcommand [list $vs set] -xscrollcommand [list $hs set]

    grid configure $t $vs -sticky news
    grid configure $hs x  -sticky news
    grid rowconfigure $f 0 -weight 1
    grid columnconfigure $f 0 -weight 1

    return $f
}

#
# Notebook demonstration:
#
proc makeNotebook {} {
    set nb [tnotebook $::BASE.nb -padding 6]
    tile::notebook::enableTraversal $nb
    set client [tframe $nb.client]
    $nb add $client -text "Demo" -underline 0
    $nb select $client

    $nb add [tframe $nb.others] -text "Others" -underline 4
    $nb add [tlabel $nb.stuff -text "Nothing to see here..."] \
	-text "Stuff" -sticky new
    $nb add [tlabel $nb.more -text "Nothing to see here either."] \
	-text "More stuff" -sticky se

    return $client
}
set client [makeNotebook]

#
# Side-by side check, radio, and menu button comparison:
#
proc fillMenu {menu} {
    foreach dir {above below left right flush} {
	$menu add command -label [string totitle $dir] \
	    -command [list [winfo parent $menu] configure -direction $dir]
    }
    $menu add cascade -label "Submenu" -menu [set submenu [menu $menu.submenu]]
    $submenu add command -label "Subcommand 1"
    $submenu add command -label "Subcommand 2"
    $submenu add command -label "Subcommand 3"

    $menu add separator
    $menu add command -label "Quit"  -command [list destroy .]
}

set l [tlabelframe $client.styled -text "Styled" -padding 6]
set r [labelframe $client.orig -text "Standard" -padx 6 -pady 6]

## Styled frame
tcheckbutton $l.cb -text "Checkbutton" -variable ::V(SELECTED) -underline 2
tradiobutton $l.rb1 -text "One" -variable ::V(CHOICE) -value 1 -underline 0
tradiobutton $l.rb2 -text "Two" -variable ::V(CHOICE) -value 2
tradiobutton $l.rb3 -text "Three" -variable ::V(CHOICE) -value 3 -under 0
tbutton $l.button -text "Button" -underline 0

tmenubutton $l.mb -text "Menubutton" -underline 2
$l.mb configure -menu [menu $l.mb.menu]
fillMenu $l.mb.menu

set ltext [ScrolledWidget $l text 1 -width 12 -height 5 -wrap none]

set s [tframe $l.scales]
tscale $s.scale -orient horizontal -from 0 -to 100 -variable ::V(SCALE)
tscale $s.vscale -orient vertical -from -25 -to 25 -variable ::V(VSCALE)
tprogress $s.progress -orient horizontal -from 0 -to 100
tprogress $s.vprogress -orient vertical -from -25 -to 25
$s.scale configure -command [list $s.progress set]
$s.vscale configure -command [list $s.vprogress set]

grid $s.scale -columnspan 2 -sticky ew
grid $s.progress -columnspan 2 -sticky ew
grid $s.vscale $s.vprogress -sticky nws
grid columnconfigure $s 0 -weight 1
grid columnconfigure $s 1 -weight 1

# NOTE TO MAINTAINERS: 
# The checkbuttons are -sticky ew / -expand x  on purpose:
# it demonstrates one of the differences between TCheckbuttons
# and standard checkbuttons.
#
grid $l.cb  -sticky ew
grid $l.rb1 -sticky ew
grid $l.rb2 -sticky ew
grid $l.rb3 -sticky ew
grid $l.button -sticky ew -padx 2 -pady 2
grid $l.mb -sticky ew -padx 2
grid $ltext -sticky news
grid $l.scales -sticky news -pady 2

grid columnconfigure $l 0 -weight 1
grid rowconfigure    $l 6 -weight 1 ; # text widget (grid is a PITA)

## Orig frame
checkbutton $r.cb -text "Checkbutton" -variable ::V(SELECTED) 
radiobutton $r.rb1 -text "One" -variable ::V(CHOICE) -value 1 
radiobutton $r.rb2 -text "Two" -variable ::V(CHOICE) -value 2 -underline 1
radiobutton $r.rb3 -text "Three" -variable ::V(CHOICE) -value 3
button $r.button -text "Button"
menubutton $r.mb -text "Menubutton" -underline 3 -takefocus 1
$r.mb configure -menu [menu $r.mb.menu] -takefocus 1 
# -relief raised -indicatoron true
set ::V(rmbIndicatoron) [$r.mb cget -indicatoron]
$r.mb.menu add checkbutton -label "Indicator?" \
    -variable ::V(rmbIndicatoron) \
    -command "$r.mb configure -indicatoron \$::V(rmbIndicatoron)" \
    ;
$r.mb.menu add separator
fillMenu $r.mb.menu

set rtext [ScrolledWidget $r text 0 -width 12 -height 5 -wrap none]
scale $r.scale -orient horizontal -from 0 -to 100 -variable ::V(SCALE)
scale $r.vscale -orient vertical -from -25 -to 25 -variable ::V(VSCALE)

grid $r.cb -sticky ew
grid $r.rb1 -sticky ew
grid $r.rb2 -sticky ew
grid $r.rb3 -sticky ew
grid $r.button -sticky ew -padx 2 -pady 2
grid $r.mb -sticky ew -padx 2 -pady 2
grid $rtext -sticky news
grid $r.scale -sticky news
grid $r.vscale -sticky nws

grid columnconfigure $r 0 -weight 1
grid rowconfigure    $r 6 -weight 1 ; # text widget

grid $client.styled $client.orig -sticky news -padx 6 -pady 6
grid rowconfigure $client 0 -weight 1
grid columnconfigure $client {0 1} -weight 1

#
# Add some text to the text boxes:
#
set msgs {
"The cat crept into the crypt, crapped and crept out again"
"Peter Piper picked a peck of pickled peppers"
"How much wood would a woodchuck chuck if a woodchuck could chuck wood"
"He thrusts his fists against the posts and still insists he sees the ghosts"
"Who put the bomb in the bom-b-bom-b-bom,"
"Is this your sister's sixth zither, sir?"
"Who put the ram in the ramalamadingdong?"
"I am not the pheasant plucker, I'm the pheasant plucker's mate."
}
set nmsgs [llength $msgs]
for {set n 0} {$n < 50} {incr n} {
    set msg [lindex $msgs [expr {$n % $nmsgs}]]
    $ltext.text insert end "$n: $msg\n"
    $rtext.text insert end "$n: $msg\n"
}

#
# Command box:
#
set cmd [tframe $::BASE.command]
tbutton $cmd.close -text Close -underline 0 -default normal \
    -command [list destroy .]
tbutton $cmd.help -text Help -underline 0 -default normal \
    -command showHelp

grid x $cmd.close $cmd.help -pady {6 4} -padx 4
grid columnconfigure $cmd 0 -weight 1

#
# Set up accelerators:
#
bind $::ROOT <KeyPress-Escape>	[list event generate $cmd.close <<Invoke>>]
bind $::ROOT <<Help>>		[list event generate $cmd.help <<Invoke>>]
keynav::enableMnemonics $::ROOT
keynav::defaultButton $cmd.help

grid $::BASE.tbar_styled - -sticky ew
grid $::BASE.tbar_orig   - -sticky ew
grid $::BASE.control     $::BASE.nb -sticky news
grid $::BASE.command     - -sticky ew
grid columnconfigure $::ROOT 1 -weight 1
grid rowconfigure    $::ROOT 2 -weight 1

#
# Add a menu
#
set menu [menu $::BASE.menu]
$::ROOT configure -menu $menu
$menu add cascade -label "File" -underline 0 \
    -menu [menu $menu.file -tearoff 0]
$menu.file add command -label "Open" -underline 0 \
    -compound left -image $::ICON(open)
$menu.file add command -label "Save" -underline 0 \
    -compound left -image $::ICON(save)
$menu.file add separator
$menu.file add cascade -label "Test submenu" -underline 0 \
    -menu [menu $menu.file.test -tearoff 0]
$menu.file add checkbutton -label "Text check" -underline 5 \
    -variable ::V(MENUCHECK1)
$menu.file add command -label "Exit" -underline 1 \
    -command [list event generate $cmd.close <<Invoke>>]

foreach label {One Two Three Four} {
    $menu.file.test add radiobutton -label $label -variable ::V(MENURADIO1)
}

# Add a theme menu.
#
proc settheme {style} {
    style theme use $style
    set ::V(STYLE) $style
}
$menu add cascade -label "Theme" -underline 3 \
    -menu [menu $menu.theme -tearoff 0]
foreach {theme name} $::THEMELIST {
    $menu.theme add radiobutton -label $name \
        -variable ::V(STYLE) -value $theme -command [list settheme $theme]
}

#
# Add a console menu item for windows.
#
if {1 || [tk windowingsystem] == "win32"} {
    proc toggleconsole {} {
        if {$::V(CONSOLE)} {console show} else {console hide}
    }
    $menu.file insert end checkbutton -label Console -underline 0 \
        -variable ::V(CONSOLE) -command toggleconsole
}

settheme $::V(STYLE)


#
# Other demos:
#
set Timers(StateMonitor) {}
set Timers(FocusMonitor) {}

set others $::BASE.nb.others

message $others.m -aspect 200
bind ShowDescription <Enter> { $BASE.nb.others.m configure -text $Desc(%W) }
bind ShowDescription <Leave> { $BASE.nb.others.m configure -text "" }

foreach {command label description} {
    trackStates "Widget states..." 
    "Display/modify widget state bits"

    scrollbarResizeDemo  "Scrollbar resize behavior..."
    "Shows how Tile and standard scrollbars differ when they're sized too large"

    trackFocus "Track keyboard focus..." 
    "Display the name of the widget that currently has focus"

} {
    set b [tbutton $others.$command -text $label -command $command]
    set Desc($b) $description
    bindtags $b [lreplace [bindtags $b] end 0 ShowDescription]

    pack $b -side top -expand false -fill x -padx 6 -pady 6
}

pack $others.m -side bottom -expand true -fill both


#
# Scrollbar resize demo:
#

proc scrollbarResizeDemo {} {
    set t .scrollbars
    destroy $t
    toplevel $t ; wm geometry $t 200x200
    frame $t.f -height 200
    grid \
	[tscrollbar $t.f.tsb -command [list sbstub $t.f.tsb]] \
	[scrollbar $t.f.sb -command [list sbstub $t.f.sb]] \
    -sticky news

    $t.f.sb set 0 0.5	;# prevent backwards-compatibility mode for old SB

    grid columnconfigure $t.f 0 -weight 1
    grid columnconfigure $t.f 1 -weight 1
    grid rowconfigure $t.f 0 -weight 1
    pack $t.f -expand true -fill both
}

#
# Track focus demo:
#
proc trackFocus {} {
    global Focus
    set t .focus
    destroy $t
    toplevel $t 
    wm title $t "Keyboard focus"
    set i 0
    foreach {label variable} {
	"Focus widget:" Focus(Widget)
	"Class:" Focus(WidgetClass)
	"Next:"  Focus(WidgetNext)
	"Grab:"  Focus(Grab)
	"Status:"  Focus(GrabStatus)
    } {
	grid [tlabel $t.l$i -text $label -anchor e] \
	     [tlabel $t.v$i -textvariable $variable \
		-width 40 -anchor w -relief groove] \
	-sticky ew;
	incr i
    }
    grid columnconfigure $t 1 -weight 1
    grid rowconfigure $t $i -weight 1

    bind $t <Destroy> {after cancel $Timers(FocusMonitor)}
    FocusMonitor
}

proc FocusMonitor {} {
    global Focus

    set Focus(Widget) [focus]
    if {$::Focus(Widget) ne ""} {
	set Focus(WidgetClass) [winfo class $Focus(Widget)]
	set Focus(WidgetNext) [tk_focusNext $Focus(Widget)]
    } else {
	set Focus(WidgetClass) [set Focus(WidgetNext) ""]
    }

    set Focus(Grab) [grab current]
    if {$Focus(Grab) ne ""} {
	set Focus(GrabStatus) [grab status $Focus(Grab)]
    } else {
	set Focus(GrabStatus) ""
    }

    set ::Timers(FocusMonitor) [after 200 FocusMonitor]
}

#
# Widget state demo:
#

variable Widget .tbar_styled.tb1

bind all <Control-Shift-ButtonPress-1> { set ::Widget %W ; UpdateStates; break }
variable states [list \
    active disabled focus pressed selected \
    background indeterminate invalid default]

proc trackStates {} {
    variable states
    set t .states
    destroy $t; toplevel $t ; wm title $t "Widget states"


    tlabel $t.info -text "Press Control-Shift-Button-1 on any widget"

    tlabel $t.lw -text "Widget:" -anchor e -relief groove
    tlabel $t.w -textvariable ::Widget -anchor w -relief groove

    grid $t.info - -sticky ew -padx 6 -pady 6
    grid $t.lw $t.w -sticky ew

    foreach state $states {
	tcheckbutton $t.s$state \
	    -text $state \
	    -variable ::State($state) \
	    -command [list ChangeState $state] ;
	grid x $t.s$state -sticky nsew
    }

    grid columnconfigure $t 1 -weight 1

    grid x [tframe $t.cmd] -sticky nse
    grid x \
    	[tbutton $t.cmd.close -text Close -command [list destroy $t]] \
    	 -padx 4 -pady {6 4};
    grid columnconfigure $t.cmd 0 -weight 1

    bind $t <KeyPress-Escape> [list event generate $t.cmd.close <<Invoke>>]

    bind $t <Destroy> { after cancel $::Timers(StateMonitor) }
    StateMonitor
}

proc StateMonitor {} {
    if {$::Widget ne ""} { UpdateStates }
    set ::Timers(StateMonitor) [after 200 StateMonitor]
}

proc UpdateStates {}  {
    variable states
    variable State
    variable Widget
    foreach state $states {
	if {[catch {set State($state) [$Widget instate $state]}]} {
	    # Not a Tile widget:
	    .states.s$state state disabled
	} else {
	    .states.s$state state !disabled
	}
    }
}

proc ChangeState {state} {
    variable State
    variable Widget
    if {$Widget ne ""} {
	if {$State($state)} { 
	    $Widget state $state
	} else {
	    $Widget state !$state
	}
    }
}

